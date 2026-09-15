#include "s11_network.h"
#include <stddef.h>

s11_process_result_t bandwidth_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state)
{
    s11_process_result_t result;
    s11_bandwidth_state_t *bw_state = (s11_bandwidth_state_t *)state;

    if (bw_state != NULL && bw_state->queued_packets >= cfg->queue_capacity)
    {
        result.status = S11_PACKET_DROPPED;
        result.delay_us = 0U;
        return result;
    }

    double packet_size_bits = (double)pkt->payload_size * 8.0;
    double own_transmission_s = packet_size_bits / cfg->bandwidth_bps;
    uint64_t own_transmission_us = (uint64_t)(own_transmission_s * 1000000.0);

    /* This packet must wait for everything already queued ahead of it to
     * clear the link before its own transmission even starts. */
    uint64_t queue_wait_us = (bw_state != NULL) ? bw_state->pending_delay_us : 0U;

    result.status = S11_PACKET_FORWARDED;
    result.delay_us = queue_wait_us + own_transmission_us;

    if (bw_state != NULL)
    {
        bw_state->queued_packets++;
        bw_state->pending_delay_us += own_transmission_us;
    }

    return result;
}