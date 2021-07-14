//
//  ce_io.c
//  cevent
//
//  Created by 孙瑞 on 15/1/10.
//  Copyright (c) 2015年 honeysense.com. All rights reserved.
//

#include "ce_io.h"
#include "ce_alloctor.h"
#include "ce_threadpool.h"
#include "ce_ringbuffer.h"
#include "ce_assert.h"

#include "dict.h"

#include <pthread.h>
#include <string.h>
#include <time.h>

struct ce_io {
    ce_io_param_t param;
    dict * io_dict;
    ce_tp_t * workers;
    bool stop;
    
    ce_rb_t * enqueue_fd_event;
    ce_rb_t * push_fd_event;
};

typedef struct ce_io_fd {
    ce_io_t * io;
    int fd;
    
    ce_rb_t * enqueue_buffer;
    pthread_mutex_t enqueue_lock;
    packet_len_t enqueue_cache_body_size;
    
    ce_rb_t * push_buffer;
    pthread_mutex_t push_lock;
    
} ce_io_fd_t;

bool _ce_io_get(ce_rb_t * rb, void ** one);
void * _ce_io_enqueue_buffer_processor(void * user_data);
void * _ce_io_push_buffer_processor(void * user_data);
void * _ce_io_delete_fd_processor(void * user_data);

ce_io_t * ce_io_new(ce_io_param_t * param)
{
    ce_io_t * io;
    
    io = ce_malloc(sizeof(ce_io_t));
    memcpy(&io->param, param, sizeof(ce_io_param_t));
    io->io_dict = dictCreate(&dictTypeHeapStrings, io);
    io->workers = ce_tp_create(param->conf.workers);
    io->stop = false;
    io->enqueue_fd_event = ce_rb_new(param->conf.per_cache_ringbuffer_size);
    io->push_fd_event = ce_rb_new(param->conf.per_cache_ringbuffer_size);
    
    return io;
}

