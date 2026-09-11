#ifndef LINK_H
#define LINK_H
#include<stdint.h>
#define LINK_MAX 32
typedef enum{
    LINK_TYPE_UP = 0,
    LINK_TYPE_DOWN,
    LINK_TYPE_DEGRADE,
    LINK_TYPE_PARTITION,
    LINK_TYPE_JAMMED
} Linkstate;
typedef struct{
    char node_a[LINK_MAX];
    char node_b[LINK_MAX];
    Linkstate state;
    uint64_t last_state_change;
} Link;
#endif