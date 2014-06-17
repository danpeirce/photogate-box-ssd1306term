# Photogate Box
The photogate Box is an interface box intended to act as an interface between a photogate and a computer. 
The Photogate box includes a microcontroller with built-in hardware timers.

This branch is for the PIC18F2620 MCU (used in Richmond photogate boxes).

There is also a [branch for the PIC18F4525 MCU](https://github.com/danpeirce/photogate-box/tree/pic18f4525) (used in Surrey photogate boxes).

## Source code in C
The source code for this project is in C and is licensed under the [GNU GPL v3](http://www.gnu.org/licenses/gpl-3.0.txt).
See the file PhotogateLV.c

Modified in 2014 for XC8 compiler and to use an external clock

##Prototype project from 2006. 

> Some changes have been made that were not included in the notes at the link given below. Details
will be added to this README.md file to reflect the photogate timer box as is currently used.

* [Photogate Box Notes from 2006-2007](http://www.kwantlen.ca/science/physics/faculty/dpeirce/notes/timer_box/)

## PIC Wiring

![image of 2014 prototype](http://www3.telus.net/danpeirce/notes/photogate-box/board_test01.jpg)

The PIC inputs and outputs as defined in the source code

```c
// Configure 2-Way Status LED

  TRISCbits.TRISC3 = 0;     // set as output 
  TRISCbits.TRISC4 = 0;     // and as output
```
  
```c
  // Configure USART module

  TRISCbits.TRISC6 = 0;     // set TX (RC6) as output 
  TRISCbits.TRISC7 = 1;     // and RX (RC7) as input
```

```c
  // Configure RC2/CCP1 and RB3/CCP2 as inputs
  // Photogate 1 is on RC2/CCP1/Pin 13 and 
  // Photogate 2 is on RB3/CCP2/Pin 24 
  TRISCbits.TRISC2 = 1;     // set RC2(CCP1) as input
  TRISBbits.TRISB3 = 1;     // set RB3(CCP2) as input 
```
  
...more details to come.

## Enclosure

...to come later...
