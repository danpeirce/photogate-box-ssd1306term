 * The following is not yet implemented
Two photogate plugs are connected to the CCP1 and CCP2 pins. xc8 peripheral library functions are used to 
capture and time falling and or rising edges at the pins. Since the built-in timers are 16 
bit, a counter is used to count timer rollovers and create a 32 bit clock.

Chip is set to 32 MHz by an external clock. Timers are set to measure in microseconds. 

Maximum time before total rollover is 2^16 * 2^16 * 1 musec = 4294 seconds = 71 minutes.



// **** original functions declarations *****
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
// **** end original functions declarations *****

// While waiting for a capture event on CCP1 it updates the upper 16 bits of our 32 bit
// clock by incrementing counter on each overflow of the Timer1 clock
// Copyright (C) 2007   Michael Coombes
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
// Copyright (C) 2007   Michael Coombes
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
// Copyright (C) 2007   Michael Coombes
unsigned int C12_Increment_Counter_on_Timer_Rollover(void)
{
    while(!PIR1bits.CCP1IF && !PIR2bits.CCP2IF  && !CANCEL) // wait for event;
    {
        if (DataRdyUSART())
        {
            if ( getcUSART() == (char)'!') CANCEL = (unsigned char)1;
        }
        // use overflow to go to a 32 bit counter
        if (PIR1bits.TMR1IF ) // Timer clock has overflowed
        {
            PIR1bits.TMR1IF = 0; // reset Timer1 clock interrupt
            // PIR2bits.TMR3IF = 0; // reset Timer3 clock interrupt
            counter++;
        }
    }

    return counter;    
}

//  Photogate 1 is read on RC1 and Photogate 2 is read by RB3 (was RC2)
//  Result of read converted to an ascii digit for convenience
//  Copyright (C) 2007   Michael Coombes
void PhotogateStatusCheck(void)
{
    unsigned char gate_status;
    gate_status = (PORTCbits.RC2 + (PORTBbits.RB3 << 1));  // read photogate pins     
    gate_status = 3 - gate_status + 48;                       // convert to an ascii digit
                                                           // '0' both off
                                                           // '1' gate 1 on, gate 2 off
                                                           // '2' gate 1 off, gate 2 on
                                                           // '3' gate 1 on, gate 2 on
    while(BusyUSART());        // wait until serial port is ready 
    WriteUSART(gate_status);   // write one byte
}

// Times falling edges on either CCP1 using Timer1 or CCP2 using Timer 3
// Copyright (C) 2007   Michael Coombes
// wanted to make use of these functions but finding the existing structure 
// not conducive to cooperative multitasking. 
void Time_FallingEdges_1Gate(void)
{
    unsigned char gate_to_use = '1'; // default is gate 1
    char string[4];                                        // string to get numbers
    unsigned int i=0, number_edges_to_time = 0, current_edge_time = 0, rollover_n = 0;
 
    number_edges_to_time = atoi(string);  // Warning - function quits at first non-appropriate symbol

    // not going to get configuration from usart!
    // here will use default gate 1

  
    counter = 0;           // reset 

    if (gate_to_use != '2') // use photogate 1 (default)
    {
        // configure Timer1 for capture mode at 8*TOSC = 1 microsec.
        OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_CCP1_T3_CCP2);
        WriteTimer1(0);  // thinking of having having timers running always
        PIR1bits.TMR1IF = 0;
        OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF); // ma move this to different function
        while(i<number_edges_to_time) // not conducive to multi-tasking
        {                             // will restructure to work as state machine
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

    // ResetUSART();
}     

