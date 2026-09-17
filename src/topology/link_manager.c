#include <string.h>
#include "s11_link.h"


#define MAX_LINKS 64


static S11Link g_links[MAX_LINKS];
static int g_link_count = 0;

static int find_link_index(const char *node_a, const char *node_b) {
    int i;
    if((node_a == NULL) || (node_b == NULL))
    {
        return -1;
    }

    for (i = 0; i < g_link_count; i++)
    {
        int same_order   = (strncmp(g_links[i].node_a, node_a, S11_NODE_ID_MAX_LEN) == 0) &&
                            (strncmp(g_links[i].node_b, node_b, S11_NODE_ID_MAX_LEN) == 0);

        int reverse_order = (strncmp(g_links[i].node_a, node_b, S11_NODE_ID_MAX_LEN) == 0) &&
                            (strncmp(g_links[i].node_b, node_a, S11_NODE_ID_MAX_LEN) == 0);

        if (same_order || reverse_order)
        {
            return i;
        }
    }


    return -1;
}

void link_manager_init(void) {
    g_link_count = 0;
}

int link_manager_add_link(const char *node_a, const char *node_b, 
                          const char *link_type,LinkState initial_state)
{

    if((node_a == NULL) || (node_b == NULL) ||
       (link_type == NULL))
    {
        return (-1);
    }

    if((node_a[0] == '\0') || (node_b[0] == '\0') || (link_type[0] == '\0'))
    {
        return (-1);
    }

    if((strlen(node_a) >= S11_NODE_ID_MAX_LEN) ||
       (strlen(node_b) >= S11_NODE_ID_MAX_LEN) ||
       (strlen(link_type) >= sizeof(((S11Link *)0)->link_type)))
    {
        return (-1);
    }

    if(g_link_count >= MAX_LINKS)
    {
        return (-1);
    }

    if(find_link_index(node_a, node_b) != -1)
    {
        return -1;
    }

    
    S11Link *new_link = &g_links[g_link_count];

    strncpy(new_link->node_a,
            node_a,
            S11_NODE_ID_MAX_LEN - 1U);
    new_link->node_a[S11_NODE_ID_MAX_LEN - 1U] = '\0';

    strncpy(new_link->node_b,
            node_b,
            S11_NODE_ID_MAX_LEN - 1U);
    new_link->node_b[S11_NODE_ID_MAX_LEN - 1U] = '\0';

    strncpy(new_link->link_type,
            link_type,
            sizeof(new_link->link_type) - 1U);
    new_link->link_type[sizeof(new_link->link_type) - 1U] = '\0';

    new_link->state = initial_state;
    new_link->last_state_change_ts = 0U;


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


int link_manager_get_type( const char *node_a, const char *node_b,
                           char *link_type, size_t link_type_size)
{
    int idx;

    if((node_a == NULL) || (node_b == NULL) ||
       (link_type == NULL) || (link_type_size == 0U))
    {
        return (-1);
    }

    idx = find_link_index(node_a, node_b);

    if(idx == -1)
    {
        return (-1);
    }

    if(strlen(g_links[idx].link_type) >= link_type_size)
    {
        return (-1);
    }

    strcpy(link_type, g_links[idx].link_type);

    return 0;
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