// Test Code //
#include <Arduino.h>
#include <Pins_Arduino.h>
#include <Wire.h>

void loop() {

  // Just wait here for interrupt to fire.

}

// Description: Used to test radio experiment timer frequency
// Implementation: Toggle an output pin and read with oscilloscope
void setup() {
  
  // Disable interrupts
  cli();
  // Clear Timer1 registers
  TCCR1A = 0;
  TCCR1B = 0;
  // Set OCR1A (Top): 16MHz/500Hz /2 = 16000 steps 
  OCR1A = 16000;
  // Configure Timer/Counter 1
  // Select P & F Correct PWM Mode, and OCR1A as TOP. 

  TCCR1B = _BV(WGM12) // CTC Mode with OCR1A as Top 
         | _BV(CS10); // Enable base clock without pre-scale
  
  TIMSK1 = _BV(OCIE1A); // Enable OCRA Compare ISR
  
  sei(); // Enable interrupts
  
  // Set OC1A as output (Mega pin 24, Digital Pin 11)
  DDRB = _BV(PB5);
}


// Using ISR to be more accurate with how program actually runs
// Interrupt Service Routine (ISR)
// Toggles pin on compare
ISR(TIMER1_COMPA_vect) {
  PORTB ^= B00100000; // Toggle 6th bit (PB5)
  }
