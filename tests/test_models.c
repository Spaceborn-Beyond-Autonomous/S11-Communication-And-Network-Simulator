/*
 * Deterministic tests for the four degradation models (Day 2/3/4).
 *
 * Each model returns its own INCREMENTAL delay_us only (not a running
 * total) and a forward/drop status. network_manager.c is responsible
 * for summing delay_us across the chain and stopping early on a drop.
 * The per-model tests below exercise each model in isolation; the
 * combined-conditions and stress tests exercise the chain locally
 * (via run_chain) since network_manager.c isn't wired up yet.
 *
 * NOTE on rand()/srand(): the exact sequence produced by a given seed
 * is only guaranteed repeatable on the SAME libc. The seed=42 baseline
 * below was recorded on glibc (Linux) - if this is ever run on a
 * different C library, that specific assertion may need re-recording.
 * The new seeds used below (3, 11, 23) have NOT been verified against
 * an exact count - they're used only with tolerance bands, so they
 * don't carry the same portability risk as the seed=42 exact-count test.
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

/* Runs all four models in sequence on one packet, summing each model's
 * incremental delay_us and stopping at the first S11_PACKET_DROPPED -
 * this mirrors the chaining logic network_manager.c is expected to do.
 * "Survived" is a logical AND across models: any single drop drops the
 * whole packet, and no delay from a model after the drop point is ever
 * counted. */
