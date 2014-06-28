# Communicating With Time_AllEdges_1Gate

## Sequence 

* Send a question mark and a Numeric three  "**?3**" (this will select Time_AllEdges_1Gate())
* Send three numeric characters representing an unsigned integer number of gate pulses to measure "**001**"
* Send a single numeric character to indicate which photogate to use "**1**"
* Receive 4 binary bytes for the "time" of the falling edge and 4 binary bytes for the time of the rising edge.
* If more than one gate pulse is expected more bytes will be received.

Sending a "**!**" while waiting for data  will tell the timer box to cancel out of the acquire loop.
