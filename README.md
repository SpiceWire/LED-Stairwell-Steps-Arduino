# LED-Stairwell-Steps-Arduino  
This Arduino sketch sequentially turns on and off LED strips that are attached under the bullnose of each step in a stairwell.  
The sketch allows several patterns of turning off and on. PIR sensors ("motion detectors") determine if a person is entering the
stairwell and the direction they are going.  

## Design  
The LED strip under each step is controlled individually. The LEDs themselves in this project are not individually addressable.  
I used 12 volt, warm white LED strips. Two PIRs determine if a person is entering the stairwell, and whether from  
the top or bottom of the stairs. Entering from the top of the stairwell gives a different lighting pattern than entering from the  
bottom of the stairwell. The stairwell has a landing and change of direction, which means the detection fields   
of the two PIR sensors do not overlap.  

Because individually controlling the 16 steps in the stairwell would require more pins than most Arduinos have, two 8-register  
74HC595 shift registers were used to "expand" the number of I/O pins run by the Arduino.  Each shift register's output is  
connected to a 2N2222 transisitor. The transistor acts as a switch to turn the LED strips on and off. 

Each step is 32 inches wide, allowing about 48 LEDs beneath each. Each step at full brightness would require about 400mA. This is well  
witin the datasheet specs of a 2N2222 transistor, which is 600 mA of continuous collector current. A 480 ohm limiting resistor  
decreases the current to the transistor's base.  I used a resistor network chip 4116R-001-471 instead of using separate resistors.  
I chose to use transistors and resistors instead of mosfets, etc. simply because I had the materials available already.  

The PIR's fit well into a breath mint container. Painted, they are unobtrusive.  

I placed LED strips under the corner molding on the stairwell landing and incorporated these as steps into the sequence of lighting.  
The lights and board are fuse protected.   

In addition to the code, this project involves calculation, soldering, circuit design, electrical knowledge and trade skills. Any  
person attempting a similar project would need to design for individual needs and circumstances.

## Hardware
Arduino Uno 
PIRs ("motion detectors")
LED strips cut to the length of the step  
Two 74HC595 shift registers  
12 volt power supply for LED strips  
9 volt power supply for Arduino  
5 volt power supply for shift registers and transistors
16 2N2222 transistors  
16 470 ohm resistors  
Screw terminals for connecting wire to a circuit board  
Circuit boards  
Fuses  
Various small electrical hardware, soldering supplies, etc.  

Software:
Arduino library ShiftPWM by elcojacobs

![PIR and LEDs on landing](/images/PA120501.JPG)
