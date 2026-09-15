#include <stdio.h>
#include "network_logger.h"
static const char *status_to_string(LogStatus status) {
    switch (status) {
        case LOG_STATUS_TX:   return "TX";
        case LOG_STATUS_RX:   return "RX";
        case LOG_STATUS_DROP: return "DROP";
        default:              return "UNKNOWN";
    }
}

static const char *event_type_to_string(S11EventType type) {
    switch (type) {
        case EVENT_LINK_UP:            return "LINK_UP";
        case EVENT_LINK_DOWN:          return "LINK_DOWN";
        case EVENT_PARTITION_START:    return "PARTITION_START";
        case EVENT_PARTITION_END:      return "PARTITION_END";
        case EVENT_JAM_START:          return "JAM_START";
        case EVENT_JAM_END:            return "JAM_END";
        case EVENT_RECOVERY_COMPLETE:  return "RECOVERY_COMPLETE";
        default:                       return "UNKNOWN_EVENT";
    }
}

void logger_log_packet(uint64_t timestamp,
                        const char *source,
                        const char *destination,
                        const char *protocol,
                        LogStatus status,
                        int delay_ms,
                        const char *description) {
    if (delay_ms >= 0) {
        printf("[t=%llums] %s->%s proto=%s status=%s delay=%dms desc=\"%s\"\n",
               (unsigned long long)timestamp, source, destination, protocol,
               status_to_string(status), delay_ms, description);
    } else {
        printf("[t=%llums] %s->%s proto=%s status=%s delay=- desc=\"%s\"\n",
               (unsigned long long)timestamp, source, destination, protocol,
               status_to_string(status), description);
    }
}

void logger_log_event(const S11Event *evt) {
    printf("[t=%llums] EVENT=%s node_a=%s node_b=%s desc=\"%s\"\n",
           (unsigned long long)evt->timestamp,
           event_type_to_string(evt->type),
           evt->node_a,
           evt->node_b,
           evt->description);
}