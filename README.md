# Photogate Box
The photogate Box is an interface box intended to act as an interface between a photogate and a computer. 
The Photogate box includes a microcontroller with built-in hardware timers.

This branch is for the PIC18F4525 MCU (used in Surrey photogate boxes).

There is also a [branch for the PIC18F2620 MCU](https://github.com/danpeirce/photogate-box/tree/pic18f2620) (used in Richmond photogate boxes).

## Source code in C
The source code for this project is in C and is licensed under the [GNU GPL v3](http://www.gnu.org/licenses/gpl-3.0.txt).
See the file PhotogateLV.c

Modified in 2014 for XC8 compiler and to use an external clock

##Prototype project from 2006. 

> Some changes have been made that were not included in the notes at the link given below. Details
will be added to this README.md file to reflect the photogate timer box as is currently used.

* [Photogate Box Notes from 2006-2007](http://www.kwantlen.ca/science/physics/faculty/dpeirce/notes/timer_box/)

## PIC Wiring

The PIC inputs and outputs as defined in the source code
```c
// Configure 2-Way Status LED

  TRISDbits.TRISD1 = 0;     // set as output 
  TRISDbits.TRISD2 = 0;     // and as output
```
  
```c
  // Configure USART module

  TRISCbits.TRISC6 = 0;     // set TX (RC6) as output 
  TRISCbits.TRISC7 = 1;     // and RX (RC7) as input
```

```c
  // Configure RC2/CCP1 and RB3/CCP2 as inputs
  // Photogate 1 is on RC2/CCP1/Pin 17 and 
  // Photogate 2 is on RB3/CCP2/Pin 36 
  TRISCbits.TRISC2 = 1;     // set RC2(CCP1) as input
  TRISBbits.TRISB3 = 1;     // set RB3(CCP2) as input 
```
  
...more details to come.

## Enclosure

...to come later...
