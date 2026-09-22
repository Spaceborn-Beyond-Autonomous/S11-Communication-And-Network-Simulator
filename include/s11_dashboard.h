#ifndef S11_DASHBOARD_H
#define S11_DASHBOARD_H

#include "s11_network.h"
#include "s11_config.h"
#include "s11_link.h"

void s11_dashboard_print(const s11_network_t *network,
                         const s11_link_model_config_t *config,
                         const char *scenario,
                         uint64_t simulated_time_us);

#endif
