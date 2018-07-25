/*********************************************************************************************
PhotogateLV.c Target PIC18F4525 Controls the PIC MCU as a two photogate timer.
	extensive rewrite for new project (started in 2018). 
	Copyright (C) 2018   Dan Peirce B.Sc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


This program is written for a PIC18F4525 chip
 * some of the old comments may have pin ref to PIC18F2620

***********************************************************************************************/

#include <xc.h>
#include <usart.h>    // XC8 Compiler Library for USART functions 
#include <stdlib.h>   // XC8 Compiler Library for atoi() function
#include <delays.h>   // XC8 Compiler Library for delay functions 
#include <timers.h>   // XC8 Compiler Library for timer functions 
#include <capture.h>  // XC8 Compiler Library for capture functions 
#include <stdio.h>     


union four_bytes
{
    unsigned long a_long;
    struct
    {
        unsigned lower_int;
        unsigned upper_int;
    };
};

union flags
{
    unsigned char a_byte;
    struct
    {
        unsigned bit0:1;
        unsigned bit1:1;
        unsigned bit2:1;
        unsigned bit3:1;
        unsigned bit4:1;
        unsigned bit5:1;
        unsigned bit6:1;
        unsigned bit7:1;
    };
};

#pragma config WDT = OFF

#pragma config OSC = EC   // using an external clock (oscillator connected to pin 9 of PIC18F2620)

#pragma config MCLRE = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF      // PORTB<4:0> are digital IO 
#pragma config CCP2MX = PORTBE   // switch CCP2 from RC1 to RB3

void txbuffertask(void);
void sendTime(unsigned int *listTmr);
void running(void);



#define SHIFTOUT 0x0E
#define REVERT 'r'

void initialization(void);

unsigned int timerCountOvrF = 0; // used to count Timer1 or Timer3 overflows and thus acts as the 
                                // upper 16 bits of a 32 bit timer

long count = 0;

char buffer[100];
char outIndexBuff = 0; 
char inIndexBuff = 0;
static char code[] = { SHIFTOUT, 'w', '2', 0 };
union flags debounceSW;
union flags inputSW;

//*********************************************************************************
//                               main
//*********************************************************************************
void main(void)
{
    char gate_mode = 0;
    Delay10KTCYx(20); 
  
    initialization();
    debounceSW.a_byte = 0;
    inputSW.a_byte = 0;
    
    while(1)
    { 
        static unsigned int listTmr[] = {0,0,0,0,0,0};
        static unsigned int indexTmr = 0;
        static unsigned int cyclecount = 0;
        
        if(TXIF && (inIndexBuff > 0)) txbuffertask();
        if (PIR1bits.TMR1IF) // Timer1 clock has overflowed
        {
            PIR1bits.TMR1IF = 0; // reset Timer1 clock interrupt flag
            timerCountOvrF++;
        }
        inputSW.bit0 = PORTDbits.RD2;
        if (!debounceSW.bit0)
        {
            if (!inputSW.bit0 && (cyclecount>2)) cyclecount--;
            if (inputSW.bit0) 
            {
                cyclecount++;
                if (cyclecount == 1)
                {
                    listTmr[indexTmr] = ReadTimer1();
                    indexTmr++;
                    listTmr[indexTmr] = timerCountOvrF;
                    indexTmr++;
                    running();
                }
            }
            if (cyclecount > 100)
            {
                cyclecount = 0;
                debounceSW.bit0 = 1;
            }
        }
        else
        {
            if (inputSW.bit0 && (cyclecount>2)) cyclecount--;
            if (!inputSW.bit0) cyclecount++;
            if (cyclecount > 100) 
            {
                cyclecount = 0;
                debounceSW.bit0 =0;
            }
        }
        /* if (PIR1bits.CCP1IF)
        {
            listTmr[indexTmr] = ReadCapture1();
            indexTmr++;
            listTmr[indexTmr] = timerCountOvrF;
            indexTmr++;
            PIR1bits.CCP1IF = 0; //clear flag for next event
        } */
        if (indexTmr == 4) 
        {    
            sendTime(listTmr);
            indexTmr = 0;
            timerCountOvrF = 0;
        }
    } 
}

