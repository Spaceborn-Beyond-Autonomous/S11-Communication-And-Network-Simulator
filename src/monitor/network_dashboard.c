#include <stdio.h>
#include <string.h>
#include "s11_dashboard.h"

static const char *state_name(LinkState state)
{
    switch (state) {
        case LINK_STATE_UP: return "UP";
        case LINK_STATE_DOWN: return "DOWN";
        case LINK_STATE_DEGRADED: return "DEGRADED";
        case LINK_STATE_PARTITIONED: return "PARTITIONED";
        case LINK_STATE_JAMMED: return "JAMMED";
        default: return "UNKNOWN";
    }
}

static void bar(uint64_t value, uint64_t max_value)
{
    const int width = 30;
    int filled = (max_value == 0U) ? 0 : (int)((value * (uint64_t)width) / max_value);
    if (filled > width) filled = width;
    for (int i = 0; i < filled; ++i) putchar('#');
    for (int i = filled; i < width; ++i) putchar('-');
}

void s11_dashboard_print(const s11_network_t *network,
                         const s11_link_model_config_t *config,
                         const char *scenario,
                         uint64_t simulated_time_us)
{
    if (network == NULL || config == NULL) return;

    const uint64_t outcomes = network->packets_received + network->packets_dropped;
    const double loss = outcomes ? (100.0 * (double)network->packets_dropped / (double)outcomes) : 0.0;
    const double avg = network->packets_transmitted ?
        ((double)network->total_delay_us / (double)network->packets_transmitted / 1000.0) : 0.0;
    const uint64_t maxv = network->packets_received > network->packets_transmitted ?
        (network->packets_received > network->packets_dropped ? network->packets_received : network->packets_dropped) :
        (network->packets_transmitted > network->packets_dropped ? network->packets_transmitted : network->packets_dropped);

    printf("\n+--------------------------------------------------------------------+\n");
    printf("|                    S11 NETWORK DASHBOARD                          |\n");
    printf("+--------------------------------------------------------------------+\n");
    printf("| NETWORK : %-20s SCENARIO : %-17s |\n", network->network_id, scenario ? scenario : "runtime");

    int count = link_manager_count();
    for (int i = 0; i < count; ++i) {
        const S11Link *link = link_manager_get_link_at(i);
        if (link == NULL) continue;
        printf("| LINK    : %-12s <------------------------------> %-12s |\n", link->node_a, link->node_b);
        printf("| TYPE    : %-12s STATE : %-14s                         |\n", link->link_type, state_name(link->state));
    }
    printf("| SIM TIME: %10.3f ms     BASE LATENCY: %8.2f ms               |\n",
           (double)simulated_time_us / 1000.0, config->base_latency_ms);
    printf("| JITTER  : %6.2f .. %6.2f ms     LOSS: %7.3f %%                  |\n",
           config->jitter_min_ms, config->jitter_max_ms, config->packet_loss_rate * 100.0);
    printf("| BANDWIDTH: %12.0f bps     QUEUE: %-6u                       |\n",
           config->bandwidth_bps, config->queue_capacity);
    printf("+--------------------------------------------------------------------+\n");
    printf("| PACKET STATISTICS                                                  |\n");
    printf("| RX   "); bar(network->packets_received, maxv); printf(" %6llu\n", (unsigned long long)network->packets_received);
    printf("| TX   "); bar(network->packets_transmitted, maxv); printf(" %6llu\n", (unsigned long long)network->packets_transmitted);
    printf("| DROP "); bar(network->packets_dropped, maxv); printf(" %6llu\n", (unsigned long long)network->packets_dropped);
    printf("|                                                                    |\n");
    printf("| AVG FORWARD DELAY : %10.3f ms     LOSS RATE : %8.2f %%         |\n", avg, loss);
    printf("| TOTAL DELAY       : %10llu us                                 |\n", (unsigned long long)network->total_delay_us);
    printf("+--------------------------------------------------------------------+\n");
}
