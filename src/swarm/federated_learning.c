/**
 * @file federated_learning.c
 * @brief Federated Learning implementation
 *
 * Privacy-preserving distributed learning for OMNISIGHT swarm.
 */

#include "federated_learning.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

// ============================================================================
// Internal Structures
// ============================================================================

struct FederatedLearner {
    uint32_t camera_id;
    LocalTrainingConfig config;

    // Local model
    ModelParameters model;

    // Local training data (simplified)
    uint32_t num_samples;
    uint32_t dataset_capacity;

    pthread_mutex_t mutex;
};

struct FederatedCoordinator {
    FederatedConfig config;

    // Participants
    uint32_t num_participants;
    ParticipantInfo participants[FL_MAX_PARTICIPANTS];

    // Current round
    RoundInfo current_round;

    // Global model
    ModelParameters global_model;

    // Statistics
    FederatedStats stats;

    // Received updates for current round
    uint32_t num_received_updates;
    ModelParameters received_updates[FL_MAX_PARTICIPANTS];

    pthread_mutex_t mutex;
};

// ============================================================================
// Utility Functions
// ============================================================================

static float randn() {
    // Box-Muller transform for Gaussian noise
    float u1 = (float)rand() / RAND_MAX;
    float u2 = (float)rand() / RAND_MAX;
    return sqrtf(-2.0f * logf(u1)) * cosf(2.0f * M_PI * u2);
}

// ============================================================================
// Privacy Functions
// ============================================================================

void fl_apply_differential_privacy(
    float* gradients,
    uint32_t num_params,
    float clip_norm,
    float noise_multiplier,
    float privacy_budget
) {
    if (!gradients || num_params == 0) return;

    // 1. Clip gradients
    fl_clip_gradients(gradients, num_params, clip_norm);

    // 2. Add calibrated Gaussian noise
    float noise_scale = noise_multiplier * clip_norm;

    for (uint32_t i = 0; i < num_params; i++) {
        gradients[i] += randn() * noise_scale;
    }
}

float fl_gradient_norm(const float* gradients, uint32_t num_params) {
    if (!gradients || num_params == 0) return 0.0f;

    float sum_squares = 0.0f;
    for (uint32_t i = 0; i < num_params; i++) {
        sum_squares += gradients[i] * gradients[i];
    }

    return sqrtf(sum_squares);
}

void fl_clip_gradients(float* gradients, uint32_t num_params, float max_norm) {
    if (!gradients || num_params == 0 || max_norm <= 0.0f) return;

    float norm = fl_gradient_norm(gradients, num_params);

    if (norm > max_norm) {
        float scale = max_norm / norm;
        for (uint32_t i = 0; i < num_params; i++) {
            gradients[i] *= scale;
        }
    }
}

// ============================================================================
// Local Learner Implementation
// ============================================================================

