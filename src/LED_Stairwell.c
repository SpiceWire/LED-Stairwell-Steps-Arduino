//#include <Arduino.h>
/* Tasks:
    make a min brightness value to replace full off
    move the maxbrightness declaration to the top
    make a select case list for turn on patterns DONE
    add code so that lights don't turn off if there's movement on the steps  DONE-ish
    add number of steps as a variable.. Declare this as an int.  DONE
    See if I need to apply pullup resistors to input pins. cf Pinmode  DONE  It needed digitalRead
    See if I can delete the else statment when I set the numRegisters
    There might be some problems with the "remainOn", might need to configure PIR's.
    Other patterns: For fade on/off, multiply number of steps by the delay per step.
    Clean up the transitions between the various patterns of turning on or off.
  --

*/

/*
   New user help:
   PWM stands for Pulse-Width Modulation. In this program, it means how fast the electricity to the LED's will be turned on and off.
   PWM can turn on and off the LED's so fast that you won't see it happening. It is a method of dimming and brighening LED lights.
   You can change variables in this program to customize it to your needs.
   This sketch allows a user to select a pattern for lighting the steps and a different pattern for de-lighting the steps.
   "Walking up the steps" and "walking down the steps" can have different patterns for both lighting and de-lighting.
   A PIR is a motion detector.
   This sketch is intended to work with at least one 74HC595 shift register
   put a 10k resistor into the on off switch, if you use it.
   You may need to adjust the transitionDelay var
*/
// Variables

unsigned int totalStairs = 16;  // The total number of stairs/steps with LED's being controlled. Can be changed by user.

//Lighting vars
int transitionDelay = 400;                // Time in milliseconds until next stair is lighted or de-lighted. Higher number means slower sequence.
//int maxBrightness = 255;                // Maximum brigntness of LEDs as set by PWM. Least bright is is 0, brightest is 255.
unsigned char minBrightness = 0;          // Minimum brightness of LEDs as set by PWM. 0 is off. Allows user to set LEDs to "dim" when stairwell is empty.
unsigned int remainLightedUP = 10000;     // Time in milliseconds that all stairs have maxBrightness when a person walks up the stairs. Max 32767. Must be positive.
unsigned int remainLightedDOWN = 5000;    // Time in milliseconds that all stairs have maxBrightness when a person walks down the stairs. Max 32767. Must be positive.
unsigned int goingUPPatternON = 1;        // The pattern the lights turn on after the bottom PIR is triggered
unsigned int goingUPPatternOFF = 1;      // the pattern the lights turn off after the bottom PIR is triggered.
unsigned int goingDOWNPatternON = 1;      // The pattern the lights turn on after the top PIR is triggered
unsigned int goingDOWNPatternOFF = 1;     // The pattern the lights turn off after the top PIR is triggered.


//Accessories vars
boolean MIDpir = false;       // Set to true if a PIR is being used on a mid-stairs landing.
boolean alarmUsed  = false;   // Set to true if using an alarm on the steps (baby, pets, etc.)
boolean lightSensor = false;  // Set to true if using a daylight sensor along with PIR sensors.
boolean LCDScreen = false;    // Set to true if using an LCD screen connected to the Arduino.
boolean onOffSwitch = true;  // Set to true if using an on off switch to override PIR's.

//PIR vars
unsigned int PIRCalibrate = 60;     // PIR calibration time(datacheet) in seconds. Might be up to 60 seconds after power is applied.
unsigned int TOPpirPin = 3;         // Data wire from the top step's PIR sensor gets connected to Arduino pin 3.
unsigned int BOTpirPin = 4;         // Data wire from the bottom step's PIR sensor gets connected to Arduino pin 4.
unsigned int MIDpirPin = 5;         // Optional. Data wire from a landing's PIR sensor gets connected to Arduino pin 5
unsigned int numRegisters;          // The total number of shift registers needed to control all the LED's. A register can control a max of 8.
boolean bLanding = false;           // Var holds output value of PIR sensor on landing, if present

//Accessories vars
unsigned int onOffSwitchPin = 6;    // Pin for on off switch to override PIR's. Pin can be changed by user.

//Shift register vars.
const int ShiftPWM_latchPin = 8; //Pin 12 (latch pin) from 74HC595 shift register gets connected to Arduino pin 8. Arduino pin be changed by user.
//Because of SPI, pin 11 (serial clock pin) from 74HC595 shift register must be connected to Arduino Uno pin 13. No clockpin var needed.
//Because of SPI, pin 14 (data pin) from 74HC595 shift register must be connected to Arduino Uno pin 11. No datapin var needed.

