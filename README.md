# Photogate Box

The photogate box will contain two interconnected circuits. The display terminal circuit is shown here.

![](image/timerbox.jpg)

![](image/terminal-front.jpg)

![](image/terminal-back.jpg)

The PIC18F2620 is part of the timing circuit which is mounted on the top board of this image showing all
mounted parts.

![](image/all_mounted_parts.jpg)

This version contains two [custom 3D printed mounting brackets](https://github.com/danpeirce/pic-box-bracket) to secure the circuit boards to the box
lid.

## pickmode

The pickmode2620 branch allows selection of different operating modes. 

1. Stopwatch
2. Pulse
3. Pendulum
4. Gate
5. Picket Fence 1

When the timer box is powered up window 1 of the display will cycle displaying possible mode selections in a repeating 
sequence. The **Mode Cycle/Reset** button will advance the displayed mode. The **mode select** button allows one to select the mode.  

**significant changes to the way this cycles will be made in a new branch**

### Stopwatch mode

When the Stopwatch mode is selected the **mode select** button becomes the Start/Stop button.
During timing window 2 of the display shows **- - -**.
When timing is stopped the time will be displayed in window 2. Time is reset automatically if/when the Start/Stop 
button is pressed again.
The Mode Reset button will restart the timer with window 1 cycling the available modes.

![](image/stopwatch1.jpg)

### Pulse mode

There are two varients of this mode.

1. **PulseS** The Photogate mode will time the duration between falling edges on the 
   photogate1 input. If new edges are detected the old time will be overwritten. Reset button
   will return to mode select state **modeS**.
2. **PulsekS** Must hold reset button while pressing mode select button. The Photogate 
   keep mode will time the duration between falling edges on the photogate1 input. The time 
   is displayed in window 2 and return to mode select state is automatic so the time in
   window 2 is not overwritten before a new mode is selected.
   

### Pendulum mode

The Pendulum mode is similar to Photogate mode but is displays the total period of a swinging pendulum.

![](image/pendulum3.jpg)

### Gate mode

The Gate mode times the duration from falling edge to rising edge. Currently this mode runs once and returns 
to the mode selection state with the time displayed in window 2. 

### Picket Fence 1

This mode measures the duration between the first falling edge (the trigger point) and each of eight subsequent 
falling edges. None of the times are displayed until they have all been recorded. The display will then continuously 
cycle through and display each time.

## State Transition Diagram

**this state transition diagram needs updates as it does not yet reflect recent changes to the firmware**

![](image/pickmode.png)

## breadboard 

Initial assembly was done on a breadboard.

![](image/pickmode2620cct.jpg)

Notes specific for this branch are at:

* [https://danpeirce.github.io/2018/oled-v1.2/oled-v1.2.html#pickmode2620](https://danpeirce.github.io/2018/oled-v1.2/oled-v1.2.html#pickmode2620)

The Photogate box includes a microcontroller with built-in hardware timers.

This branch is for the PIC18F2620 MCU. This branch uses an external 32 Mhz oscillator.

![](image/timeswitchcct.jpg)

## Previous Project

The prototype from 2014 looked like this and had a USB interface and no display.

![image of 2014 prototype](image/box-gate.jpg)

## Source code in C

The source code for this project is in C and is licensed under the [GNU GPL v3](http://www.gnu.org/licenses/gpl-3.0.txt).

## PIC Wiring

The PIC inputs and outputs as defined in the source code.

## Microchip Documents

* [Links to Microchip Documents and Install files](doc/MicrochipDocs.md)

## Legacy Peripheral libraries

* When installing xc8 on new computer the legacy peripheral libraries must also be downloaded and installed for 
  this project. The Plib is a separate download starting with XC8 version 1.35.
  
* MPLAB-8:  **Project > Build Options > Project**, select **Linker tab**, and under Runtime options check **Link in 
  Peripheral Library.**
  
* Also add the path to the location of the peripheral libraries.
