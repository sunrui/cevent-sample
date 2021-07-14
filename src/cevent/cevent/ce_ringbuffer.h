/**
 * ring buffer
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-13
 */
#ifndef CE_RINGBUFFER_H
#define CE_RINGBUFFER_H

typedef struct ce_rb ce_rb_t;

ce_rb_t * ce_rb_new(int capacity);
int ce_rb_capacity(ce_rb_t * rb);
int ce_rb_can_read(ce_rb_t * rb);
int ce_rb_can_write(ce_rb_t * rb);
int ce_rb_read(ce_rb_t * rb, void ** data, int count);
int ce_rb_write(ce_rb_t * rb, const void * data, int count);
void ce_rb_free(ce_rb_t ** rb);

#endif