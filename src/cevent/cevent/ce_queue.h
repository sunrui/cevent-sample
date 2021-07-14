/**
 * send queue, use for maintenance send/recv pools
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-7-31
 */
#ifndef CE_QUEUE_H
#define CE_QUEUE_H

#include <stdbool.h>

typedef struct ce_queue ce_queue_t;

ce_queue_t * ce_queue_new();
bool ce_queue_push(ce_queue_t * queue, void * item);

/**
 * @return
 *     1 get it ok
 *     0 no more buffers now (timeout reached)
 *    -1 abort
 */
int ce_queue_get(ce_queue_t * queue, void ** body, int timeout);

bool ce_queue_stop(ce_queue_t * queue);
void ce_queue_destroy(ce_queue_t ** queue);

#endif