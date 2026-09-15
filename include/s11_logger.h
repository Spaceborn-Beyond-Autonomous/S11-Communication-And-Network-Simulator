/**
 * @file    S11_logger.h
 * @date    2026-09-10
 * @brief   Defines the public logging interface used by the S11 simulator.
 *
 * @details
 * This header provides the public logging APIs used by the topology,
 * failure, network, and monitoring components of the S11 Communication
 * & Network Simulator.
 *
 * The logger records packet outcomes and significant network events,
 * including packet transmission, reception, packet drops, link state
 * changes, network partitions, jamming, and recovery events.
 *
 * The logging interface is intentionally independent of the internal
 * implementation of the logger so that other S11 modules can record
 * simulation activity through a common API.
 */

#ifndef S11_LOGGER_H
#define S11_LOGGER_H

/************************************* Include Part ************************************* */

#include <stdint.h>

#include "s11_events.h"

/************************************* Macros Part ************************************* */

/************************************* User Data Types Part ************************************* */

/**
 * @enum LogStatus
 * @brief Represents the outcome of a packet during network simulation.
 *
 * @details
 * The status is used by the packet logging API to identify whether
 * a packet was transmitted, received, or dropped.
 */
typedef enum
{
    /**
     * @brief Indicates that the packet was transmitted successfully.
     */
    LOG_STATUS_TX = 0,

    /**
     * @brief Indicates that the packet was received successfully.
     */
    LOG_STATUS_RX,

    /**
     * @brief Indicates that the packet was dropped.
     *
     * @details
     * A packet may be dropped because of conditions such as
     * link failure, packet loss, partitioning, or jamming.
     */
    LOG_STATUS_DROP

} LogStatus;


/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Logs the outcome of a simulated network packet.
 *
 * @details
 * Records the packet timestamp, source and destination nodes,
 * communication protocol, packet outcome, applied delay, and an
 * additional description of the event.
 *
 * The protocol is represented as a short human-readable string,
 * such as "ROS2", "MAVLink", or "TEST".
 *
 * A negative delay value indicates that the delay is not applicable,
 * such as when a packet is dropped before a delay is calculated.
 *
 * @param[in] timestamp
 * Simulation timestamp associated with the packet event.
 *
 * @param[in] source
 * Identifier of the node that generated or transmitted the packet.
 *
 * @param[in] destination
 * Identifier of the node that should receive the packet.
 *
 * @param[in] protocol
 * Human-readable name of the communication protocol.
 *
 * @param[in] status
 * Outcome of the packet, represented by @ref LogStatus.
 *
 * @param[in] delay_s
 * Packet delay in milliseconds.
 * Use a negative value when the delay is not applicable.
 *
 * @param[in] description
 * Additional human-readable information describing the packet event.
 */
void logger_log_packet(
    uint64_t timestamp,
    const char *source,
    const char *destination,
    const char *protocol,
    LogStatus status,
    int delay_ms,
    const char *description
);


/**
 * @brief Logs a topology or network failure event.
 *
 * @details
 * Records a significant event affecting the simulated network,
 * such as a link state change, network partition, jamming event,
 * or recovery event.
 *
 * The event details are provided through the common @ref s11_event_t
 * structure defined by the S11 event interface.
 *
 * @param[in] evt
 * Pointer to the network event to be logged.
 */
void logger_log_event(const s11_event_t *evt);


/************************************* End of File ************************************* */

#endif /* S11_LOGGER_H */

