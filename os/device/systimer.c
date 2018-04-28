/*
 * systimer.c
 *
 *  Created on: Apr 22, 2018
 *      Author: sonny
 */

#include "stm32f7xx.h"
#include "defs.h"
#include "systimer.h"
#include "event.h"

static volatile uint64_t usec_counter = 0;
static volatile uint32_t msec_counter = 0;

static TIM_OC_InitTypeDef sConfig[2] =
{
{ 0 },
{ 0 } };
static TIM_HandleTypeDef TimHandle[2] =
{
{ 0 },
{ 0 } };
static TIM_MasterConfigTypeDef sMasterConfig =
{ 0 };
static TIM_SlaveConfigTypeDef sSlaveConfig =
{ 0 };
/*
 * Timers is sorted so that the next timer to
 * fire is at the head.
 */
static systimer_t * systimers = NULL;
static systimer_t * rollovers = NULL;

static void systimer_insert(systimer_t *);
static void systimer_schedule(systimer_t *);
static uint16_t systimer_current(void);
static void systimers_start(void);
static void systimers_stop(void);

/*
 * SysTimer will consist of two HW timers in master-slave mode.
 * SysTimer0 (TIM10) by default will have a 1mHz freq and be
 * used for the usec-timer. The Autoreload will be configured for
 * a 1ms period (set to 1000 us). The UpdateEvent will be set as
 * TRGO and used for the input to clock TIM9. TIM9 capture/compare
 * will be used for the systimer interrupt handler.
 *
 */

#define SYSTIMERM_PERIOD 500
#define SYSTIMERS_PERIOD 0x10000

void systimer_init(void)
{
	TimHandle[0].Instance = SYSTIMERM;
	TimHandle[1].Instance = SYSTIMERS;

	/* setup master */
	TimHandle[0].Init.Period = SYSTIMERM_PERIOD - 1; // 1ms
	TimHandle[0].Init.Prescaler = (SYSTIMERM_CLK / 1000000) * 2 - 1; // 1 us period
	TimHandle[0].Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_OC_Init(&TimHandle[0]);

	sConfig[0].OCMode = TIM_OCMODE_PWM1;
	sConfig[0].Pulse = SYSTIMERM_PERIOD / 2 - 1;
	HAL_TIM_OC_ConfigChannel(&TimHandle[0], &sConfig[0], TIM_CHANNEL_1);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	// HAL_TIMEx_MasterConfigSynchronization(&TimHandle[0], &sMasterConfig); NOTE: does not work for TIM10

	/* setup slave */
	TimHandle[1].Init.Period = SYSTIMERS_PERIOD - 1;
	TimHandle[1].Init.Prescaler = 0;
	TimHandle[1].Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_OC_Init(&TimHandle[1]);

	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
	sSlaveConfig.InputTrigger = TIM_TS_ITR2;
	sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	sSlaveConfig.TriggerFilter = 0;
	HAL_TIM_SlaveConfigSynchronization(&TimHandle[1], &sSlaveConfig);

	sConfig[1].OCMode = TIM_OCMODE_TIMING;
//	sConfig[1].Pulse      = 4;
	HAL_TIM_OC_ConfigChannel(&TimHandle[1], &sConfig[1], TIM_CHANNEL_1);

	SYSTIMERM->DIER |= 1; // enable update event interrupts
	SYSTIMERS->DIER |= 1; // enable update event interrupts
	HAL_TIM_OC_Start(&TimHandle[1], TIM_CHANNEL_1);
	HAL_TIM_OC_Start(&TimHandle[0], TIM_CHANNEL_1);
}

systimer_t * systimer_create_exec(size_t period, timer_callback callback,
		void * ctx)
{
	assert(
			period <= 0xffff
					&& "Periods greater than timer period not supported.");

	systimer_t *timer = malloc(sizeof(systimer_t));
	timer->next = NULL;
	timer->period = period;
	timer->type = TIMER_EXEC_IRQ;
	timer->callback = callback;
	timer->cb_ctx = ctx;
	timer->event = NULL;
	systimer_schedule(timer);
	systimers_start();
	return timer;
}

static inline uint16_t systimer_current(void)
{
	return SYSTIMERS->CNT;
}

