#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include "link.h"
#include "s11_link.h"

#define EVENT_MAX 32

typedef enum {
    event_link_up = 0,
    event_link_down,
    event_partition_start,
    event_partition_end,
    event_jammed_start,
    event_jammed_end,
} event_type;

typedef struct {
    event_type type;
    char node_a[LINK_MAX];
    char node_b[LINK_MAX];
    uint64_t timestamp;
    char description[EVENT_MAX];
} event;

#endif