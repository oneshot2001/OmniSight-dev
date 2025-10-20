/**
 * @file federated_learning.h
 * @brief Federated Learning for OMNISIGHT Swarm
 *
 * Privacy-preserving distributed learning:
 * - Cameras train locally on their data
 * - Only model updates (gradients) are shared, NOT raw video
 * - Coordinator aggregates updates (FedAvg algorithm)
 * - Improved models distributed back to cameras
 *
 * Key Privacy Features:
 * - No raw video or images leave the camera
 * - Differential privacy with gradient clipping
 * - Secure aggregation (optional)
 */

#ifndef OMNISIGHT_FEDERATED_LEARNING_H
#define OMNISIGHT_FEDERATED_LEARNING_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Forward Declarations
// ============================================================================

typedef struct FederatedLearner FederatedLearner;
typedef struct FederatedCoordinator FederatedCoordinator;

// ============================================================================
// Constants
// ============================================================================

#define FL_MAX_PARTICIPANTS 100     // Maximum cameras in federation
#define FL_MAX_MODEL_PARAMS 1000000 // Maximum model parameters

// ============================================================================
// Enums
// ============================================================================

/**
 * Federated learning algorithm
 */
typedef enum {
    FL_ALGORITHM_FEDAVG,        // Federated Averaging (standard)
    FL_ALGORITHM_FEDPROX,       // Federated Proximal (better for heterogeneous)
    FL_ALGORITHM_FEDADAM        // Federated Adam (adaptive learning rate)
} FederatedAlgorithm;

/**
 * Privacy mechanism
 */
typedef enum {
    PRIVACY_NONE,               // No additional privacy
    PRIVACY_GRADIENT_CLIPPING,  // Clip gradients to bound sensitivity
    PRIVACY_DIFFERENTIAL,       // Differential privacy with noise
    PRIVACY_SECURE_AGGREGATION  // Secure multi-party aggregation
} PrivacyMechanism;

/**
 * Training round status
 */
typedef enum {
    ROUND_STATUS_WAITING,       // Waiting for participants
    ROUND_STATUS_TRAINING,      // Participants training locally
    ROUND_STATUS_AGGREGATING,   // Coordinator aggregating updates
    ROUND_STATUS_DISTRIBUTING,  // Distributing new model
    ROUND_STATUS_COMPLETE       // Round complete
} RoundStatus;

// ============================================================================
// Data Structures
// ============================================================================

/**
 * Model gradient/parameter update
 */
typedef struct {
    uint32_t num_params;        // Number of parameters
    float* params;              // Parameter values (or gradients)
    uint32_t version;           // Model version

    // Training metadata
    uint32_t num_samples;       // Training samples used
    float loss;                 // Training loss
    float accuracy;             // Validation accuracy
    uint64_t training_time_ms;  // Time spent training
} ModelParameters;

/**
 * Local training configuration
 */
typedef struct {
    uint32_t batch_size;        // Local batch size
    uint32_t num_epochs;        // Local training epochs
    float learning_rate;        // Learning rate
    float momentum;             // SGD momentum
    float weight_decay;         // L2 regularization

    // Privacy
    PrivacyMechanism privacy;
    float gradient_clip_norm;   // Max gradient L2 norm
    float noise_multiplier;     // Differential privacy noise scale
    float privacy_budget;       // Privacy budget (epsilon)
} LocalTrainingConfig;

/**
 * Federated learning configuration
 */
typedef struct {
    FederatedAlgorithm algorithm; // FL algorithm
    uint32_t min_participants;    // Minimum participants per round
    uint32_t max_participants;    // Maximum participants per round
    float participation_rate;     // Fraction of cameras selected per round
    uint32_t rounds_per_epoch;    // Rounds before global epoch

    // Aggregation
    bool weighted_aggregation;    // Weight by num_samples
    float aggregation_threshold;  // Min improvement to accept

    // Privacy
    PrivacyMechanism privacy;
    float privacy_budget;         // Total privacy budget

    // Timeouts
    uint32_t round_timeout_ms;    // Round timeout
    uint32_t update_timeout_ms;   // Update submission timeout
} FederatedConfig;

/**
 * Participant information
 */
typedef struct {
    uint32_t camera_id;         // Camera ID
    bool is_selected;           // Selected for current round
    uint32_t num_samples;       // Local dataset size
    float loss;                 // Latest loss
    float accuracy;             // Latest accuracy
    uint64_t last_update_ms;    // Last update timestamp
    uint32_t rounds_participated; // Total rounds participated
} ParticipantInfo;

/**
 * Training round information
 */
typedef struct {
    uint32_t round_number;      // Round number
    RoundStatus status;         // Round status
    uint64_t start_time_ms;     // Round start time

    // Participants
    uint32_t num_selected;      // Selected participants
    uint32_t num_submitted;     // Submitted updates
    ParticipantInfo participants[FL_MAX_PARTICIPANTS];

    // Aggregated results
    float global_loss;          // Global loss
    float global_accuracy;      // Global accuracy
    float improvement;          // Improvement over previous round
} RoundInfo;

/**
 * Federated learning statistics
 */
typedef struct {
    uint32_t total_rounds;      // Total rounds completed
    uint32_t current_round;     // Current round number
    uint32_t global_epoch;      // Global epoch

    float best_accuracy;        // Best accuracy achieved
    float current_accuracy;     // Current accuracy
    float current_loss;         // Current loss

    uint32_t total_updates;     // Total updates received
    uint64_t total_samples;     // Total samples processed

    float avg_round_time_ms;    // Average round duration
    float privacy_spent;        // Privacy budget spent
} FederatedStats;

