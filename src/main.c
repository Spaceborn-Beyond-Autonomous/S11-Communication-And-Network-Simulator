#include <stdio.h>
#include <stdint.h>

#include "s11_network.h"
#include "s11_config.h"

int main(void)
{
    const uint8_t payload[] = "Hello S11";

    s11_network_config_t config;
    s11_packet_t *packet = NULL;
    s11_network_t *network = NULL;
    s11_process_result_t result;

    /*=========================
     * Configuration
    *=========================*/

    if (s11_config_init(&config) == false)
    {
        printf("TEST FAILED: configuration initialization Failed\n");
        return 1;
    }

    if (s11_config_validate(&config) == false)
    {
        printf("TEST FAILED: default configuration validation Faild\n");
        return 1;
    }

    printf("TEST PASSED: configuration initialized and validated Success\n");

    /*=========================
     * Packet Creation
    *=========================*/

    packet = s11_packet_create(
        "robot_01",
        "robot_02",
        S11_PROTOCOL_ROS2,
        payload,
        sizeof(payload) - 1U,
        1000000U
    );

    if (packet == NULL)
    {
        printf("TEST FAILED: valid packet creation Fail\n");
        return 1;
    }

    if (packet->packet_id == 0U)
    {
        printf("TEST FAILED: packet ID generation\n");
        s11_packet_destroy(packet);
        return 1;
    }

printf("TEST PASSED: packet ID generated successfully\n");

    printf("TEST PASSED: packet created and validated Pass\n");

    /*=========================
     * Network Creation
    *=========================*/

    network = s11_network_create("test_network");

    if (network == NULL)
    {
        printf("TEST FAILED: network creation Fail\n");
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: network created pass\n");

    /*=========================
     * Core Integration
    *=========================*/

    result = s11_network_process(network, packet);

    if ((result.status == S11_PACKET_FORWARDED) &&
        (network->packets_received == 1U) &&
        (network->packets_transmitted == 1U) &&
        (network->packets_dropped == 0U))
    {
        printf("TEST PASSED: packet successfully processed by S11\n");
    }
    else
    {
        printf("TEST FAILED: packet processing\n");

        s11_packet_destroy(packet);
        s11_network_destroy(network);

        return 1;
    }

    /*=========================
     * Failure Path
    *=========================*/

    s11_packet_t invalid_packet = {0};

    invalid_packet.packet_id = 1U;
    invalid_packet.source[0] = '\0';
    snprintf(invalid_packet.destination, S11_NODE_ID_MAX_LEN, "%s", "robot_02");
    invalid_packet.protocol = S11_PROTOCOL_ROS2;
    invalid_packet.payload = (uint8_t *)payload;
    invalid_packet.payload_size = sizeof(payload) - 1U;
    invalid_packet.timestamp_us = 1000000U;

    result = s11_network_process(network, &invalid_packet);

    if ((result.status == S11_PACKET_DROPPED) &&
        (network->packets_dropped == 1U))
    {
        printf("TEST PASSED: invalid packet rejected and dropped\n");
    }
    else
    {
        printf("TEST FAILED: invalid packet handling\n");

        s11_packet_destroy(packet);
        s11_network_destroy(network);

        return 1;
    }

    /*=========================
     * Cleanup
    *=========================*/

    s11_packet_destroy(packet);
    s11_network_destroy(network);

    printf("\nAll Day 2 core integration tests passed.\n");

    return 0;
}