# Host Scripts and Modules

## Run From Command Prompt

* nameVirtualCOM.py 
  Will report the name of the first USB virtual com port
* photogate-timer.py
  Sends a command to Timer box for simple gate timing of photogate, waits for result and prints it

## Load in the QT IPython

* gate1.py
  Can be used as shown in gate1_IPython.txt (IPthon session copied here)

```python
Python 2.7.7 |Anaconda 2.0.1 (32-bit)| (default, Jun 11 2014, 10:41:43) [MSC v.1500 32 bit (Intel)]
Type "copyright", "credits" or "license" for more information.

IPython 2.1.0 -- An enhanced Interactive Python.
Anaconda is brought to you by Continuum Analytics.
Please check out: http://continuum.io/thanks and https://binstar.org
? -> Introduction and overview of IPython's features.
%quickref -> Quick reference.
help -> Python's own help system.
object? -> Details about 'object', use 'object??' for extra details.
%guiref -> A brief reference about the graphical user interface.

In [1]: cd 'd:\photogate-box\host'
d:\photogate-box\host

In [2]: import gate1

In [3]: ser = gate1.time_1gate()
Port COM5 has been opened.
Sending command for one gate time

In [4]: gate1.report_time(ser)
10.077498 sec

In [5]: 
```