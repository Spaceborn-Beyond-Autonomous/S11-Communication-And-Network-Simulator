#include "s11_network.h"

#include <stdlib.h>
#include <string.h>

s11_network_t *s11_network_create(const char *network_id)
{
    s11_network_t *network;

    if( (network_id == NULL ) ||
        (strlen(network_id) >= S11_NETWORK_ID_MAX_LEN) )
    {
        return (NULL);
    }

    network= malloc(sizeof(s11_network_t ));
    if(network == NULL)
    {
        return (NULL);
    }

    strcpy(network->network_id , network_id);
    network->packets_transmitted = 0U;
    network->packets_received    = 0U;
    network->packets_dropped     = 0U;

    return (network);
}

void s11_network_destroy(s11_network_t *network)
{
    if(network == NULL)
    {
        return;
    }

    free(network);
}

s11_process_result_t s11_network_process(
    s11_network_t *network,
    const s11_packet_t *packet
)
{
    s11_process_result_t  result = {0};

    if((network == NULL) || (packet == NULL))
    {
        result.status = S11_PACKET_DROPPED;
        return (result);
    }

    if(!s11_packet_validate(packet))
    {
        network->packets_dropped++;
        result.status = S11_PACKET_DROPPED;
        return (result);
    }

    network->packets_received++;

    /*
     * TODO:
     * Check link state.
     * Apply packet-loss model.
     * Calculate latency and jitter.
     * Check bandwidth.
    */

    network->packets_transmitted++;
    result.status = S11_PACKET_FORWARDED ;
    result.delay_us = 0U;

    return (result);
}