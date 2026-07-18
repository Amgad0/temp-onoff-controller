#include "TempController.h"
#include "uart.h"

void UART_Init(void)//UART initialization
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);            // enable clock for UART0
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);            // enable clock for Port A
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}

  GPIOPinConfigure(GPIO_PA0_U0RX);                        // PA0 -> U0RX
  GPIOPinConfigure(GPIO_PA1_U0TX);                        // PA1 -> U0TX
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

  /* Clock UART0 from the 16 MHz PIOSC so the baud rate is independent of the
     system clock, matching the original register-level configuration. */
  UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
  UARTConfigSetExpClk(UART0_BASE, 16000000, 9600,
                      UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
  UARTEnable(UART0_BASE);
}
char UART0_Receiver(void) //Receives data entered by user
{
    return (char)UARTCharGet(UART0_BASE);   /* blocks until a character is available */
}

void UART0_Transmitter(unsigned char data) //Transmits data
{
    UARTCharPut(UART0_BASE, data);          /* blocks until there is room in the TX FIFO */
}

void printstring(char *str) //print data on PC
{
  while(*str)
	{
		UART0_Transmitter(*(str++));
	}
}

void Delay(unsigned long counter) //Software Delay
{
	unsigned long i = 0;

	for(i=0; i< counter; i++);
}