//sketch vars
boolean occupied = false;  //occupied = false if stairs are unoccupied, program sets this to true when PIR is triggered
const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

unsigned char maxBrightness = 190;
unsigned char pwmFrequency = 75;
//unsigned int numRegisters = 2;    //this value is changed below

void setup() {
  //Using the number of stairs to calculate the number of shift registers needed:
  numRegisters = totalStairs / 8;    //integer division drops any remainders
  if (totalStairs % 8 > 0) {         //modulo division tests for remainders
    numRegisters = numRegisters + 1; //if totalStairs/8 has a remainder, increase the number of shift registers by 1
  }
  else {
    numRegisters = numRegisters;    //if there is no remainder, then no shift registers are partially filled.
  }

  Serial.begin(9600);

  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.Start(pwmFrequency, maxBrightness);

  // set up 74HC595 pin
  pinMode(ShiftPWM_latchPin, OUTPUT); //No pinMode needed for serial clock and data

  // set up PIR pins
  pinMode(TOPpirPin, INPUT);
  pinMode(BOTpirPin, INPUT);
  if (MIDpir == true) {
    pinMode(MIDpirPin, INPUT);    //sets mid-stairs PIR input pin on Arduino if PIR on landing is present
  }

  // set up optional onOffSwitch
  if (onOffSwitch == true) {
    Serial.println("optional on off switch is present.");
    pinMode(onOffSwitchPin, INPUT);
  }

  // set up pins  for optional accessories (LCD screens, alarms, light sensors)

  // turn all lights off, in case any are on
  ShiftPWM.SetAll(0);

  occupied = false;  //loop begins with stairs being unoccupied
}

void loop() {
  // First check on/off switch, then PIR's

  if (onOffSwitch == true) {
    while (digitalRead(onOffSwitchPin) == HIGH) {
      ShiftPWM.SetAll(maxBrightness);
    }
    ShiftPWM.SetAll(minBrightness);
  }

  // Checks  PIR at bottom of stairs. If high, and stairs are unoccupied, start the UP sequence.
  if (digitalRead(BOTpirPin) == HIGH && occupied == false) {
    Serial.println("Bottom sensor tripped.");
    travellingUP();
  }
  // Checks PIR at top of stairs. If high, and stairs are unoccupied, start the DOWN sequence.
  if (digitalRead(TOPpirPin) == HIGH && occupied == false) {
    Serial.println("Top sensor tripped.");
    travellingDOWN();
  }
}


void travellingUP() {       //person is travelling up the stairs
  Serial.println("Going up.");
  occupied = true;
  switch (goingUPPatternON) {
    case 1://Stairs turn on and stay on sequentially
      {
        for (int i = 0; i < totalStairs; i++) {   //Var i turns on lights sequentially, var j keeps them  on.
          ShiftPWM.SetOne(i, maxBrightness);
          delay(transitionDelay);
          for (int j = 0; j < i; j++) {
            ShiftPWM.SetOne(j, maxBrightness);
          }
        }
      }
      break;

    case 2: //Each sequential step lights up on a background of dim stairs
      {
        for (int i = 0; i < totalStairs; i ++) {
          ShiftPWM.SetAll(minBrightness + 60);
          ShiftPWM.SetOne(i, maxBrightness);
          delay(transitionDelay);
        }
      }
      break;

    case 3: //Like an airport landing strip. One step at a time lights up and turns off, pattern repeats.
      {
        for (int i = 0; i < totalStairs; i++) {
          for (int j = 0; j < totalStairs; j++) {
            ShiftPWM.SetAll(minBrightness + 60);
            ShiftPWM.SetOne(j, maxBrightness);
            delay(transitionDelay);
          }
        }
      }
      break;

    case 4: //All steps fade on at the same time
      {
        for (int j = minBrightness; j < maxBrightness; j++) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
      }
      break;
  }
  Serial.println("Finished sequence of turning on when going up.");

  remainON(remainLightedUP);

  switch (goingUPPatternOFF) {  //sequence of de-lighting the path after person traveled up
    case 1://Stairs turn off and stay off sequentially, starting at the bottom
      {
        for (int i = 0; i < totalStairs; i++) {
          ShiftPWM.SetOne(i, 0);
          delay(transitionDelay);
          for (int j = totalStairs - 1; j < i; j--) {
            ShiftPWM.SetOne(j, maxBrightness);
          }
        }
      }
      break;

    case 2: //Each sequential step turns dim, starting at the bottom
      {
        for (int i = 0; i < totalStairs; i ++) {
          ShiftPWM.SetOne(i, minBrightness + 60);
          delay(transitionDelay);
        }
      }
      break;

    case 3: //All steps dim at the same time
      {
        ShiftPWM.SetAll(minBrightness + 60);
      }
    case 4: //All steps fade on, then off at the same time
      {
        for (int j = minBrightness; j < maxBrightness; j++) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
        for (int j = maxBrightness; j > minBrightness; j--) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
      }
      break;
  }
  occupied = false;
}


