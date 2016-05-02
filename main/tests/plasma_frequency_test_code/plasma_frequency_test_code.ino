// Test Code //
#include <Arduino.h>
#include <Pins_Arduino.h>
#include <Wire.h>

void loop() {

  // Just wait here for interrupt to fire.

}

// Description: Used to test plasma experiment timer frequency
// Implementation: Toggle an output pin and read with oscilloscope
// Using Timer 3, to allow possibility of multiple experiments simultaneously
// At vastly different frequencies if necessary
// i.e. Edit Timer3's pre-scale wihout affecting Timer1's prescale 
void setup() {
  
  // Disable interrupts
  cli();
  // Clear Timer1 registers
  TCCR3A = 0;
  TCCR3B = 0;
  // Set OCR3A (Top): 16MHz/200Hz /2 = 40000 steps 
  OCR3A = 40000;
  // Configure Timer/Counter 3
  // Select P & F Correct PWM Mode, and OCR1A as TOP. 

  TCCR3B = _BV(WGM12) // CTC Mode with OCR1A as Top 
         | _BV(CS10); // Enable base clock without pre-scale
  
  TIMSK3 = _BV(OCIE3A); // Enable OCRA Compare ISR
  
  sei(); // Enable interrupts
  
  // Set OC3A as output (Mega pin 5, Digital Pin 5)
  DDRB = _BV(PE3);
}


// Using ISR to be more accurate with how program actually runs
// Interrupt Service Routine (ISR)
// Toggles pin on compare
ISR(TIMER3_COMPA_vect) {
  PORTE ^= B00001000; // Toggle 4th bit (PE3)
  }
