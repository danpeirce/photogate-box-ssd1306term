# Host Scripts and Modules

## Run From Command Prompt

* nameVirtualCOM.py  
  Will report the name of the first USB virtual com port

## GUI Example TKinter GUI gate1gui.pyw  
  
The GUI gate1gui.pyw was written as an example GUI and is a modification of gate1.py.
It functions but could use some more development.  
  
## Load in the IPython QT Console

* gate1.py  
  Can be used as shown in gate1_IPython.txt (IPthon session copied here)

```python
Python 2.7.7 |Anaconda 2.0.1 (32-bit)| (default, Jun 11 2014, 10:41:43) [MSC v.1500 32 bit (Intel)]
Type "copyright", "credits" or "license" for more information.

IPython 2.1.0 -- An enhanced Interactive Python.
Anaconda is brought to you by Continuum Analytics.
Please check out: http://continuum.io/thanks and https://binstar.org
?         -> Introduction and overview of IPython's features.
%quickref -> Quick reference.
help      -> Python's own help system.
object?   -> Details about 'object', use 'object??' for e xtra details.
%guiref   -> A brief reference about the graphical user interface.

In [1]: cd 'd:/photogate-box/host'
d:\photogate-box\host

In [2]: import gate1

In [3]: gate1.time1gate()
running time_1gate
Port  COM5  has been opened.
 
10.009272 sec

In [4]: 
 
```