FederatedLearner* fl_learner_init(
    uint32_t camera_id,
    const LocalTrainingConfig* config
) {
    if (!config) {
        fprintf(stderr, "[FL] ERROR: NULL config\n");
        return NULL;
    }

    FederatedLearner* learner = calloc(1, sizeof(FederatedLearner));
    if (!learner) {
        fprintf(stderr, "[FL] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    learner->camera_id = camera_id;
    memcpy(&learner->config, config, sizeof(LocalTrainingConfig));

    pthread_mutex_init(&learner->mutex, NULL);

    // Initialize local dataset
    learner->dataset_capacity = 10000;  // Placeholder capacity
    learner->num_samples = 0;

    printf("[FL] Learner initialized (camera_id=%u)\n", camera_id);

    return learner;
}

void fl_learner_destroy(FederatedLearner* learner) {
    if (!learner) return;

    if (learner->model.params) {
        free(learner->model.params);
    }

    pthread_mutex_destroy(&learner->mutex);
    free(learner);
}

bool fl_learner_set_model(
    FederatedLearner* learner,
    const ModelParameters* params
) {
    if (!learner || !params) return false;

    pthread_mutex_lock(&learner->mutex);

    // Free old model if exists
    if (learner->model.params) {
        free(learner->model.params);
    }

    // Copy new model
    learner->model.num_params = params->num_params;
    learner->model.version = params->version;

    learner->model.params = malloc(params->num_params * sizeof(float));
    if (!learner->model.params) {
        pthread_mutex_unlock(&learner->mutex);
        return false;
    }

    memcpy(learner->model.params, params->params,
           params->num_params * sizeof(float));

    pthread_mutex_unlock(&learner->mutex);

    printf("[FL] Model updated (version=%u, params=%u)\n",
           params->version, params->num_params);

    return true;
}

bool fl_learner_train(
    FederatedLearner* learner,
    ModelParameters* update
) {
    if (!learner || !update) return false;

    pthread_mutex_lock(&learner->mutex);

    if (learner->num_samples == 0) {
        fprintf(stderr, "[FL] WARNING: No training samples\n");
        pthread_mutex_unlock(&learner->mutex);
        return false;
    }

    // TODO: Implement actual training
    // For now, simulate training with random gradients

    uint32_t num_params = learner->model.num_params;
    if (num_params == 0) {
        num_params = 1000;  // Default size
    }

    update->num_params = num_params;
    update->version = learner->model.version;
    update->num_samples = learner->num_samples;

    update->params = malloc(num_params * sizeof(float));
    if (!update->params) {
        pthread_mutex_unlock(&learner->mutex);
        return false;
    }

    // Simulate gradient computation
    for (uint32_t i = 0; i < num_params; i++) {
        update->params[i] = ((float)rand() / RAND_MAX) * 0.1f - 0.05f;
    }

    // Apply privacy mechanisms
    if (learner->config.privacy == PRIVACY_GRADIENT_CLIPPING) {
        fl_clip_gradients(update->params, num_params,
                         learner->config.gradient_clip_norm);
    }
    else if (learner->config.privacy == PRIVACY_DIFFERENTIAL) {
        fl_apply_differential_privacy(
            update->params,
            num_params,
            learner->config.gradient_clip_norm,
            learner->config.noise_multiplier,
            learner->config.privacy_budget
        );
    }

    // Simulate training metrics
    update->loss = 0.5f + ((float)rand() / RAND_MAX) * 0.3f;
    update->accuracy = 0.7f + ((float)rand() / RAND_MAX) * 0.2f;
    update->training_time_ms = 1000 + rand() % 3000;

    pthread_mutex_unlock(&learner->mutex);

    printf("[FL] Training complete (samples=%u, loss=%.3f, acc=%.3f)\n",
           update->num_samples, update->loss, update->accuracy);

    return true;
}

bool fl_learner_add_sample(
    FederatedLearner* learner,
    const float* features,
    uint32_t label
) {
    if (!learner || !features) return false;

    pthread_mutex_lock(&learner->mutex);

    if (learner->num_samples >= learner->dataset_capacity) {
        fprintf(stderr, "[FL] WARNING: Dataset capacity reached\n");
        pthread_mutex_unlock(&learner->mutex);
        return false;
    }

    // TODO: Actually store sample
    // For now, just increment counter

    learner->num_samples++;

    pthread_mutex_unlock(&learner->mutex);

    return true;
}

uint32_t fl_learner_get_num_samples(const FederatedLearner* learner) {
    return learner ? learner->num_samples : 0;
}

// ============================================================================
// Coordinator Implementation
// ============================================================================

FederatedCoordinator* fl_coordinator_init(const FederatedConfig* config) {
    if (!config) {
        fprintf(stderr, "[FL] ERROR: NULL config\n");
        return NULL;
    }

    FederatedCoordinator* coordinator = calloc(1, sizeof(FederatedCoordinator));
    if (!coordinator) {
        fprintf(stderr, "[FL] ERROR: Failed to allocate memory\n");
        return NULL;
    }

    memcpy(&coordinator->config, config, sizeof(FederatedConfig));

    pthread_mutex_init(&coordinator->mutex, NULL);

    coordinator->current_round.status = ROUND_STATUS_WAITING;

    printf("[FL] Coordinator initialized (algorithm=%d)\n", config->algorithm);

    return coordinator;
}

void fl_coordinator_destroy(FederatedCoordinator* coordinator) {
    if (!coordinator) return;

    if (coordinator->global_model.params) {
        free(coordinator->global_model.params);
    }

    for (uint32_t i = 0; i < coordinator->num_received_updates; i++) {
        if (coordinator->received_updates[i].params) {
            free(coordinator->received_updates[i].params);
        }
    }

    pthread_mutex_destroy(&coordinator->mutex);
    free(coordinator);
}

bool fl_coordinator_register_participant(
    FederatedCoordinator* coordinator,
    uint32_t camera_id,
    uint32_t num_samples
) {
    if (!coordinator) return false;

    pthread_mutex_lock(&coordinator->mutex);

    if (coordinator->num_participants >= FL_MAX_PARTICIPANTS) {
        fprintf(stderr, "[FL] ERROR: Max participants reached\n");
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    // Check if already registered
    for (uint32_t i = 0; i < coordinator->num_participants; i++) {
        if (coordinator->participants[i].camera_id == camera_id) {
            // Update sample count
            coordinator->participants[i].num_samples = num_samples;
            pthread_mutex_unlock(&coordinator->mutex);
            return true;
        }
    }

    // Add new participant
    ParticipantInfo* p = &coordinator->participants[coordinator->num_participants++];
    memset(p, 0, sizeof(ParticipantInfo));
    p->camera_id = camera_id;
    p->num_samples = num_samples;

    pthread_mutex_unlock(&coordinator->mutex);

    printf("[FL] Participant registered (id=%u, samples=%u)\n",
           camera_id, num_samples);

    return true;
}

void fl_coordinator_unregister_participant(
    FederatedCoordinator* coordinator,
    uint32_t camera_id
) {
    if (!coordinator) return;

    pthread_mutex_lock(&coordinator->mutex);

    for (uint32_t i = 0; i < coordinator->num_participants; i++) {
        if (coordinator->participants[i].camera_id == camera_id) {
            // Remove by shifting
            for (uint32_t j = i; j < coordinator->num_participants - 1; j++) {
                coordinator->participants[j] = coordinator->participants[j + 1];
            }
            coordinator->num_participants--;
            break;
        }
    }

    pthread_mutex_unlock(&coordinator->mutex);
}

bool fl_coordinator_start_round(
    FederatedCoordinator* coordinator,
    RoundInfo* round_info
) {
    if (!coordinator) return false;

    pthread_mutex_lock(&coordinator->mutex);

    if (coordinator->num_participants < coordinator->config.min_participants) {
        fprintf(stderr, "[FL] ERROR: Not enough participants\n");
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    // Initialize new round
    coordinator->current_round.round_number++;
    coordinator->current_round.status = ROUND_STATUS_TRAINING;
    coordinator->current_round.start_time_ms = 0;  // TODO: Get timestamp
    coordinator->current_round.num_selected = 0;
    coordinator->current_round.num_submitted = 0;

    // Select participants (random sampling)
    uint32_t target_count = (uint32_t)(coordinator->num_participants *
                                      coordinator->config.participation_rate);
    if (target_count < coordinator->config.min_participants) {
        target_count = coordinator->config.min_participants;
    }
    if (target_count > coordinator->config.max_participants) {
        target_count = coordinator->config.max_participants;
    }

    // Simple selection: take first N participants
    // TODO: Implement better selection (random, stratified, etc.)
    for (uint32_t i = 0; i < target_count && i < coordinator->num_participants; i++) {
        coordinator->participants[i].is_selected = true;
        coordinator->current_round.participants[coordinator->current_round.num_selected++] =
            coordinator->participants[i];
    }

    // Reset received updates
    coordinator->num_received_updates = 0;

    if (round_info) {
        *round_info = coordinator->current_round;
    }

    pthread_mutex_unlock(&coordinator->mutex);

    printf("[FL] Round %u started (%u participants selected)\n",
           coordinator->current_round.round_number,
           coordinator->current_round.num_selected);

    return true;
}

bool fl_coordinator_submit_update(
    FederatedCoordinator* coordinator,
    uint32_t camera_id,
    const ModelParameters* update
) {
    if (!coordinator || !update) return false;

    pthread_mutex_lock(&coordinator->mutex);

    // Check if participant is selected for this round
    bool is_selected = false;
    for (uint32_t i = 0; i < coordinator->current_round.num_selected; i++) {
        if (coordinator->current_round.participants[i].camera_id == camera_id) {
            is_selected = true;
            coordinator->current_round.participants[i].loss = update->loss;
            coordinator->current_round.participants[i].accuracy = update->accuracy;
            coordinator->current_round.participants[i].rounds_participated++;
            break;
        }
    }

    if (!is_selected) {
        fprintf(stderr, "[FL] WARNING: Update from non-selected participant %u\n",
               camera_id);
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    // Store update
    if (coordinator->num_received_updates >= FL_MAX_PARTICIPANTS) {
        fprintf(stderr, "[FL] ERROR: Too many updates\n");
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    ModelParameters* stored = &coordinator->received_updates[coordinator->num_received_updates++];
    stored->num_params = update->num_params;
    stored->version = update->version;
    stored->num_samples = update->num_samples;
    stored->loss = update->loss;
    stored->accuracy = update->accuracy;
    stored->training_time_ms = update->training_time_ms;

    stored->params = malloc(update->num_params * sizeof(float));
    if (!stored->params) {
        coordinator->num_received_updates--;
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    memcpy(stored->params, update->params, update->num_params * sizeof(float));

    coordinator->current_round.num_submitted++;
    coordinator->stats.total_updates++;

    pthread_mutex_unlock(&coordinator->mutex);

    printf("[FL] Update received from camera %u (%u/%u)\n",
           camera_id,
           coordinator->current_round.num_submitted,
           coordinator->current_round.num_selected);

    return true;
}

bool fl_coordinator_aggregate(
    FederatedCoordinator* coordinator,
    ModelParameters* global_model
) {
    if (!coordinator || !global_model) return false;

    pthread_mutex_lock(&coordinator->mutex);

    if (coordinator->num_received_updates == 0) {
        fprintf(stderr, "[FL] ERROR: No updates to aggregate\n");
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    coordinator->current_round.status = ROUND_STATUS_AGGREGATING;

    // FedAvg: Weighted average of model updates
    uint32_t num_params = coordinator->received_updates[0].num_params;
    uint64_t total_samples = 0;

    // Calculate total samples
    for (uint32_t i = 0; i < coordinator->num_received_updates; i++) {
        total_samples += coordinator->received_updates[i].num_samples;
    }

    // Allocate global model
    global_model->num_params = num_params;
    global_model->version = coordinator->global_model.version + 1;
    global_model->params = malloc(num_params * sizeof(float));
    if (!global_model->params) {
        pthread_mutex_unlock(&coordinator->mutex);
        return false;
    }

    // Initialize to zero
    memset(global_model->params, 0, num_params * sizeof(float));

    // Weighted average
    for (uint32_t i = 0; i < coordinator->num_received_updates; i++) {
        ModelParameters* update = &coordinator->received_updates[i];
        float weight = (float)update->num_samples / total_samples;

        for (uint32_t j = 0; j < num_params; j++) {
            global_model->params[j] += weight * update->params[j];
        }
    }

    // Calculate aggregated metrics
    float total_loss = 0.0f;
    float total_accuracy = 0.0f;
    for (uint32_t i = 0; i < coordinator->num_received_updates; i++) {
        total_loss += coordinator->received_updates[i].loss;
        total_accuracy += coordinator->received_updates[i].accuracy;
    }

    coordinator->current_round.global_loss = total_loss / coordinator->num_received_updates;
    coordinator->current_round.global_accuracy = total_accuracy / coordinator->num_received_updates;

    // Update statistics
    coordinator->stats.current_accuracy = coordinator->current_round.global_accuracy;
    coordinator->stats.current_loss = coordinator->current_round.global_loss;
    if (coordinator->current_round.global_accuracy > coordinator->stats.best_accuracy) {
        coordinator->stats.best_accuracy = coordinator->current_round.global_accuracy;
    }

    coordinator->current_round.status = ROUND_STATUS_COMPLETE;
    coordinator->stats.total_rounds++;

    pthread_mutex_unlock(&coordinator->mutex);

    printf("[FL] Aggregation complete (loss=%.3f, acc=%.3f)\n",
           coordinator->current_round.global_loss,
           coordinator->current_round.global_accuracy);

    return true;
}

bool fl_coordinator_get_round_info(
    const FederatedCoordinator* coordinator,
    RoundInfo* round_info
) {
    if (!coordinator || !round_info) return false;

    pthread_mutex_lock((pthread_mutex_t*)&coordinator->mutex);
    *round_info = coordinator->current_round;
    pthread_mutex_unlock((pthread_mutex_t*)&coordinator->mutex);

    return true;
}

void fl_coordinator_get_stats(
    const FederatedCoordinator* coordinator,
    FederatedStats* stats
) {
    if (!coordinator || !stats) return;

    pthread_mutex_lock((pthread_mutex_t*)&coordinator->mutex);
    *stats = coordinator->stats;
    pthread_mutex_unlock((pthread_mutex_t*)&coordinator->mutex);
}
