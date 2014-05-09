/*********************************************************************************************
PhotogateLV.c Target PIC18L4525 Controls the PIC MCU as a two photogate timer.
    Copyright (C) 2007   Michael Coombes

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


This program is written for a PIC18F4525 chip with a sparkfun serial-to-USB connector.

Two photogate plugs are connected to the CCP1 and CCP2 pins. C-18 librry functions are used to 
capture and time falling and or rising edges at the pins. Since the built-in timers are 16 
bit, a counter is used to count timer rollovers and create a 32 bit clock.

Chip is set to 32 MHz and may be tuned using the OSCTUNE SPR. Calibration is done with the 
digital scope reference of 1 KHz. Timers are set to measure in microseconds. 

Maximum time before total rollover is 2^16 * 2^16 * 1 musec = 4294 seconds = 71 minutes.

A two-colour LED between RC1 and RC2 is used as an indicator: Red for ready, green for busy, 
flashing for error.

***********************************************************************************************/

#include <p18F4525.h>
#include <usart.h>    // C-18 Compiler Library for USART functions 
#include <stdlib.h>   // C-18 Compiler Library for atoi() function
#include <delays.h>   // C-18 Compiler Library for delay functions 
#include <timers.h>   // C-18 Compiler Library for timer functions 
#include <capture.h>  // C-18 Compiler Library for capture functions 
#include "osc.h"      // local header for oscillator speed setting control function in osc.c

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
#pragma config OSC = INTIO7      // puts osc/4 on pin 14 to check freq
#pragma config MCLRE = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF      // PORTB<4:0> are digital IO 
#pragma config CCP2MX = PORTBE   // switch CCP2 from RC1 to RB3


void wait_for_questionmark(void);
void StatusLED_Red_Ready(void);
void StatusLED_Green_Working(void);
void ErrorLED(void);
unsigned int C1_Increment_Counter_on_Timer1_Rollover(void);
unsigned int C2_Increment_Counter_on_Timer3_Rollover(void);
unsigned int C12_Increment_Counter_on_Timer_Rollover(void);
void PhotogateStatusCheck(void);
void Time_FallingEdges_1Gate(void);
void Time_FallingEdges_2Gates(void);
void Time_AllEdges_1Gate(void);
void Time_AllEdges_2Gates(void);
void newline(void);
void integer_bytes_to_USART(unsigned int i, unsigned int j);
void ResetUSART(void);

unsigned int counter = 0; // used to count Timer1 or Timer3 overflows and thus acts as the 
                          // upper 16 bits of a 32 bit timer

unsigned char CANCEL;     //override for timing events 


//*********************************************************************************
//                               main
//*********************************************************************************
void main (void)
{
  char gate_mode = 0; 
   
  set_osc_32MHz();          // to change the internal oscillator frequency
  
  Delay10KTCYx(20); 
  
  // To avoid stray input noise turn all DIO pins to outputs 
  TRISA = 0b00000000;
  TRISB = 0b00000000;
  TRISC = 0b00000000;
  TRISD = 0b00000000;

  // Configure 2-Way Status LED

  TRISDbits.TRISD1 = 0;     // set as output 
  TRISDbits.TRISD2 = 0;     // and as output
 
  // Configure USART module

  TRISCbits.TRISC6 = 0;     // set TX (RC6) as output 
  TRISCbits.TRISC7 = 1;     // and RX (RC7) as input

  OpenUSART( USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & 
             USART_CONT_RX & USART_BRGH_HIGH, 1 );   
          // baud rate is 2 000 000 / (SPBRG+1)
          // SPBRG = 1, baud rate is 1 000 000, 
          // SPBRG = 16, baud rate is 115 200 (good for hyperterminal debufgging)

  // Configure RC2/CCP1 and RB3/CCP2 as inputs
  // Photogate 1 is on RC2/CCP1/Pin 17 and 
  // Photogate 2 is on RB3/CCP2/Pin 36 
  TRISCbits.TRISC2 = 1;     // set RC2(CCP1) as input
  TRISBbits.TRISB3 = 1;     // set RB3(CCP2) as input 
  
  Delay10KTCYx(10);
 

while(1==1)
   {

      CANCEL = 0;  // reset
      counter = 0; // reset
	  StatusLED_Red_Ready();
      // get operational parameters	
      wait_for_questionmark();
      while (!DataRdyUSART());  // wait until there is a byte to read
      gate_mode = ReadUSART();  // read one byte 
 
 	  if ( gate_mode < 48 || gate_mode >  53) gate_mode = '6'; // repeat
  
  
	  switch(gate_mode)
         {
	         case '0':	PhotogateStatusCheck();
						break;
	         case '1': 	Time_FallingEdges_1Gate();
						break;
	         case '2': 	Time_FallingEdges_2Gates();
						break;
	         case '3': 	Time_AllEdges_1Gate();
						break;
	         case '4': 	Time_AllEdges_2Gates();
						break;
	         case '5': 	ResetUSART();
                        StatusLED_Green_Working();
                        Delay10KTCYx(255);   
						break;
			 default:
						ErrorLED();
						break;	 
		 }
    } 
}


