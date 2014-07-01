# For a host computer to communicate with the KPU photogate timer box
# file photogate-timer.py
#                              **This file is a work in progress**

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
	
#import serial

portName = USBport()
if portName is None:
    print 'No Virtual Ports Found'
else:
    print portName
	
	