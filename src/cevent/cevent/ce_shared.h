//
//  ce_shared.h
//  cevent
//
//  Created by 孙瑞 on 15/1/10.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CE_SHARED_H
#define CE_SHARED_H

#include <stdint.h>

typedef uint16_t packet_len_t;

#define MAX_BUFFER (packet_len_t)(-1)

typedef enum ce_exception {
    ce_packet_illegal,
    ce_packet_send_error
} ce_exception;

typedef void (* ce_on_accept)(int fd, char * ip, int port, void * user_data);
typedef void (* ce_on_received)(int fd, const char * data, packet_len_t size, void * user_data);
typedef void (* ce_on_exception)(int fd, ce_exception exception, void * user_data);
typedef void (* ce_on_close)(int fd, void * user_data);

typedef enum ce_push_type {
    ce_push_fd,
    ce_push_except_fd
} ce_push_type;

typedef struct ce_conf {
    ce_on_accept on_accept;
    ce_on_received on_received;
    ce_on_exception on_exception;
    ce_on_close on_close;
    void * user_data;
    
    packet_len_t per_max_packet_body_size;
    int per_cache_ringbuffer_size;
    int workers;
    
    char ip[17];
    int port;
    int timeout;
    int keepalive;
    int heartbeat;
} ce_conf_t;

#endif