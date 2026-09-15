/**
 * @file    s11_network.h
 * @author  Sara Saad Mahmoud
 * @date    2026-09-10
 * @brief   Defines the network state and public APIs of the S11 simulator.
 *
 * @details
 * This header provides the network-level representation and public
 * interfaces used by the S11 Communication & Network Simulator.
 *
 * The network structure represents the runtime state of the simulation
 * network, while packet-level processing is performed through the
 * network processing API.
*/

#ifndef S11_NETWORK_H
#define S11_NETWORK_H

/************************************* Include Part ************************************* */

#include <stdint.h>
#include "s11_packet.h"

/************************************* Macros Part ************************************* */

#define S11_NETWORK_ID_MAX_LEN 64

/************************************* User Data Types Part ************************************* */

/**
 * @enum s11_process_status_t
 * @brief Represents the result of processing a packet.
*/
typedef enum
{
    S11_PACKET_FORWARDED = 0,
    S11_PACKET_DROPPED

} s11_process_status_t;


/**
 * @struct s11_process_result_t
 * @brief Contains the result of processing a packet.
*/
typedef struct
{
    /**
     * @brief Final processing status of the packet.
    */
    s11_process_status_t status;

    /**
     * @brief Network delay applied to the packet in microseconds.
    */
    uint64_t delay_us;

} s11_process_result_t;


/**
 * @struct s11_network_t
 * @brief Represents the runtime state of the S11 simulation network.
*/
typedef struct
{
    /**
     * @brief Identifier of the simulation network.
    */
    char network_id[S11_NETWORK_ID_MAX_LEN];

    /**
     * @brief Number of packets received by the network.
    */
    uint64_t packets_received;

    /**
     * @brief Number of packets successfully transmitted.
    */
    uint64_t packets_transmitted;

    /**
     * @brief Number of packets dropped by the network.
    */
    uint64_t packets_dropped;

} s11_network_t;


/**
 * @struct s11_link_model_config_t
 * @brief  Per-link degradation configuration for a communication link.
 *
 * @details
 * Populated from config/network.yaml. One instance exists per link
 * type (e.g. "lte", "5g", "mesh", "lora", "satellite").
*/
typedef struct
{
    /**
     * @brief Identifier of the link type (e.g. "lte", "5g", "mesh").
    */
    char link_type[16];

    /**
     * @brief Fixed baseline latency applied to packets on this link, in milliseconds.
    */
    double base_latency_ms;

    /**
     * @brief Lower bound of random jitter added on top of base latency, in milliseconds.
    */
    double jitter_min_ms;

    /**
     * @brief Upper bound of random jitter added on top of base latency, in milliseconds.
    */
    double jitter_max_ms;

    /**
     * @brief Probability that a packet on this link is dropped, in range 0.0-1.0.
    */
    double packet_loss_rate;

    /**
     * @brief Maximum throughput of this link, in bits per second.
    */
    double bandwidth_bps;

    /**
     * @brief Maximum number of packets this link can buffer before dropping.
    */
    uint32_t queue_capacity;

} s11_link_model_config_t;

typedef struct
{
    uint32_t queued_packets;
    uint64_t pending_delay_us; /* sum of transmission times of packets currently ahead in queue */
} s11_bandwidth_state_t;

/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Creates and initializes a new S11 network.
 *
 * @param[in] network_id
 * Identifier of the simulation network.
 *
 * @return
 * Pointer to the newly created network on success.
 * NULL if the network could not be created.
*/
s11_network_t *s11_network_create(const char *network_id);


/**
 * @brief Destroys an S11 network and releases its resources.
 *
 * @param[in] network
 * Pointer to the network to be destroyed.
 */
void s11_network_destroy(s11_network_t *network);


/**
 * @brief Processes a packet through the S11 network.
 *
 * @param[in,out] network
 * Pointer to the network runtime state.
 *
 * @param[in,out] packet
 * Pointer to the packet to be processed.
 *
 * @return
 * Processing result containing the packet status and applied delay.
 */
s11_process_result_t s11_network_process(
    s11_network_t *network,
    const s11_packet_t *packet
);

/**
 * @brief Function signature common to all four degradation models
 *        (latency, jitter, packet loss, bandwidth).
 *
 * @details
 * Each model reports only its own incremental delay contribution in
 * the returned result's delay_us field, not a running total. The
 * caller (network_manager.c) is responsible for summing delay_us
 * across all model calls and for stopping the chain early if any
 * model returns S11_PACKET_DROPPED.
 *
 * @param[in] pkt
 * Pointer to the packet being processed.
 * @param[in] cfg
 * Pointer to the link configuration to apply.
 * @param[in,out] state
 * Optional model-specific state for models needing memory across
 * calls (e.g. a smoothed jitter model remembering its last delay).
 * Private to that model only; NULL if unused.
 *
 * @return
 * Result indicating this model's forward/drop status and its own
 * incremental delay contribution in microseconds.
*/
typedef s11_process_result_t (*s11_model_apply_fn)(
    const s11_packet_t *pkt,
    const s11_link_model_config_t *cfg,
    void *state
);
/************************************* End of File ************************************* */

#endif /* S11_NETWORK_H */

