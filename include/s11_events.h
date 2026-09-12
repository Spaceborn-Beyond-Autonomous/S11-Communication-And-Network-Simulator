/**
 * @file    s11_events.h
 * @author  Sara Saad Mahmoud
 * @date    2026-09-10
 * @brief   Defines the network events used by the S11 simulator.
 *
 * @details
 * This header provides the common event representation and public APIs
 * used throughout the S11 Communication & Network Simulator.
 *
 * Events represent significant changes in the runtime state of the
 * communication network, such as link failures, jamming, network
 * partitions, and their recovery.
 *
 * The event representation is shared between the network, topology,
 * failure, monitoring, and logging components.
 */

#ifndef S11_EVENTS_H
#define S11_EVENTS_H

/************************************* Include Part ************************************* */

#include <stdint.h>
#include <stdbool.h>
#include "s11_common.h"

/************************************* Macros Part ************************************* */


/************************************* User Data Types ************************************* */

/**
 * @enum s11_event_type_t
 * @brief Represents the type of a network event.
 */
typedef enum
{
    S11_EVENT_LINK_DOWN = 0,

    S11_EVENT_LINK_UP,

    S11_EVENT_JAMMING_STARTED,

    S11_EVENT_JAMMING_STOPPED,

    S11_EVENT_PARTITION_CREATED,

    S11_EVENT_PARTITION_RECOVERED

} s11_event_type_t;


/**
 * @struct s11_event_t
 * @brief Represents a significant event in the S11 simulation network.
 */
typedef struct
{
    /**
     * @brief Simulation time when the event occurred.
     *
     * @note
     * The timestamp is expressed in microseconds and represents
     * simulation time rather than wall-clock time.
     */
    uint64_t timestamp_us;

    /**
     * @brief Type of the network event.
     */
    s11_event_type_t type;

    /**
     * @brief Identifier of the link affected by the event.
     */
    char link_id[S11_LINK_ID_MAX_LEN];

} s11_event_t;


/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Initializes an S11 network event.
 *
 * @details
 * This function initializes an event with the specified event type,
 * affected link, and simulation timestamp.
 *
 * @param[out] event
 * Pointer to the event to be initialized.
 *
 * @param[in] type
 * Type of the network event.
 *
 * @param[in] link_id
 * Identifier of the link affected by the event.
 *
 * @param[in] timestamp_us
 * Simulation timestamp in microseconds.
 *
 * @return
 * true if the event was initialized successfully.
 * false if the provided parameters are invalid.
 */
bool s11_event_init(
    s11_event_t *event,
    s11_event_type_t type,
    const char *link_id,
    uint64_t timestamp_us
);


/**
 * @brief Converts an S11 event type to a human-readable string.
 *
 * @param[in] type
 * Event type to be converted.
 *
 * @return
 * Pointer to a constant string representing the event type.
 * NULL if the event type is invalid.
 */
const char *s11_event_type_to_string(
    s11_event_type_t type
);


/************************************* End of File ************************************* */

#endif /* S11_EVENTS_H */