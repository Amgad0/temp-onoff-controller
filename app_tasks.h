#ifndef __APP_TASKS__
#define __APP_TASKS__

void Buzzer_Task(void *pvParameters);
void UART_Task(void *pvParameters);
void Main_Task(void *pvParameters);
void LCD_Task(void *pvParameters);

char* itoa(int value, char* buffer, int base);	//string to int function
void swap(char *x, char *y);	//itoa helping function
char* reverse(char *buffer, int i, int j);	//itoa helping function

#endif
