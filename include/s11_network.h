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
#include "s11_config.h"

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
    int64_t delay_us;

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

    s11_network_config_t config;

} s11_network_t;

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

/************************************* End of File ************************************* */

#endif /* S11_NETWORK_H */