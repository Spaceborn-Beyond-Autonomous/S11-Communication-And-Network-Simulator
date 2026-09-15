
#ifndef NETWORK_MONITOR_H
#define NETWORK_MONITOR_H

#include "network_logger.h"   
void monitor_init(void);


void monitor_record_packet(LogStatus status, int delay_ms);


void monitor_print_snapshot(void);

#endif 