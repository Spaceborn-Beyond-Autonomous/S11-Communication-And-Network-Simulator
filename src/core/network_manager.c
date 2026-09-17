#include "s11_network.h"
#include "s11_packet.h"
#include "s11_model.h"
#include "s11_link.h"
#include "s11_config.h"
#include "s11_logger.h"
#include "s11_monitor.h"

#include <stdlib.h>
#include <string.h>


static s11_bandwidth_state_t g_bandwidth_state =
{
    0U,
    0U
};

static const char *protocol_to_string(s11_protocol_t protocol);

static s11_process_result_t apply_models(
    const s11_packet_t *packet,
    const s11_link_model_config_t *config
);

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
    const s11_packet_t *packet)
{
    s11_process_result_t result = {
        .status = S11_PACKET_DROPPED,
        .delay_us = -1
    };

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

    logger_log_packet(packet->timestamp_us / 1000,
                    packet->source,
                    packet->destination,
                    protocol_to_string(packet->protocol),
                    LOG_STATUS_RX,
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

        logger_log_packet(packet->timestamp_us / 1000,
                  packet->source,
                  packet->destination,
                  protocol_to_string(packet->protocol),
                  LOG_STATUS_DROP,
                  -1,
                  "Link unavailable");

        return (result);
    }

    s11_link_model_config_t config;
    char link_type[S11_LINK_ID_MAX_LEN];
    s11_process_result_t model_result;

    if(link_manager_get_type( packet->source, packet->destination,
       link_type, sizeof(link_type)) != 0)
    {
        network->packets_dropped++;
        result.status = S11_PACKET_DROPPED;

        logger_log_packet(
            packet->timestamp_us / 1000,
            packet->source,
            packet->destination,
            protocol_to_string(packet->protocol),
            LOG_STATUS_DROP,
            -1,
            "Link type unavailable");

        return (result);
    }

    if(!s11_config_load(&config, "config/network.yaml", link_type))
    {
        network->packets_dropped++;
        result.status = S11_PACKET_DROPPED; 

        logger_log_packet(
            packet->timestamp_us / 1000,
            packet->source,
            packet->destination,
            protocol_to_string(packet->protocol),
            LOG_STATUS_DROP,
            -1,
            "Link configuration unavailable");

        return (result);
    }

    model_result = apply_models(packet, &config);

    if(model_result.status == S11_PACKET_DROPPED)
    {
        network->packets_dropped++;
        result = model_result;

        logger_log_packet(
            packet->timestamp_us / 1000,
            packet->source,
            packet->destination,
            protocol_to_string(packet->protocol),
            LOG_STATUS_DROP,
            (int)(result.delay_us / 1000),
            "Packet dropped by network model");

        return (result);
    }

    network->packets_transmitted++;

    result = model_result;

    logger_log_packet(
        packet->timestamp_us / 1000,
        packet->source,
        packet->destination,
        protocol_to_string(packet->protocol),
        LOG_STATUS_TX,
        (int)(result.delay_us / 1000),
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

static s11_process_result_t apply_models( const s11_packet_t *packet,
                                          const s11_link_model_config_t *config
)
{
    s11_process_result_t final_result;
    int64_t total_delay_us = 0;

    s11_model_apply_fn models[] =
    {
        packet_loss_apply,
        latency_apply,
        jitter_apply,
        bandwidth_apply
    };

    void *states[] =
    {
        NULL,
        NULL,
        NULL,
        &g_bandwidth_state
    };

    const size_t model_count = sizeof(models) / sizeof(models[0]);

    final_result.status = S11_PACKET_FORWARDED;
    final_result.delay_us = 0;

    for (size_t i = 0U; i < model_count; ++i)
    {
        s11_process_result_t result;

        result = models[i](
            packet,
            config,
            states[i]
        );

        if (result.status == S11_PACKET_DROPPED)
        {
            final_result.status = S11_PACKET_DROPPED;
            final_result.delay_us = total_delay_us;

            return final_result;
        }

        total_delay_us += result.delay_us;
    }

    final_result.status = S11_PACKET_FORWARDED;
    final_result.delay_us = total_delay_us;

    return final_result;
}