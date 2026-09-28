#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include <stdbool.h>
#include "s11_link.h"

bool s11_network_monitor_is_link_available(int source, int destination);

#endif