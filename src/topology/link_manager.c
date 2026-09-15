
#include <string.h>   
#include <stdio.h>  
#include "s11_link.h"


#define MAX_LINKS 64

static S11Link g_links[MAX_LINKS];
static int g_link_count = 0;

static int find_link_index(const char *node_a, const char *node_b) {
    int i;
    for (i = 0; i < g_link_count; i++) {
        int same_order   = (strncmp(g_links[i].node_a, node_a, S11_MAX_NODE_NAME_LEN) == 0) &&
                            (strncmp(g_links[i].node_b, node_b, S11_MAX_NODE_NAME_LEN) == 0);
        int reverse_order = (strncmp(g_links[i].node_a, node_b, S11_MAX_NODE_NAME_LEN) == 0) &&
                            (strncmp(g_links[i].node_b, node_a, S11_MAX_NODE_NAME_LEN) == 0);
        if (same_order || reverse_order) {
            return i;
        }
    }
    return -1;
}

void link_manager_init(void) {
    g_link_count = 0;
}

int link_manager_add_link(const char *node_a, const char *node_b, LinkState initial_state) {
    if (g_link_count >= MAX_LINKS) {
        return -1; 
    }
    if (find_link_index(node_a, node_b) != -1) {
        return -1;
    }

    S11Link *new_link = &g_links[g_link_count];
    strncpy(new_link->node_a, node_a, S11_MAX_NODE_NAME_LEN - 1);
    new_link->node_a[S11_MAX_NODE_NAME_LEN - 1] = '\0';
    strncpy(new_link->node_b, node_b, S11_MAX_NODE_NAME_LEN - 1);
    new_link->node_b[S11_MAX_NODE_NAME_LEN - 1] = '\0';
    new_link->state = initial_state;
    new_link->last_state_change_ts = 0;

    g_link_count++;
    return 0;
}

LinkState link_manager_get_state(const char *node_a, const char *node_b) {
    int idx = find_link_index(node_a, node_b);
    if (idx == -1) {
        return LINK_STATE_DOWN;
    }
    return g_links[idx].state;
}

int link_manager_set_state(const char *node_a, const char *node_b, LinkState new_state, uint64_t timestamp) {
    int idx = find_link_index(node_a, node_b);
    if (idx == -1) {
        return -1; 
    }
    g_links[idx].state = new_state;
    g_links[idx].last_state_change_ts = timestamp;
    return 0;
}

int link_manager_count(void) {
    return g_link_count;
}

const S11Link *link_manager_get_link_at(int index) {
    if (index < 0 || index >= g_link_count) {
        return NULL; 
    }
    return &g_links[index];
}