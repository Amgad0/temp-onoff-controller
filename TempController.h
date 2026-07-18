#ifndef __TEMPCONTROLLER__
#define __TEMPCONTROLLER__

#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include "FreeRTOSConfig.h"
#include "tm4c123gh6pm.h"
#include <task.h>
#include "queue.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#define PART_TM4C123GH6PM   /* required for driverlib pin_map.h to expose the PA0/PA1 UART mux macros */
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"
#include "LCD.h"
#include "TM4C123.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************
*   Board wiring — named pin/peripheral constants (driverlib style)
*******************************************************************/
/* Port F actuator outputs */
#define HEATER_PIN          GPIO_PIN_3   /* PF3 - heater relay */
#define LED_PIN             GPIO_PIN_2   /* PF2 - status LED (mirrors heater state) */
#define BUZZER_PIN          GPIO_PIN_1   /* PF1 - over-temperature buzzer */

/* Port E general-purpose outputs configured at init (PE0-2, PE4-5; PE3 is the analog input) */
#define PORTE_OUTPUT_PINS   (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5)

/* Analog temperature-sensor input */
#define TEMP_SENSOR_PIN     GPIO_PIN_3   /* PE3 / AIN0 */
#define TEMP_ADC_SEQUENCER  3            /* ADC0 sample sequencer SS3 (1 sample) */

/*******************************************************************
*   Shared inter-task queues (defined in TempController.c)
*******************************************************************/
extern QueueHandle_t xUARTQueue;   /* new setpoint, UART_Task -> Main_Task */
extern QueueHandle_t xLCDQueue;    /* measured + setpoint text, Main_Task -> LCD_Task */
extern QueueHandle_t xBuzzerQueue; /* alarm on/off flag, Main_Task -> Buzzer_Task */

void PROJECT_Init(void);

#endif