ce_io_param_t * ce_io_param(ce_io_t * io)
{
    if (io == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    return &io->param;
}

bool ce_io_add(ce_io_t * io, int fd)
{
    dictEntry * entry;
    ce_io_fd_t * io_fd;
    
    if (io->stop) {
        ce_assert(0);
        return false;
    }
    
    entry = dictFind(io->io_dict, &fd);
    if (entry != NULL) {
        ce_assert(0);
        return false;
    }
    
    io_fd = (ce_io_fd_t *)ce_malloc(sizeof(ce_io_fd_t));
    io_fd->io = io;
    io_fd->fd = fd;
    io_fd->enqueue_buffer = ce_rb_new(io->param.conf.per_cache_ringbuffer_size);
    pthread_mutex_init(&io_fd->enqueue_lock, NULL);
    io_fd->enqueue_cache_body_size = 0;
    io_fd->push_buffer = ce_rb_new(io->param.conf.per_cache_ringbuffer_size);
    pthread_mutex_init(&io_fd->push_lock, NULL);
    
    return dictAdd(io->io_dict, &fd, io_fd) == DICT_OK;
}

bool ce_io_enqueue(ce_io_t * io, int fd, const char * data, packet_len_t size)
{
    dictEntry * entry;
    ce_io_fd_t * io_fd;
    
    if (io == NULL || fd <= 0 || data == NULL || size <= 0) {
        ce_assert(0);
        return false;
    }
    
    if (io->stop) {
        return false;
    }
    
    entry = dictFind(io->io_dict, &fd);
    if (entry == NULL) {
        /* the fd is marked as delete */
        return false;
    }
    
    io_fd = (ce_io_fd_t *)entry->key;
    
    {
        int write_size;
        
        write_size = ce_rb_write(io_fd->enqueue_buffer, data, size);
        if (write_size != size) {
            ce_assert(0);
            return false;
        }
        
        write_size = ce_rb_write(io_fd->io->enqueue_fd_event, &io_fd, sizeof(uintptr_t));
        if (write_size != sizeof(uintptr_t)) {
            ce_assert(0);
            return false;
        }
    }
    
    return ce_tp_worker_register(io->workers, _ce_io_enqueue_buffer_processor, NULL, io) != NULL;
}

bool ce_io_push(ce_io_t * io, ce_push_type push_type, int fd, const char * data, packet_len_t size)
{
    dictEntry * entry;
    ce_io_fd_t * io_fd;
    
    if (io == NULL || fd <= 0 || data == NULL || size <= 0) {
        ce_assert(0);
        return false;
    }
    
    if (io->stop) {
        return false;
    }
    
    entry = dictFind(io->io_dict, &fd);
    if (entry == NULL) {
        /* the fd is marked as delete */
        return false;
    }
    
    io_fd = (ce_io_fd_t *)entry->key;
    
    {
        int write_size;
        
        write_size = ce_rb_write(io_fd->enqueue_buffer, &size, sizeof(packet_len_t));
        if (write_size != sizeof(packet_len_t)) {
            ce_assert(0);
            return false;
        }
        
        write_size = ce_rb_write(io_fd->enqueue_buffer, data, size);
        if (write_size != size) {
            ce_assert(0);
            return false;
        }
    }
    
    return ce_tp_worker_register(io->workers, _ce_io_enqueue_buffer_processor, NULL, io) != NULL;
}

bool ce_io_del(ce_io_t * io, int fd)
{
    dictEntry * entry;
    ce_io_fd_t * io_fd;
    
    if (io == NULL || fd <= 0) {
        ce_assert(0);
        return false;
    }
    
    if (io->stop) {
        ce_assert(0);
        return false;
    }
    
    entry = dictFind(io->io_dict, &fd);
    if (entry == NULL) {
        ce_assert(0);
        return false;
    }

    io_fd = (ce_io_fd_t *)entry->key;
    dictDelete(io->io_dict, &fd);
    ce_tp_worker_register(io->workers, _ce_io_delete_fd_processor, NULL, io_fd);
    
    return true;
}

void ce_io_destory(ce_io_t * io)
{
    if (io == NULL) {
        ce_assert(0);
        return;
    }
    
    io->stop = true;
    
    {
        dictIterator * iter;
        ce_io_fd_t * io_fd;
        
        iter = dictGetSafeIterator(io->io_dict);
        while (dictNext(iter)) {
            io_fd = (ce_io_fd_t *)iter->entry->key;
            ce_io_del(io, io_fd->fd);
        }
        dictReleaseIterator(iter);
    }
    
    ce_tp_destroy(&io->workers, tp_kill_wait);
    ce_rb_free(&io->enqueue_fd_event);
    ce_rb_free(&io->push_fd_event);
    ce_free(io);
}

bool _ce_io_get(ce_rb_t * rb, void ** one)
{
    if (rb == NULL || one == NULL) {
        ce_assert(0);
        return false;
    }
    
    if (ce_rb_can_read(rb) < sizeof(uintptr_t)) {
        return false;
    }
    
    {
        int read;
        
        read = ce_rb_read(rb, (void **)&one, sizeof(uintptr_t));
        ce_assert(read == sizeof(uintptr_t));
    }
    
    return true;
}

void * _ce_io_enqueue_buffer_processor(void * user_data)
{
    ce_io_fd_t * io_fd;
    ce_io_t * io;
    
    char * read_data;
    int read_size;
    
    io = (ce_io_t *)user_data;
    
    packet_len_t enqueue_body_size;
    packet_len_t * _enqueue_body_size;
    
    while (ce_rb_can_read(io->push_fd_event) >= sizeof(uintptr_t)) {
        ce_rb_read(io->push_fd_event, (void **)&io_fd, sizeof(uintptr_t));
        
        pthread_mutex_lock(&io_fd->enqueue_lock);
        
        for (;;) {
            if (ce_rb_can_read(io_fd->enqueue_buffer) <= sizeof(packet_len_t)) {
                break;
            }
            
            ce_rb_read(io_fd->enqueue_buffer, (void **)&_enqueue_body_size, sizeof(packet_len_t));
            if (*_enqueue_body_size == 0) {
                /* maybe a heartbeat packet which size will be 0 */
                continue;
            }
            enqueue_body_size = *_enqueue_body_size;
            
            if (enqueue_body_size > io_fd->io->param.conf.per_max_packet_body_size) {
                /* illegal packet  */
                io_fd->io->param.conf.on_exception(io_fd->fd, ce_packet_illegal, io_fd->io->param.user_data);
                ce_io_del(io_fd->io, io_fd->fd);
                ce_assert(0);
                break;
            } else if (enqueue_body_size < ce_rb_can_read(io_fd->enqueue_buffer)) {
                /* not enough for a complete packet */
                io_fd->enqueue_cache_body_size = enqueue_body_size;
                break;
            }
            
            read_size = ce_rb_read(io_fd->enqueue_buffer, (void **)&read_data, enqueue_body_size);
            ce_assert(read_size == enqueue_body_size);
            
            /* notify a complete packet received */
            io_fd->io->param.conf.on_received(io_fd->fd, read_data, read_size, io_fd->io->param.user_data);
            io_fd->enqueue_cache_body_size = 0;
        }
        
        pthread_mutex_unlock(&io_fd->enqueue_lock);
    }

    return NULL;
}

#define CE_SEND_PER_SIZE 100 * 1024 /* 100k */
void * _ce_io_push_buffer_processor(void * user_data)
{
    ce_io_fd_t * io_fd;
    ce_io_t * io;
    
    char * read_data;
    int read_size;
    int send_size;
    
    io = (ce_io_t *)user_data;
    
    while (ce_rb_can_read(io->push_fd_event) >= sizeof(uintptr_t)) {
        ce_rb_read(io->push_fd_event, (void **)&io_fd, sizeof(uintptr_t));
        
        pthread_mutex_lock(&io_fd->push_lock);
        
        while ((read_size = ce_rb_read(io_fd->push_buffer, (void **)&read_data, CE_SEND_PER_SIZE)) > 0) {
            send_size = io->param.send(io_fd->fd, read_data, read_size, io->param.send_extra);
            if (send_size != read_size) {
                io->param.conf.on_exception(io_fd->fd, ce_packet_send_error, io->param.user_data);
                ce_io_del(io_fd->io, io_fd->fd);
                break;
            }
        }
        
        pthread_mutex_unlock(&io_fd->push_lock);
    }
    
    return NULL;
}

void * _ce_io_delete_fd_processor(void * user_data)
{
    ce_io_fd_t * io_fd;

    io_fd = (ce_io_fd_t *)user_data;
    pthread_mutex_lock(&io_fd->enqueue_lock);
    ce_rb_free(&io_fd->enqueue_buffer);
    pthread_mutex_unlock(&io_fd->enqueue_lock);
    pthread_mutex_destroy(&io_fd->enqueue_lock);
    pthread_mutex_lock(&io_fd->push_lock);
    ce_rb_free(&io_fd->push_buffer);
    pthread_mutex_unlock(&io_fd->push_lock);
    pthread_mutex_destroy(&io_fd->push_lock);
    ce_free(io_fd);

    return NULL;
}

