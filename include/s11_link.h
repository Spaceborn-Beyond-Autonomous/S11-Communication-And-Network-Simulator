#ifndef S11_LINK_H
#define S11_LINK_H

#include <stdbool.h>

#define S11_MAX_NODES 32

typedef struct
{
    int source;
    int destination;
    bool enabled;
    bool jammed;
} S11Link;

void s11_link_manager_init(void);

bool s11_link_enable(int source, int destination);
bool s11_link_disable(int source, int destination);

bool s11_link_jam(int source, int destination);
bool s11_link_unjam(int source, int destination);

bool s11_link_is_available(int source, int destination);

#endif