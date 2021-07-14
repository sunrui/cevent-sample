//
//  ce_client.h
//  cevent
//
//  Created by 孙瑞 on 15/1/11.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CE_CLIENT_H
#define CE_CLIENT_H

#include "ce_shared.h"

#include <stdbool.h>

typedef struct ce_client ce_client_t;

typedef enum ce_connect_result {
    ce_connect_ok = 0,
    ce_connect_socket_error,
    ce_connect_server_no_response,
    ce_connect_init_error
} ce_connect_result;

ce_client_t * ce_client_connect(ce_conf_t * conf, ce_connect_result * result);
bool ce_client_send(ce_client_t * client, const char * data, packet_len_t size);
void ce_client_disconnect(ce_client_t * client);

#endif