// Check for ? key press
// Polls serial port until '?' is received
void wait_for_questionmark(void)
{
 unsigned char repeat = ' ';
 while (repeat != '?')
 	{
		 while (!DataRdyUSART());  // wait until there is a byte to read
		 repeat = ReadUSART();     // read one byte
     }
}

// Turn 2-Way Status LED to red - ready for data
void StatusLED_Red_Ready(void)
{
PORTDbits.RD2 = 0;
PORTDbits.RD1 = 1;
}

// Turn 2-Way Status LED to green  - working
void StatusLED_Green_Working(void)
{
PORTDbits.RD2 = 1;
PORTDbits.RD1 = 0;
}

void ErrorLED(void)
{
int i;

for( i=0; i<10; i++ )
   {
    StatusLED_Green_Working();
	Delay10KTCYx(100); //0.10 second delay
    StatusLED_Red_Ready();
	Delay10KTCYx(100); //0.10 second delay
   }
   
}
// While waiting for a capture event on CCP1 it updates the upper 16 bits of our 32 bit
// clock by incrementing counter on each overflow of the Timer1 clock
unsigned int C1_Increment_Counter_on_Timer1_Rollover(void)
{
while(!PIR1bits.CCP1IF && !CANCEL) // wait for event;
	 {
      if (DataRdyUSART())
         {
          if ( getcUSART() == (char)'!') CANCEL = (unsigned int)1;
         }
	  // use overflow to go to a 32 bit counter
	  if (PIR1bits.TMR1IF) // Timer1 clock has overflowed
	     {
	      PIR1bits.TMR1IF = 0; // reset Timer1 clock interrupt
	      counter++;
	     }
	 }
PIR1bits.CCP1IF = 0; //clear flag for next event
return counter;
}


// While waiting for a capture event on CCP2 it updates the upper 16 bits of our 32 bit
// clock by incrementing counter on each overflow of the Timer3 clock
unsigned int C2_Increment_Counter_on_Timer3_Rollover(void)
{
while(!PIR2bits.CCP2IF && !CANCEL) // wait for event;
	 {
      if (DataRdyUSART())
         {
           if ( getcUSART() == (char)'!') CANCEL = (unsigned int)1;  
         }
	  // use overflow to go to a 32 bit counter
	  if (PIR2bits.TMR3IF) // Timer3 clock has overflowed 
  	     {
	      PIR2bits.TMR3IF = 0; // reset Timer3 clock interrupt
	      counter++;
	     }
	 }
PIR2bits.CCP2IF = 0; //clear flag for next event
return counter;
}

