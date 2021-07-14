//
//  ce_io.h
//  cevent
//
//  Created by 孙瑞 on 15/1/10.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#ifndef CE_IO_H
#define CE_IO_H

#include "ce_shared.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct ce_io ce_io_t;

typedef int (* ce_io_send)(int fd, const char * data, packet_len_t insize, void * extra);

typedef struct ce_io_param {
    ce_conf_t conf;

    ce_io_send send;
    void * send_extra;

    void * user_data;
} ce_io_param_t;

ce_io_t * ce_io_new(ce_io_param_t * param);
ce_io_param_t * ce_io_param(ce_io_t * io);
bool ce_io_add(ce_io_t * io, int fd);
bool ce_io_enqueue(ce_io_t * io, int fd, const char * data, packet_len_t size);
bool ce_io_push(ce_io_t * io, ce_push_type push_type, int fd, const char * data, packet_len_t size);
bool ce_io_del(ce_io_t * io, int fd);
void ce_io_destory(ce_io_t * io);

#endif