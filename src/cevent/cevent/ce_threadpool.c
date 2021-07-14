/**
 * thread pool
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-12
 */
#include "ce_threadpool.h"
#include "ce_assert.h"
#include "ce_alloctor.h"

#include <semaphore.h>
#include <pthread.h>

#include <sys/queue.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

struct ce_tp_worker {
    ce_tp_t * pool;
    
    ce_tp_process_cb process_cb;
    ce_tp_complete_cb complete_cb;
    
    void * user_data;
    void * result;
    
    ce_tp_state state;
    sem_t sem;
    
    TAILQ_ENTRY(ce_tp_worker) entries;
};

struct ce_tp {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    TAILQ_HEAD(, ce_tp_worker) worker_queue;
    pthread_t * thread;
    int nthreads;
    bool abort;
    
    int routines;
};

void * ce_tp_routine(void * param)
{
    struct ce_tp * pool = (struct ce_tp *)param;
    
    pool->routines++;
    
    for (; !pool->abort;) {
        ce_tp_worker_t * worker;
        
        pthread_mutex_lock(&pool->mutex);
        while ((worker = TAILQ_FIRST(&pool->worker_queue)) == NULL) {
            if (pool->abort) {
                pthread_mutex_unlock(&pool->mutex);
                pthread_exit(NULL);
                return NULL;
            }
            
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        
        TAILQ_REMOVE(&pool->worker_queue, worker, entries);
        pthread_mutex_unlock(&pool->mutex);
        ce_assert(worker != NULL);
        
        worker->state = tp_state_busy;
        worker->result = (*worker->process_cb)(worker->user_data);
        worker->state = tp_state_complete;
        sem_post(&worker->sem);
        
        if (worker->complete_cb != NULL) {
            (*worker->complete_cb)(worker, worker->result, worker->user_data);
        }
        
        ce_tp_worker_free(worker);
    }
    
    return NULL;
}

ce_tp_t * ce_tp_create(int nthreads)
{
    ce_tp_t * pool;
    
    pool = (ce_tp_t *)ce_calloc(1, sizeof(ce_tp_t));
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->cond, NULL);
    pool->thread = (pthread_t *)ce_calloc(1, nthreads * sizeof(pthread_t));
    pool->nthreads = nthreads;
    TAILQ_INIT(&pool->worker_queue);
    pool->abort = false;
    
    {
        int index;
        for (index = 0; index < nthreads; index++) {
            pthread_create(&pool->thread[index], NULL, ce_tp_routine, pool);
            while (pool->routines < pool->nthreads) usleep(10);
        }
    }
    
    return pool;
}

ce_tp_worker_t * ce_tp_worker_register(ce_tp_t * pool, ce_tp_process_cb process_cb, ce_tp_complete_cb complete_cb, void * user_data)
{
    ce_tp_worker_t * worker;
    
    if (pool == NULL || pool->abort || process_cb == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    worker = (ce_tp_worker_t *)ce_calloc(1, sizeof(ce_tp_worker_t));
    worker->pool = pool;
    worker->process_cb = process_cb;
    worker->complete_cb = complete_cb;
    worker->user_data = user_data;
    worker->result = NULL;
    worker->state = tp_state_idle;
    sem_init(&worker->sem, 0, 0);
    
    pthread_mutex_lock(&pool->mutex);
    TAILQ_INSERT_TAIL(&pool->worker_queue, worker, entries);
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);
    
    return worker;
}

ce_tp_state ce_tp_worker_touch(ce_tp_worker_t * worker)
{
    if (worker == NULL) {
        ce_assert(0);
        return tp_state_invalid;
    }
    
    return worker->state;
}

void * ce_tp_worker_wait(ce_tp_worker_t * worker)
{
    if (worker == NULL) {
        ce_assert(0);
        return NULL;
    }
    
    sem_wait(&worker->sem);
    return worker->result;
}

void ce_tp_worker_free(ce_tp_worker_t * worker)
{
    ce_tp_worker_t * a_worker;
    
    pthread_mutex_lock(&worker->pool->mutex);
    TAILQ_FOREACH(a_worker, &worker->pool->worker_queue, entries) {
        if (a_worker == worker) {
            ce_assert(worker->state == tp_state_idle);
            TAILQ_REMOVE(&worker->pool->worker_queue, a_worker, entries);
            break;
        }
    }
    pthread_mutex_unlock(&worker->pool->mutex);
    
    sem_post(&worker->sem);
    sem_destroy(&worker->sem);
    ce_free(worker);
}

void ce_tp_destroy(ce_tp_t ** pool, ce_tp_kill type)
{
    if (pool == NULL || *pool == NULL || (*pool)->abort) {
        ce_assert(0);
        return;
    }
    
    if (type == tp_kill_wait) {
        while (TAILQ_FIRST(&(*pool)->worker_queue) != NULL) {
            usleep(50 * 1000);
        }
    }
    
    pthread_mutex_lock(&(*pool)->mutex);
    (*pool)->abort = true;
    pthread_cond_broadcast(&(*pool)->cond);
    pthread_mutex_unlock(&(*pool)->mutex);
    
    {
        int index;
        
        for (index = 0; index < (*pool)->nthreads; index++) {
            pthread_join((*pool)->thread[index], NULL);
        }
    }
    
    {
        ce_tp_worker_t * worker;
        
        while ((worker = TAILQ_FIRST(&(*pool)->worker_queue)) != NULL) {
            TAILQ_REMOVE(&(*pool)->worker_queue, worker, entries);
            sem_post(&worker->sem);
            sem_destroy(&worker->sem);
            ce_free(worker);
        }
    }
    
    pthread_mutex_destroy(&(*pool)->mutex);
    pthread_cond_destroy(&(*pool)->cond);
    ce_free((*pool)->thread);
    ce_freep(*pool);
}

#ifdef TEST_CE_THREADPOOL_H
#include <stdio.h>

#include "ce_threadpool.h"

void * process_cb(void * user_data)
{
    int taskid;
    taskid = (int)user_data;
    printf("taskid = %d.\n", taskid);
    Sleep(2000);
    
    return NULL;
}

void complete_cb(ce_tp_worker_t * worker, void * result, void * user_data)
{
}

int main()
{
    ce_tp_t * pool;
    
    pool = ce_tp_create(5);
    
    {
        ce_tp_process_cb pcb;
        ce_tp_complete_cb ccb;
        void * user_data;
        int tasks;
        
        pcb = process_cb;
        ccb = complete_cb;
        
        for (tasks = 0; tasks < 19; tasks++) {
            ce_tp_worker_t * worker;
            ce_tp_state state;
            void * result;
            
            user_data = (void *)tasks;
            worker = ce_tp_worker_register(pool, pcb, ccb, user_data);
            state = ce_tp_worker_touch(worker);
            
            result = NULL;
            //result = ce_tp_worker_wait(worker);
        }
        
        ce_tp_destroy(&pool, tp_kill_wait);
        printf("destroy ok.\n");
    }
}
#endif
