/*
 * network_logger.h
 *
 * Public logging API for the topology/failure/observability module.
 * Any module can call these to record what happened.
 *
 * NOTE: this header isn't in the original 5-header list the team agreed
 * on (s11_link, s11_events, s11_network, s11_packet, s11_protocol).
 * Adding it because logging needs its own small interface — flag this
 * with sara/the team so it's an intentional addition, not a surprise.
 */

#ifndef NETWORK_LOGGER_H
#define NETWORK_LOGGER_H

#include <stdint.h>
#include "s11_events.h"

/* Packet outcome, for the TX/RX/DROP log line. */
typedef enum {
    LOG_STATUS_TX = 0,   /* packet sent */
    LOG_STATUS_RX,       /* packet received successfully */
    LOG_STATUS_DROP      /* packet dropped (link down, loss model, etc.) */
} LogStatus;

/* Log a single packet's outcome.
 * protocol: short string like "ROS2", "MAVLink", "TEST".
 * delay_ms: -1 if not applicable (e.g. for a DROP). */
void logger_log_packet(uint64_t timestamp,
                        const char *source,
                        const char *destination,
                        const char *protocol,
                        LogStatus status,
                        int delay_ms,
                        const char *description);

/* Log a topology/failure event (link up/down, partition, jam, recovery). */
void logger_log_event(const S11Event *evt);

#endif /* NETWORK_LOGGER_H */
