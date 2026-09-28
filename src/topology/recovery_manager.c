#include "s11_link.h"

bool s11_recovery_restore_link(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    if (s11_link_enable(source, destination))
    {
        return true;
    }

    return false;
}