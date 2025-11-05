/**
 * @file larod_inference.h
 * @brief OMNISIGHT Larod ML Inference Module
 *
 * Provides hardware-accelerated object detection using Axis Larod API.
 * Optimized for ARTPEC-9 DLPU with TensorFlow Lite models.
 *
 * Copyright (C) 2025 OMNISIGHT
 * Based on Axis ACAP Native SDK examples
 */

#ifndef OMNISIGHT_LAROD_INFERENCE_H
#define OMNISIGHT_LAROD_INFERENCE_H

#include <stdint.h>
#include <stdbool.h>

#include <glib.h>
#include <larod.h>
#include <vdo-buffer.h>
#include <vdo-types.h>

#include "perception.h"  // For DetectedObject

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct LarodInference LarodInference;

/**
 * Larod inference configuration
 */
typedef struct {
    const char* model_path;      // Path to TensorFlow Lite model
    const char* device_name;     // "dlpu", "cpu", "ambarella-cvflow"
    unsigned int width;          // Model input width (e.g., 300, 416)
    unsigned int height;         // Model input height
    VdoFormat input_format;      // VDO_FORMAT_YUV or VDO_FORMAT_RGB
    float confidence_threshold;  // Minimum detection confidence (0.0-1.0)
    unsigned int max_detections; // Maximum objects per frame
} LarodInferenceConfig;

/**
 * Model metadata
 */
typedef struct {
    unsigned int input_width;
    unsigned int input_height;
    VdoFormat input_format;
    unsigned int num_outputs;
    size_t input_buffer_size;
    size_t output_buffer_size;
} LarodModelInfo;

/**
 * Initialize Larod inference engine
 *
 * Connects to Larod, loads model onto specified device (DLPU/CPU),
 * allocates input/output tensors.
 *
 * @param config Inference configuration
 * @return LarodInference instance or NULL on failure
 */
LarodInference* larod_inference_init(const LarodInferenceConfig* config);

/**
 * Run inference on VDO buffer
 *
 * Takes YUV or RGB frame from VDO, runs object detection,
 * parses outputs into DetectedObject array.
 *
 * @param inference LarodInference instance
 * @param vdo_buffer Video frame from VDO capture
 * @param objects Output array for detected objects
 * @param max_objects Maximum objects to return
 * @param num_objects Output: actual number of objects detected
 * @return true on success, false on failure
 */
bool larod_inference_run(LarodInference* inference,
                         VdoBuffer* vdo_buffer,
                         DetectedObject* objects,
                         uint32_t max_objects,
                         uint32_t* num_objects);

/**
 * Get model metadata
 *
 * @param inference LarodInference instance
 * @param info Output structure for model metadata
 * @return true on success, false on failure
 */
bool larod_inference_get_model_info(LarodInference* inference,
                                    LarodModelInfo* info);

/**
 * Update crop region (for aspect ratio adjustment)
 *
 * @param inference LarodInference instance
 * @param x Crop X offset
 * @param y Crop Y offset
 * @param width Crop width
 * @param height Crop height
 */
void larod_inference_set_crop(LarodInference* inference,
                              unsigned int x,
                              unsigned int y,
                              unsigned int width,
                              unsigned int height);

/**
 * Get performance statistics
 *
 * @param inference LarodInference instance
 * @param avg_inference_ms Average inference time
 * @param min_inference_ms Minimum inference time
 * @param max_inference_ms Maximum inference time
 * @param total_inferences Total inferences performed
 */
void larod_inference_get_stats(LarodInference* inference,
                               float* avg_inference_ms,
                               float* min_inference_ms,
                               float* max_inference_ms,
                               uint64_t* total_inferences);

/**
 * Destroy Larod inference instance
 *
 * Unloads model, disconnects from Larod, frees resources.
 *
 * @param inference LarodInference instance
 */
void larod_inference_destroy(LarodInference* inference);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_LAROD_INFERENCE_H
