#include "s11_network.h"

s11_process_result_t bandwidth_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state)
{
    (void)state;

    s11_process_result_t result;
    result.status = S11_PACKET_FORWARDED;  // no drops yet at this simple stage

    double packet_size_bits = (double)pkt->payload_size * 8.0;
    double transmission_time_s = packet_size_bits / cfg->bandwidth_bps;

    result.delay_us = (uint64_t)(transmission_time_s * 1000000.0);

    return result;
}