void sendTime(unsigned int *listTmr)
{
    union four_bytes valone, valtwo, result;
    
    valtwo.upper_int = listTmr[3];
    valtwo.lower_int = listTmr[2];
    valone.upper_int = listTmr[1];
    valone.lower_int = listTmr[0];
    result.a_long = valtwo.a_long - valone.a_long;
    if (result.a_long < 1000000 ) inIndexBuff = inIndexBuff + sprintf( buffer+inIndexBuff, "%s%lu us\n", code, result.a_long);
    else if (result.a_long < 10000000ul )
    {
        float resultfloat;
        resultfloat = result.a_long / 1000000.0;
        inIndexBuff = inIndexBuff + sprintf( buffer+inIndexBuff, "%s%f s\n", code, resultfloat);
    }
    else
    {
        float resultfloat;
        resultfloat = result.a_long /1000000.0;
        inIndexBuff = inIndexBuff + sprintf( buffer+inIndexBuff, "%s%.3f s\n", code, resultfloat);
    }
}

void running(void)
{
    inIndexBuff = inIndexBuff + sprintf( buffer+inIndexBuff, "%s- - -\n", code);
}

void txbuffertask(void)
{
    TXREG = buffer[outIndexBuff];
    outIndexBuff++;
    if (inIndexBuff == outIndexBuff) 
    {
        inIndexBuff = 0;
        outIndexBuff= 0;
    }
}

void initialization(void)
{

    // Configure USART module

	TRISCbits.TRISC7 = 1;     // and RX (RC7) as input
    TRISCbits.TRISC6 = 0;     // set TX (RC6) as output  
    TRISCbits.TRISC5 = 0;     // unused pin
	TRISCbits.TRISC4 = 0;     // unused pin	
	TRISCbits.TRISC3 = 0;     // unused pin	
	// Configure RC2/CCP1 and RB3/CCP2 as inputs
    // Photogate 1 is on RC2/CCP1/Pin 13 and 
    // Photogate 2 is on RB3/CCP2/Pin 24 
    TRISCbits.TRISC2 = 1;     // set RC2(CCP1) as input
	TRISCbits.TRISC1 = 0;     // unused pin
    TRISCbits.TRISC0 = 0;     // unused pin	

	TRISBbits.TRISB7 = 0;     // unused pin
	TRISBbits.TRISB6 = 0;     // unused pin
	TRISBbits.TRISB5 = 0;     // unused pin
	TRISBbits.TRISB4 = 0;     // unused pin
    // Configure RC2/CCP1 and RB3/CCP2 as inputs
    // Photogate 1 is on RC2/CCP1/Pin 13 and 
    // Photogate 2 is on RB3/CCP2/Pin 24     
    TRISBbits.TRISB3 = 1;     // set RB3(CCP2) as input
    TRISBbits.TRISB2 = 0;     // unused pin
	TRISBbits.TRISB1 = 0;     // unused pin 	

	TRISA = 0; // none of the pins are used
	
	TRISD = 0;  // for PIC18F4525 only
	TRISDbits.TRISD2 = 1;     // pushbutton switch on pin18f4525 -- will change port for pic18f2620 

    OpenUSART( USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & 
             USART_CONT_RX & USART_BRGH_HIGH, 16 );   
          // baud rate is 2 000 000 / (SPBRG+1)
          // SPBRG = 1, baud rate is 1 000 000, 
          // SPBRG = 16, baud rate is 115 200 (good for hyperterminal debugging)

    OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
    WriteTimer1(0);  // thinking of having having timers running always
    PIR1bits.TMR1IF = 0;
    OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF); // ma move this to different function  
    Delay10KTCYx(10);	
}	




