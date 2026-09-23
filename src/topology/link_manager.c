#include <stdio.h>
#include <stdbool.h>

#define S11_MAX_NODES 32

typedef struct
{
    int source;
    int destination;
    bool enabled;
    bool jammed;
} S11Link;

static S11Link links[S11_MAX_NODES][S11_MAX_NODES];

void s11_link_manager_init(void)
{
    for (int i = 0; i < S11_MAX_NODES; i++)
    {
        for (int j = 0; j < S11_MAX_NODES; j++)
        {
            links[i][j].source = i;
            links[i][j].destination = j;
            links[i][j].enabled = false;
            links[i][j].jammed = false;
        }
    }
}

bool s11_link_enable(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    links[source][destination].enabled = true;
    links[source][destination].jammed = false;

    return true;
}

bool s11_link_disable(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    links[source][destination].enabled = false;

    return true;
}

bool s11_link_jam(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    if (!links[source][destination].enabled)
    {
        return false;
    }

    links[source][destination].jammed = true;

    return true;
}

bool s11_link_unjam(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    links[source][destination].jammed = false;

    return true;
}

bool s11_link_is_available(int source, int destination)
{
    if (source < 0 || source >= S11_MAX_NODES ||
        destination < 0 || destination >= S11_MAX_NODES)
    {
        return false;
    }

    return links[source][destination].enabled &&
           !links[source][destination].jammed;
}