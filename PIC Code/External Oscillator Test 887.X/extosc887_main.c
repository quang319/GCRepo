
/* Alan Hale III
 * GCRobotics
 *
 * 2013/4/15
 *
 * Code to configure and test the external oscillator
 *
 */

#include <htc.h>
#include <pic16f887.h>

// Turn on external oscillator --> FOSC_HS
__CONFIG(FOSC_INTRC_NOCLKOUT & WDTE_OFF & PWRTE_OFF & MCLRE_OFF & CP_OFF &
        CPD_OFF & BOREN_OFF & IESO_ON & FCMEN_OFF & LVP_ON);
__CONFIG(BOR4V_BOR40V & WRT_OFF);


void delay1sec();

int main()
{
    OPTION_REG = 0b10010111;
    TRISD = 0x00;
    PORTD = 0xFF;

    TMR0 = 0;

    int x = 0;

    while(1)
    {
        PORTD = x;
        x++;
        delay1sec();
    }


    return 1;
}


void delay1sec()
{
    int i = 0;
    for(i; i <= 76; i++)
    {
        while (T0IF == 0)
            asm("nop");
        T0IF = 0;
    }

}