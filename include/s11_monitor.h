/**
 * @file    network_monitor.h
 * @date    2026-09-17
 * @brief   Defines the network monitoring and statistics APIs.
 *
 * @details
 * This header provides the public interfaces used by the S11
 * Communication & Network Simulator to record packet processing
 * results and display runtime network statistics.
 *
 * The monitor records packet status and processing delay and can
 * periodically provide a snapshot of the current network activity.
 */

#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

/************************************* Include Part ************************************* */

#include "s11_logger.h"

/************************************* Macros Part ************************************* */


/************************************* User Data Types Part ************************************* */


/************************************* Function Prototypes Part ************************************* */

/**
 * @brief Initializes the network monitor.
 *
 * @details
 * Initializes the internal monitoring state and prepares the monitor
 * to record packet processing results.
 *
 * This function should be called before using the packet recording
 * or snapshot functions.
 */
void monitor_init(void);


/**
 * @brief Records the result of processing a network packet.
 *
 * @details
 * Stores the packet processing status and the delay introduced during
 * packet transmission or processing.
 *
 * @param[in] status
 * Status of the processed packet.
 *
 * @param[in] delay_ms
 * Processing or transmission delay associated with the packet,
 * in milliseconds.
 */
void monitor_record_packet(LogStatus status, int delay_ms);


/**
 * @brief Prints a snapshot of the current network statistics.
 *
 * @details
 * Displays the monitoring information accumulated by the simulator,
 * including packet processing statistics and recorded network delays.
 */
void monitor_print_snapshot(void);


/************************************* End of File ************************************* */

#endif /* NETWORK_MONITOR_H */