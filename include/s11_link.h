#ifndef S11_LINK_H
#define S11_LINK_H

#include <stdint.h>

#define LINK_MAX 32
#define S11_MAX_NODE_NAME_LEN 32

typedef enum {
    LINK_STATE_UP = 0,
    LINK_STATE_DOWN,
    LINK_STATE_DEGRADED,
    LINK_STATE_PARTITIONED,
    LINK_STATE_JAMMED
} LinkState;

typedef LinkState Linkstate;

typedef struct {
    char node_a[S11_MAX_NODE_NAME_LEN];
    char node_b[S11_MAX_NODE_NAME_LEN];
    LinkState state;
    uint64_t last_state_change_ts;
} S11Link;

void link_manager_init(void);
int link_manager_add_link(const char *node_a, const char *node_b, LinkState initial_state);
LinkState link_manager_get_state(const char *node_a, const char *node_b);
int link_manager_set_state(const char *node_a, const char *node_b, LinkState new_state, uint64_t timestamp);
int link_manager_count(void);

static inline LinkState link_manager_get_link_state(const char *node_a, const char *node_b) {
    return link_manager_get_state(node_a, node_b);
}

static inline int link_manager_set_link_state(const char *node_a, const char *node_b, LinkState new_state, uint64_t timestamp) {
    return link_manager_set_state(node_a, node_b, new_state, timestamp);
}

#endif