// While waiting for a capture event on CCP1 or CCP2 it updates the upper 16 bits of our 
// 32 bit clock by incrementing counter on each overflow of the Timer1 or Timer3 clock
unsigned int C12_Increment_Counter_on_Timer_Rollover(void)
{
while(!PIR1bits.CCP1IF && !PIR2bits.CCP2IF  && !CANCEL) // wait for event;
	 {
      if (DataRdyUSART())
         {
          if ( getcUSART() == (char)'!') CANCEL = (unsigned char)1;
		 }
	  // use overflow to go to a 32 bit counter
	  if (PIR1bits.TMR1IF || PIR2bits.TMR3IF) // Timer clock has overflowed
	     {
	      PIR1bits.TMR1IF = 0; // reset Timer1 clock interrupt
	      PIR2bits.TMR3IF = 0; // reset Timer3 clock interrupt
	      counter++;
		 }
     }

return counter;	
}

//  Photogate 1 is read on RC1 and Photogate 2 is read by RB3 (was RC2)
//  Result of read converted to an ascii digit for convenience
void PhotogateStatusCheck(void)
{
	unsigned char gate_status;
    StatusLED_Green_Working(); // set LED
	gate_status = (PORTCbits.RC2 + (PORTBbits.RB3 << 1));  // read photogate pins     
    gate_status = 3 - gate_status + 48;					   // convert to an ascii digit
														   // '0' both off
                                                           // '1' gate 1 on, gate 2 off
                                                           // '2' gate 1 off, gate 2 on
                                                           // '3' gate 1 on, gate 2 on
    while(BusyUSART());        // wait until serial port is ready 
    WriteUSART(gate_status);   // write one byte
}

// Times falling edges on either CCP1 using Timer1 or CCP2 using Timer 3
void Time_FallingEdges_1Gate(void)
{
  unsigned char gate_to_use = '1'; // default is gate 1
  char string[4];                                        // string to get numbers
  unsigned int i=0, number_edges_to_time = 0, current_edge_time = 0, rollover_n = 0;
 
  while (!DataRdyUSART());              // wait until there is a byte to read
  getsUSART(string,3);                  // read a single character from buffer
  number_edges_to_time = atoi(string);  // Warning - function quits at first non-appropriate symbol

  while (!DataRdyUSART());              // wait until there is a byte to read
  gate_to_use = getcUSART();            // using this variable to keep track of photogate to use

  StatusLED_Green_Working();
  
  counter = 0;           // reset 

  if (gate_to_use != '2') // use photogate 1 (default)
     {
   	  // configure Timer1 for capture mode at 8*TOSC = 1 microsec.
      OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
      WriteTimer1(0);
      PIR1bits.TMR1IF = 0;
      OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
      while(i<number_edges_to_time)
          {
   	       rollover_n = C1_Increment_Counter_on_Timer1_Rollover(); 
           current_edge_time = ReadCapture1();  
           if (!CANCEL)
              { 
			   integer_bytes_to_USART(rollover_n, current_edge_time); // takes about 60 musec at 1 Mbaud
              }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }
		   i++;
          }
      CloseCapture1();
     }
  else
     {
	  // configure Timer3 for capture mode at 8*TOSC = 1 microsec.
      OpenTimer3(TIMER_INT_OFF & T3_16BIT_RW & T3_SOURCE_INT & T3_PS_1_8 & T1_CCP1_T3_CCP2);
      WriteTimer3(0);
      PIR2bits.TMR3IF = 0;
      OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
      while(i<number_edges_to_time)
          {
           rollover_n = C2_Increment_Counter_on_Timer3_Rollover();
           current_edge_time = ReadCapture2();  
		   if (!CANCEL)
              {     
		       integer_bytes_to_USART(rollover_n, current_edge_time); // takes about 60 musec at 1 Mbaud
              }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }   
	       i++;
          }
 	  CloseCapture2();
 	 }
     
   if (gate_to_use != '2') // use photogate 1 (default)
      {
       CloseTimer1();
      }     
   else
      {
       CloseTimer3();
      }

  ResetUSART();
}     

