/**
 * @file    s11_MODEL.h
 * @date    2026-09-10
 * @brief   Defines the network model configuration, state, and public APIs.
 *
 * @details
 * This header provides the data structures and public interfaces used
 * by the network degradation models of the S11 Communication & Network
 * Simulator.
 *
 * The models simulate network effects such as latency, jitter, packet
 * loss, and bandwidth limitations. Each model operates on a packet and
 * a link configuration and reports its own processing result.
 */

#ifndef S11_MODEL_H
#define S11_MODEL_H

/************************************* Include Part ************************************* */

#include <stdint.h>
#include "s11_packet.h"
#include "s11_network.h"

/************************************* Macros Part ************************************* */


/************************************* User Data Types Part ************************************* */

/**
 * @struct s11_bandwidth_state_t
 * @brief  Maintains runtime state for the bandwidth degradation model.
 *
 * @details
 * This structure stores the number of packets currently waiting in the
 * bandwidth queue and the accumulated transmission delay of packets
 * already ahead in the queue.
 */
typedef struct
{
    /**
     * @brief Number of packets currently waiting in the bandwidth queue.
     */
    uint32_t queued_packets;

    /**
     * @brief Total transmission delay of packets currently ahead in the queue,
     *        in microseconds.
     */
    uint64_t pending_delay_us;

} s11_bandwidth_state_t;


/**
 * @struct s11_link_model_config_t
 * @brief  Defines the degradation parameters for a communication link.
 *
 * @details
 * This structure represents the network conditions applied by the
 * degradation models during packet processing.
 *
 * The configuration is populated from config/network.yaml.
 * One instance exists for each supported link type, such as
 * "lte", "5g", "mesh", "lora", or "satellite".
 */
typedef struct
{
    /**
     * @brief Identifier of the communication link type.
     *
     * @details
     * Examples include "lte", "5g", "mesh", "lora", and "satellite".
     */
    char link_type[16];

    /**
     * @brief Fixed baseline latency applied to packets on this link,
     *        in milliseconds.
     */
    double base_latency_ms;

    /**
     * @brief Minimum random jitter added to the baseline latency,
     *        in milliseconds.
     */
    double jitter_min_ms;

    /**
     * @brief Maximum random jitter added to the baseline latency,
     *        in milliseconds.
     */
    double jitter_max_ms;

    /**
     * @brief Probability that a packet is dropped on this link.
     *
     * @details
     * The value is represented as a probability in the range
     * 0.0 to 1.0, where 0.0 means no packet loss and 1.0 means
     * all packets are dropped.
     */
    double packet_loss_rate;

    /**
     * @brief Maximum throughput supported by this link,
     *        in bits per second.
     */
    double bandwidth_bps;

    /**
     * @brief Maximum number of packets that can be buffered by
     *        the link before packets are dropped.
     */
    uint32_t queue_capacity;

} s11_link_model_config_t;


/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Common function signature for all network degradation models.
 *
 * @details
 * This function pointer type defines the common interface used by
 * the latency, jitter, packet loss, and bandwidth models.
 *
 * Each model reports only its own incremental delay contribution
 * through the returned result's delay_us field rather than maintaining
 * a running total.
 *
 * The caller, typically network_manager.c, is responsible for:
 * - Calling the degradation models in sequence.
 * - Accumulating the delay contributions returned by each model.
 * - Stopping the processing chain when a model reports
 *   S11_PACKET_DROPPED.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 *
 * @param[in] cfg
 * Pointer to the communication link configuration to be applied.
 *
 * @param[in,out] state
 * Optional model-specific runtime state.
 *
 * @details
 * This parameter is used by models that require state to be preserved
 * across multiple calls, such as the bandwidth model. The state is
 * private to the corresponding model and may be NULL when no state
 * is required.
 *
 * @return
 * Processing result containing the model's forwarding or dropping
 * decision and its own incremental delay contribution in microseconds.
 */
typedef s11_process_result_t (*s11_model_apply_fn)(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);


/**
 * @brief Applies the latency degradation model to a packet.
 *
 * @details
 * Calculates the delay contribution introduced by the configured
 * baseline latency of the communication link.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 *
 * @param[in] cfg
 * Pointer to the link configuration containing the latency parameters.
 *
 * @param[in,out] state
 * Optional model-specific state. May be NULL if unused.
 *
 * @return
 * Processing result containing the latency model's delay contribution
 * and packet forwarding status.
 */
s11_process_result_t latency_apply(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);


/**
 * @brief Applies the jitter degradation model to a packet.
 *
 * @details
 * Calculates the additional delay introduced by network jitter based
 * on the configured minimum and maximum jitter values.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 *
 * @param[in] cfg
 * Pointer to the link configuration containing the jitter parameters.
 *
 * @param[in,out] state
 * Optional model-specific state. May be NULL if unused.
 *
 * @return
 * Processing result containing the jitter model's delay contribution
 * and packet forwarding status.
 */
s11_process_result_t jitter_apply(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);


/**
 * @brief Applies the packet loss degradation model to a packet.
 *
 * @details
 * Determines whether the packet should be dropped based on the
 * configured packet loss probability.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 *
 * @param[in] cfg
 * Pointer to the link configuration containing the packet loss rate.
 *
 * @param[in,out] state
 * Optional model-specific state. May be NULL if unused.
 *
 * @return
 * Processing result indicating whether the packet is forwarded or
 * dropped by the packet loss model.
 */
s11_process_result_t packet_loss_apply(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);


/**
 * @brief Applies the bandwidth degradation model to a packet.
 *
 * @details
 * Calculates the transmission delay introduced by the configured
 * link bandwidth and manages the associated bandwidth queue state.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 *
 * @param[in] cfg
 * Pointer to the link configuration containing the bandwidth and
 * queue capacity parameters.
 *
 * @param[in,out] state
 * Pointer to the bandwidth model state used to track queued packets
 * and pending transmission delay.
 *
 * @return
 * Processing result containing the bandwidth model's transmission
 * delay contribution and packet forwarding status.
 */
s11_process_result_t bandwidth_apply(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);


/************************************* End of File ************************************* */

#endif /* S11_MODEL_H */