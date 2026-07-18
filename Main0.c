#include "Trial2.h"

int main()
{

	PROJECT_Init();
	xTaskCreate(Main_Task,"Main Controller",400, NULL,7, NULL);
	xTaskCreate(UART_Task,"UART Controller",400, NULL,7, NULL);
	xTaskCreate(LCD_Task,"LCD Controller",400, NULL,7, NULL);
	xTaskCreate(Buzzer_Task,"Buzzer Controller",400, NULL,7, NULL);

	vTaskStartScheduler();// Start the RTOS scheduler

	while(1);
	return 0;
}

/* Called by the kernel if a task overflows its stack (configCHECK_FOR_STACK_OVERFLOW > 0).
   Traps here so the offending task/name can be inspected in the debugger. */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	(void)xTask;
	(void)pcTaskName;
	taskDISABLE_INTERRUPTS();
	for(;;);
}
