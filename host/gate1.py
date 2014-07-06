# For a host computer to communicate with the KPU photogate timer box
# file gate1.py

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

    while ( ser.inWaiting() < 8 ):
        time.sleep(0.2)
    try:	
        reading1 = ser.read(4)   # read falling edge time
        reading2 = ser.read(4)   # read rising edge time
    except ValueError:
        print 'Did not find timing edges'
    except SerialException:
        print 'Serial Port not open'
	# Python expects characters so they have to be converted to integers
	# The time we need is the difference between the falling edge and rising edge 
    try:
        theTime = int(reading2.encode('hex'), 16) - int(reading1.encode('hex'), 16) 
    except ValueError:
        theTime = 0
        print 'Did not detect full pulse. Repeat command.'
    # Convert the time from micro seconds to seconds
    theTime = theTime / 1000000.0
    print theTime,
    print 'sec'

    ser.write('!')  # tell firmware to stop waiting for another edge
                    # this should not be necessary but just in case 
    ser.close()

	
def time1gate():
    print 'running time_1gate'
    try:
        portName = USBport()
        if portName is None:
            print 'No Virtual Ports Found'
        else:
            ser = serial.Serial()
            ser.baudrate = 1000000
            ser.port = portName
            ser.timeout = 0
            ser.open()
    except SerialException:
        print 'error opening port'
		
    if ser.isOpen() :
        print 'Port ',
        print portName,
        print ' has been opened.'
        time.sleep(0.25)
        print ' '
        time_AllEdges_1Gate(ser)
    else:
        print 'cannot open Port ',
        print portName

	


    
    
	
	
