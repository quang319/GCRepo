/* Alan Hale III
 * GCRobotics
 *
 * Created on February 20, 2013, 7:40 PM
 *
 * GCRobotics Primary PIC main function
 * Controls the aspects of the PICs that are in charge of
 *  controlling the motors and sending feedback to the CPU,
 *  namely the PWM, I2C, and Encoder functions.
 *
 *
 * April 15, 2013, 11:20 AM
 * Created version of project compatible with PIC16F887, which for our
 *  purposes is the same as the 16F917
 *
 */

#include <pic16f887.h>          // 16F887 header file, for all
                                //  processor-specific declarations
#include <htc.h>                // htc.h necessary for PIC16F917 Configuration
                                //  Bit Settings
#include "pwmlib887.h"             // allows for use of PWM module
                                //  (use double quotes for user-defined)
#include "enclib887.h"             // allows for use of Encoder module
#include "i2cSlave887.h"             // allows for use of I2C module

// Configuration bit settings
// Use 20MHz external oscillator --> FOSC_HS
// Turn on external oscillator --> FOSC_HS
  // NOTE:  MCLRE must be set to ON because of this family of PIC (887.
  //  Must be used in conjunction with the MCLR pin via weak pullup.
  //  See section 14.2.2, page 211, of the 887 datasheet.
__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_ON & MCLRE_ON &
        CP_OFF & CPD_OFF & BOREN_OFF & IESO_OFF & FCMEN_OFF & LVP_OFF);
__CONFIG(BOR4V_BOR40V & WRT_OFF);



// Pre-processor definitions that specify an individual PIC.  That is, all
//  the main code can be exactly the same, from PIC to PIC for the robot,
//  depending on the motor position (front left motor, for example), only
//  these definitions will need to change

// Motor 1:  Front Right.  Address = 0x02.  Forward = 1.
// Motor 2:  Back Right.  Address = 0x04.  Forward = 1.
// Motor 3:  Back Left.  Address = 0x06.  Forward = 0.
// Motor 4:  Front Left.  Address = 0x08.  Forward = 0.

#define I2C_ADDRESS 0x02        // I2C address; unique to specific PIC
#define FORWARD 1               // PIC specific depending on wheel orientation
#define BACKWARD !FORWARD       // ^


#define CYCLES_PER_REV 650      // Should be nearly the same for all PICs,
                                //  but again could vary across motors
#define PWM_FOR_RPS 125         // The PWM pulse width that is the closest
                                //  for achieving 1 revolution per second
#define FLAG_ADDRESS 0xAA       // Address for user defined flag register
                                //  According to datasheet, 0xAA is free
#define KP 3.0                  // PID P coefficient
#define KI 0.4                  // PID I coefficient
#define KD 0                    // PID D coefficient
#define KPID 1.0                // PID cycle/s to PWM dampening factor



// Function Prototypes
void Initialise();              // contains all initializing functions
void interrupt isr();           // general interrupt vector
void UpdateData(int c);         // takes in most recent count measurements and
                                //  adds them to current total
void setDirection(int dir);     // Sets the direction bit (PORTB bit 3)
int abs(int a, int b);          // Returns the absolute value of the difference
                                //  between two numbers

//Test Functions
void delay(int length);
void CalcPulse(int speed);
void delay1sec();



//Global Variables
int TARGET = 0;                 // target speed passed down from CPU
int DIRECTION = FORWARD;        // target derection passed down from CPU
int DIR_READ = FORWARD;         // value read from encoder flip-flop used
                                //  to keep track of current direction

int COUNTS = 0;                 // TMR1 encoder counts --> passed to CPU

int TMR0_OverflowCount = 0;     // variables that makes the time between PID
int TMR0_OverflowTarget = 8;    //  loop executions dynamic, based upon the
                                //  desired number of counts passed from CPU

// Register that holds flags that are set in software upon determination of
//  the cause of an interrupt.  These flags are continuously checked in the
//  main program.  Some may be unimplemented.
unsigned char FLAG @ FLAG_ADDRESS;
bit I2C     @ ((unsigned)&FLAG*8)+0;    // I2C flag
bit T0      @ ((unsigned)&FLAG*8)+1;    // TMR0 flag
bit T1      @ ((unsigned)&FLAG*8)+2;    // TMR1 flag
bit DIR     @ ((unsigned)&FLAG*8)+3;    // Direction flag
bit ERR     @ ((unsigned)&FLAG*8)+4;    // H-Bridge Error flag
union {
    struct {
        unsigned    I2C     : 1;
        unsigned    T0      : 1;
        unsigned    T1      : 1;
        unsigned    DIR     : 1;
        unsigned    ERR     : 1;
        unsigned            : 3;
    };
}F @ FLAG_ADDRESS;


