# Photogate Box
The photogate Box is an interface box intended to act as a photogate timer with a display. This repository
was imported from  [https://github.com/danpeirce/photogate-box.git](https://github.com/danpeirce/photogate-box.git) which
was a repository for a project that interfaced a photogate to a computer but lacked a display. Considerable modification
of that project will be necessary for the new user interface which will now be switches and a SSD1306 display
terminal. Notes on the display terminal are available at 
[https://danpeirce.github.io/2018/oled-v1.2/oled-v1.2.html](https://danpeirce.github.io/2018/oled-v1.2/oled-v1.2.html).

![](https://danpeirce.github.io/2018/oled-v1.2/photogateTimer.jpg)

**The process if changing the code is only just beginning (as of June 24, 2018).**

The Photogate box includes a microcontroller with built-in hardware timers.

For more [information on what a photogate is *link*](https://answers.yahoo.com/question/index?qid=20080614212815AAqek64).

This branch is for the PIC18F2620 MCU. 

![image of 2014 prototype](image/box-gate.jpg)

## Source code in C
The source code for this project is in C and is licensed under the [GNU GPL v3](http://www.gnu.org/licenses/gpl-3.0.txt).
See the file PhotogateLV.c

##Prototype project from 2006. 

> Some changes have been made that were not included in the notes at the link given below. Details
will be added to this README.md file to reflect the photogate timer box as is currently used.

* [Photogate Box Notes from 2006-2007](https://danpeirce.github.io/2006/timer_box/index.html)

## PIC Wiring

![image of 2014 prototype](image/board_test01.jpg)

The PIC inputs and outputs as defined in the source code

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

## Microchip Documents

* [Links to Microchip Documents and Install files](doc/MicrochipDocs.md)

## Enclosure

Photo of PCB's mounted on bracket plate which is mounted on the box lid. More detail about the mounting plate at <https://github.com/danpeirce/pic-box-bracket>

![Photo of PCB's mounted on bracket plate which is mounted on the box lid](image/boards-mounted-bracket.jpg)

...more to come later...

## Legacy Peripheral libraries

* When installing xc8 on new computer the legacy peripheral libraries must also be downloaded and installed for 
  this project. The Plib is a separate download starting with XC8 version 1.35.
  
* MPLAB-8:  **Project > Build Options > Project**, select **Linker tab**, and under Runtime options check **Link in 
  Peripheral Library.**
  
* Also add the path to the location of the peripheral libraries.