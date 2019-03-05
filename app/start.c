/*
  Main startup sequence
*/

#include <can/can_term_driver.h>
#include "terminal.h"
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "watchdog.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/


/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/


/*****************************************************************************
 * Private functions
 ****************************************************************************/
#ifdef DEVEL_BOARD
static void alive_task(void *pvParameters)
{
  (void *) pvParameters;
  while (1)
  {
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    DEBUGSTR("tick\n");
  }
}
#endif //DEVEL_BOARD


static void check_system_stack_size(void)
{
  extern unsigned long _vStackTop[], _pvHeapStart[];
  unsigned long ulInterruptStackSize;
	/* The size of the stack used by main and interrupts is not defined in
	the linker, but just uses whatever RAM is left.  Calculate the amount of
	RAM available for the main/interrupt/system stack, and check it against
	a reasonable number.  If this assert is hit then it is likely you don't
	have enough stack to start the kernel, or to allow interrupts to nest.
	Note - this is separate to the stacks that are used by tasks.  The stacks
	that are used by tasks are automatically checked if
	configCHECK_FOR_STACK_OVERFLOW is not 0 in FreeRTOSConfig.h - but the stack
	used by interrupts is not.  Reducing the conifgTOTAL_HEAP_SIZE setting will
	increase the stack available to main() and interrupts. */
	ulInterruptStackSize = ( ( unsigned long ) _vStackTop ) - ( ( unsigned long ) _pvHeapStart );
	configASSERT( ulInterruptStackSize > 350UL );

}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

int main(void)
{
	__disable_irq();

	// Set up and initialize all required blocks and
  // functions related to the board hardware
  Board_Init();

  #ifdef DEVEL_BOARD
  xTaskCreate(alive_task, "alv_tsk",configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL), NULL);
  #endif

  CAN_init();
  terminal_init();
  WDT_Init();

  check_system_stack_size();

  __enable_irq();

  WDT_Feed();

  /* Start the kernel.  From here on, only tasks and interrupts will run. */
  vTaskStartScheduler();

  return 1;
}

/* Used for run time statistics */
void vConfigureTimerForRunTimeStats(void)
{
  /* Power up and feed the timer with a clock. */
  Chip_TIMER_Init(LPC_TIMER32_0);
  Chip_TIMER_TIMER_SetCountClockSrc(LPC_TIMER32_0, TIMER_CAPSRC_RISING_PCLK, 1);

  /* Reset Timer 0 */
  Chip_TIMER_Reset(LPC_TIMER32_0);

  /* Prescale to a frequency that is good enough to get a decent resolution,
  but not too fast so as to overflow all the time. */
  Chip_TIMER_PrescaleSet(LPC_TIMER32_0, ( configCPU_CLOCK_HZ / 10000UL ) - 1UL);

  /* Start the counter. */
  Chip_TIMER_Enable(LPC_TIMER32_0);
}

void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */

  WDT_Feed(); // Feed HW watchdog
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationTickHook( void )
{
	/* This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
}

