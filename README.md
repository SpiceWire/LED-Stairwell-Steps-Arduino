# LED-Stairwell-Steps-Arduino  
This Arduino sketch sequentially turns on and off LED strips that are attached under the bullnose of each step in a stairwell.  

The sketch allows several patterns of turning off and on. PIR sensors ("motion detectors") determine if a person is entering the
stairwell and the direction they are going.  

## Design  
The LED strip under each step is controlled individually. The LEDs themselves in this project are not individually addressable.  

I used 12 volt, warm white LED strips. Two PIRs determine if a person is entering the stairwell and whether from  
the top or bottom of the stairs. Entering from the top of the stairwell gives a different lighting pattern than entering from the bottom of the stairwell. The stairwell has a landing and change of direction, which means the detection fields of the two PIR sensors do not overlap.  

Because individually controlling the 16 steps in the stairwell would require more pins than most Arduinos have, two 8-register  
74HC595 shift registers were used to "expand" the number of I/O pins run by the Arduino.  Each shift register's output is  
connected to a 2N2222 transisitor. The transistors act as a switch to turn the LED strips on and off. 

Each step is 32 inches wide, allowing about 48 LEDs beneath each. Each step at full brightness would require about 400mA. This is well  
witin the datasheet specs of a 2N2222 transistor, which is 600 mA of continuous collector current. A 480 ohm limiting resistor  
decreases the current to the transistor's base.  I used a resistor network chip 4116R-001-471 instead of using separate resistors.  
I chose to use transistors and resistors instead of mosfets, etc. simply because I had the materials available already.  

The PIR's fit well into a breath mint container. Painted, they are unobtrusive.  

I placed LED strips under the corner molding on the stairwell landing and incorporated these as steps into the sequence of lighting.  

The Arduino library ShiftPWM by elcojacobs is flexible and offers many features beyond what this project needs. 

## LEDs under corner molding, PIR sensor inside breath mint container:
![PIR and LEDs on landing](/images/landing.JPG)

The lights and board are fuse protected.   

In addition to the code, this project involves calculation, soldering, circuit design, electrical knowledge and trade skills. Any  
person attempting a similar project would need to design for individual needs and circumstances.

## Hardware
Arduino Uno or similar  
PIRs ("motion detectors") such as HC SR501  
LED strips cut to the length of the step. I used a strip with warm white 2835 LEDs, about 48 per step.  
Two 74HC595 shift registers  
12 volt power supply for LED strips   
5 volt power supply for shift registers and transistors  
16 2N2222 transistors  
16 470 ohm resistors or 2 resistor network chips such as 4116R-001-471  
Screw terminals for connecting wire to a circuit board  
Circuit boards  
Fuses  
Various small electrical hardware, soldering supplies, etc.  

# Schematics  

## Arduino Uno  
The Arduino Uno uses SPI to tell the shift registers which LED strips to activate and deactivate, and how bright each strip should be. The MOSI pin sends data to the first 74HC595 chip, which then forwards relevant data to the second 74HC595 chip. The Arduino also takes input from two PIR sensors and a control switch. 

<p align="center">
  ![Arduino](/images/ArduinoToShiftRegister.png)  
</p>

## Shift Registers  
Serial data from the first 74HC595 chip is fed to the input of the second chip. Each output pin of the shift registers controls a single LED strip about 32 inches long.  
<p align="center">
  <![ShiftRegisters](/images/Shift_Register_Schematic.png)>  
</p>  

## LED Strips  
A single representative example of the 16 LED strips is shown. The output of each shift register pin is fed to a current limiting resistor attached to the base of a 2N2222 transistor. It is important that the LED strips draw power from the power supply and are controlled by the transistor on the "return" line. The emitter leg of the transistor goes directly to ground.  

Because the power requirements of the LED strip are dependent on the type of LED, number of LEDs per unit length, and overall length of strip, the combimation of 480 ohm resistor and 2N2222 transistor will not work with all situations. Alternative methods can use MOSFETS or the Texas Instruments TLC5940 chip.  
<p align="center">
  <![LEDStrips](/images/LEDStrips.png)>   
</p>  

## Control switch and PIR sensors  
The control switch (see code) activates or deactivates the LED strips to assist with troubleshooting.  
PIR sensors send signals to the Arduino when movement is detected. One PIR sensor is at the bottom of the stairwell. The other is at the top of the stairwell. The two sensors do not have overlapping fields due to a landing and change of direction in the middle of the stairwell. '
<p align="center">
  <![ControlSwitch](/images/Switch.png)>  
  
  ![PIRSensors](/images/PIRSensors.png)
</p>  

