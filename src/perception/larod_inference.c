/**
 * @file larod_inference.c
 * @brief OMNISIGHT Larod ML Inference Implementation
 *
 * Hardware-accelerated object detection using Axis Larod API.
 * Optimized for ARTPEC-9 DLPU with TensorFlow Lite models.
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on Axis ACAP Native SDK examples
 */

#include "larod_inference.h"
#include "perception.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <syslog.h>
#include <pthread.h>
#include <time.h>

#include <glib.h>
#include <larod.h>
#include <vdo-buffer.h>
#include <vdo-types.h>

// Maximum retries for power availability
#define MAX_POWER_RETRIES 50
#define POWER_RETRY_DELAY_MS 250

/**
 * Larod inference engine instance
 */
struct LarodInference {
    LarodInferenceConfig config;

    // Larod connection and model
    larodConnection* conn;
    larodModel* model;
    larodModel* preprocessing_model;

    // Tensors
    larodTensor** input_tensors;
    size_t num_inputs;
    larodTensor** output_tensors;
    size_t num_outputs;
    larodTensor** pp_input_tensors;
    size_t pp_num_inputs;
    larodTensor** pp_output_tensors;
    size_t pp_num_outputs;

    // Job requests
    larodJobRequest* inf_req;
    larodJobRequest* pp_req;

    // Input buffer (memory-mapped)
    int input_fd;
    void* input_addr;
    size_t input_size;

    // Crop parameters for aspect ratio adjustment
    larodMap* crop_map;
    unsigned int crop_x;
    unsigned int crop_y;
    unsigned int crop_w;
    unsigned int crop_h;

    // Model metadata
    LarodModelInfo model_info;
    bool use_preprocessing;

    // Statistics
    float total_inference_ms;
    float min_inference_ms;
    float max_inference_ms;
    uint64_t inference_count;

    // Thread safety
    pthread_mutex_t mutex;
    bool initialized;
};

// Internal helper functions
static uint64_t get_time_ms(void);
static larodModel* create_inference_model(LarodInference* inference, GError** error);
static larodModel* create_preprocessing_model(LarodInference* inference, GError** error);
static bool setup_tensors(larodConnection* conn, larodModel* model,
                         larodTensor*** inputs, size_t* num_inputs,
                         larodTensor*** outputs, size_t* num_outputs,
                         GError** error);
static bool setup_preprocessing(LarodInference* inference, GError** error);
static bool parse_detection_outputs(LarodInference* inference,
                                   DetectedObject* objects,
                                   uint32_t max_objects,
                                   uint32_t* num_objects);

// ============================================================================
// Public API Implementation
// ============================================================================

