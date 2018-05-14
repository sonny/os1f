/*
 * systimer.c
 *
 *  Created on: Apr 22, 2018
 *      Author: sonny
 */

#include "defs.h"
#include "systimer.h"
#include "event.h"
#include "assertions.h"
#include <malloc.h>
#include <string.h>

static volatile uint64_t usec_counter = 0;
static volatile uint32_t msec_counter = 0;

/*
 * Timers is sorted so that the next timer to
 * fire is at the head.
 */

static list_t timers[2] = {
		LIST_STATIC_INIT(timers[0]),
		LIST_STATIC_INIT(timers[1]),
};

static list_t * systimers = &timers[0];
static list_t * rollovers = &timers[1];

static void systimer_config_timers(void);
static void systimer_insert(systimer_t *);
static void systimer_remove(systimer_t * t);

static void systimer_schedule(systimer_t *);
static uint16_t systimer_current(void);
static void systimer_schedule_next(void);
static void systimer_disable_all(void);
static void systimer_start_protected(void*);
static void systimer_stop_protected(void*);
static systimer_t * list_to_timer(list_t *);
static list_t * timer_to_list(systimer_t *);

#define SYSTIMERM_PERIOD 1000
#define SYSTIMERS_PERIOD 0x10000

#define list_to_timer(list) ((systimer_t*)(list))
#define timer_to_list(timer) (&(timer)->node)

uint32_t msec_time(void)
{
	return msec_counter + SYSTIMERS->CNT;
}

uint64_t usec_time(void)
{
	return usec_counter + (SYSTIMERS->CNT * 1000) + SYSTIMERM->CNT;
}


void systimer_init(void)
{
	systimer_config_timers();
}

void systimer_start(systimer_t * t)
{
	service_call(systimer_start_protected, t, false);
}

static
void systimer_start_protected(void * ctx)
{
	systimer_t * t = ctx;
	__disable_irq();
	systimer_schedule(t);
	systimer_schedule_next();
	__enable_irq();
}

void systimer_stop(systimer_t * t)
{
	service_call(systimer_stop_protected, t, false);
}

static
void systimer_stop_protected(void * ctx)
{
	systimer_t * t = ctx;
	__disable_irq();
	systimer_remove(t);
	systimer_schedule_next();
	__enable_irq();
}

static inline
systimer_t * systimer_allocate(void)
{
	systimer_t *timer = malloc(sizeof(systimer_t));
	memset(timer, 0, sizeof(systimer_t));
	list_init(&timer->node);
	return timer;
}

void systimer_destroy(systimer_t * timer)
{
	systimer_stop(timer);
	free(timer);
}

systimer_t * systimer_create_event_onetime(size_t period, event_t * event)
{
	systimer_t * t = systimer_allocate();
	t->period = period;
	t->type = TIMER_EVENT_ONCE;
	t->event = event;
	systimer_start(t);
	return t;
}

systimer_t * systimer_create_exec(size_t period, timer_callback callback,
		void * ctx)
{
	assert(period <= 0xffff	&& "Periods greater than timer period not supported.");

	systimer_t * t = systimer_allocate();
	t->period = period;
	t->type = TIMER_EXEC_IRQ;
	t->callback = callback;
	t->cb_ctx = ctx;
	systimer_start(t);
	return t;
}

static inline
uint16_t systimer_current(void)
{
	return SYSTIMERS->CNT;
}

static void systimer_schedule(systimer_t *t)
{
	assert_protected();
	t->exec_at = (systimer_current() + t->period) % SYSTIMERS_PERIOD;
	systimer_insert(t);
}

static bool systimer_insert_condition(list_t * node, list_t * new)
{
	// true if current node is executed later than new node
	return (list_to_timer(node)->exec_at > list_to_timer(new)->exec_at);
}

static void systimer_insert(systimer_t * t)
{
	assert_protected();
	size_t systime = systimer_current();
	list_t * head;
	if (t->exec_at < systime)
		head = rollovers;
	else
		head = systimers;

	list_insert_condition(head, timer_to_list(t), systimer_insert_condition);
}

static
void systimer_remove(systimer_t * t)
{
	assert_protected();
	list_remove(timer_to_list(t));
}

static inline
void systimer_schedule_next(void)
{
	assert_protected();
	if (!list_empty(systimers))
	{
		SYSTIMERS->CCR1 = list_to_timer(list_head(systimers))->exec_at;
		SYSTIMERS->DIER |= TIM_IT_CC1;
	}
}