// Times falling edges on both CCP1 using Timer1 and CCP2 using Timer 3
void Time_FallingEdges_2Gates(void)
{
 char string[4];                                        // string to get numbers
 char gate_identifier = 'X';
 unsigned int number_edge_pairs_per_gate = 0, edge_time = 0, rollover_n = 0;
 unsigned int i, Gate1_counter = 0, Gate2_counter = 0, Gate_counter_end = 0;

 while (!DataRdyUSART());    			     // wait until there is a byte to read
 getsUSART(string,3);                		 // read a three characters from buffer
 number_edge_pairs_per_gate = atoi(string);	 // function quits at first non-appropriate symbol

 Gate_counter_end = 2 * number_edge_pairs_per_gate;

 StatusLED_Green_Working();

 // two independent timers
 // configure Timers for capture mode at 8*TOSC = 1 microsec.
 OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
 OpenTimer3(TIMER_INT_OFF & T3_16BIT_RW & T3_SOURCE_INT & T3_PS_1_8 & T1_CCP1_T3_CCP2);
 PIE1bits.TMR1IE = 1;

 counter = 0;
                
 OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
 OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);

 WriteTimer1(0); 			// Reset and synchronize timers
 WriteTimer3(5);            // 5 because of time already passed from previous line 
 PIR1bits.TMR1IF = 0;		// Reset Timer1 interrupt flag
 PIR2bits.TMR3IF = 0;		// Reset Timer3 interrupt flag

 i = 0;
 while( i<Gate_counter_end)
     {
      rollover_n = C12_Increment_Counter_on_Timer_Rollover();

	  if (!CANCEL)
	     { 
 	     // event happened so determine which gate CCP1 or CCP2
	      if (PIR1bits.CCP1IF ) // event occurred on CCP1 - Gate 1
	         {  
	           edge_time = ReadCapture1();
	           PIR1bits.CCP1IF = 0; //clear flag for next event
	           Gate1_counter++;
	           if ( Gate1_counter == number_edge_pairs_per_gate ) 
	              CloseCapture1(); 									//all CCP1 edges found
	           gate_identifier = 'X'; // 'X' for gate 1 
	           i++;    
		 	   while(BusyUSART());
			   WriteUSART(gate_identifier);
	 		   integer_bytes_to_USART(rollover_n, edge_time); // takes about 60 musec at 1 Mbaud
	          }
		      
	      if (PIR2bits.CCP2IF ) // event occurred on CCP2 - Gate 2
	         {
	           edge_time = ReadCapture2();
	           PIR2bits.CCP2IF = 0; //clear flag for next event
	 	       Gate2_counter++;
	 	       if ( Gate2_counter == number_edge_pairs_per_gate) 
	              CloseCapture2(); 									// all CCP2 edges found  
	           gate_identifier = 'Y'; // 'Y' for gate 2 
	           i++; 
		 	   while(BusyUSART());
			   WriteUSART(gate_identifier);
	           integer_bytes_to_USART(rollover_n, edge_time); // takes about 60 musec at 1 Mbaud
	         }
		  }
		else
          { 		 	           
           Gate1_counter++;
           gate_identifier = 'X'; // 'X' for gate 1 
           i++;    
	 	   while(BusyUSART());
		   WriteUSART(gate_identifier);
		   integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
		   Gate2_counter++;
		   gate_identifier = 'Y'; // 'Y' for gate 2 
           i++; 
	 	   while(BusyUSART());
		   WriteUSART(gate_identifier);
           integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
		  }
     
     }//end while  
     

 CloseCapture1();
 CloseCapture2();

 CloseTimer1();
 CloseTimer3();

 ResetUSART();
 
}

