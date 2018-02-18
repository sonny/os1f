#include <stdint.h>
#include "defs.h"
#include "kernel.h"
#include "kernel_task.h"
#include "display.h"
#include "task.h"


void os_start(void)
{
  HAL_Init();
  display_init();

  #ifdef ENABLE_FPU
  kernel_FPU_enable();
  #endif /* ENABLE_FP */
  
  kernel_task_init();
  // after here we are in user mode
}

/* static */
/* void context_switch(void * cxt) */
/* { */
/*   //uint32_t exc_return = (uint32_t)cxt; */
/*   kernel_critical_begin(); */
/*   kernel_task_save_context(0); */

/*   kernel_task_update_local_SP(); */
/*   kernel_task_schedule(); */
/*   kernel_task_wakeup(); */
/*   kernel_task_active_next(); */
/*   kernel_task_update_global_SP(); */

/*   kernel_task_load_context(); */
/*   kernel_critical_end(); */
/* } */

inline
void protected_kernel_context_switch(void * cxt)
{
  //pend_service_call(context_switch, NULL);
  pend_service_call(NULL, NULL);
}

inline
void kernel_context_switch(void)
{
  service_call(protected_kernel_context_switch, NULL);
}

void SysTick_Handler(void)
{
  HAL_IncTick();
  static int counter = 0;

  if (++counter > TIME_SLICE) {
    counter = 0;
    //    pend_service_call(context_switch, NULL);
    pend_service_call(NULL, NULL);
  }
}


