/*
 * Deterministic tests for the four degradation models (Day 2/3).
 *
 * Each model returns its own INCREMENTAL delay_us only (not a running
 * total) and a forward/drop status. network_manager.c is responsible
 * for summing delay_us across the chain and stopping early on a drop -
 * these tests exercise each model in isolation, not combined.
 *
 * NOTE on rand()/srand(): the exact sequence produced by a given seed
 * is only guaranteed repeatable on the SAME libc. The seed=42 baseline
 * below was recorded on glibc (Linux) - if this is ever run on a
 * different C library, that specific assertion may need re-recording.
 */
#include "s11_network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern s11_process_result_t latency_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state);
extern s11_process_result_t packet_loss_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state);
extern s11_process_result_t jitter_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state);
extern s11_process_result_t bandwidth_apply(const s11_packet_t *pkt, const s11_link_model_config_t *cfg, void *state);

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(desc, cond) do { \
    if (cond) { g_pass++; printf("  [PASS] %s\n", desc); } \
    else      { g_fail++; printf("  [FAIL] %s\n", desc); } \
} while (0)

static s11_link_model_config_t make_cfg(void) {
    s11_link_model_config_t cfg;
    /* Zero every field first so unset fields (e.g. bandwidth_bps in the
     * latency test) are a known 0, not leftover garbage from the stack. */
    memset(&cfg, 0, sizeof(cfg));
    /* strncpy, not strcpy: caps the copy at (buffer size - 1) bytes so a
     * longer source string can never overflow the fixed-size link_type
     * array. The "- 1" reserves the last byte for the '\0' terminator. */
    strncpy(cfg.link_type, "test", sizeof(cfg.link_type) - 1);
    return cfg;
}

static s11_packet_t make_packet(uint32_t payload_size) {
    s11_packet_t pkt;
    /* Same reasoning as make_cfg: zero the whole struct first (packet_id,
     * source, destination, protocol, payload, timestamp_us all become a
     * known 0/NULL) then set only the one field this test actually needs.
     * payload_size lives on the packet (this specific message's size),
     * not on the config (which only describes the link in general). */
    memset(&pkt, 0, sizeof(pkt));
    pkt.payload_size = payload_size;
    return pkt;
}

/* ---------------- Latency ---------------- */
static void test_latency(void) {
    printf("\n-- latency_apply --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(100);

    cfg.base_latency_ms = 30.0;
    s11_process_result_t r1 = latency_apply(&pkt, &cfg, NULL);
    CHECK("base 30ms -> 30000us", r1.delay_us == 30000);
    CHECK("never drops",         r1.status == S11_PACKET_FORWARDED);

    cfg.base_latency_ms = 0.0;
    s11_process_result_t r2 = latency_apply(&pkt, &cfg, NULL);
    CHECK("base 0ms -> 0us", r2.delay_us == 0);
}

/* ---------------- Packet loss ---------------- */
static void test_packet_loss(void) {
    printf("\n-- packet_loss_apply --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(100);
    const int trials = 1000;

    /* Edge case: 0% loss -> never drops */
    cfg.packet_loss_rate = 0.0;
    srand(1);
    int drops = 0;
    for (int i = 0; i < trials; i++) {
        if (packet_loss_apply(&pkt, &cfg, NULL).status == S11_PACKET_DROPPED) drops++;
    }
    CHECK("rate 0.0 -> 0 drops in 1000 trials", drops == 0);

    /* Edge case: 100% loss -> always drops */
    cfg.packet_loss_rate = 1.0;
    srand(1);
    drops = 0;
    for (int i = 0; i < trials; i++) {
        if (packet_loss_apply(&pkt, &cfg, NULL).status == S11_PACKET_DROPPED) drops++;
    }
    CHECK("rate 1.0 -> 1000 drops in 1000 trials", drops == trials);

    /* Fixed seed, mid-range rate -> repeatable count (regression baseline) */
    cfg.packet_loss_rate = 0.5;
    srand(42);
    drops = 0;
    for (int i = 0; i < trials; i++) {
        if (packet_loss_apply(&pkt, &cfg, NULL).status == S11_PACKET_DROPPED) drops++;
    }
    CHECK("seed=42, rate=0.5 -> 517/1000 drops (regression baseline)", drops == 517);
    CHECK("delay_us always 0", packet_loss_apply(&pkt, &cfg, NULL).delay_us == 0);
}

/* ---------------- Jitter ---------------- */
static void test_jitter(void) {
    printf("\n-- jitter_apply --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(100);
    cfg.jitter_min_ms = 5.0;
    cfg.jitter_max_ms = 20.0;

    srand(7);
    int in_range = 1;
    for (int i = 0; i < 1000; i++) {
        s11_process_result_t r = jitter_apply(&pkt, &cfg, NULL);
        if (r.delay_us < 5000 || r.delay_us > 20000) { in_range = 0; break; }
        if (r.status != S11_PACKET_FORWARDED) { in_range = 0; break; }
    }
    CHECK("1000 samples all within [5000,20000]us and forwarded", in_range);

    /* Degenerate range: min == max -> always exact value */
    cfg.jitter_min_ms = 10.0;
    cfg.jitter_max_ms = 10.0;
    s11_process_result_t r = jitter_apply(&pkt, &cfg, NULL);
    CHECK("min==max==10ms -> exactly 10000us", r.delay_us == 10000);
}

/* ---------------- Bandwidth ---------------- */
static void test_bandwidth(void) {
    printf("\n-- bandwidth_apply --\n");
    s11_link_model_config_t cfg = make_cfg();
    cfg.bandwidth_bps = 1000000.0; /* 1 Mbps */

    s11_packet_t pkt1 = make_packet(1000); /* 1000 bytes = 8000 bits */
    s11_process_result_t r1 = bandwidth_apply(&pkt1, &cfg, NULL);
    CHECK("1000B @ 1Mbps -> 8000us", r1.delay_us == 8000);
    CHECK("never drops (queue logic not yet implemented)", r1.status == S11_PACKET_FORWARDED);

    s11_packet_t pkt0 = make_packet(0);
    s11_process_result_t r0 = bandwidth_apply(&pkt0, &cfg, NULL);
    CHECK("0-byte payload -> 0us", r0.delay_us == 0);
}

int main(void) {
    test_latency();
    test_packet_loss();
    test_jitter();
    test_bandwidth();

    printf("\n==================\n");
    printf("PASS: %d  FAIL: %d\n", g_pass, g_fail);
    printf("==================\n");

    return g_fail == 0 ? 0 : 1;
}
