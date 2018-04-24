/*
 * virt_led.c
 *
 *  Created on: Apr 24, 2018
 *      Author: sonny
 */
#include <stdint.h>
#include "stm32746g_discovery_lcd.h"
#include "virtled.h"
#include "lcd.h"

static void virtled_draw(int i);

#define MAX_VIRTLED_COUNT 16
#define VIRTLED_WIDTH  20
#define VIRTLED_HEIGHT 20

typedef enum
{
	VIRTLED_OFF = 0, VIRTLED_ON = 1
} virtled_state_e;

typedef struct
{
	uint16_t xpos;
	uint16_t ypos;
	uint32_t color;
	virtled_state_e state;
} virtled_t;

static virtled_t virtleds[MAX_VIRTLED_COUNT];
static uint32_t virtled_colors[] =
{
LCD_COLOR_BLUE,
LCD_COLOR_GREEN,
LCD_COLOR_RED,
LCD_COLOR_CYAN,
LCD_COLOR_MAGENTA,
LCD_COLOR_YELLOW, };

static const int virtled_color_count = sizeof(virtled_colors)
		/ sizeof(virtled_colors[0]);

void virtled_init(void)
{
	int i;
	int ypos = BSP_LCD_GetYSize() - VIRTLED_HEIGHT;

	for (i = 0; i < MAX_VIRTLED_COUNT; ++i)
	{
		int xpos = i * VIRTLED_WIDTH;
		virtleds[i].color = virtled_colors[i % virtled_color_count];
		virtleds[i].xpos = xpos;
		virtleds[i].ypos = ypos;

		virtled_set(i);
		virtled_draw(i);
		virtled_reset(i);
		virtled_draw(i);
	}
}

void virtled_set(int i)
{
	virtleds[i].state = VIRTLED_ON;
	virtled_draw(i);
}

void virtled_reset(int i)
{
	virtleds[i].state = VIRTLED_OFF;
	virtled_draw(i);
}

void virtled_toggle(int i)
{
	virtleds[i].state ^= VIRTLED_ON;
	virtled_draw(i);
}

static void virtled_draw(int i)
{
	int xpos = virtleds[i].xpos;
	int ypos = virtleds[i].ypos;
	int w = VIRTLED_WIDTH;
	int h = VIRTLED_HEIGHT;

	uint32_t bgcolor = BSP_LCD_GetBackColor();
	uint32_t ledcolor = virtleds[i].color;
	uint32_t txtcolor = bgcolor;

	char c = i < 10 ? i + '0' : i + 'A' - 10;
	int cxpos = xpos + 7;
	int cypos = ypos + 5;

	if (virtleds[i].state == VIRTLED_OFF)
	{
		xpos += 2;
		ypos += 2;
		w -= 4;
		h -= 4;
		ledcolor = bgcolor;
		txtcolor = virtleds[i].color;
	}

	if (spinlock_try_lock(lcd_lock))
	{
		BSP_LCD_SetTextColor(ledcolor);
		BSP_LCD_FillRect(xpos, ypos, w, h);
		BSP_LCD_SetTextColor(txtcolor);
		BSP_LCD_SetBackColor(ledcolor);
		BSP_LCD_DisplayChar(cxpos, cypos, c);
		BSP_LCD_SetBackColor(bgcolor);
		spinlock_unlock(lcd_lock);
	}

}