// Times falling and rising edges on either CCP1 using Timer1 or CCP2 using Timer 3
void Time_AllEdges_1Gate(void)
{
  unsigned char gate_to_use = '1';
  char string[4];                                        // string to get numbers
  unsigned int i, number_edges_to_time = 0, fall_time = 0, rise_time = 0,
                  rollover_n_Fall = 0, rollover_n_Rise = 0;

  while (!DataRdyUSART());  			 // wait until there is a byte to read
  getsUSART(string,3);             		 // read a three characters from buffer
  number_edges_to_time = atoi(string);	 // function quits at first non-appropriate symbol

  number_edges_to_time *= 2;  

  while (!DataRdyUSART());  // wait until there is a byte to read
  gate_to_use = getcUSART();  

  StatusLED_Green_Working();
  
  counter = 0;           // reset 

  if (gate_to_use != '2') //use photogate 1 (default)
     {
	  // configure Timer1 for capture mode at 8*TOSC = 1 microsec.
      OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
      WriteTimer1(0);
      PIR1bits.TMR1IF = 0;
      
      for (i=0; i<number_edges_to_time; i+=2)
          {
           // time falling edge   
           OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_ON);
    	   rollover_n_Fall = C1_Increment_Counter_on_Timer1_Rollover();
           fall_time = ReadCapture1();  

		   if (!CANCEL)
              {     
    		   integer_bytes_to_USART(rollover_n_Fall, fall_time); // takes 60 musec to complete
              }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }     
           // time rising edge 
           OpenCapture1(C1_EVERY_RISE_EDGE & CAPTURE_INT_ON);
   	       rollover_n_Rise = C1_Increment_Counter_on_Timer1_Rollover();
           rise_time = ReadCapture1();

		   if (!CANCEL)
              {     
    		   integer_bytes_to_USART(rollover_n_Rise, rise_time); // takes 60 musec to complete
              }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }     
         }
      CloseCapture1();
     }
  else
     {
      OpenTimer3(TIMER_INT_OFF & T3_16BIT_RW & T3_SOURCE_INT & T3_PS_1_8 & T1_CCP1_T3_CCP2);
      WriteTimer3(0);
      PIR2bits.TMR3IF = 0;
      for (i=0; i<number_edges_to_time; i+=2)
          {
           // time falling edge   
           OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
    	   rollover_n_Fall = C2_Increment_Counter_on_Timer3_Rollover();
		   fall_time = ReadCapture2();  
		   if (!CANCEL)
              {     
      		   integer_bytes_to_USART(rollover_n_Fall, fall_time); // takes 60 musec to complete
    		  }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }     
           
		   // time rising edge   
  		   OpenCapture2(C2_EVERY_RISE_EDGE & CAPTURE_INT_OFF);
		   rollover_n_Rise = C2_Increment_Counter_on_Timer3_Rollover();
           rise_time = ReadCapture2();  

		   if (!CANCEL)
              {     
      		   integer_bytes_to_USART(rollover_n_Rise, rise_time); // takes 60 musec to complete
              }
		   else
              {
               integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
              }     
          }
      CloseCapture2();
   	 }
 
  if (gate_to_use != '2') //use photogate 1 (default)
      { 
       CloseTimer1();
      }
  else
      {  
       CloseTimer3();
      }
  
  ResetUSART();

}     

