#include "s11_network.h"
#include <stdlib.h>

s11_process_result_t jitter_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state)
{
    (void)pkt;
    (void)state;

    s11_process_result_t result;
    result.status = S11_PACKET_FORWARDED;  // jitter never drops packets

    double roll = (double)rand() / RAND_MAX;
    double jitter_ms = cfg->jitter_min_ms + roll * (cfg->jitter_max_ms - cfg->jitter_min_ms);

    result.delay_us = (uint64_t)(jitter_ms * 1000.0);

    return result;
}