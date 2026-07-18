#ifndef __TRIAL2__
#define __TRIAL2__

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

void PORTE_init(void);
void PORTF_init(void);
void UART_Init(void);
void ADC_Init(void);
void PROJECT_Init(void);

char* itoa(int value, char* buffer, int base);	//string to int function
void swap(char *x, char *y);	//itoa helping function
char* reverse(char *buffer, int i, int j);	//itoa helping function



void Buzzer_Task(void *pvParameters);
void UART_Task(void *pvParameters);
void Main_Task(void *pvParameters);
void LCD_Task(void *pvParameters);

void Delay(unsigned long counter);
char UART0_Receiver(void);
void UART0_Transmitter(unsigned char data);
void printstring(char *str);


#endif
