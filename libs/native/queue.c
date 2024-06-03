/* -*- mode: c; tab-width: 8 -*-
 */

#include "queue.h"

bgrt_st_t bgrt_queue_init_cs(bgrt_queue_t *q, uint16_t size, uint16_t flags)
{
	BGRT_ASSERT(q, "#queue must not be NULL");
	BGRT_ASSERT(q->deq, "#queue->deq must not be NULL");
	BGRT_ASSERT(((flags & Q_CS) || q->enq.sem), "#queue->enq.sem must not be NULL");

	q->size = size -= 1;
	q->flags = flags;
	q->head = q->tail = 0;
	bgrt_sem_init_cs(q->deq, (flags & Q_REV) ? size : 0);
	if ((flags & Q_CS))
		q->enq.counter = size;
	else
		bgrt_sem_init_cs(q->enq.sem, (flags & Q_REV) ? 0 : size);

	return BGRT_ST_OK;
}

bgrt_st_t bgrt_queue_init(bgrt_queue_t *q, uint16_t size, uint16_t flags)
{
	bgrt_st_t st;
	BGRT_INT_LOCK();
	st = bgrt_queue_init_cs(q, size, flags);
	BGRT_INT_FREE();
	return st;
}

/*
 *
 */
static void * _queue_idx_inc(bgrt_queue_t *q, bool enqueue)
{
	_Atomic(uint16_t) *obj;
	uint16_t idx, next;

	enqueue ^= !!(q->flags & Q_REV);
	obj = enqueue ? &q->head : &q->tail;
	idx = *obj;

	do {
		next = (idx + 1) & q->size;
	} while(!atomic_compare_exchange_strong(obj, &idx, next));

	return &q->queue[idx];
}

/*
 *
 */
bgrt_st_t bgrt_queue_post_fn(bgrt_queue_t *q, void *msg,
			     _queue_sem_t sem_lock_fn,
			     _queue_post_cb_t post_cb)
{
	bgrt_st_t st;
	void **p;

	if ((st = sem_lock_fn(q->enq.sem)) != BGRT_ST_OK)
		return st;

	p = _queue_idx_inc(q, true);
	if (post_cb)
		post_cb(p, msg);
	else
		*p = msg;

	return bgrt_sem_free(q->deq);
}

bgrt_st_t bgrt_queue_post_cs_fn(bgrt_queue_t *q, void *msg,
				_queue_post_cb_t post_cb)
{
	void **p;

	if (q->enq.counter == 0)
		return BGRT_ST_ROLL;

	atomic_fetch_sub(&q->enq.counter, 1);

	p = _queue_idx_inc(q, true);
	if (post_cb)
		post_cb(p, msg);
	else
		*p = msg;

	return bgrt_sem_free_cs(q->deq);
}

bgrt_st_t bgrt_queue_fetch_fn(bgrt_queue_t *q, void **msg,
			      _queue_sem_t sem_lock_fn)
{
	bool q_sw = !!(q->flags & Q_SW);
	bool q_cs = !!(q->flags & Q_CS);
	bgrt_st_t st;
	void **p;
	void *t;

	if ((st = sem_lock_fn(q->deq)) != BGRT_ST_OK)
		return st;

	p = _queue_idx_inc(q, false);
	t = *p;

	if (q_sw) *p = *msg;
	*msg = t;

	return q_cs ?
		atomic_fetch_add(&q->enq.counter, 1), BGRT_ST_OK :
		bgrt_sem_free(q->enq.sem);
}
