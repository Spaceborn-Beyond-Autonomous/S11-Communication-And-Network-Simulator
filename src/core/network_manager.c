#include "s11_network.h"
#include "s11_link.h"
#include "s11_logger.h"

#include <stdlib.h>
#include <string.h>

static const char *protocol_to_string(s11_protocol_t protocol);

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

    if(!s11_config_init(&network->config))
    {
        free(network);
        return NULL;
    }

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
    const s11_packet_t *packet)
{
    s11_process_result_t result = {
        .status = S11_PACKET_DROPPED,
        .delay_us = -1
    };
    LogStatus log_state;

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
    log_state = LOG_STATUS_RX;

    logger_log_packet(packet->timestamp_us / 1000,
                    packet->source,
                    packet->destination,
                    protocol_to_string(packet->protocol),
                    log_state,
                    -1,
                    "Packet received");


    LinkState link_state;
    link_state = link_manager_get_state(packet->source, packet->destination);

    if((LINK_STATE_DOWN == link_state) ||
       (LINK_STATE_PARTITIONED == link_state) ||
       (LINK_STATE_JAMMED == link_state))
    {
        network->packets_dropped++;
        result.status = S11_PACKET_DROPPED;
        log_state = LOG_STATUS_DROP;

        logger_log_packet(packet->timestamp_us / 1000,
                  packet->source,
                  packet->destination,
                  protocol_to_string(packet->protocol),
                  log_state,
                  -1,
                  "Link unavailable");

        return (result);
    }

    /*
     * TODO:
     * Apply packet-loss model.
     * Calculate latency and jitter.
     * Check bandwidth.
    */

    network->packets_transmitted++;
    result.status = S11_PACKET_FORWARDED ;
    log_state = LOG_STATUS_TX;
    result.delay_us = 0U;

    logger_log_packet(packet->timestamp_us/ 1000,
                      packet->source,
                      packet->destination,
                      protocol_to_string(packet->protocol),
                      log_state,
                      result.delay_us/ 1000,
                      "Packet forwarded");


    return (result);
}

static const char *protocol_to_string(s11_protocol_t protocol)
{
    switch(protocol)
    {
        case S11_PROTOCOL_ROS2:
            return "ROS2";

        case S11_PROTOCOL_DDS:
            return "DDS";

        case S11_PROTOCOL_MAVLINK:
            return "MAVLink";

        case S11_PROTOCOL_UNKNOWN:
        default:
            return "UNKNOWN";
    }
}