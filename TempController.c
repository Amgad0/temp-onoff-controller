#include "TempController.h"
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
//initialising portE
void PORTE_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);            // enable clock to Port E
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) {}   // wait for clock stability
  GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, PORTE_OUTPUT_PINS);   // PE0-2, PE4-5 as digital outputs
}

void PORTF_init(void)
{
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            // enable clock to Port F
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}   // wait for clock stability
  GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, HEATER_PIN | LED_PIN | BUZZER_PIN); // PF1-3 outputs
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this is the main task that controls the oven temperature. This task reads the analog temperature
from the LM35 temperature sensor chip which is connected to (port pin
PE5) of the microcontroller. the alarm value is set to 70�C. New setpoint values are received through the
xUARTQueue from Uart Task. The relay at port pin PF3 is configured as an output and also the
LED at port pin PF2 is configured as an output.
If the measured temperature is less than the setpoint then the relay is activated
to turn ON the heater. If on the other hand the measured temperature is greater than the
setpoint then the relay is de-activated to turn OFF the heater. The measured temperature is
compared with the pre-defined alarm value and if it is higher than the alarm value, then a
1 is sent to Task 4 (Buzzer Controller) using the xBuzzerQueue so that Buzzer Task activates the
buzzer. If the measured temperature is less than the alarm value, then a 0 is sent to Buzzer Task to
stop the buzzer. The measured and the setpoint temperature values are sent to the LCD via
the xLCDQueue so that they can be displayed on the LCD. A structure is created to store the
measured and the setpoint temperature values in character arrays Txt1 an Txt2 respectively.*/

