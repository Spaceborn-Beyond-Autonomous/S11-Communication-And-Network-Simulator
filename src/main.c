#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "s11_config.h"
#include "s11_dashboard.h"
#include "s11_events.h"
#include "s11_link.h"
#include "s11_logger.h"
#include "s11_network.h"
#include "s11_packet.h"
#include "s11_protocol.h"
#include "s11_monitor.h"

static void usage(const char *name)
{
    printf("Usage: %s [--dashboard] [--scenario NAME] [--packets N] [--link TYPE]\n", name);
    printf("Scenarios: normal, high_latency, high_packet_loss, unstable, low_bandwidth, partition, recovery, jamming, all\n");
    printf("Link types are loaded from config/network.yaml (lte, 5g, mesh, lora, satellite).\n");
}

static void event(uint64_t ts, s11_event_type_t type, const char *a, const char *b)
{
    s11_event_t e = {0};
    e.timestamp_us = ts;
    e.type = type;
    snprintf(e.node_a, sizeof(e.node_a), "%s", a);
    snprintf(e.node_b, sizeof(e.node_b), "%s", b);
    logger_log_event(&e);
}

static s11_link_model_config_t scenario_config(const s11_link_model_config_t *base, const char *scenario)
{
    s11_link_model_config_t c = *base;
    if (strcmp(scenario, "high_latency") == 0) c.base_latency_ms *= 4.0;
    else if (strcmp(scenario, "high_packet_loss") == 0) c.packet_loss_rate = 0.50;
    else if (strcmp(scenario, "unstable") == 0) { c.packet_loss_rate = 0.25; c.jitter_min_ms *= 2.0; c.jitter_max_ms *= 4.0; }
    else if (strcmp(scenario, "low_bandwidth") == 0) c.bandwidth_bps = c.bandwidth_bps / 20.0;
    return c;
}

static int run_packet(s11_network_t *network, uint64_t id, uint64_t ts,
                      s11_protocol_t protocol, const char *scenario,
                      const s11_link_model_config_t *base_config)
{
    char payload[96];
    snprintf(payload, sizeof(payload), "S11 %s traffic packet %llu", scenario, (unsigned long long)id);
    s11_packet_t *p = s11_packet_create("robot_01", "robot_02", protocol,
                                         (const unsigned char *)payload,
                                         (uint32_t)strlen(payload), ts);
    if (!p) return 1;

    /* The selected scenario is reflected by temporarily applying its parameters
       through the same config file contract used by the network pipeline. */
    s11_link_model_config_t effective = scenario_config(base_config, scenario);
    s11_network_set_runtime_config(&effective);
    s11_process_result_t r = s11_network_process(network, p);
    printf("  packet #%03llu  %-9s  delay=%7.2f ms\n",
           (unsigned long long)id,
           r.status == S11_PACKET_FORWARDED ? "FORWARDED" : "DROPPED",
           (double)r.delay_us / 1000.0);
    s11_packet_destroy(p);
    return 0;
}

int main(int argc, char **argv)
{
    const char *scenario = "all";
    const char *link_type = "lte";
    int dashboard = 1;
    int packets = 2;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) { usage(argv[0]); return 0; }
        if (strcmp(argv[i], "--dashboard") == 0) { dashboard = 1; continue; }
        if (strcmp(argv[i], "--no-dashboard") == 0) { dashboard = 0; continue; }
        if (strcmp(argv[i], "--scenario") == 0 && i + 1 < argc) { scenario = argv[++i]; continue; }
        if (strcmp(argv[i], "--packets") == 0 && i + 1 < argc) { packets = atoi(argv[++i]); if (packets < 1) packets = 1; continue; }
        if (strcmp(argv[i], "--link") == 0 && i + 1 < argc) { link_type = argv[++i]; continue; }
        fprintf(stderr, "Unknown argument: %s\n", argv[i]); usage(argv[0]); return 2;
    }

    srand(7U);
    link_manager_init();
    monitor_init();

    s11_link_model_config_t config;
    s11_link_model_config_t dashboard_config;
    if (!s11_config_load(&config, "config/network.yaml", link_type)) {
        fprintf(stderr, "Failed to load config/network.yaml for link '%s'\n", link_type);
        return 1;
    }
    if (link_manager_add_link("robot_01", "robot_02", link_type, LINK_STATE_UP) != 0) {
        fprintf(stderr, "Failed to create simulation link\n");
        return 1;
    }

    s11_network_t *network = s11_network_create("S11-Simulation");
    if (!network) return 1;

    printf("\nS11 Communication & Network Simulator\n");
    printf("Config : config/network.yaml\nLink   : %s\n\n", link_type);

    dashboard_config = config;

    uint64_t ts = 1000000U;
    uint64_t id = 1U;

    const char *scenarios[] = {"normal", "high_latency", "high_packet_loss", "unstable", "low_bandwidth", "partition", "recovery", "jamming"};
    size_t scenario_count = sizeof(scenarios) / sizeof(scenarios[0]);

    for (size_t si = 0; si < scenario_count; ++si) {
        const char *current = scenarios[si];
        if (strcmp(scenario, "all") != 0 && strcmp(scenario, current) != 0) continue;

        printf("[%s]\n", current);
        if (strcmp(current, "partition") == 0) {
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_PARTITIONED, ts);
            event(ts, S11_EVENT_PARTITION_CREATED, "robot_01", "robot_02");
        } else if (strcmp(current, "recovery") == 0) {
            /* A standalone recovery scenario first creates the failure, then
             * performs the recovery so the scenario is meaningful by itself. */
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_PARTITIONED, ts);
            event(ts, S11_EVENT_PARTITION_CREATED, "robot_01", "robot_02");
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_UP, ts + 100000U);
            event(ts + 100000U, S11_EVENT_PARTITION_RECOVERED, "robot_01", "robot_02");
            event(ts + 200000U, S11_EVENT_RECOVERY_COMPLETE, "robot_01", "robot_02");
        } else if (strcmp(current, "jamming") == 0) {
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_JAMMED, ts);
            event(ts, S11_EVENT_JAMMING_STARTED, "robot_01", "robot_02");
        } else {
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_UP, ts);
        }

        dashboard_config = scenario_config(&config, current);
        int count = packets;
        if (strcmp(current, "partition") == 0 || strcmp(current, "jamming") == 0) count = 1;
        for (int n = 0; n < count; ++n) {
            run_packet(network, id++, ts + 100000U, S11_PROTOCOL_ROS2, current, &config);
            ts += 1000000U;
        }

        if (strcmp(current, "partition") == 0) {
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_UP, ts);
            event(ts, S11_EVENT_PARTITION_RECOVERED, "robot_01", "robot_02");
        }
        if (strcmp(current, "jamming") == 0) {
            link_manager_set_state("robot_01", "robot_02", LINK_STATE_UP, ts);
            event(ts, S11_EVENT_JAMMING_STOPPED, "robot_01", "robot_02");
            event(ts + 100000U, S11_EVENT_RECOVERY_COMPLETE, "robot_01", "robot_02");
        }
    }

    if (strcmp(scenario, "all") != 0) {
        /* For a single degradation scenario the network manager uses the actual
           configured link profile. The dashboard reports the same source values. */
        printf("\nScenario run complete: %s\n", scenario);
    }

    if (dashboard) s11_dashboard_print(network, &dashboard_config, scenario, ts);
    else monitor_print_snapshot();

    printf("Detailed runtime log: logs/s11_network.log\n");
    s11_network_destroy(network);
    return 0;
}
