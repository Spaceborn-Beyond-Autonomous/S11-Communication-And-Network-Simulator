#include <stdio.h>
#include "network_monitor.h"
#include "s11_link.h"

static uint64_t g_tx_count = 0;
static uint64_t g_rx_count = 0;
static uint64_t g_drop_count = 0;
static long g_delay_sum_ms = 0;
static int g_delay_samples = 0;

static const char *link_state_to_string(LinkState state) {
    switch (state) {
        case LINK_STATE_UP:          return "UP";
        case LINK_STATE_DOWN:        return "DOWN";
        case LINK_STATE_DEGRADED:    return "DEGRADED";
        case LINK_STATE_PARTITIONED: return "PARTITIONED";
        case LINK_STATE_JAMMED:      return "JAMMED";
        default:                     return "UNKNOWN";
    }
}

void monitor_init(void) {
    g_tx_count = 0;
    g_rx_count = 0;
    g_drop_count = 0;
    g_delay_sum_ms = 0;
    g_delay_samples = 0;
}

void monitor_record_packet(LogStatus status, int delay_ms) {
    switch (status) {
        case LOG_STATUS_TX:
            g_tx_count++;
            break;
        case LOG_STATUS_RX:
            g_rx_count++;
            if (delay_ms >= 0) {
                g_delay_sum_ms += delay_ms;
                g_delay_samples++;
            }
            break;
        case LOG_STATUS_DROP:
            g_drop_count++;
            break;
    }
}

void monitor_print_snapshot(void) {
    double avg_latency_ms = (g_delay_samples > 0)
        ? ((double)g_delay_sum_ms / (double)g_delay_samples)
        : 0.0;

    uint64_t total_outcomes = g_rx_count + g_drop_count;
    double loss_rate_pct = (total_outcomes > 0)
        ? (100.0 * (double)g_drop_count / (double)total_outcomes)
        : 0.0;

    printf("---- S11 network monitor ----\n");
    printf("TX: %llu  RX: %llu  DROP: %llu\n",
           (unsigned long long)g_tx_count,
           (unsigned long long)g_rx_count,
           (unsigned long long)g_drop_count);
    printf("Avg latency: %.1fms   Loss rate: %.1f%%\n", avg_latency_ms, loss_rate_pct);

    printf("Links:\n");
    int count = link_manager_count();
    if (count == 0) {
        printf("  (none registered)\n");
    }
    for (int i = 0; i < count; i++) {
        const S11Link *link = link_manager_get_link_at(i);
        if (link != NULL) {
            printf("  %s <-> %s : %s\n", link->node_a, link->node_b, link_state_to_string(link->state));
        }
    }
    printf("------------------------------\n");
}