#ifndef BGRT_CONFIG_H
#define BGRT_CONFIG_H

#include <libopencm3/cm3/scb.h>

#define BGRT_SC_TBL(a) const bgrt_scsr_t a
#define BGRT_SC_TBL_READ(a) a

typedef unsigned long bgrt_stack_t;

typedef unsigned long bgrt_map_t;
#define BGRT_BITS_IN_INDEX_T 		(32)

typedef unsigned long bgrt_prio_t;

typedef unsigned long bgrt_flag_t;

typedef unsigned long bgrt_st_t;

typedef unsigned long bgrt_cnt_t;
#define BGRT_CONFIG_CNT_MAX 		(0xffffffff)

typedef unsigned long bgrt_tmr_t;

typedef unsigned long bgrt_bool_t;

typedef volatile unsigned long bgrt_syscall_t;

#define BGRT_CONFIG_HARD_RT
#define BGRT_CONFIG_USE_VIC
#define BGRT_CONFIG_USER_SEARCH(map)	(__builtin_ctzl(map))

#define BGRT_SYSTEM_TIMER_ISR		sys_tick_handler
#define BGRT_SYSCALL_ISR		pend_sv_handler

#define BGRT_CONFIG_PRIO_BITS		4
#define BGRT_CONFIG_SYSCALL_PRIO 	14
#define BGRT_CONFIG_CRITSEC_PRIO 	15
#define BGRT_CONFIG_SCHED_PRIO		15

#define BGRT_PROC_STACK_SIZE		128

#define BGRT_KERNEL_PREEMPT()

#if defined(BGRT_DEBUG)

int printf_(const char* format, ...);

#define BGRT_CONFIG_printf(fmt, args...)				\
	do {                                                            \
		printf_("[%08lx] " fmt, bgrt_kernel.timer.val, ##args);	\
	} while (0)

#define BGRT_DEBUGF(fmt, args...)					\
	do {								\
		BGRT_CONFIG_printf("%s(): " fmt, __func__, ##args);	\
	} while(0)

#define BGRT_ASSERT(c, msg)						\
	do {                                                            \
		if ((c)) break;						\
		__asm__ __volatile__("cpsid i" ::: "memory");		\
		BGRT_CONFIG_printf("Assertion \"%s\" failed"		\
				   " at line %d in %s\n",		\
				   msg, __LINE__, __FILE__);		\
		__asm__ __volatile__("bkpt #0");			\
	} while(0)

#else

#define BGRT_DEBUGF(fmt, args...)
#define BGRT_ASSERT(c, msg)

#endif

#endif /*BGRT_CONFIG_H*/