static inline
void systimer_disable_all(void)
{
	assert_protected();
	SYSTIMERS->DIER &= ~TIM_IT_CC1;
}

void systimers_slave_CC1_callback(void)
{
	assert(systimers && "Something bad happened to timer queue");
	__disable_irq();
	systimer_disable_all();

	size_t systime = systimer_current();

	while (!list_empty(systimers) &&
			list_to_timer(list_head(systimers))->exec_at == systime)
	{
		systimer_t * t = list_to_timer(list_removeFront(systimers));

		__enable_irq(); // not protected

		switch (t->type)
		{
		case TIMER_TIMEOUT:
			assert(0 && "Not implemented yet");
			break;
		case TIMER_EVENT:
		case TIMER_EVENT_ONCE:
			event_notify_irq(t->event);
			break;
		case TIMER_EXEC_IRQ:
		case TIMER_EXEC_IRQ_ONCE:
			t->callback(t->cb_ctx);
			break;
		case TIMER_EXEC_DEF:
		case TIMER_EXEC_DEF_ONCE:
			assert(0 && "Not implemented yet");
			break;
		case TIMER_NONE:
		default:
			assert(0 && "Invalid timer type");
		}

		__disable_irq();

		if (t->type == TIMER_EVENT    ||
			t->type == TIMER_EXEC_IRQ ||
			t->type == TIMER_EXEC_DEF )
		{
			systimer_schedule(t);
		}
	}

	systimer_schedule_next();
	__enable_irq();
}

void systimers_slave_UE_callback(void)
{
	msec_counter += SYSTIMERS_PERIOD;
	usec_counter += SYSTIMERS_PERIOD * SYSTIMERM_PERIOD;

	assert(list_empty(systimers) && "Bad Logic");
	__disable_irq();
	list_t * temp = rollovers;
	rollovers = systimers;
	systimers = temp;
	systimer_schedule_next();
	__enable_irq();
}

void SYSTIMERS_IRQHandler(void)
{
	// Check compare/capture 1 event
	if (SYSTIMERS->SR & TIM_FLAG_CC1)
	{
		systimers_slave_CC1_callback();
		SYSTIMERS->SR &= ~TIM_IT_CC1;
	}
	/* TIM Update event */
	if (SYSTIMERS->SR & TIM_FLAG_UPDATE)
	{
		systimers_slave_UE_callback();
		SYSTIMERS->SR &= ~TIM_IT_UPDATE;
	}

}

static void systimer_config_timers(void)
{
	/* Enable RCC and IRQs */
	SYSTIMERM_CLK_ENABLE();
	SYSTIMERS_CLK_ENABLE();
	HAL_NVIC_EnableIRQ(SYSTIMERS_IRQn);

	/* setup master */
	SYSTIMERM->CR1 = TIM_CR1_URS | TIM_COUNTERMODE_UP;
	SYSTIMERM->ARR = SYSTIMERM_PERIOD - 1;          // 1ms
	SYSTIMERM->PSC = (SYSTIMERM_CLK / 1000000) - 1; // 1 us period
	SYSTIMERM->DIER = 0;
	SYSTIMERM->EGR = TIM_EGR_UG;

	// Configure Master Capture/Compare
	SYSTIMERM->CCMR1 |= TIM_OCMODE_PWM1;
	SYSTIMERM->CCR1 = SYSTIMERM_PERIOD / 2 - 1;
	SYSTIMERM->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);

	/* setup slave */
	SYSTIMERS->CR1 = TIM_CR1_URS | TIM_COUNTERMODE_UP;
	SYSTIMERS->ARR = SYSTIMERS_PERIOD - 1;
	SYSTIMERS->PSC = 0;
	SYSTIMERS->DIER = TIM_DIER_UIE | TIM_IT_CC1;
	SYSTIMERS->EGR = TIM_EGR_UG;

	// Configure Slave Capture/Compare
	SYSTIMERS->CCMR1 |= TIM_OCMODE_TIMING;
	SYSTIMERS->CCER |= (uint32_t)(TIM_CCx_ENABLE << TIM_CHANNEL_1);

	// Configure Slave Mode
	SYSTIMERS->SMCR = TIM_TS_ITR2 | TIM_SLAVEMODE_EXTERNAL1;

	// Enable both timers
	SYSTIMERS->CR1 |= TIM_CR1_CEN;
	SYSTIMERM->CR1 |= TIM_CR1_CEN;
}