static void systimer_schedule(systimer_t *t)
{
	t->exec_at = (systimer_current() + t->period) % SYSTIMERS_PERIOD;
	systimer_insert(t);
}

static void systimer_insert(systimer_t * t)
{
	size_t systime = systimer_current();
	t->next = NULL;

	systimer_t * current, * prev;
	systimer_t **head;

	if (t->exec_at < systime) {
		current = prev = rollovers;
		head = &rollovers;
	}
	else {
		current = prev = systimers;
		head = &systimers;
	}

	if (*head == NULL) {
		*head = t; // assign rollovers or systimers
		return;
	}

	// Insertion sort from here
	// Advance to the insertion point
	while (current && current->exec_at <= t->exec_at)
	{
		prev = current;
		current = current->next;
	}

	// case : insert at head
	if (current == *head)
	{
		t->next = current;
		*head = t;
	}

	// case : insert after head
	else
	{
		t->next = prev->next;
		prev->next = t;
	}
}

static void systimers_start(void)
{
	if (systimers)
	{
		SYSTIMERS->CCR1 = systimers->exec_at;
		//HAL_TIM_OC_Start_IT(&TimHandle[1], TIM_CHANNEL_1);
		__HAL_TIM_ENABLE_IT(&TimHandle[1], TIM_IT_CC1);
	}
}

static void systimers_stop(void)
{
	//HAL_TIM_OC_Stop_IT(&TimHandle[1], TIM_CHANNEL_1);
	__HAL_TIM_DISABLE_IT(&TimHandle[1], TIM_IT_CC1);
}

void systimers_slave_CC1_callback(void)
{
	assert(systimers && "Something bad happened to timer queue");
	//assert(timers->exec_at == systimer_current() && "Invalid timer");
	systimers_stop();

	size_t systime = systimer_current();

	while (systimers && systimers->exec_at == systime)
	{
		systimer_t * t = systimers;
		systimers = systimers->next;
		t->next = NULL;

		switch (t->type)
		{
		case TIMER_TIMEOUT:
			assert(0 && "Not implemented yet");
			break;
		case TIMER_EVENT:
			event_notify_irq(t->event);
			break;
		case TIMER_EXEC_IRQ:
			t->callback(t->cb_ctx);
			break;
		case TIMER_EXEC_DEF:
			assert(0 && "Not implemented yet");
			break;
		case TIMER_NONE:
		default:
			assert(0 && "Invalid timer type");
		}

		if (t->period != 0)
		{
			systimer_schedule(t);
		}
	}

	systimers_start();
}

void systimers_slave_UE_callback(void)
{
	assert(systimers == NULL && "Bad logic");
	systimers = rollovers;
	rollovers = NULL;
	systimers_start();
}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{
	SYSTIMERM_CLK_ENABLE()
	;
	SYSTIMERS_CLK_ENABLE()
	;
	HAL_NVIC_EnableIRQ(SYSTIMERM_IRQn);
	HAL_NVIC_EnableIRQ(SYSTIMERS_IRQn);
}

void SYSTIMERS_IRQHandler(void)
{
	// Check compare/capture 1 event
	if (__HAL_TIM_GET_FLAG(&TimHandle[1], TIM_FLAG_CC1))
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TimHandle[1], TIM_IT_CC1))
		{
			__HAL_TIM_CLEAR_IT(&TimHandle[1], TIM_IT_CC1);

			/* Output capture event */
			if ((SYSTIMERS->CCMR1 & TIM_CCMR1_CC1S) == 0x00)
				systimers_slave_CC1_callback();
		}
	}
	/* TIM Update event */
	if (__HAL_TIM_GET_FLAG(&TimHandle[1], TIM_FLAG_UPDATE))
	{
		if (__HAL_TIM_GET_IT_SOURCE(&TimHandle[1], TIM_IT_UPDATE))
		{
			__HAL_TIM_CLEAR_IT(&TimHandle[1], TIM_IT_UPDATE);
			systimers_slave_UE_callback();
		}
	}

}

void SYSTIMERM_IRQHandler(void)
{
	usec_counter += SYSTIMERM_PERIOD;
	__HAL_TIM_CLEAR_IT(&TimHandle[0], TIM_IT_UPDATE);
}
