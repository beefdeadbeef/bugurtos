#ifndef QUEUE_H
#define QUEUE_H

#include <bugurt.h>
#include <stdatomic.h>
BGRT_CDECL_BEGIN
#include "sem.h"

typedef enum {
	Q_REV	= (1 << 0),	/* reverse indices */
	Q_SW	= (1 << 1),	/* swap on fetch */
	Q_CS	= (1 << 2),	/* enqueue from irq context */
} q_flags;

struct bgrt_priv_queue_t
{
	uint16_t flags;
	uint16_t size;
	_Atomic(uint16_t) head;
	_Atomic(uint16_t) tail;
	union {
		_Atomic(uint32_t) counter;
		bgrt_sem_t *sem;
	} enq;
	bgrt_sem_t *deq;
	void *queue[];
};

#define BGRT_Q_HDR_SZ (offsetof(struct bgrt_priv_queue_t, queue))
#define BGRT_Q_SIZEOF(x) (BGRT_Q_HDR_SZ + (x) * sizeof(void *))

typedef struct bgrt_priv_queue_t bgrt_queue_t;

typedef bgrt_st_t (*_queue_sem_t)(bgrt_sem_t *sem);
typedef void (*_queue_post_cb_t)(void **qentry, void *msg);

/*
 *
 */
bgrt_st_t bgrt_queue_init(bgrt_queue_t *q, uint16_t size, uint16_t flags);

bgrt_st_t bgrt_queue_init_cs(bgrt_queue_t *q, uint16_t size, uint16_t flags);

/*
 *
 */
bgrt_st_t bgrt_queue_post_fn(bgrt_queue_t *q, void *msg,
			     _queue_sem_t sem_fn,
			     _queue_post_cb_t cb_fn);

bgrt_st_t bgrt_queue_post_cs_fn(bgrt_queue_t *q, void *msg,
				_queue_post_cb_t cb_fn);

#define bgrt_queue_post(q, msg)				\
	bgrt_queue_post_fn(q, msg, bgrt_sem_lock, NULL)

#define bgrt_queue_trypost(q, msg)				\
	bgrt_queue_post_fn(q, msg, bgrt_sem_try_lock, NULL)

#define bgrt_queue_trypost_cs(q, msg)			\
	bgrt_queue_post_cs_fn(q, msg, NULL)

#define bgrt_queue_post_cb(q, msg, cb)			\
	bgrt_queue_post_fn(q, msg, bgrt_sem_lock, cb)

#define bgrt_queue_trypost_cb(q, msg, cb)		\
	bgrt_queue_post_fn(q, msg, bgrt_sem_try_lock, cb)

#define bgrt_queue_trypost_cb_cs(q, msg, cb)		\
	bgrt_queue_post_cs_fn(q, msg, cb)

/*
 *
 */
bgrt_st_t bgrt_queue_fetch_fn(bgrt_queue_t *q, void **msg,
			      _queue_sem_t sem_fn);

#define bgrt_queue_swap bgrt_queue_fetch

#define bgrt_queue_fetch(q, msg)		\
	bgrt_queue_fetch_fn(q, msg, bgrt_sem_lock)

#define bgrt_queue_try_fetch(q, msg)		\
	bgrt_queue_fetch_fn(q, msg, bgrt_sem_try_lock)

BGRT_CDECL_END
#endif // QUEUE_H