// Times falling and rising edges on both CCP1 using Timer1 and CCP2 using Timer 3
void Time_AllEdges_2Gates(void)
{
 char string[4];                                        // string to get numbers
 unsigned char gate_identifier = 'X';
 unsigned int number_edge_pairs_per_gate = 0, edge_time = 0, rollover_n = 0;
 unsigned int i, Gate1_counter = 0, Gate2_counter = 0, Gate_counter_end = 0;

 while (!DataRdyUSART());  			     // wait until there is a byte to read
 getsUSART(string,3);             		 // read a three characters from buffer
 number_edge_pairs_per_gate = atoi(string);	 // function quits at first non-appropriate symbol
 
 Gate_counter_end = 2 * number_edge_pairs_per_gate;

 StatusLED_Green_Working();

 // configure Timers for capture mode at 8*TOSC = 1 microsec.
 OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
 OpenTimer3(TIMER_INT_OFF & T3_16BIT_RW & T3_SOURCE_INT & T3_PS_1_8 & T1_CCP1_T3_CCP2);
 PIE1bits.TMR1IE = 1;
 counter = 0;
               
 OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
 OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);

 WriteTimer1(0); 			// Reset and synchronize timers
 WriteTimer3(5); 			// 5 because of time already passed from previous line 
 PIR1bits.TMR1IF = 0;		// Reset Timer1 interrupt flag
 PIR2bits.TMR3IF = 0;		// Reset Timer3 interrupt flag

 i = 0;
 while( i<(4*number_edge_pairs_per_gate) )
     {
      rollover_n = C12_Increment_Counter_on_Timer_Rollover();

       if (!CANCEL)
	      { 
	      // event happened so determine which gate CCP1 or CCP2
	      if (PIR1bits.CCP1IF ) // event occurred on CCP1 - Gate 1
	         {  
	            edge_time = ReadCapture1();
	   			
	            PIR1bits.CCP1IF = 0; //clear flag for next event
	            
			    if ( (Gate1_counter % 2) == (unsigned int)0 ) // even number next edge is a rise 
	            	{
	                 // set up for the next, rising, edge   
	                 OpenCapture1(C1_EVERY_RISE_EDGE & CAPTURE_INT_OFF);
	               	}
	            else // odd - next edge is a fall
	                {
	                 // set up for next, falling, edge  
	                 OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
	                }
	
		        Gate1_counter++;
		        i++; 
	            if ( Gate1_counter == Gate_counter_end ) 
	                CloseCapture1(); 							//all CCP1 edges found
	            gate_identifier = 'X'; // 'X' for gate 1 
	            while(BusyUSART());
	            WriteUSART(gate_identifier);
	            integer_bytes_to_USART(rollover_n, edge_time); // takes 60 musec to complete!
	         }
	            
	     if (PIR2bits.CCP2IF ) // event occurred on CCP2 - Gate 2
	        {
	          edge_time = ReadCapture2();
	
	          PIR2bits.CCP2IF = 0; //clear flag for next event
	            
	   	      if ((Gate2_counter % 2) == (unsigned int)0 ) // even number next edge is a rise  
	     	     {
	              // set up for the next, rising, edge   
	              OpenCapture2(C2_EVERY_RISE_EDGE & CAPTURE_INT_OFF);
	             }
	          else // odd - next edge is a fall
	             {
	              // set up for next, falling, edge  
	              OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
	             }
	
	 	      Gate2_counter++;
	 	      i++;
	          if ( Gate2_counter == Gate_counter_end) 
	              CloseCapture2(); 								// all CCP2 edges found  
	          gate_identifier = 'Y'; // 'Y' for gate 2 
	          while(BusyUSART());
	          WriteUSART(gate_identifier);
	          integer_bytes_to_USART(rollover_n, edge_time); // takes 60 musec to complete
	         }  
		  }
		else
          { 		 	           
           Gate1_counter++;
           gate_identifier = 'X'; // 'X' for gate 1 
           i++;    
	 	   while(BusyUSART());
		   WriteUSART(gate_identifier);
		   integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
		   Gate2_counter++;
		   gate_identifier = 'Y'; // 'Y' for gate 2 
           i++; 
	 	   while(BusyUSART());
		   WriteUSART(gate_identifier);
           integer_bytes_to_USART(0, 0); // takes about 60 musec at 1 Mbaud
		  }

     }
     
  CloseTimer1();
  CloseTimer3();        
  ResetUSART();
}

// if I need a newline
void newline(void)
{
while(BusyUSART());
WriteUSART(13);   //Carriage Return - signifies end of number characters
WriteUSART(10);   //newline character
}

// takes two 16-bit integers and breaks them into 4 bytes to send to the serial port.
void integer_bytes_to_USART(unsigned int i, unsigned int j)
{
union two_bytes value;

value.an_integer = i;
while(BusyUSART());
WriteUSART((char)value.upper_byte);
WriteUSART((char)value.lower_byte);
value.an_integer = j;
while(BusyUSART());
WriteUSART((char)value.upper_byte);
WriteUSART((char)value.lower_byte);

}

// Close and Reopen USART - seems necessary on some SparkFun Serial USB boards.
void ResetUSART(void)
{
 while(BusyUSART()); // wait until the buffer is free before closing
 CloseUSART();
 OpenUSART( USART_TX_INT_OFF & USART_RX_INT_OFF & USART_ASYNCH_MODE & USART_EIGHT_BIT & 
             USART_CONT_RX & USART_BRGH_HIGH, 1 );   
 Delay10KTCYx(10);
}
