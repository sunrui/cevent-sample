/**
 * send queue, use for maintenance send/recv pools
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2012-7-31 12:29
 */
#include "ce_queue.h"
#include "ce_alloctor.h"
#include "ce_assert.h"

#include <sys/queue.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct ce_queue_body_t {
    void * body; /* need send/recv buffers */
    TAILQ_ENTRY(ce_queue_body_t) entries;
} ce_queue_body_t;

struct ce_queue {
    TAILQ_HEAD(, ce_queue_body_t) queue; /* unique event queue instance */
    pthread_cond_t cond; /* use for queue thread-safe */
    pthread_mutex_t mutex; /* use for queue thread-safe */
    int stop; /* whether received a signal to stop */
};

ce_queue_t * ce_queue_new()
{
    ce_queue_t * queue;
    
    queue = (ce_queue_t *)ce_calloc(1, sizeof(ce_queue_t));
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->stop = 0;
    TAILQ_INIT(&queue->queue);
    
    return queue;
}

bool ce_queue_push(ce_queue_t * queue, void * body)
{
    ce_queue_body_t * item;
    
    if (queue == NULL) {
        ce_assert(0);
        return false;
    }
    
    item = (ce_queue_body_t *)ce_calloc(1, sizeof(ce_queue_body_t));
    item->body = body;
    
    pthread_mutex_lock(&queue->mutex);
    TAILQ_INSERT_TAIL(&queue->queue, item, entries);
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    
    return true;
}

/**
 * @return
 *     1 get it ok
 *     0 no more buffers now (timeout reached)
 *    -1 abort
 */
int ce_queue_get(ce_queue_t * queue, void ** body, int timeout)
{
    ce_queue_body_t * item;
    int ret;
    
    if (queue == NULL || body == NULL) {
        ce_assert(0);
        return 0;
    }
    
    pthread_mutex_lock(&queue->mutex);
    
    *body = NULL;
    item = TAILQ_FIRST(&queue->queue);
    while (item == NULL && !queue->stop) {
        int wait_r;
        
        if (timeout > 0) {
            struct timeval tp;
            struct timespec ts;
            
            gettimeofday(&tp, NULL);
            
            ts.tv_sec = tp.tv_sec + timeout;
            ts.tv_nsec = tp.tv_usec * 1000;
            
            wait_r = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
        } else if (timeout == 0) {
            pthread_mutex_unlock(&queue->mutex);
            *body = NULL;
            ret = 0;
            
            return ret;
        } else {
            wait_r = pthread_cond_wait(&queue->cond, &queue->mutex);
        }
        
        /* if waked by user stop sign */
        if (queue->stop) {
            pthread_mutex_unlock(&queue->mutex);
            *body = NULL;
            ret = -1;
            
            return ret;
        }
        
        item = TAILQ_FIRST(&queue->queue);
        if (item == NULL) {
            if (wait_r != 0) { /* timeout reached */
                pthread_mutex_unlock(&queue->mutex);
                *body = NULL;
                ret = 0;
                
                return ret;
            } else { /* by user abort */
                pthread_mutex_unlock(&queue->mutex);
                *body = NULL;
                ret = -1;
                
                return ret;
            }
        } else { /* wait ok and receive a body */
            break;
        }
    }
    
    if (item != NULL) {
        *body = item->body;
        TAILQ_REMOVE(&queue->queue, item, entries);
        ce_free(item);
        ret = 1;
    } else {
        ret = 0;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return ret;
}


bool ce_queue_stop(ce_queue_t * queue)
{
    if (queue == NULL) {
        ce_assert(0);
        return false;
    }
    
    pthread_mutex_lock(&queue->mutex);
    queue->stop = 1;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
    
    return true;
}

void ce_queue_destroy(ce_queue_t ** queue)
{
    if (queue != NULL && *queue != NULL) {
        ce_queue_stop(*queue);
        ce_assert(TAILQ_FIRST(&(*queue)->queue) == NULL);
        
        pthread_mutex_lock(&(*queue)->mutex);
        pthread_mutex_unlock(&(*queue)->mutex);
        
        pthread_mutex_destroy(&(*queue)->mutex);
        pthread_cond_destroy(&(*queue)->cond);
        ce_freep(*queue);
    }
}