// ============================================================================
// Local Learner API (runs on each camera)
// ============================================================================

/**
 * Initialize local federated learner
 *
 * @param camera_id This camera's ID
 * @param config Local training configuration
 * @return Learner handle, or NULL on error
 */
FederatedLearner* fl_learner_init(
    uint32_t camera_id,
    const LocalTrainingConfig* config
);

/**
 * Destroy local learner
 *
 * @param learner Learner to destroy
 */
void fl_learner_destroy(FederatedLearner* learner);

/**
 * Set current model parameters
 *
 * Called when coordinator distributes new global model.
 *
 * @param learner Local learner
 * @param params Model parameters
 * @return true on success, false on error
 */
bool fl_learner_set_model(
    FederatedLearner* learner,
    const ModelParameters* params
);

/**
 * Train locally on camera's data
 *
 * Trains model on local dataset without sharing data.
 * Returns gradients/updates to share with coordinator.
 *
 * @param learner Local learner
 * @param update Output parameter update
 * @return true on success, false on error
 */
bool fl_learner_train(
    FederatedLearner* learner,
    ModelParameters* update
);

/**
 * Add training sample to local dataset
 *
 * @param learner Local learner
 * @param features Input features
 * @param label Ground truth label
 * @return true on success, false on error
 */
bool fl_learner_add_sample(
    FederatedLearner* learner,
    const float* features,
    uint32_t label
);

/**
 * Get number of local training samples
 *
 * @param learner Local learner
 * @return Number of samples
 */
uint32_t fl_learner_get_num_samples(const FederatedLearner* learner);

// ============================================================================
// Coordinator API (runs on coordinator camera or server)
// ============================================================================

/**
 * Initialize federated coordinator
 *
 * @param config Federated learning configuration
 * @return Coordinator handle, or NULL on error
 */
FederatedCoordinator* fl_coordinator_init(const FederatedConfig* config);

/**
 * Destroy coordinator
 *
 * @param coordinator Coordinator to destroy
 */
void fl_coordinator_destroy(FederatedCoordinator* coordinator);

/**
 * Register participant camera
 *
 * @param coordinator Coordinator
 * @param camera_id Camera ID
 * @param num_samples Number of local samples
 * @return true on success, false on error
 */
bool fl_coordinator_register_participant(
    FederatedCoordinator* coordinator,
    uint32_t camera_id,
    uint32_t num_samples
);

/**
 * Unregister participant camera
 *
 * @param coordinator Coordinator
 * @param camera_id Camera ID
 */
void fl_coordinator_unregister_participant(
    FederatedCoordinator* coordinator,
    uint32_t camera_id
);

/**
 * Start new training round
 *
 * Selects participants and broadcasts training request.
 *
 * @param coordinator Coordinator
 * @param round_info Output round information
 * @return true on success, false on error
 */
bool fl_coordinator_start_round(
    FederatedCoordinator* coordinator,
    RoundInfo* round_info
);

/**
 * Submit participant update
 *
 * Called when participant submits local training update.
 *
 * @param coordinator Coordinator
 * @param camera_id Participant camera ID
 * @param update Model update
 * @return true on success, false on error
 */
bool fl_coordinator_submit_update(
    FederatedCoordinator* coordinator,
    uint32_t camera_id,
    const ModelParameters* update
);

/**
 * Aggregate updates and create new global model
 *
 * Uses FedAvg or other algorithm to combine updates.
 *
 * @param coordinator Coordinator
 * @param global_model Output global model
 * @return true on success, false on error
 */
bool fl_coordinator_aggregate(
    FederatedCoordinator* coordinator,
    ModelParameters* global_model
);

/**
 * Get current round information
 *
 * @param coordinator Coordinator
 * @param round_info Output round info
 * @return true on success, false on error
 */
bool fl_coordinator_get_round_info(
    const FederatedCoordinator* coordinator,
    RoundInfo* round_info
);

/**
 * Get federated learning statistics
 *
 * @param coordinator Coordinator
 * @param stats Output statistics
 */
void fl_coordinator_get_stats(
    const FederatedCoordinator* coordinator,
    FederatedStats* stats
);

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Apply differential privacy to gradients
 *
 * Clips gradients and adds calibrated noise.
 *
 * @param gradients Gradient array
 * @param num_params Number of parameters
 * @param clip_norm Maximum L2 norm
 * @param noise_multiplier Noise scale
 * @param privacy_budget Privacy budget (epsilon)
 */
void fl_apply_differential_privacy(
    float* gradients,
    uint32_t num_params,
    float clip_norm,
    float noise_multiplier,
    float privacy_budget
);

/**
 * Calculate gradient L2 norm
 *
 * @param gradients Gradient array
 * @param num_params Number of parameters
 * @return L2 norm
 */
float fl_gradient_norm(const float* gradients, uint32_t num_params);

/**
 * Clip gradients to maximum norm
 *
 * @param gradients Gradient array (modified in-place)
 * @param num_params Number of parameters
 * @param max_norm Maximum L2 norm
 */
void fl_clip_gradients(float* gradients, uint32_t num_params, float max_norm);

#ifdef __cplusplus
}
#endif

#endif // OMNISIGHT_FEDERATED_LEARNING_H
