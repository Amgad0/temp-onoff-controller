#include "TempController.h"
#include "gpio_init.h"

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
