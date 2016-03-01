/*
  OSv09
  main.c
 */

#include <stdlib.h>
#include "stm32f746xx.h"
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_lcd.h"

#include "task.h"
#include "os.h"
#include <stdio.h>
#include "lcd.h"
#include "fonts.h"

struct task_data {
  uint32_t interval;
};

void task(void *p);
void adc_task(void *p);
static void LCD_Config(void);

static struct task_data td[4] = {
  { interval:500},
  { interval:250},
  { interval:125},
  { interval:800}
};

int main()
{
  osInit();
  LCD_Config();

  task_start(task, 0, &td[0]);
  task_start(task, 0, &td[1]);
  task_start(task, 0, &td[2]);
  task_start(adc_task, 512, NULL);
  

  /* Our main starts here */
  uint16_t ypos = 0, ymax = 0;
  int8_t yincr = 1;
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&Font16);
  
  int count = 0;
  while (1) {
    uint32_t now = HAL_GetTick();
    lcd_printf_at(10, 10, "Task Main: %6d", count++);
    task_sleep_until(now + 1000);
  }
  return 0;

}
void task(void *p)
{
  struct task_data *q = p;
  uint32_t counter = 0;
  
  int id = task_current()->id;
  int xpos = 10, ypos = (id*20 + 20);
  
  while(1)
    {
      uint32_t now = HAL_GetTick();
      lcd_printf_at(xpos, ypos, "Task   %2d: %6d", id, counter++);
      task_sleep_until(now + q->interval);
    } 
}

static void LCD_Config(void)
{
  /* LCD Initialization */
  BSP_LCD_Init();

  /* LCD Initialization */
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

  /* Enable the LCD */
  BSP_LCD_DisplayOn();

  /* Select the LCD Background Layer  */
  BSP_LCD_SelectLayer(0);

  /* Clear the Background Layer */
  BSP_LCD_Clear(LCD_COLOR_BLACK);

  /* Configure the transparency for background */
  BSP_LCD_SetTransparency(0, 100);
}
