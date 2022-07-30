#include "serial_io.h"

#include <adc.h>
#include <sci.h>
#include <sys_common.h>

#include <stdio.h>

void wait(uint32 time);

int main(void)
{
    adcData_t adc_data[1];

    /* initialize gio     */
    sciInit();
    adcInit();                 

    sciMutexInit();
    
    while(1)
    {            
        adcStartConversion(adcREG1, adcGROUP1);

        /* ... wait and read the conversion count */
        while((adcIsConversionComplete(adcREG1,adcGROUP1)) == 0);
        adcGetData(adcREG1, adcGROUP1, adc_data);
           
        /* conversion results :                                       */
        /* adc_data[0] -> should have conversions for Group1 channel 6 */

        char text[38];
        snprintf(text, 38, "Ambient Light ADC Value: %d\r\n", (int)adc_data[0].value);
        sciPrintText(scilinREG, (unsigned char *)text, 38);

        wait(0xFFFFFF);
    };       
    /* NOT REACHED */   
    return 0;
}

void wait(uint32 time)
{
    while(time){time--;};
}