//-----------------------------------------------------------------------------
// MAIN STARTS HERE -----------------------------------------------------------
int main()
{
    // Variables for PID
    int counts = 0,                 // number of counts since last PID loop
        P,                          // error variable
        I,                          // integral variable
        D,                          // differential variable
        P_old = 0,                  // old error variable
        PID;                        // sum of P, I, and D values

    int currentPWM = 0;             // current pulse width pushed to PWM


    // BEGIN
    Initialise();

    TARGET = 0;
    SetPulse(0);
    T0IE = 0;
    delay1sec();                    // for testing purposes
    T0IE = 1;
    TMR1 = 0;

    while(1)
    {
        /*/ Some test code
        TMR1 = 0;
        SetPulse(75);
        for (P = 0; P < 10; P++)
            delay1sec();
        SetPulse(0);
        asm("nop");     //*/

	PORTD = TMR1;
//        SetPulse(150);
//        setDirection(FORWARD);




        if (F.I2C == 1)
        {
            // Perform some I2C operation


            // If read, reset Data and PID information

            // Perform operation for TARGET and TMR0_OverflowTarget here

            TARGET = i2cSpeed;
            setDirection(i2cDirection);
            SetPulse(i2cSpeed);

            COUNTS = 0;
            I = 0;
            D = 0;
            P_old = 0;

            // Clear Flag
            F.I2C = 0;
        }

        if (F.DIR == 1)
        {
            // Update counts before updating direction
            EncUpdate(&counts);
            UpdateData(counts);

            // Update direction
            DIR_READ = RB5;

            // Clear Flag
            F.DIR = 0;
        }


        // PID Loop (assuming counts does not overflow)
        //  PID Loop occurs at regular time intervals
        if (F.T0 == 1)
        {
            // Update to most recent encoder counts
            EncUpdate(&counts);
            UpdateData(counts);

            // Perform PID
            P = TARGET - counts;
            I = I + P;

            if (I > 200)
                I = 200;
            else if (I < -200)
                I = -200;

//            D = abs(P, P_old);              // calculate differential error
            PID = (P * KP) + (I * KI);// + (D * KD);   // calculate new output
            P_old = P;                      // save error for next time

            if (P != 0)
            {
                currentPWM = PID;
//                SetPulse(currentPWM + 128);       // set new PWM
            }


            F.T0 = 0;                   // reset TMR0 flag
        } // end PID Loop               */

    } // end while(1)

    return 1;                   // standard ending for an "int main"
}   // END MAIN ---------------------------------------------------------------
//-----------------------------------------------------------------------------


// Any PIC initialization that is necessary goes here
void Initialise()
{
    FLAG = 0;
    BeginPWM();             // initialize PWM associated registers
    BeginEncoder();         // initialize encoder registers (TMR0 & TMR1)
    i2cInit(I2C_ADDRESS);   // initialize I2C

    // Configure interrupts
    PEIE = 1;               // generic peripheral interrupts enabled
    RBIE = 1;               // PORTB interrupts enabled
    T0IE = 1;               // TMR0 interrupts enabled
    PIE1 = 0b00001001;      // I2C and TMR1 interrupts enabled
    PIE2 = 0;               // other peripherals disabled
    // Clear flags
    SSPIF = 0;
    RBIF = 0;
    T0IF = 0;
    TMR1IF = 0;
    // Enable all interrupts
    GIE = 1;

    TRISB = 0b11110111;
    PORTBbits.RB3 = FORWARD;    // default to forward

    TRISD = 0;
    PORTCbits.RC1 = 0;
}


// Primary Interrupt vector
void interrupt isr()
{
    if (SSPIF == 1)             // interrupt is I2C related
    {
        F.I2C = 1;              // set i2c flag bit
//        SSPIF = 0;
        i2cIsrHandler();
    } else if (T0IF == 1)       // overflow of timer 0
    {
        // TMR0 overflows every 13.11 ms
        //  Time between F.T0 activation is 13ms * TMR0_OverflowTarget

        if (TMR0_OverflowCount == TMR0_OverflowTarget)
        {
            F.T0 = 1;
            TMR0_OverflowCount = 0;
        } else
            TMR0_OverflowCount++;

        T0IF = 0;
    } else if (TMR1IF == 1)     // overflow of counts; probably never happens
    {
        COUNTS += 65536;
        TMR1IF = 0;
    } else if (RBIF == 1)       // PORTB change
    {
        if (RB5 != DIR_READ)    // verify that it was a direction change
        {
            F.DIR = 1;          // set direction flag bit
        } else                  // Error, probably H-Bridge related
        {
            TRISD = 0;
            PORTD = 0x90;
        }
        RBIF = 0;
    } else                      // Any other interrupt error
    {
        TRISD = 0;
        PORTD = 0xF0;
    }
}       // end interrupt function


// Takes in variables holding the most recent time read and count read and adds
//  (or subtracsts, depending on the direction) those to the current totals
void UpdateData(int c)
{
    // Assuming no overflow occurs in longs

    // Add counts if going forward, subtract if going backwards
    if (DIR_READ == FORWARD)
    {
        COUNTS += c;
    } else
    {
        COUNTS -= c;
    }
}


// Sets the direction according to the direction value
void setDirection(int dir)
{
    if (dir == 0)          // if I2C passes down direction of 0, it is forward
        PORTBbits.RB3 = FORWARD;        // forward
    else if (dir == 1)
        PORTBbits.RB3 = BACKWARD;       // reverse
    else
        PORTBbits.RB3 = FORWARD;        // default to motor forward
}


// Returns the absolute value of the difference between two numbers
int abs(int a, int b)
{
    int temp;
    temp = a - b;
    if (temp < 0)
        temp = temp * -1;
    return temp;
}


void delay1sec()
{
    int i;
    // Loop 76 times for 1 second delay (20MHz, 256 prescale)
    for(i = 0; i <= 76; i++)
    {
        while (T0IF == 0)
            asm("nop");
        T0IF = 0;
    }
}


