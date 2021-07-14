//
//  ce_server.h
//  cevent
//
//  Created by 孙瑞 on 15/1/11.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CE_SERVER_H
#define CE_SERVER_H

#include "ce_shared.h"

#include <stdbool.h>

typedef struct ce_server ce_server_t;

ce_server_t * ce_server_startup(ce_conf_t * conf, char * err);
bool ce_server_push(ce_server_t * server, int fd, const char * data, packet_len_t size);
void ce_server_wait(ce_server_t * server);
void ce_server_stop(ce_server_t * server);

#endif