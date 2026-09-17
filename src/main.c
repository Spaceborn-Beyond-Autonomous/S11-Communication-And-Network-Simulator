#include <stdio.h>
#include <string.h>

#include "s11_config.h"
#include "s11_link.h"
#include "s11_network.h"
#include "s11_packet.h"
#include "s11_protocol.h"

int main(void)
{
    s11_link_model_config_t config;
    s11_packet_t *packet;
    s11_network_t *network;
    s11_process_result_t result;

    /*
     * Initialize link manager.
     */
    link_manager_init();

    /*
     * Load LTE network configuration.
     */
    if (!s11_config_load(&config, "config/network.yaml", "lte"))
    {
        printf("TEST FAILED: LTE configuration could not be loaded\n");
        return 1;
    }

    printf("\n========== Network Configuration ==========\n");
    printf("Link Type        : %s\n", config.link_type);
    printf("Base Latency     : %.2f ms\n", config.base_latency_ms);
    printf("Jitter Min       : %.2f ms\n", config.jitter_min_ms);
    printf("Jitter Max       : %.2f ms\n", config.jitter_max_ms);
    printf("Packet Loss Rate : %.4f\n", config.packet_loss_rate);
    printf("Bandwidth        : %.2f bps\n", config.bandwidth_bps);
    printf("Queue Capacity   : %u\n", config.queue_capacity);
    printf("===========================================\n");

    printf("TEST PASSED: LTE configuration loaded successfully\n");

    /*
     * Create a test packet.
     */
    const char payload[] = "Hello S11";

    packet = s11_packet_create(
        "robot_01",
        "robot_02",
        S11_PROTOCOL_ROS2,
        payload,
        (uint32_t)strlen(payload),
        1000000U
    );

    if (packet == NULL)
    {
        printf("TEST FAILED: packet creation failed\n");
        return 1;
    }

    printf("TEST PASSED: packet created successfully\n");

    if (packet->packet_id == 0U)
    {
        printf("TEST FAILED: packet ID was not generated\n");
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: packet ID generated successfully\n");

    /*
     * Create the simulated network.
     */
    network = s11_network_create("test_network");

    if (network == NULL)
    {
        printf("TEST FAILED: network creation failed\n");
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: network created successfully\n");

    /*
     * Create an LTE link between the two test nodes.
     */
    if (link_manager_add_link(
            "robot_01",
            "robot_02",
            "lte",
            LINK_STATE_UP) != 0)
    {
        printf("TEST FAILED: link creation failed\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: link created successfully\n");

    /*
     * Process the packet through the complete S11 network pipeline.
     */
    result = s11_network_process(network, packet);

    if (result.status != S11_PACKET_FORWARDED)
    {
        printf("TEST FAILED: packet was not forwarded\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    if (network->packets_received != 1U)
    {
        printf("TEST FAILED: unexpected received packet count\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    if (network->packets_transmitted != 1U)
    {
        printf("TEST FAILED: unexpected transmitted packet count\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    if (network->packets_dropped != 0U)
    {
        printf("TEST FAILED: unexpected dropped packet count\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: packet successfully processed by S11\n");
    printf("TEST INFO: total network delay = %lld us\n",
           (long long)result.delay_us);

    /*
     * Test packet validation with an invalid source.
     */
    packet->source[0] = '\0';

    result = s11_network_process(network, packet);

    if (result.status != S11_PACKET_DROPPED)
    {
        printf("TEST FAILED: invalid packet was not rejected\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    if (network->packets_dropped != 1U)
    {
        printf("TEST FAILED: invalid packet was not counted as dropped\n");
        s11_network_destroy(network);
        s11_packet_destroy(packet);
        return 1;
    }

    printf("TEST PASSED: invalid packet rejected and dropped\n");

    /*
     * Cleanup.
     */
    s11_network_destroy(network);
    s11_packet_destroy(packet);

    printf("\nAll current core integration tests passed.\n");

    return 0;
}