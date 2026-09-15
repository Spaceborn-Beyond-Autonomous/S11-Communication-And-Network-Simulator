#include "s11_config.h"
#include <stddef.h>
bool s11_config_init( s11_network_config_t *config )
{
    if(config == NULL)
    {
        return (false);
    }

    config->packet_loss_percent = 0.0;
    config->jitter_ms = 0.0;
    config->latency_ms = 0.0;
    config->bandwidth_mbps = 0.0; /* not configured */

    return(true);

}

bool s11_config_validate( const s11_network_config_t *config )
{
    if(config == NULL)
    {
        return (false);
    }

    if((config ->packet_loss_percent < 0.0 ) || (config ->packet_loss_percent > 100.0))
    {
        return (false);
    }

    if(config ->bandwidth_mbps <= 0.0)
    {
        return(false);
    }

    if(config ->jitter_ms < 0.0)
    {
        return (false);
    }

    if(config ->latency_ms < 0.0)
    {
        return (false);
    }

    return(true);

}

bool s11_config_load( s11_network_config_t *config, const char *config_file )
{
     if ((config == NULL) || (config_file == NULL))
    {
        return (false);
    }

    /*
     * TODO:
     * Load configuration values from the configuration file.
     */

    return (false);
}