/**
 * @file larod_inference.h
 * @brief Larod ML inference wrapper for OMNISIGHT
 *
 * Provides high-level interface to Axis Larod API for running
 * TensorFlow Lite models on ARTPEC-8 DLPU
 *
 * Optimized for:
 * - INT8 per-tensor quantization
 * - Batch size = 1
 * - Input size multiple of 32
 * - Regular convolutions (preferred over depthwise)
 */

#ifndef OMNISIGHT_LAROD_INFERENCE_H
#define OMNISIGHT_LAROD_INFERENCE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LarodInference LarodInference;

/**
 * Chip selection for inference
 */
typedef enum {
    LAROD_CHIP_CPU = 0,
    LAROD_CHIP_DLPU,
    LAROD_CHIP_AUTO,  // Automatically select best available
} LarodChip;

/**
 * Tensor data type
 */
typedef enum {
    LAROD_DTYPE_UINT8,
    LAROD_DTYPE_INT8,
    LAROD_DTYPE_FLOAT32,
} LarodDataType;

/**
 * Tensor descriptor
 */
typedef struct {
    const char* name;
    LarodDataType dtype;
    uint32_t dims[4];      // [batch, height, width, channels] or [batch, ...]
    uint32_t num_dims;
    float scale;           // Quantization scale
    int32_t zero_point;    // Quantization zero point
} LarodTensor;

/**
 * Model information
 */
typedef struct {
    const char* model_path;
    LarodChip chip;
    uint32_t num_threads;

    // Input tensors
    LarodTensor* inputs;
    uint32_t num_inputs;

    // Output tensors
    LarodTensor* outputs;
    uint32_t num_outputs;
} LarodModelInfo;

/**
 * Inference configuration
 */
typedef struct {
    const char* model_path;
    LarodChip chip;
    uint32_t num_threads;
    bool async_mode;
    uint32_t timeout_ms;
} LarodInferenceConfig;

/**
 * Inference callback for async mode
 */
typedef void (*LarodInferenceCallback)(
    const void** outputs,
    const LarodTensor* output_tensors,
    uint32_t num_outputs,
    void* user_data
);

/**
 * Initialize Larod inference engine
 *
 * @param config Inference configuration
 * @return Inference instance, NULL on failure
 */
LarodInference* larod_inference_init(const LarodInferenceConfig* config);

/**
 * Get model information (inputs, outputs, dimensions)
 *
 * @param inference Inference instance
 * @param info Output model information structure
 * @return true on success, false on failure
 */
bool larod_inference_get_model_info(
    LarodInference* inference,
    LarodModelInfo* info
);

/**
 * Run synchronous inference
 *
 * @param inference Inference instance
 * @param inputs Array of input data pointers
 * @param num_inputs Number of inputs
 * @param outputs Array to store output data pointers
 * @param num_outputs Number of outputs
 * @return true on success, false on failure
 */
bool larod_inference_run(
    LarodInference* inference,
    const void** inputs,
    uint32_t num_inputs,
    void** outputs,
    uint32_t num_outputs
);

/**
 * Run asynchronous inference
 *
 * @param inference Inference instance
 * @param inputs Array of input data pointers
 * @param num_inputs Number of inputs
 * @param callback Function called when inference completes
 * @param user_data User data passed to callback
 * @return true on success, false on failure
 */
bool larod_inference_run_async(
    LarodInference* inference,
    const void** inputs,
    uint32_t num_inputs,
    LarodInferenceCallback callback,
    void* user_data
);

/**
 * Wait for async inference to complete
 *
 * @param inference Inference instance
 * @param timeout_ms Timeout in milliseconds (0 = infinite)
 * @return true if completed, false on timeout
 */
bool larod_inference_wait(LarodInference* inference, uint32_t timeout_ms);

/**
 * Preprocess image data for model input
 *
 * Handles resizing, normalization, and quantization
 *
 * @param inference Inference instance
 * @param image_data Input image (RGB or BGR)
 * @param image_width Image width
 * @param image_height Image height
 * @param image_channels Number of channels (3 for RGB/BGR)
 * @param output_buffer Pre-allocated buffer for preprocessed data
 * @param tensor_index Input tensor index (usually 0)
 * @return true on success, false on failure
 */
bool larod_inference_preprocess_image(
    LarodInference* inference,
    const uint8_t* image_data,
    uint32_t image_width,
    uint32_t image_height,
    uint32_t image_channels,
    void* output_buffer,
    uint32_t tensor_index
);

/**
 * Dequantize output tensor to float32
 *
 * @param output Quantized output data
 * @param output_tensor Output tensor descriptor
 * @param float_output Buffer for dequantized float values
 * @param num_elements Number of elements to dequantize
 */
void larod_inference_dequantize(
    const void* output,
    const LarodTensor* output_tensor,
    float* float_output,
    uint32_t num_elements
);

/**
 * Get inference performance statistics
 *
 * @param inference Inference instance
 * @param avg_latency_ms Average inference latency
 * @param min_latency_ms Minimum latency observed
 * @param max_latency_ms Maximum latency observed
 * @param total_inferences Total inferences performed
 */
void larod_inference_get_stats(
    LarodInference* inference,
    float* avg_latency_ms,
    float* min_latency_ms,
    float* max_latency_ms,
    uint64_t* total_inferences
);

/**
 * Reset performance statistics
 *
 * @param inference Inference instance
 */
void larod_inference_reset_stats(LarodInference* inference);

/**
 * Destroy inference engine and free resources
 *
 * @param inference Inference instance
 */
void larod_inference_destroy(LarodInference* inference);

/**
 * Utility: Print model information
 *
 * @param info Model information structure
 */
void larod_inference_print_model_info(const LarodModelInfo* info);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_LAROD_INFERENCE_H
