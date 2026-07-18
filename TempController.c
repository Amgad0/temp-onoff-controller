#include "TempController.h"
#include "gpio_init.h"
#include "uart.h"
#include "adc.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Project
	QueueHandle_t xUARTQueue;
	QueueHandle_t xLCDQueue;
	QueueHandle_t xBuzzerQueue;

void PROJECT_Init(void)
{

	PORTF_init();
	PORTE_init();
	ADC_Init();
	UART_Init();
	xUARTQueue= xQueueCreate(1,1); //used by main task and UART task
	xLCDQueue= xQueueCreate(1,8); //used by main task and LCD task
	xBuzzerQueue= xQueueCreate(1,1); //used by main task and buzzer task
}