void travellingDOWN() {   //person is travelling down the stairs
  Serial.println("going down.");
  occupied = true;

  switch (goingDOWNPatternON) {
    case 1://Stairs turn on and stay off sequentially, starting at the top
      {
        //Var i turns off lights sequentially, var j keeps them  on.
        for (int i = totalStairs - 1; i > -1; i--) {
          ShiftPWM.SetOne(i, maxBrightness);
          delay(transitionDelay);
          for (int j = totalStairs - 1; j > i; j--) {
            ShiftPWM.SetOne(j, maxBrightness);
          }
        }
      }
      break;

    case 2: //Each sequential step lights up and then turns off, on a background of dim stairs
      {
        for (int i = totalStairs - 1; i > -1; i--) {
          ShiftPWM.SetAll(minBrightness + 60);
          ShiftPWM.SetOne(i, maxBrightness);
          delay(transitionDelay);
        }
      }
      break;

    case 3: //Like an airport landing strip. One step at a time lights up and turns off, pattern repeats with fewer steps each iteration
      {
        for (int i = 0; i < totalStairs; i++) {
          for (int j = totalStairs - 1; j > -1; j--) {
            ShiftPWM.SetAll(minBrightness + 60);
            ShiftPWM.SetOne(j, maxBrightness);
            delay(transitionDelay);
          }
        }
      }

    case 4: //All steps fade on at the same time
      {
        for (int j = minBrightness; j < maxBrightness; j++) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
        break;
      }
  }

  remainON(remainLightedDOWN);    //How long to keep all lights on

  switch (goingDOWNPatternOFF) {
    case 1://Stairs turn off and stay off sequentially, starting at the bottom
      { //Var i turns off lights sequentially. Var j keeps remaining lights on.
        for (int i = totalStairs - 1; i > 0; i--) {
          ShiftPWM.SetOne(i, 0);
          delay(transitionDelay);
          for (int j = totalStairs - 1; j > i; j++) {
            ShiftPWM.SetOne(j, maxBrightness);
          }
        }
      }
      break;

    case 2: //Each sequential step turns dim, starting at the top
      {
        for (int i = totalStairs - 1; i > -1; i--) {
          ShiftPWM.SetOne(i, minBrightness + 60);
          delay(transitionDelay);
        }
      }
      break;

    case 3: //All steps dim at the same time
      {
        ShiftPWM.SetAll(minBrightness + 60);
      }
    case 4: //All steps fade on, then off at the same time
      {
        for (int j = minBrightness; j < maxBrightness; j++) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
        for (int j = maxBrightness; j > minBrightness; j--) {
          ShiftPWM.SetAll(j);
          delay(20);
        }
      }
      break;
  }
  occupied = false;
}


void remainON(unsigned int remainLighted) {
  if (MIDpir == true) {
    bLanding = digitalRead(MIDpirPin);
  }
  else {
    bLanding = LOW;
  }
  //If the landing PIR is installed for use, set the boolean var bLanding = the landing's PIR logic output
  //Might need a counter here to confirm landing is clear
  Serial.println("TOPpir   BOTpir   MIDpir from trigger") ;
  Serial.print(digitalRead(TOPpirPin));
  Serial.print("       ");
  Serial.print(digitalRead(BOTpirPin));
  Serial.print("       ");
  Serial.println(bLanding);
  delay(remainLighted);
  while (digitalRead(BOTpirPin) == HIGH || digitalRead(TOPpirPin) == HIGH || bLanding == HIGH) {
    delay(remainLighted);
    Serial.println("TOPpir   BOTpir   MIDpir  from while loop");
    Serial.print(digitalRead(TOPpirPin));
    Serial.print("       ");
    Serial.print(digitalRead(BOTpirPin));
    Serial.print("       ");
    Serial.println(bLanding);
  }
  Serial.println("remainON is finished. Starting de-lighting.");
}
