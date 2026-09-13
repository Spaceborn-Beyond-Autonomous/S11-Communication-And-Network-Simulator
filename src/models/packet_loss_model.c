#include "s11_network.h"
#include <stdlib.h> 

s11_process_result_t packet_loss_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state)
{
    (void)pkt;
    (void)state;

    s11_process_result_t result;
    result.delay_us = 0;   // packet loss never adds delay — you're right about that part

    double roll = (double)rand() / RAND_MAX;   // random value in [0.0, 1.0]

    if (roll < cfg->packet_loss_rate) {
        result.status = S11_PACKET_DROPPED; //packet is dropped only packe_loss_rate % 
    } else {
        result.status = S11_PACKET_FORWARDED;
    }

    return result;
}