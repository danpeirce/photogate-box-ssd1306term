/*********************************************************************************************
PhotogateLV.c Target PIC18F4525 Controls the PIC MCU as a two photogate timer.
	extensive rewrite for new project (started in 2018). 
	Copyright (C) 2018   Dan Peirce B.Sc.
	
	main() is being essentially completely rewritten as new program requires
	        pushbutton switch input rather than serial input

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


union two_bytes
{
    unsigned int an_integer;
    struct
    {
        unsigned lower_byte:8;
        unsigned upper_byte:8;
    };  
};

#pragma config WDT = OFF
// initial testing done without external oscillator
//#pragma config OSC = EC   // using an external clock (oscillator connected to pin 9 of PIC18F2620)
#pragma config OSC = INTIO67  // allows osc1 (pin 13) and osc2 (pin 14) to be used as inputs
                              // note there is a crystal attached to these pins on the 
                              // brainboard
#pragma config MCLRE = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF      // PORTB<4:0> are digital IO 
#pragma config CCP2MX = PORTBE   // switch CCP2 from RC1 to RB3

void set_osc_32MHz(void);
void keypresstask(void);
void txbuffertask(void);



#define SHIFTOUT 0x0E
#define REVERT 'r'

void initialization(void);

unsigned int counter = 0; // used to count Timer1 or Timer3 overflows and thus acts as the 
                          // upper 16 bits of a 32 bit timer
						  // ??????  why is there only one for two overflows????

unsigned char CANCEL;     //override for timing events 
                          // ???? why all upper case -- I'd use that for a macro ?????

long count = 0;

char buffer[100];
char outIndexBuff = 0; 
char inIndexBuff = 0;
static char code[] = { SHIFTOUT, 'w', '2', 0 };

//*********************************************************************************
//                               main
//*********************************************************************************
void main(void)
{
    set_osc_32MHz(); // only used when using internal oscillator fir initial 
                     // testing
    char gate_mode = 0;
    Delay10KTCYx(20); 
  
    initialization();
 
    while(1)
    {  
        static int loopcount=0;
        if (loopcount > 500)
        {
            keypresstask();
            loopcount=0;
        }
        loopcount++;
        txbuffertask();
    } 
}

// only used when using internal oscillator for initial testing
void set_osc_32MHz(void)
{
  int i;
 
  OSCCONbits.IRCF2 = 1;     // Set the OSCILLATOR Control Register to 8 MHz
  OSCCONbits.IRCF1 = 1;      
  OSCCONbits.IRCF0 = 1;     
 
  OSCTUNEbits.PLLEN = 1;    // Enable PLL, boost by 4 -> 32 MHz

  for(i=0;i<500;i++);       // delay to allow clock PLL to lock (stabilize)

      
}

void keypresstask(void)
{
    static char keyp = 0, keyplast =0;
    static unsigned int countpresses = 0;
    keyp = PORTDbits.RD2;
    if (keyp != keyplast)
    {
        if (keyp == 1) 
        {
            countpresses++;
            inIndexBuff = inIndexBuff + sprintf( buffer+inIndexBuff, "%sKPress %i\n", code, countpresses);
        }

    }
    keyplast = keyp;
}

void txbuffertask(void)
{
    if(TXIF && (inIndexBuff > 0))
    {
        TXREG = buffer[outIndexBuff];
        outIndexBuff++;
        if (inIndexBuff == outIndexBuff) 
        {
            inIndexBuff = 0;
            outIndexBuff= 0;
        }
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

  
    Delay10KTCYx(10);	
}	




