/**
 * @file    s11_link.h
 * @date    2026-09-10
 * @brief   Defines the network link representation and management APIs.
 *
 * @details
 * This header provides the link representation and public management
 * interfaces used by the S11 Communication & Network Simulator.
 *
 * A network link represents a communication connection between two
 * simulated nodes. The link state is used by the network manager to
 * determine whether packets can be forwarded between the connected
 * nodes.
 *
 * The link manager is responsible for creating links, querying their
 * current state, updating their state, and reporting the number of
 * configured links.
 *
 * Supported link states include normal operation, failure, degraded
 * communication, network partitioning, and jamming.
 */

#ifndef S11_LINK_H
#define S11_LINK_H

/************************************* Include Part ************************************* */

#include <stdint.h>
#include <stddef.h>

#include "s11_common.h"


/************************************* Macros Part ************************************* */


/************************************* User Data Types Part ************************************* */

/**
 * @enum LinkState
 * @brief Represents the current runtime state of a network link.
 *
 * @details
 * The link state describes the availability and operational condition
 * of a communication connection between two simulated nodes.
 */
typedef enum
{
    /**
     * @brief Indicates that the link is available and operating normally.
     */
    LINK_STATE_UP = 0,

    /**
     * @brief Indicates that the link is unavailable.
     *
     * @details
     * Packets cannot be forwarded through a link in this state.
     */
    LINK_STATE_DOWN,

    /**
     * @brief Indicates that the link is available but operating
     *        with degraded conditions.
     *
     * @details
     * Packets may still be forwarded while additional network models
     * determine the resulting performance degradation.
     */
    LINK_STATE_DEGRADED,

    /**
     * @brief Indicates that the link is affected by a network partition.
     *
     * @details
     * Communication between the affected nodes is considered unavailable
     * while the partition is active.
     */
    LINK_STATE_PARTITIONED,

    /**
     * @brief Indicates that the link is affected by network jamming.
     *
     * @details
     * Communication through the affected link is considered unavailable
     * while jamming is active.
     */
    LINK_STATE_JAMMED

} LinkState;


/**
 * @struct S11Link
 * @brief Represents a communication link between two simulated nodes.
 *
 * @details
 * The structure stores the two endpoints of the link, its current
 * runtime state, and the simulation timestamp of the most recent
 * state change.
 */
typedef struct
{
    /**
     * @brief Identifier of the first node connected by the link.
     */
    char node_a[S11_NODE_ID_MAX_LEN];

    /**
     * @brief Identifier of the second node connected by the link.
     */
    char node_b[S11_NODE_ID_MAX_LEN];

    /**
     * @brief Current runtime state of the communication link.
     */
    LinkState state;

    /**
     * @brief Simulation timestamp of the last link state change.
     *
     * @note
     * The timestamp is expressed in microseconds and represents
     * simulation time rather than wall-clock time.
     */
    uint64_t last_state_change_ts;

    /**
     * @brief Physical link type (e.g. "lte", "5g", "mesh", "lora",
     * "satellite"). Used to select the matching s11_link_model_config_t
     * from config/network.yaml.
    */
    char link_type[16];

} S11Link;


/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Initializes the link manager.
 *
 * @details
 * Resets the internal link collection and prepares the link manager
 * to register and manage network links.
 */
void link_manager_init(void);


/**
 * @brief Adds a new communication link to the network.
 *
 * @param[in] node_a
 * Identifier of the first node connected by the link.
 *
 * @param[in] node_b
 * Identifier of the second node connected by the link.
 *
 * @param[in] initial_state
 * Initial runtime state assigned to the new link.
 *
 * @param[in] link_type
 * Physical or logical communication type assigned to the link,
 * such as "lte", "5g", "mesh", "lora", or "satellite".
 *
 * @return
 * 0 if the link was added successfully.
 * A negative value if the link could not be added.
 */
int link_manager_add_link(
    const char *node_a,
    const char *node_b,
    const char *link_type,
    LinkState initial_state
);


/**
 * @brief Retrieves the current state of a communication link.
 *
 * @param[in] node_a
 * Identifier of the first node connected by the link.
 *
 * @param[in] node_b
 * Identifier of the second node connected by the link.
 *
 * @return
 * Current state of the requested link.
 * If the link does not exist, @ref LINK_STATE_DOWN is returned.
 */
LinkState link_manager_get_state(
    const char *node_a,
    const char *node_b
);

/**
 * @brief Retrieves the physical type of a communication link.
 *
 * @details
 * The link type identifies the physical or logical communication medium
 * used by the link and is used to select the corresponding network
 * degradation configuration.
 *
 * @param[in] node_a
 * Identifier of the first node connected by the link.
 *
 * @param[in] node_b
 * Identifier of the second node connected by the link.
 *
 * @param[out] link_type
 * Buffer that receives the link type string.
 *
 * @param[in] link_type_size
 * Size of the output buffer in bytes.
 *
 * @return
 * 0 if the link type was retrieved successfully.
 * A negative value if the link does not exist or the output buffer
 * is invalid or too small.
 */
int link_manager_get_type( const char *node_a, const char *node_b, char *link_type, size_t link_type_size);


/**
 * @brief Updates the runtime state of a communication link.
 *
 * @details
 * The function updates both the current link state and the simulation
 * timestamp associated with the state change.
 *
 * @param[in] node_a
 * Identifier of the first node connected by the link.
 *
 * @param[in] node_b
 * Identifier of the second node connected by the link.
 *
 * @param[in] new_state
 * New runtime state to assign to the link.
 *
 * @param[in] timestamp
 * Simulation timestamp of the state change in microseconds.
 *
 * @return
 * 0 if the link state was updated successfully.
 * A negative value if the requested link does not exist.
 */
int link_manager_set_state(
    const char *node_a,
    const char *node_b,
    LinkState new_state,
    uint64_t timestamp
);


/**
 * @brief Returns the number of currently configured network links.
 *
 * @return
 * Number of links currently managed by the link manager.
 */
int link_manager_count(void);


const S11Link *link_manager_get_link_at(int index);




/************************************* End of File ************************************* */

#endif /* S11_LINK_H */
