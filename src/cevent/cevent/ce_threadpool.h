/**
 * thread pool
 *
 * Copyright (C) 2012-2013 honeysense.com
 * @author rui.sun 2013-4-12
 */
#ifndef CE_THREADPOOL_H
#define CE_THREADPOOL_H

typedef struct ce_tp_worker ce_tp_worker_t;
typedef struct ce_tp ce_tp_t;

typedef void (* ce_tp_complete_cb)(ce_tp_worker_t * worker, void * result, void * user_data);
typedef void * (* ce_tp_process_cb)(void * user_data);

typedef enum ce_tp_state {
	tp_state_idle = 0,
	tp_state_busy,
	tp_state_complete,
	tp_state_invalid
} ce_tp_state;

typedef enum ce_tp_kill {
	tp_kill_immediate = 0,
	tp_kill_wait
} ce_tp_kill;

ce_tp_t * ce_tp_create(int nthreads);
ce_tp_worker_t * ce_tp_worker_register(ce_tp_t * pool, ce_tp_process_cb process_cb, ce_tp_complete_cb complete_cb, void * user_data);
ce_tp_state ce_tp_worker_touch(ce_tp_worker_t * worker);
void * ce_tp_worker_wait(ce_tp_worker_t * worker);
void ce_tp_worker_free(ce_tp_worker_t * worker);
void ce_tp_destroy(ce_tp_t ** pool, ce_tp_kill type);

#endif