LarodInference* larod_inference_init(const LarodInferenceConfig* config) {
    if (!config || !config->model_path) {
        syslog(LOG_ERR, "[Larod] Invalid configuration");
        return NULL;
    }

    LarodInference* inference = calloc(1, sizeof(LarodInference));
    if (!inference) {
        syslog(LOG_ERR, "[Larod] Failed to allocate inference structure");
        return NULL;
    }

    // Initialize mutex
    if (pthread_mutex_init(&inference->mutex, NULL) != 0) {
        syslog(LOG_ERR, "[Larod] Failed to initialize mutex");
        free(inference);
        return NULL;
    }

    inference->config = *config;
    inference->initialized = false;
    inference->total_inference_ms = 0.0f;
    inference->min_inference_ms = 1000000.0f;
    inference->max_inference_ms = 0.0f;
    inference->inference_count = 0;
    inference->crop_map = NULL;

    GError* error = NULL;

    // Connect to Larod service
    if (!larodConnect(&inference->conn, &error)) {
        syslog(LOG_ERR, "[Larod] Could not connect to larod: %s", error->message);
        g_error_free(error);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    syslog(LOG_INFO, "[Larod] Connected to Larod service");

    // Load inference model
    inference->model = create_inference_model(inference, &error);
    if (!inference->model) {
        syslog(LOG_ERR, "[Larod] Failed to load model: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    // Setup input/output tensors
    if (!setup_tensors(inference->conn, inference->model,
                      &inference->input_tensors, &inference->num_inputs,
                      &inference->output_tensors, &inference->num_outputs,
                      &error)) {
        syslog(LOG_ERR, "[Larod] Failed to setup tensors: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    if (inference->num_inputs != 1) {
        syslog(LOG_ERR, "[Larod] Only single input tensor supported (got %zu)",
               inference->num_inputs);
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    // Get model dimensions
    const larodTensorDims* input_dims =
        larodGetTensorDims(inference->input_tensors[0], &error);
    if (!input_dims) {
        syslog(LOG_ERR, "[Larod] Failed to get input dimensions: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    // Store model info
    inference->model_info.input_width = config->width;
    inference->model_info.input_height = config->height;
    inference->model_info.input_format = config->input_format;
    inference->model_info.num_outputs = inference->num_outputs;

    // Get input buffer size and map memory
    if (!larodGetTensorFdSize(inference->input_tensors[0],
                             &inference->input_size, &error)) {
        syslog(LOG_ERR, "[Larod] Failed to get tensor size: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    inference->input_fd = larodGetTensorFd(inference->input_tensors[0], &error);
    if (inference->input_fd == LAROD_INVALID_FD) {
        syslog(LOG_ERR, "[Larod] Failed to get tensor fd: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    inference->input_addr = mmap(NULL, inference->input_size,
                                PROT_READ | PROT_WRITE, MAP_SHARED,
                                inference->input_fd, 0);
    if (inference->input_addr == MAP_FAILED) {
        syslog(LOG_ERR, "[Larod] Failed to map input tensor: %s", strerror(errno));
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
        larodDestroyModel(&inference->model);
        larodDisconnect(&inference->conn, NULL);
        pthread_mutex_destroy(&inference->mutex);
        free(inference);
        return NULL;
    }

    inference->model_info.input_buffer_size = inference->input_size;

    // Setup preprocessing if needed (VDO provides YUV, model needs RGB)
    if (config->input_format == VDO_FORMAT_YUV) {
        if (!setup_preprocessing(inference, &error)) {
            syslog(LOG_ERR, "[Larod] Failed to setup preprocessing: %s",
                   error ? error->message : "unknown error");
            if (error) g_error_free(error);
            munmap(inference->input_addr, inference->input_size);
            larodDestroyTensors(inference->conn, &inference->input_tensors,
                               inference->num_inputs, NULL);
            larodDestroyTensors(inference->conn, &inference->output_tensors,
                               inference->num_outputs, NULL);
            larodDestroyModel(&inference->model);
            larodDisconnect(&inference->conn, NULL);
            pthread_mutex_destroy(&inference->mutex);
            free(inference);
            return NULL;
        }
    } else {
        // No preprocessing needed - create direct inference job
        inference->use_preprocessing = false;
        inference->inf_req = larodCreateJobRequest(
            inference->model,
            inference->input_tensors,
            inference->num_inputs,
            inference->output_tensors,
            inference->num_outputs,
            NULL,
            &error
        );

        if (!inference->inf_req) {
            syslog(LOG_ERR, "[Larod] Failed to create job request: %s",
                   error ? error->message : "unknown error");
            if (error) g_error_free(error);
            munmap(inference->input_addr, inference->input_size);
            larodDestroyTensors(inference->conn, &inference->input_tensors,
                               inference->num_inputs, NULL);
            larodDestroyTensors(inference->conn, &inference->output_tensors,
                               inference->num_outputs, NULL);
            larodDestroyModel(&inference->model);
            larodDisconnect(&inference->conn, NULL);
            pthread_mutex_destroy(&inference->mutex);
            free(inference);
            return NULL;
        }
    }

    inference->initialized = true;

    syslog(LOG_INFO, "[Larod] Initialization complete");
    syslog(LOG_INFO, "[Larod] Model: %s", config->model_path);
    syslog(LOG_INFO, "[Larod] Device: %s", config->device_name);
    syslog(LOG_INFO, "[Larod] Input: %ux%u", config->width, config->height);
    syslog(LOG_INFO, "[Larod] Preprocessing: %s",
           inference->use_preprocessing ? "enabled" : "disabled");

    return inference;
}

bool larod_inference_run(LarodInference* inference,
                        VdoBuffer* vdo_buffer,
                        DetectedObject* objects,
                        uint32_t max_objects,
                        uint32_t* num_objects) {
    if (!inference || !vdo_buffer || !objects || !num_objects) {
        return false;
    }

    if (!inference->initialized) {
        syslog(LOG_ERR, "[Larod] Inference not initialized");
        return false;
    }

    pthread_mutex_lock(&inference->mutex);

    uint64_t start_time = get_time_ms();
    GError* error = NULL;
    bool success = false;

    // Get VDO buffer data
    void* vdo_data = vdo_buffer_get_data(vdo_buffer);
    if (!vdo_data) {
        syslog(LOG_ERR, "[Larod] Failed to get VDO buffer data");
        goto cleanup;
    }

    // Copy VDO data to input tensor
    if (inference->use_preprocessing) {
        // Copy to preprocessing input
        void* pp_input_addr = NULL;
        int pp_input_fd = larodGetTensorFd(inference->pp_input_tensors[0], &error);
        if (pp_input_fd == LAROD_INVALID_FD) {
            syslog(LOG_ERR, "[Larod] Failed to get preprocessing input fd");
            goto cleanup;
        }

        size_t pp_input_size = 0;
        if (!larodGetTensorFdSize(inference->pp_input_tensors[0],
                                 &pp_input_size, &error)) {
            syslog(LOG_ERR, "[Larod] Failed to get preprocessing input size");
            goto cleanup;
        }

        pp_input_addr = mmap(NULL, pp_input_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, pp_input_fd, 0);
        if (pp_input_addr == MAP_FAILED) {
            syslog(LOG_ERR, "[Larod] Failed to map preprocessing input");
            goto cleanup;
        }

        memcpy(pp_input_addr, vdo_data, pp_input_size);
        munmap(pp_input_addr, pp_input_size);

        // Run preprocessing job (YUV → RGB)
        if (!larodRunJob(inference->conn, inference->pp_req, &error)) {
            syslog(LOG_ERR, "[Larod] Preprocessing failed: %s", error->message);
            g_error_free(error);
            error = NULL;
            goto cleanup;
        }

        // Run inference job
        if (!larodRunJob(inference->conn, inference->inf_req, &error)) {
            syslog(LOG_ERR, "[Larod] Inference failed: %s", error->message);
            g_error_free(error);
            error = NULL;
            goto cleanup;
        }
    } else {
        // Direct copy to input tensor
        memcpy(inference->input_addr, vdo_data, inference->input_size);

        // Run inference
        if (!larodRunJob(inference->conn, inference->inf_req, &error)) {
            syslog(LOG_ERR, "[Larod] Inference failed: %s", error->message);
            g_error_free(error);
            error = NULL;
            goto cleanup;
        }
    }

    // Parse outputs to DetectedObject array
    if (!parse_detection_outputs(inference, objects, max_objects, num_objects)) {
        syslog(LOG_ERR, "[Larod] Failed to parse detection outputs");
        goto cleanup;
    }

    success = true;

    // Update statistics
    uint64_t end_time = get_time_ms();
    float latency = (float)(end_time - start_time);

    inference->total_inference_ms += latency;
    inference->inference_count++;

    if (latency < inference->min_inference_ms) {
        inference->min_inference_ms = latency;
    }
    if (latency > inference->max_inference_ms) {
        inference->max_inference_ms = latency;
    }

cleanup:
    pthread_mutex_unlock(&inference->mutex);
    return success;
}

bool larod_inference_get_model_info(LarodInference* inference,
                                   LarodModelInfo* info) {
    if (!inference || !info) {
        return false;
    }

    pthread_mutex_lock(&inference->mutex);
    *info = inference->model_info;
    pthread_mutex_unlock(&inference->mutex);

    return true;
}

void larod_inference_set_crop(LarodInference* inference,
                              unsigned int x,
                              unsigned int y,
                              unsigned int width,
                              unsigned int height) {
    if (!inference) {
        return;
    }

    pthread_mutex_lock(&inference->mutex);

    inference->crop_x = x;
    inference->crop_y = y;
    inference->crop_w = width;
    inference->crop_h = height;

    // Update crop map
    if (inference->crop_map) {
        larodDestroyMap(&inference->crop_map);
    }

    GError* error = NULL;
    inference->crop_map = larodCreateMap(&error);
    if (!inference->crop_map) {
        syslog(LOG_ERR, "[Larod] Failed to create crop map: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
        pthread_mutex_unlock(&inference->mutex);
        return;
    }

    if (!larodMapSetIntArr4(inference->crop_map, "image.input.crop",
                           x, y, width, height, &error)) {
        syslog(LOG_ERR, "[Larod] Failed to set crop parameters: %s",
               error ? error->message : "unknown error");
        if (error) g_error_free(error);
    }

    pthread_mutex_unlock(&inference->mutex);
}

void larod_inference_get_stats(LarodInference* inference,
                               float* avg_inference_ms,
                               float* min_inference_ms,
                               float* max_inference_ms,
                               uint64_t* total_inferences) {
    if (!inference) {
        return;
    }

    pthread_mutex_lock(&inference->mutex);

    if (avg_inference_ms) {
        *avg_inference_ms = (inference->inference_count > 0) ?
            (inference->total_inference_ms / inference->inference_count) : 0.0f;
    }
    if (min_inference_ms) {
        *min_inference_ms = inference->min_inference_ms;
    }
    if (max_inference_ms) {
        *max_inference_ms = inference->max_inference_ms;
    }
    if (total_inferences) {
        *total_inferences = inference->inference_count;
    }

    pthread_mutex_unlock(&inference->mutex);
}

void larod_inference_destroy(LarodInference* inference) {
    if (!inference) {
        return;
    }

    pthread_mutex_lock(&inference->mutex);

    // Destroy crop map
    if (inference->crop_map) {
        larodDestroyMap(&inference->crop_map);
    }

    // Destroy job requests
    if (inference->pp_req) {
        larodDestroyJobRequest(&inference->pp_req);
    }
    if (inference->inf_req) {
        larodDestroyJobRequest(&inference->inf_req);
    }

    // Unmap input buffer
    if (inference->input_addr != MAP_FAILED && inference->input_addr != NULL) {
        munmap(inference->input_addr, inference->input_size);
    }

    // Destroy tensors
    if (inference->pp_input_tensors) {
        larodDestroyTensors(inference->conn, &inference->pp_input_tensors,
                           inference->pp_num_inputs, NULL);
    }
    if (inference->pp_output_tensors) {
        larodDestroyTensors(inference->conn, &inference->pp_output_tensors,
                           inference->pp_num_outputs, NULL);
    }
    if (inference->input_tensors) {
        larodDestroyTensors(inference->conn, &inference->input_tensors,
                           inference->num_inputs, NULL);
    }
    if (inference->output_tensors) {
        larodDestroyTensors(inference->conn, &inference->output_tensors,
                           inference->num_outputs, NULL);
    }

    // Destroy models
    if (inference->preprocessing_model) {
        larodDestroyModel(&inference->preprocessing_model);
    }
    if (inference->model) {
        larodDestroyModel(&inference->model);
    }

    // Disconnect from Larod
    if (inference->conn) {
        larodDisconnect(&inference->conn, NULL);
    }

    pthread_mutex_unlock(&inference->mutex);
    pthread_mutex_destroy(&inference->mutex);

    free(inference);

    syslog(LOG_INFO, "[Larod] Inference engine destroyed");
}

// ============================================================================
// Internal Helper Functions
// ============================================================================

static uint64_t get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static larodModel* create_inference_model(LarodInference* inference,
                                          GError** error) {
    // Open model file
    FILE* model_file = fopen(inference->config.model_path, "rb");
    if (!model_file) {
        syslog(LOG_ERR, "[Larod] Failed to open model file: %s",
               strerror(errno));
        return NULL;
    }

    int model_fd = fileno(model_file);

    // Get device
    const larodDevice* device = larodGetDevice(
        inference->conn,
        inference->config.device_name,
        0,
        error
    );

    if (!device) {
        fclose(model_file);
        return NULL;
    }

    // Load model with retry for power availability
    larodModel* model = NULL;
    int retry_count = 0;

    while (!model && retry_count < MAX_POWER_RETRIES) {
        model = larodLoadModel(
            inference->conn,
            model_fd,
            device,
            LAROD_ACCESS_PRIVATE,
            "omnisight_detection",
            NULL,
            error
        );

        if (!model && *error && (*error)->code == LAROD_ERROR_POWER_NOT_AVAILABLE) {
            syslog(LOG_WARNING, "[Larod] DLPU power not available, retrying... (%d/%d)",
                   retry_count + 1, MAX_POWER_RETRIES);
            larodClearError(error);
            usleep(POWER_RETRY_DELAY_MS * 1000 * (retry_count + 1));
            retry_count++;
        } else {
            break;
        }
    }

    fclose(model_file);

    if (!model) {
        syslog(LOG_ERR, "[Larod] Failed to load model after %d retries",
               retry_count);
    } else {
        syslog(LOG_INFO, "[Larod] Model loaded successfully on device: %s",
               inference->config.device_name);
    }

    return model;
}

static larodModel* create_preprocessing_model(LarodInference* inference,
                                              GError** error) {
    // Create preprocessing map for YUV → RGB conversion
    larodMap* map = larodCreateMap(error);
    if (!map) {
        return NULL;
    }

    // Input format: YUV (from VDO)
    if (!larodMapSetStr(map, "image.input.format", "nv12", error)) {
        larodDestroyMap(&map);
        return NULL;
    }

    if (!larodMapSetIntArr2(map, "image.input.size",
                           inference->config.width,
                           inference->config.height, error)) {
        larodDestroyMap(&map);
        return NULL;
    }

    // Output format: RGB (for model)
    if (!larodMapSetStr(map, "image.output.format", "rgb-interleaved", error)) {
        larodDestroyMap(&map);
        return NULL;
    }

    if (!larodMapSetIntArr2(map, "image.output.size",
                           inference->config.width,
                           inference->config.height, error)) {
        larodDestroyMap(&map);
        return NULL;
    }

    // Get preprocessing device (CPU-based conversion)
    const larodDevice* pp_device = larodGetDevice(
        inference->conn,
        "cpu-proc",
        0,
        error
    );

    if (!pp_device) {
        larodDestroyMap(&map);
        return NULL;
    }

    // Load preprocessing model
    larodModel* pp_model = larodLoadModel(
        inference->conn,
        -1,  // No file descriptor for built-in preprocessing
        pp_device,
        LAROD_ACCESS_PRIVATE,
        "",
        map,
        error
    );

    larodDestroyMap(&map);

    if (pp_model) {
        syslog(LOG_INFO, "[Larod] Preprocessing model created (YUV → RGB)");
    }

    return pp_model;
}

static bool setup_tensors(larodConnection* conn, larodModel* model,
                         larodTensor*** inputs, size_t* num_inputs,
                         larodTensor*** outputs, size_t* num_outputs,
                         GError** error) {
    // Get input tensors
    *inputs = larodGetModelInputs(model, num_inputs, error);
    if (!*inputs) {
        return false;
    }

    // Get output tensors
    *outputs = larodGetModelOutputs(model, num_outputs, error);
    if (!*outputs) {
        larodDestroyTensors(conn, inputs, *num_inputs, NULL);
        return false;
    }

    syslog(LOG_INFO, "[Larod] Model has %zu inputs, %zu outputs",
           *num_inputs, *num_outputs);

    return true;
}

static bool setup_preprocessing(LarodInference* inference, GError** error) {
    // Create preprocessing model
    inference->preprocessing_model = create_preprocessing_model(inference, error);
    if (!inference->preprocessing_model) {
        return false;
    }

    // Setup preprocessing tensors
    if (!setup_tensors(inference->conn, inference->preprocessing_model,
                      &inference->pp_input_tensors, &inference->pp_num_inputs,
                      &inference->pp_output_tensors, &inference->pp_num_outputs,
                      error)) {
        larodDestroyModel(&inference->preprocessing_model);
        return false;
    }

    // Create preprocessing job request
    inference->pp_req = larodCreateJobRequest(
        inference->preprocessing_model,
        inference->pp_input_tensors,
        inference->pp_num_inputs,
        inference->pp_output_tensors,
        inference->pp_num_outputs,
        inference->crop_map,
        error
    );

    if (!inference->pp_req) {
        larodDestroyTensors(inference->conn, &inference->pp_input_tensors,
                           inference->pp_num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->pp_output_tensors,
                           inference->pp_num_outputs, NULL);
        larodDestroyModel(&inference->preprocessing_model);
        return false;
    }

    // Create inference job request (uses preprocessing output as input)
    inference->inf_req = larodCreateJobRequest(
        inference->model,
        inference->pp_output_tensors,
        inference->pp_num_outputs,
        inference->output_tensors,
        inference->num_outputs,
        NULL,
        error
    );

    if (!inference->inf_req) {
        larodDestroyJobRequest(&inference->pp_req);
        larodDestroyTensors(inference->conn, &inference->pp_input_tensors,
                           inference->pp_num_inputs, NULL);
        larodDestroyTensors(inference->conn, &inference->pp_output_tensors,
                           inference->pp_num_outputs, NULL);
        larodDestroyModel(&inference->preprocessing_model);
        return false;
    }

    inference->use_preprocessing = true;
    return true;
}

static bool parse_detection_outputs(LarodInference* inference,
                                   DetectedObject* objects,
                                   uint32_t max_objects,
                                   uint32_t* num_objects) {
    // Standard object detection model outputs (SSD/MobileNet format):
    // output[0]: Detection boxes [N, 4] (y1, x1, y2, x2 normalized)
    // output[1]: Detection classes [N]
    // output[2]: Detection scores [N]
    // output[3]: Number of detections [1]

    *num_objects = 0;

    if (inference->num_outputs < 4) {
        syslog(LOG_ERR, "[Larod] Expected 4 output tensors, got %zu",
               inference->num_outputs);
        return false;
    }

    GError* error = NULL;

    // Map output tensors
    int boxes_fd = larodGetTensorFd(inference->output_tensors[0], &error);
    int classes_fd = larodGetTensorFd(inference->output_tensors[1], &error);
    int scores_fd = larodGetTensorFd(inference->output_tensors[2], &error);
    int num_detections_fd = larodGetTensorFd(inference->output_tensors[3], &error);

    if (boxes_fd == LAROD_INVALID_FD || classes_fd == LAROD_INVALID_FD ||
        scores_fd == LAROD_INVALID_FD || num_detections_fd == LAROD_INVALID_FD) {
        syslog(LOG_ERR, "[Larod] Failed to get output tensor fds");
        return false;
    }

    size_t boxes_size, classes_size, scores_size, num_det_size;
    larodGetTensorFdSize(inference->output_tensors[0], &boxes_size, &error);
    larodGetTensorFdSize(inference->output_tensors[1], &classes_size, &error);
    larodGetTensorFdSize(inference->output_tensors[2], &scores_size, &error);
    larodGetTensorFdSize(inference->output_tensors[3], &num_det_size, &error);

    float* boxes = mmap(NULL, boxes_size, PROT_READ, MAP_SHARED, boxes_fd, 0);
    float* classes = mmap(NULL, classes_size, PROT_READ, MAP_SHARED, classes_fd, 0);
    float* scores = mmap(NULL, scores_size, PROT_READ, MAP_SHARED, scores_fd, 0);
    float* num_det = mmap(NULL, num_det_size, PROT_READ, MAP_SHARED, num_detections_fd, 0);

    if (boxes == MAP_FAILED || classes == MAP_FAILED ||
        scores == MAP_FAILED || num_det == MAP_FAILED) {
        syslog(LOG_ERR, "[Larod] Failed to map output tensors");
        return false;
    }

    uint32_t detected = (uint32_t)num_det[0];
    if (detected > max_objects) {
        detected = max_objects;
    }

    uint64_t timestamp = get_time_ms();

    // Parse detections
    uint32_t valid_count = 0;
    for (uint32_t i = 0; i < detected; i++) {
        float confidence = scores[i];

        // Filter by confidence threshold
        if (confidence < inference->config.confidence_threshold) {
            continue;
        }

        // Extract bounding box (y1, x1, y2, x2 normalized [0,1])
        float y1 = boxes[i * 4 + 0];
        float x1 = boxes[i * 4 + 1];
        float y2 = boxes[i * 4 + 2];
        float x2 = boxes[i * 4 + 3];

        // Convert to (x, y, width, height)
        objects[valid_count].bbox.x = x1;
        objects[valid_count].bbox.y = y1;
        objects[valid_count].bbox.width = x2 - x1;
        objects[valid_count].bbox.height = y2 - y1;

        objects[valid_count].confidence = confidence;
        objects[valid_count].class_id = (ObjectClass)((int)classes[i]);
        objects[valid_count].id = inference->inference_count * 100 + i;
        objects[valid_count].timestamp_ms = timestamp;

        // Clear features (will be populated by tracking module)
        memset(objects[valid_count].features, 0, sizeof(objects[valid_count].features));

        valid_count++;

        if (valid_count >= max_objects) {
            break;
        }
    }

    *num_objects = valid_count;

    // Unmap tensors
    munmap(boxes, boxes_size);
    munmap(classes, classes_size);
    munmap(scores, scores_size);
    munmap(num_det, num_det_size);

    return true;
}
