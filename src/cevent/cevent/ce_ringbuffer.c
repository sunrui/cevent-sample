/**
 * ring buffer
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-13
 */
#include "ce_ringbuffer.h"
#include "ce_assert.h"
#include "ce_alloctor.h"

#include <string.h>
#include <stdlib.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

struct ce_rb
{
    void * buffer;
    int head;
    int tail;
    int wrap;
    
    int capacity;
    
    struct ce_rb_retval {
        void * buffer;
        int size;
    } retval;
};

ce_rb_t * ce_rb_new(int capacity)
{
    ce_rb_t * rb;
    
    ce_assert(capacity > 0);
    rb = (ce_rb_t * )ce_malloc(sizeof(ce_rb_t));
    rb->capacity = capacity;
    rb->buffer = (void *)ce_malloc(rb->capacity);
    memset(rb->buffer, 0, rb->capacity);
    rb->head = 0;
    rb->tail = 0;
    rb->wrap = 0;
    rb->retval.buffer = NULL;
    rb->retval.size = 0;
    
    return rb;
}

int ce_rb_can_read(ce_rb_t * rb)
{
    if (rb == NULL) {
        ce_assert(0);
        return 0;
    }
    
    if (rb->tail == rb->head) {
        return rb->wrap ? rb->capacity : 0;
    } else if (rb->tail > rb->head) {
        return rb->tail - rb->head;
    } else {
        return rb->capacity + rb->tail - rb->head;
    }
}

int ce_rb_can_write(ce_rb_t * rb)
{
    return rb->capacity - ce_rb_can_read(rb);
}

int ce_rb_read(ce_rb_t * rb, void ** data, int count)
{
    if (rb == NULL || data == NULL || count < 0) {
        return 0;
    }
    
    count = min(count, ce_rb_can_read(rb));
    
    if (count == 0) {
        *data = NULL;
        return 0;
    }
    
    /* case in no need wrapper lines */
    {
        int tail_chunk;
        
        tail_chunk = rb->capacity - rb->head;
        if (tail_chunk > count) {
            *data = rb->buffer + rb->head;
            rb->head = (rb->head + count) % rb->capacity;
            return count;
        }
    }
    
    /* case in need wrapper lines */
    {
        int tail_chunk;
        int head_chunk;
        
        tail_chunk = min(count, rb->capacity - rb->head);
        head_chunk = count - tail_chunk;
        
        if (rb->retval.size < tail_chunk + head_chunk) {
            if (rb->retval.buffer != NULL) {
                ce_free(rb->retval.buffer);
            }
            
            rb->retval.size = tail_chunk + head_chunk;
            rb->retval.buffer = (void *)ce_malloc(rb->retval.size);
        }
        
        *data = rb->retval.buffer;
        memcpy(rb->retval.buffer, rb->buffer + rb->head, tail_chunk);
        rb->head = (rb->head + tail_chunk) % rb->capacity;
        
        if (tail_chunk < count) {
            memcpy(rb->retval.buffer + tail_chunk, rb->buffer + rb->head, head_chunk);
            rb->head = (rb->head + head_chunk) % rb->capacity;
        }
    }
    
    if (rb->wrap) {
        rb->wrap = 0;
    }
    
    return count;
}

int ce_rb_write(ce_rb_t * rb, const void * data, int count)
{
    if (rb == NULL || data == NULL || count <= 0) {
        ce_assert(0);
        return 0;
    }
    
    count = min(count, ce_rb_can_write(rb));
    if (count == 0) {
        return 0;
    }
    
    {
        int tail_chunk;
        
        tail_chunk = min(count, rb->capacity - rb->tail);
        memcpy(rb->buffer + rb->tail, data, tail_chunk);
        rb->tail = (rb->tail + tail_chunk) % rb->capacity;
        
        if (tail_chunk < count) {
            int head_chunk;
            
            head_chunk = count - tail_chunk;
            memcpy(rb->buffer + rb->tail, data + tail_chunk, head_chunk);
            rb->tail = (rb->tail + head_chunk) % rb->capacity;
        }
    }
    
    if (rb->head == rb->tail) {
        rb->wrap = 1;
    }
    
    return count;
}

void ce_rb_free(ce_rb_t ** rb)
{
    if (rb != NULL && * rb != NULL) {
        if ((*rb)->retval.buffer != NULL) {
            ce_free((*rb)->retval.buffer);
        }
        
        ce_free((*rb)->buffer);
        ce_freep(*rb);
    }
}
