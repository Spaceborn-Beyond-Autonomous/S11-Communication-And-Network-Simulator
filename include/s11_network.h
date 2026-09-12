// include/s11_network.h
#ifndef S11_NETWORK_H
#define S11_NETWORK_H

#include <stdint.h>
#include <stdbool.h>
#include "s11_packet.h"

// Per-link config, one instance per link (from config/network.yaml)
typedef struct {
    char link_type[16];        // "lte", "5g", "mesh", "lora", "satellite", "default"

    double base_latency_ms;
    double jitter_min_ms;
    double jitter_max_ms;
    double packet_loss_rate;   // 0.0 - 1.0
    double bandwidth_bps;
    uint32_t queue_capacity;
} link_model_config_t;

// Result returned by every model
typedef struct {
    bool forward;             // false = packet dropped
    double added_delay_ms;
} model_result_t;

// Common signature for all four models
// pkt: from s11_packet.h
// state: for models needing memory across calls, else NULL
typedef model_result_t (*model_apply_fn)(const packet_t *pkt,
                                          const link_model_config_t *cfg,
                                          void *state);

#endif // S11_NETWORK_H