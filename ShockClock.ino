#include <Time.h>

#include <Bounce.h>

#include <LiquidCrystal.h>
#include <Servo.h> 
#include "protothread.h"
#include "UserInterface.h"

// initialize the library with the numbers of the interface pins
//                fs,en,d4,d5,d6,d7
//                0, 1, 2, 3, 4, 5
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);
Servo greenFlag;
Servo redFlag;

Bounce redButton = Bounce(16, 50);
Bounce blueButton = Bounce(15, 50);

 
HardwareSerial uart = HardwareSerial(); 
 
 

int greenRaised = 0;
int redRaised = 0;
 
// shocker is pin 11

// green button is pin 15
// red button is pin 16

// keypad is analog pin A0 (21)
 
void setup() { 
  uart.begin(9600);
  
  lcd.begin(16, 2);
  lcd.print("  don't panic");
  
  greenFlag.attach(10);
  redFlag.attach(9);
  
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  
  pinMode(16, INPUT);
  pinMode(15, INPUT);
  digitalWrite(16, HIGH);
  digitalWrite(15, HIGH);

  lowerRed();
  lowerGreen();
} 

void raiseRed() {
  redFlag.write(45);
  redRaised = 1;
}

void lowerRed() {
  redFlag.write(155);
  redRaised = 0;
}
 
void raiseGreen() {
  greenFlag.write(155);
  greenRaised = 1;
}

void lowerGreen() {
  greenFlag.write(45);
  greenRaised = 0;
}
 

void loop() { 
  
  if (uart.available() > 0) {
    int cmd = uart.read();
    
    if (cmd == 's') {
      digitalWrite(12, HIGH);
      delay(5);
      digitalWrite(12, LOW);
      
    } else if (cmd == 'R') {
      raiseRed();
      
    } else if (cmd == 'r') {
      lowerRed();
      
    } else if (cmd == 'G') {
      raiseGreen();
    
    } else if (cmd == 'g') {
      lowerGreen();
    }  
  }
  
  redButton.update();
  if (redButton.risingEdge()) {
    redRaised = !redRaised;
    
    if (redRaised) {
      raiseRed();
    } else {
      lowerRed();
    }
  }
  
  blueButton.update();
  if (blueButton.risingEdge()) {
    greenRaised = !greenRaised;
    
    if (greenRaised) {
      raiseGreen();
    } else {
      lowerGreen();
    }
  }
    
  
  
} 

