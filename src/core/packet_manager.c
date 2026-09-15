#include "s11_packet.h"
#include <stdlib.h>
#include <string.h>

static uint64_t next_packet_id = 1U;

s11_packet_t *s11_packet_create(const char *source,
                                const char *destination,
                                s11_protocol_t protocol,
                                const uint8_t *payload,
                                uint32_t payload_size,
                                uint64_t timestamp_us)
{
    s11_packet_t packet = {0};
    s11_packet_t *packet_ptr = NULL;

    if((source == NULL) || (destination == NULL))
    {
        return NULL;
    }

    if((payload_size > 0U) && (payload == NULL))
    {
        return NULL;
    }

    if((strlen(source) >= S11_NODE_ID_MAX_LEN )|| 
       (strlen(destination) >= S11_NODE_ID_MAX_LEN))
    {
        return NULL;
    }

    strcpy(packet.source, source);
    strcpy(packet.destination, destination);

    packet.protocol = protocol;
    packet.payload_size = payload_size;
    packet.timestamp_us = timestamp_us;
    packet.payload = payload;

    if(s11_packet_validate(&packet) == false)
    {
        return NULL;
    }

    packet_ptr = malloc(sizeof(s11_packet_t));

    if(packet_ptr == NULL)
    {
        return NULL;
    }

    *packet_ptr = packet;
    packet_ptr->packet_id = next_packet_id++;

    if(payload_size > 0U)
    {
        packet_ptr->payload = malloc(payload_size);

        if(packet_ptr->payload == NULL)
        {
            free(packet_ptr);
            return NULL;
        }

        memcpy(packet_ptr->payload, payload, payload_size);
    }

    return packet_ptr;
}


void s11_packet_destroy(s11_packet_t *packet)
{
   if (packet == NULL)
    {
        return;
    }

    free(packet->payload);
    free(packet);
}

bool s11_packet_validate(const s11_packet_t *packet)
{
    if(packet == NULL)
    {
        return (false);
    }

    if((packet->source[0] == '\0' ) || 
        (memchr(packet->source,'\0', S11_NODE_ID_MAX_LEN) == NULL))
    {
        return (false);
    }

    if(( packet->destination[0] == '\0' ) || 
        (memchr(packet->destination,'\0', S11_NODE_ID_MAX_LEN) == NULL))
    {
        return(false);
    }

    switch(packet->protocol)
    {
        case S11_PROTOCOL_ROS2:
        case S11_PROTOCOL_DDS:
        case S11_PROTOCOL_MAVLINK:
            break;
        
        case S11_PROTOCOL_UNKNOWN:
        default:
            return(false);
    }

    if((packet->payload_size > 0U) && (packet->payload == NULL))
    {
        return (false);
    }

    return true;
}