// Times falling edges on both CCP1 and CCP2  using Timer1 
// Copyright (C) 2007   Michael Coombes
void Time_FallingEdges_2Gates(void)
{
    char string[4];                                        // string to get numbers
    char gate_identifier = 'X';
    unsigned int number_edge_pairs_per_gate = 0, edge_time = 0, rollover_n = 0;
    unsigned int i, Gate1_counter = 0, Gate2_counter = 0, Gate_counter_end = 0;

    while (!DataRdyUSART());                     // wait until there is a byte to read
    getsUSART(string,3);                         // read a three characters from buffer
    number_edge_pairs_per_gate = atoi(string);     // function quits at first non-appropriate symbol

    Gate_counter_end = 2 * number_edge_pairs_per_gate;

//    StatusLED_Green_Working();

    // configure Timer for capture mode at 8*TOSC = 1 microsec.
    OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_SOURCE_CCP);
    PIE1bits.TMR1IE = 1;

    counter = 0;
                
    OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
    OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);

    WriteTimer1(0);             // Reset 
    PIR1bits.TMR1IF = 0;        // Reset Timer1 interrupt flag

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
                if ( Gate1_counter == number_edge_pairs_per_gate ) CloseCapture1();  //all CCP1 edges found
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
                if ( Gate2_counter == number_edge_pairs_per_gate) CloseCapture2(); // all CCP2 edges found  
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

    ResetUSART();
 
}

// Times falling and rising edges on either CCP1 using Timer1 or CCP2 using Timer 3
// Copyright (C) 2007   Michael Coombes
void Time_AllEdges_1Gate(void)
{
    unsigned char gate_to_use = '1';
    char string[4];                                        // string to get numbers
    unsigned int i, number_edges_to_time = 0, fall_time = 0, rise_time = 0,
                  rollover_n_Fall = 0, rollover_n_Rise = 0;  

    while (!DataRdyUSART());                   // wait until there is a byte to read
    getsUSART(string,3);                       // read a three characters from buffer
    number_edges_to_time = atoi(string);       // function quits at first non-appropriate symbol

    number_edges_to_time *= 2;  

    while (!DataRdyUSART());  // wait until there is a byte to read
    gate_to_use = getcUSART();  

//    StatusLED_Green_Working();
  
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

// Times falling and rising edges on both CCP1 and CCP2 using Timer1  
// Copyright (C) 2007   Michael Coombes
void Time_AllEdges_2Gates(void)
{
    char string[4];                                        // string to get numbers
    unsigned char gate_identifier = 'X';
    unsigned int number_edge_pairs_per_gate = 0, edge_time = 0, rollover_n = 0;
    unsigned int i, Gate1_counter = 0, Gate2_counter = 0, Gate_counter_end = 0;

    while (!DataRdyUSART());                               // wait until there is a byte to read
    getsUSART(string,3);                      // read a three characters from buffer
    number_edge_pairs_per_gate = atoi(string);     // function quits at first non-appropriate symbol
 
    Gate_counter_end = 2 * number_edge_pairs_per_gate;

//    StatusLED_Green_Working();

    // configure Timers for capture mode at 8*TOSC = 1 microsec.
    OpenTimer1(TIMER_INT_OFF & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_8 & T1_SOURCE_CCP);
    PIE1bits.TMR1IE = 1;
    counter = 0;
               
    OpenCapture1(C1_EVERY_FALL_EDGE & CAPTURE_INT_OFF);
    OpenCapture2(C2_EVERY_FALL_EDGE & CAPTURE_INT_OFF);

    WriteTimer1(0);             // Reset 
    PIR1bits.TMR1IF = 0;        // Reset Timer1 interrupt flag

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
                if ( Gate1_counter == Gate_counter_end )  CloseCapture1();    //all CCP1 edges found
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
                if ( Gate2_counter == Gate_counter_end)  CloseCapture2();      // all CCP2 edges found  
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
    ResetUSART();
}

// if I need a newline
// Copyright (C) 2007   Michael Coombes
void newline(void)
{
    while(BusyUSART());
    WriteUSART(13);   //Carriage Return - signifies end of number characters
    WriteUSART(10);   //newline character
}

// takes two 16-bit integers and breaks them into 4 bytes to send to the serial port.
// Copyright (C) 2007   Michael Coombes
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