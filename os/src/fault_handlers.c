#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdio.h>

extern void printmsg(char *m);

void HardFault_Handler(void)
{
  static char buffer[64];
  printmsg("Hard Fault Handler\n");
  snprintf(buffer, 64, "Hard Fault HFSR 0x%x\n", SCB->HFSR);
  printmsg(buffer);
           
  if ((SCB->HFSR & (1<<30)) != 0) {
    snprintf(buffer, 64, "Hard Fault is Forced CFSR 0x%x\n", SCB->CFSR);
    printmsg(buffer);
 }
  
  __asm volatile("BKPT #01");
  while(1);
}

void UsageFault_Handler(void)
{
  static volatile char * name = "USAGE FAULT HANDLER";
  while(1);
}

void BusFault_Handler(void)
{
  static volatile char * name = "Bus FAULT HANDLER";
  while(1);
}

void MemFault_Handler(void)
{
  static volatile char * name = "MEM FAULT HANDLER";
  while(1);
}

void NMI_Handler(void)
{
  printmsg("NMI Handler\n");
  while(1);
}
