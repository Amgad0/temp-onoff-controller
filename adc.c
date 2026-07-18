#include "TempController.h"
#include "adc.h"

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
