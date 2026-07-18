#ifndef __UART_DRIVER__
#define __UART_DRIVER__

void UART_Init(void);
char UART0_Receiver(void);
void UART0_Transmitter(unsigned char data);
void printstring(char *str);
void Delay(unsigned long counter);

#endif