static s11_process_result_t run_chain(const s11_packet_t *pkt, const s11_link_model_config_t *cfg) {
    s11_model_apply_fn models[4] = { latency_apply, packet_loss_apply, jitter_apply, bandwidth_apply };
    s11_process_result_t total;
    memset(&total, 0, sizeof(total));
    total.status = S11_PACKET_FORWARDED;

    for (int i = 0; i < 4; i++) {
        s11_process_result_t r = models[i](pkt, cfg, NULL);
        if (r.status == S11_PACKET_DROPPED) {
            total.status = S11_PACKET_DROPPED;
            return total;
        }
        total.delay_us += r.delay_us;
    }
    return total;
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

/* ---------------- Bandwidth (base transmission-time math, state=NULL) ----------------
 * These calls pass NULL for state, which per bandwidth_apply's contract
 * means "no queue tracking" - only the base transmission-time
 * calculation runs, and the queue-full/accumulation branches are
 * skipped entirely. See test_bandwidth_queue() below for coverage of
 * the actual queue logic. */
static void test_bandwidth(void) {
    printf("\n-- bandwidth_apply (state=NULL, transmission time only) --\n");
    s11_link_model_config_t cfg = make_cfg();
    cfg.bandwidth_bps = 1000000.0; /* 1 Mbps */

    s11_packet_t pkt1 = make_packet(1000); /* 1000 bytes = 8000 bits */
    s11_process_result_t r1 = bandwidth_apply(&pkt1, &cfg, NULL);
    CHECK("1000B @ 1Mbps -> 8000us", r1.delay_us == 8000);
    CHECK("never drops with state=NULL (queue check skipped)", r1.status == S11_PACKET_FORWARDED);

    s11_packet_t pkt0 = make_packet(0);
    s11_process_result_t r0 = bandwidth_apply(&pkt0, &cfg, NULL);
    CHECK("0-byte payload -> 0us", r0.delay_us == 0);
}

/* ---------------- Bandwidth queue logic (state != NULL) ----------------
 * Exercises the two branches that state=NULL skips: the queue-full
 * check (drop when queued_packets has reached cfg.queue_capacity) and
 * the pending-delay accumulation (each accepted packet adds its own
 * transmission time onto state->pending_delay_us, and delay_us returned
 * is cumulative, not just this packet's own transmission time).
 *
 * ASSUMPTION: s11_bandwidth_state_t (declared in s11_link.h, not
 * re-pasted into this doc) exposes at least:
 *   uint32_t queued_packets;      // packets currently queued
 *   uint64_t pending_delay_us;    // accumulated queued transmission time
 * and s11_link_model_config_t exposes:
 *   uint32_t queue_capacity;      // max queued_packets before drop
 * If the actual field names differ, only the struct-literal
 * initializers below need updating - the test logic/assertions hold. */
static void test_bandwidth_queue(void) {
    printf("\n-- bandwidth_apply (state != NULL, queue logic) --\n");
    s11_link_model_config_t cfg = make_cfg();
    cfg.bandwidth_bps    = 1000000.0; /* 1 Mbps -> 1000B takes 8000us */
    cfg.queue_capacity   = 3;

    /* Case 1: queue-full drop. State already at capacity before this
     * call, so bandwidth_apply must reject the packet outright: status
     * DROPPED and delay_us == 0 (no transmission time is charged for a
     * packet that never gets queued). */
    {
        s11_bandwidth_state_t full_state;
        memset(&full_state, 0, sizeof(full_state));
        full_state.queued_packets = cfg.queue_capacity;

        s11_packet_t pkt = make_packet(1000);
        s11_process_result_t r = bandwidth_apply(&pkt, &cfg, &full_state);
        CHECK("queue at capacity -> status DROPPED", r.status == S11_PACKET_DROPPED);
        CHECK("queue at capacity -> delay_us == 0",  r.delay_us == 0);
    }

    /* Case 2: cumulative wait time. Same state pointer reused across
     * three successive calls (space available each time, since
     * queue_capacity=3 and we never exceed 2 queued packets here).
     * Each call's delay_us must strictly increase, proving
     * pending_delay_us is being carried forward and added to, not
     * recomputed fresh as just this packet's own transmission time. */
    {
        s11_bandwidth_state_t state;
        memset(&state, 0, sizeof(state));
        s11_packet_t pkt = make_packet(1000); /* 8000us each, 1 Mbps */

        s11_process_result_t r1 = bandwidth_apply(&pkt, &cfg, &state);
        s11_process_result_t r2 = bandwidth_apply(&pkt, &cfg, &state);
        s11_process_result_t r3 = bandwidth_apply(&pkt, &cfg, &state);

        CHECK("call 1 forwarded", r1.status == S11_PACKET_FORWARDED);
        CHECK("call 2 forwarded", r2.status == S11_PACKET_FORWARDED);
        CHECK("call 3 forwarded", r3.status == S11_PACKET_FORWARDED);
        CHECK("call 2 delay_us > call 1 delay_us (accumulating, not fresh)",
              r2.delay_us > r1.delay_us);
        CHECK("call 3 delay_us > call 2 delay_us (accumulating, not fresh)",
              r3.delay_us > r2.delay_us);
    }

    /* Case 3: state mutation after a successful call. queued_packets
     * must increment by exactly one, and pending_delay_us must grow by
     * exactly this packet's own transmission time (8000us @ 1Mbps for
     * a 1000B packet) - not by some other amount, and not left
     * unchanged. */
    {
        s11_bandwidth_state_t state;
        memset(&state, 0, sizeof(state));
        s11_packet_t pkt = make_packet(1000);

        uint32_t queued_before  = state.queued_packets;
        uint64_t pending_before = state.pending_delay_us;

        s11_process_result_t r = bandwidth_apply(&pkt, &cfg, &state);

        CHECK("successful call forwards", r.status == S11_PACKET_FORWARDED);
        CHECK("queued_packets incremented by exactly 1",
              state.queued_packets == queued_before + 1);
        CHECK("pending_delay_us grew by exactly this packet's own transmission time (8000us)",
              state.pending_delay_us == pending_before + 8000);
    }
}

/* ---------------- Day 3: combined conditions (chained) ---------------- */
static void test_combined_conditions(void) {
    printf("\n-- combined conditions (chain: latency -> loss -> jitter -> bandwidth) --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(1000);

    /* Case A: every model forwards. Expected delay = latency(50000us,
     * fixed) + bandwidth(1000B@20Mbps=400us, fixed) + jitter(random in
     * [5000,20000]us) + packet_loss(0us, always). Range: [55400,70400]. */
    cfg.base_latency_ms  = 50.0;
    cfg.jitter_min_ms    = 5.0;
    cfg.jitter_max_ms    = 20.0;
    cfg.packet_loss_rate = 0.0;
    cfg.bandwidth_bps    = 20000000.0;

    srand(3);
    s11_process_result_t rA = run_chain(&pkt, &cfg);
    CHECK("case A: all forward -> status FORWARDED", rA.status == S11_PACKET_FORWARDED);
    CHECK("case A: total delay within [55400,70400]us",
          rA.delay_us >= 55400 && rA.delay_us <= 70400);

    /* Case B: guaranteed drop at the packet_loss stage. Only latency's
     * delay (50000us, first in the chain) should be counted - jitter
     * and bandwidth must never run, proving the short-circuit. */
    cfg.packet_loss_rate = 1.0;
    srand(3);
    s11_process_result_t rB = run_chain(&pkt, &cfg);
    CHECK("case B: guaranteed loss -> status DROPPED", rB.status == S11_PACKET_DROPPED);
    CHECK("case B: delay stops at latency only (50000us)", rB.delay_us == 50000);
}

/* ---------------- Day 4: stress profiles ---------------- */
static void test_high_latency(void) {
    printf("\n-- stress: high latency --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(100);

    cfg.base_latency_ms = 5000.0;
    s11_process_result_t r = latency_apply(&pkt, &cfg, NULL);
    CHECK("base 5000ms -> 5000000us", r.delay_us == 5000000);
    CHECK("never drops even at extreme latency", r.status == S11_PACKET_FORWARDED);
}

static void test_high_packet_loss(void) {
    printf("\n-- stress: high packet loss --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(100);
    const int trials = 1000;

    cfg.packet_loss_rate = 0.99;
    srand(11);
    int drops = 0;
    for (int i = 0; i < trials; i++) {
        if (packet_loss_apply(&pkt, &cfg, NULL).status == S11_PACKET_DROPPED) drops++;
    }
    /* Expected mean 990, std-dev ~3.15 (binomial). Generous tolerance
     * band since this isn't a locked exact-count regression seed. */
    CHECK("rate 0.99 -> drops within [960,1000] of 1000 trials",
          drops >= 960 && drops <= 1000);
}

static void test_low_bandwidth(void) {
    printf("\n-- stress: low bandwidth --\n");
    s11_link_model_config_t cfg = make_cfg();
    cfg.bandwidth_bps = 1000.0; /* 1 kbps */

    s11_packet_t pkt = make_packet(1000); /* 8000 bits */
    s11_process_result_t r = bandwidth_apply(&pkt, &cfg, NULL);
    CHECK("1000B @ 1kbps -> 8000000us (8s), no overflow", r.delay_us == 8000000);
    CHECK("still forwards with state=NULL (queue check skipped)", r.status == S11_PACKET_FORWARDED);
}

static void test_unstable_combined(void) {
    printf("\n-- stress: unstable link (all four combined) --\n");
    s11_link_model_config_t cfg = make_cfg();
    s11_packet_t pkt = make_packet(1000);

    cfg.base_latency_ms  = 2000.0;
    cfg.jitter_min_ms    = 100.0;
    cfg.jitter_max_ms    = 500.0;
    cfg.packet_loss_rate = 0.3;
    cfg.bandwidth_bps    = 5000.0;

    const int trials = 1000;
    int drops = 0;
    int forwarded_delay_ok = 1;

    srand(23);
    for (int i = 0; i < trials; i++) {
        s11_process_result_t r = run_chain(&pkt, &cfg);
        if (r.status == S11_PACKET_DROPPED) {
            drops++;
        } else if (r.delay_us <= 2000000) {
            /* Every forwarded packet must include at least the fixed
             * 2000ms base latency contribution. */
            forwarded_delay_ok = 0;
        }
    }

    /* Expected mean drops = 300 (30% of 1000 trials); generous tolerance. */
    CHECK("~30% loss rate -> drops within [250,350] of 1000 trials",
          drops >= 250 && drops <= 350);
    CHECK("all forwarded trials include base latency (>2000000us)",
          forwarded_delay_ok);
}

int main(void) {
    test_latency();
    test_packet_loss();
    test_jitter();
    test_bandwidth();
    test_bandwidth_queue();
    test_combined_conditions();
    test_high_latency();
    test_high_packet_loss();
    test_low_bandwidth();
    test_unstable_combined();

    printf("\n==================\n");
    printf("PASS: %d  FAIL: %d\n", g_pass, g_fail);
    printf("==================\n");

    return g_fail == 0 ? 0 : 1;
}