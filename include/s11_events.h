#ifndef S11_EVENTS_H
#define S11_EVENTS_H

#include <stdint.h>
#include "s11_link.h"

#define EVENT_MAX 32
#define S11_MAX_EVENT_DESC_LEN EVENT_MAX

typedef enum {
    EVENT_LINK_UP = 0,
    EVENT_LINK_DOWN,
    EVENT_PARTITION_START,
    EVENT_PARTITION_END,
    EVENT_JAM_START,
    EVENT_JAM_END,
    EVENT_RECOVERY_COMPLETE
} S11EventType;

typedef S11EventType event_type;

typedef struct S11Event {
    S11EventType type;
    char node_a[S11_MAX_NODE_NAME_LEN];
    char node_b[S11_MAX_NODE_NAME_LEN];
    uint64_t timestamp;
    char description[S11_MAX_EVENT_DESC_LEN];
} S11Event;

typedef S11Event event;

#endif