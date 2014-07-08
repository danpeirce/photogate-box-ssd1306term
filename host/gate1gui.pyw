# For a host computer to communicate with the KPU photogate timer box
# file gate1gui.py

# Except as noted (getAvailablePorts())
#     copyright 2014 Dan Peirce B.Sc.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
  


# function getAvailablePorts() copied and modified from
#  http://www.nullege.com/codes/show/src@m@e@MeasurementValueLogging-0.1.2@MeasurementValueLogging@devices@devicemanager.py/102/serial.tools.list_ports.comports
# it was part of a larger module released under the GPL version 3
# Copyright (C) 2013  Leonard Lausen <leonard@lausen.nl>
def getAvailiablePorts():
    """Returns a list of valid and available serial ports."""
  
    import serial.tools.list_ports
    portsTuple = serial.tools.list_ports.comports()
    portsList = []
  
    for i in portsTuple:
        portsList.append(i[0])
  
    return tuple(portsList)

#this function written for windows
# virtual USB com ports are typically COM5 or greater on windows machines
# for KPU physics lab computers typically get assigned com7 or higher but on the netbook 
# the port is assigned COM5. 	
def USBport():
    availablePorts = getAvailiablePorts()
    for i in availablePorts:
        if  i!= 'COM1' and i!= 'COM2' and i!= 'COM3' and i!= 'COM4' :
		    return i


from Tkinter import *
import serial
import time

# Send a question mark and a Numeric three "?3" (this will select Time_AllEdges_1Gate())
# Send three numeric characters representing an unsigned integer number of gate pulses to measure "001"
# Send a single numeric character to indicate which photogate to use "1"
# Receive 4 binary bytes for the "time" of the falling edge and 4 binary bytes for the time of the rising edge.
# If more than one gate pulse is expected more bytes will be received.

def time_AllEdges_1Gate(ser):
    ser.write('?3')          # start of command
    time.sleep(0.001)        # delay a ms as PIC has only two byte buffer
    ser.write('00')
    time.sleep(0.001)
    ser.write('11')          # end of command 



	
def time1gate(ser):
    if ser.isOpen() :
        text.insert(INSERT, 'Port ')
        text.insert(INSERT, portName)
        text.insert(INSERT, ' has been opened.\n\n')
        time.sleep(0.25)
        text.insert(INSERT, ' ')
        time_AllEdges_1Gate(ser)
    else:
        text.insert(INSERT, 'cannot open Port ')
        text.insert(INSERT, portName )
        text.insert(INSERT, ' \n\n')

#make a TkInter Window
root = Tk()
root.wm_title("Time Gate Mode")

# make a text box to put the serial output
text = Text ( root, width=30, height=10, takefocus=0)
text.pack()

text.insert(INSERT, 'running time1gate\n')
try:
    portName = USBport()
    if portName is None:
        text.insert(INSERT, 'No Virtual Ports Found\n')
    else:
        ser = serial.Serial()
        ser.baudrate = 1000000
        ser.port = portName
        ser.timeout = 0
        ser.open()
except SerialException:
    text.insert(INSERT, 'error opening port\n')

time1gate(ser)

def get_display_time():
    if ( ser.inWaiting() >= 8 ):
        try:	
            reading1 = ser.read(4)   # read falling edge time
            reading2 = ser.read(4)   # read rising edge time
        except ValueError:
            text.insert(INSERT, '\nDid not find timing edges\n')
        except SerialException:
            text.insert(INSERT, '\nSerial Port not open\n')
	    # Python expects characters so they have to be converted to integers
	    # The time we need is the difference between the falling edge and rising edge 
        try:
             theTime = int(reading2.encode('hex'), 16) - int(reading1.encode('hex'), 16) 
        except ValueError:
            theTime = 0
            text.insert(INSERT, 'Did not detect full pulse. Repeat command.\n' )
        # Convert the time from micro seconds to seconds
        theTime = theTime / 1000000.0
        text.insert(INSERT, str(theTime) )
        text.insert(INSERT, ' sec\n' )

        ser.write('!')  # tell firmware to stop waiting for another edge
                        # this should not be necessary but just in case 
        ser.close()
    else:
        root.after(10, get_display_time)

root.after(10, get_display_time)	
root.mainloop()
    
	
	
