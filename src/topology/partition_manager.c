#include "s11_link.h"

bool s11_partition_is_connected(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    return s11_link_is_available(source, destination);
}