void Main_Task(void *pvParameters) //Main Controller Task
{
	//character arrays to store the measured and setpoint values
	typedef struct Message
	{
		unsigned char Txt1[4];
		unsigned char Txt2[4];
	} AMessage;

	AMessage msg;




	unsigned char setpoint=25;   // the setpoint entered by user (default until UART sends one)
	unsigned char Temperature; //the actual measured temperature
	float mV;
	float adc_value;
	uint32_t adcRaw;   //raw 12-bit ADC sample from SS3
	unsigned char AlarmValue=70; //the alarm value which controls the buzzer
	int state;
	state=0;


	int on=1;   //to indicate that buzzer should be turned ON
	int off=0;  //to indicate that buzzer should be turned OFF

	while(1)
 {
		xQueueReceive(xUARTQueue, &setpoint,0);  //Receive the setpoint entered by user through uart

				ADCProcessorTrigger(ADC0_BASE, TEMP_ADC_SEQUENCER);            /* start a conversion on AIN0 */
        while(!ADCIntStatus(ADC0_BASE, TEMP_ADC_SEQUENCER, false)) ;   /* wait until conversion completes */
        ADCSequenceDataGet(ADC0_BASE, TEMP_ADC_SEQUENCER, &adcRaw);    /* read result from SS3 FIFO */
        ADCIntClear(ADC0_BASE, TEMP_ADC_SEQUENCER);                    /* clear the conversion flag */
        adc_value = (float)adcRaw;
				mV= ( (adc_value/4096.0)*3300.0 ) ; //mV value
				Temperature=(int)(mV/10.0); //Temp as integer


			if (Temperature > (setpoint+2) ){
				state=1;
				GPIOPinWrite(GPIO_PORTF_BASE, HEATER_PIN | LED_PIN, 0);                     // heater + LED OFF
			}
			else if(Temperature<(setpoint-1)){
				state=0;
				GPIOPinWrite(GPIO_PORTF_BASE, HEATER_PIN | LED_PIN, HEATER_PIN | LED_PIN);  // heater + LED ON
			}
			if(Temperature>=(setpoint-1) && Temperature <= (setpoint+2) && state==0)
			{
				GPIOPinWrite(GPIO_PORTF_BASE, HEATER_PIN | LED_PIN, HEATER_PIN | LED_PIN);  // hold ON inside hysteresis band
			}
			else if (Temperature>=(setpoint-1) && Temperature <= (setpoint+2) && state==1)
			{
				GPIOPinWrite(GPIO_PORTF_BASE, HEATER_PIN | LED_PIN, 0);                     // hold OFF inside hysteresis band
			}



			itoa(Temperature, msg.Txt1,10); //Measured Value
			itoa(setpoint, msg.Txt2,10); //setpoint Value
			xQueueSend(xLCDQueue, &msg,0);

	if(Temperature> AlarmValue) //Alarm??
	{
		xQueueSend(xBuzzerQueue, &on,0); //Buzzer On
	}
	else{
		xQueueSend(xBuzzerQueue, &off,0); //Buzzer Off
	}

  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*This is the Buzzer Controller task. At the beginning of this task, port pin PF1 is configured
as output. The task receives the buzzer state through the queue xBuzzerQueue. If 1 is
received, then an alarm condition is assumed and the buzzer is activated. If on the other hand
0 is received, then the buzzer is de-activated.*/

void Buzzer_Task(void *pvParameters)//Buzzer Task //PF1
{
	char BuzzerState;
	BuzzerState=0;

	while(1)
	{
		xQueueReceive(xBuzzerQueue,&BuzzerState,0); //Get Data from main task

		if(BuzzerState==1)//Alarm?
		{
			GPIOPinWrite(GPIO_PORTF_BASE, BUZZER_PIN, BUZZER_PIN); //Buzzer is turned ON
		}
		else{
			GPIOPinWrite(GPIO_PORTF_BASE, BUZZER_PIN, 0); //Buzzer is turned OFF
		}

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

/*This is the LCD Controller task. At the beginning of this task the LCD is initialized, display
is cleared, and the cursor is set to be OFF so that it is not visible. The task receives the measured
and the setpoint temperature values through queue xLCDQueue in the form of a structure.
Character arrays Txt1 and Txt2 store the measured and the setpoint temperature values
respectively. The LCD display is refreshed every second.*/

void LCD_Task(void *pvParameters)
{
	typedef struct Message
		{
			char Txt1[4];
			char Txt2[4];
		}Amessage;

		Amessage msg;
	LCD LCD;
	LCD=LCD_create();
	LCD_setup(&LCD);
	LCD_init(&LCD);

		while(1)
		{
			xQueueReceive(xLCDQueue, &msg,0); //Receive Data from main task
			LCD_clear(&LCD);
			LCD_setPosition(&LCD,1,1);
			LCD_sendString(&LCD, "Measured: "); //Heading
			LCD_sendString(&LCD,msg.Txt1); //display temperature
			LCD_sendByte(&LCD,1,(char)223);
			LCD_sendString(&LCD, "C"); //degree unit
			LCD_setPosition(&LCD,2,1);
			LCD_sendString(&LCD, "Setpoint: "); //Heading
			LCD_sendString(&LCD,msg.Txt2); //display setpoint
			LCD_sendByte(&LCD,1,(char)223);
			LCD_sendString(&LCD, "C"); //degree unit
			vTaskDelay(pdMS_TO_TICKS(1000)); //wait one second
		}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

/*This is the UART Controller task. At the beginning of this task, UART is initialized to operate
at 9600 baud. The message Enter Temperature Setpoint (Degrees): is displayed on the PC
screen using the Putty terminal emulation software. The required setpoint value (in integer)
is read from the keyboard and is sent to Task 1 through xUARTQueue. Then the message
Temperature setpoint changed� is displayed on the PC screen.*/

void UART_Task(void *pvParameters)
{
 unsigned char N;
 //unsigned AdcValue;
 unsigned char Total;
 printstring(" Hello!! ");
 while (1)
	 {

		 printstring("\n\r\n\r Please Enter Temperature Setpoint (Degrees): ");
		 N=0;
		 Total=0;
		 while(1)
		 {
			 N= UART0_Receiver();          //get a character from UART0
		   UART0_Transmitter(N);         // echo that character

			 if (N == '\n') //if Enter break from this while loop
				 {
				 break;
				 }
       N= N-'0';     //Pure number
			 Total = 10*Total + N; //Total number

		 }

    xQueueSend(xUARTQueue, &Total, pdMS_TO_TICKS(30)); //send via queue
		printstring("\n\r Temperature Setpoint changed.. ");
	}

}


// Function to swap two numbers
void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}

// Function to reverse `buffer[i�j]`
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

// Iterative function to implement `itoa()` function in C
char* itoa(int value, char* buffer, int base)
{
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }

    // consider the absolute value of the number
    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }

        n = n / base;
    }

    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }

    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

void ADC_Init(void){
    /* Enable clock to ADC0 and Port E */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    /* PE3 / AIN0 as analog input */
    GPIOPinTypeADC(GPIO_PORTE_BASE, TEMP_SENSOR_PIN);

    /* Sample sequencer SS3: software (processor) trigger, single sample on channel 0 */
    ADCSequenceDisable(ADC0_BASE, TEMP_ADC_SEQUENCER);
    ADCSequenceConfigure(ADC0_BASE, TEMP_ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, TEMP_ADC_SEQUENCER, 0,
                             ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); /* set flag on the sample, last step */
    ADCSequenceEnable(ADC0_BASE, TEMP_ADC_SEQUENCER);
}