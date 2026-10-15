/**************************************************************************//**
 * @file     main.c
 * @brief    
 * @version  
 * @date     
 ******************************************************************************/

#include <stdint.h>
#include "tm4c.h"


/* on-board LEDs bitmasks for Port A */
#define LED_RED   (1U << 2)     //PA2
#define LED_BLUE  (1U << 3)     //PA3
#define LED_GREEN (1U << 4)     //PA4

#define TEST_PIN  (1U << 4)

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif




int main(void)
{
    //
    // Hello!
    //
    UARTprintf("Hello, world!\n");

    RCGCGPIO |= (1U << 0); //clock, set Bit 0 for Port A
    GPIOHBCTL |= (1U << 0); //hpb, set Bit 0 for Port A


    while(1)
    {
       //red led
        GPIOA_AHB->DATA_Bits[LED_GREEN] = LED_GREEN; //happy

        //
        // Delay for a bit.
        //
        SysCtlDelay(SysCtlClockGet() / 10 / 3);

        GPIOA_AHB->DATA_Bits[LED_GREEN] = 0U;
        GPIOA_AHB->DATA_Bits[LED_RED] = LED_RED; //hungry

        //
        // Delay for a bit.
        //
        SysCtlDelay(SysCtlClockGet() / 10 / 3);

        GPIOA_AHB->DATA_Bits[LED_RED] = 0U;
        

    }


}
