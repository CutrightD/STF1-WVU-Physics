
// Test Code //
#include <TimerFour.h>
#include <avr/interrupt.h>  // Used to debug ISRs along with next function

ISR(BADISR_vect){
  Serial.println("Bad ISR.");
}

boolean plasmaSweep = false;

// Pin definitions
const int Timer4_A_PIN = 6; // Arduino Digital Pin # 6 / (Mega Pin 15)
const int Timer4_B_PIN = 7; // Arduino Digital Pin # 7 / (Mega Pin 16)

void setup() {
  Serial.begin(9600);
  Serial.println("Setting up.");
  Timer4.initialize(40); //40us = 25Khz
  delay(2000);
 
    
  // Use Timer 5 to not interfere with the other Timers (1,3 & 4)
  // Disable interrupts
  cli();
  // Clear Timer5 registers
  TCCR5A = 0;
  TCCR5B = 0;
  // Want 200Hz relative to base clock
  // Set OCR5A (Top): 16MHz/200Hz /2 = 40000 steps 
  OCR5A = 40000;
  // Configure Timer/Counter 5
  // Select P & F Correct PWM Mode, and OCR5A as TOP. 

  TCCR5B = _BV(WGM52) // CTC Mode with OCR5A as Top 
         | _BV(CS50); // Enable base clock with no prescale
  TIMSK5 = _BV(OCIE5A); // Enable OCRA Compare ISR


  sei(); // Enable interrupts
  
  delay(1000);

}

void loop() {

  Serial.println("Main loop");
  // Just wait here for interrupt to fire.
  if (plasmaSweep == true){
    test_func();
  }
}

// Description: Used to test plasma experiment timer frequency
// Implementation: Toggle an output pin and read with oscilloscope
// Using Timer 4, to allow possibility of multiple experiments simultaneously
// At vastly different frequencies if necessary
// i.e. Edit Timer4's pre-scale wihout affecting Timer1 or Timer3 prescale 


// Should create sawtooth output?
void test_func() {

  // Region Boundaries (Volts) & Step Sizes (Volts)
  // [-5, -3] @ 0.2V Steps
  // [-3, 1]  @ 0.02V Steps
  // [1, 5]   @ 0.1V Steps

  Serial.println("Entering test function.");

  // Region boundaries - Expressed as a percentage of the duty cycle of the op-amp output voltage
  float dutyMaxNeg = 41.6667; // Maximum negative voltage allowable: (-) 5v/12v * 100 = 41.6667%
  float dutyCritNeg = 25.000; // Negative voltage at lower end of critical region: (-)3v/12v * 100 = 25%
  float dutyCritPos = 8.3333; // Maximum positive voltage in critical region: 1v/12v * 100 = 8.3333%
  float dutyMaxPos = 83.3333; // Maximum positive voltage allowable: 10v/12v = 0.8333 = 83.3333%

  //int testPin = Timer4_A_PIN;
  // Step Sizes
  float criticalStepSize = 0.1667; // 0.02 Volts/Step:  0.02/12v * 100 = 0.1667 
  float smallStepSize = 0.8333;    // 0.1 Volts/Step:    .1v/12v * 100 = 0.8333
  float bigStepSize = 1.6667;      // 0.2 Volts/Step:   0.2v/12v * 100 = 1.6667
  
  // Use Timer4_A_PIN (Mega Pin 15) for negative voltages
  
  // Should be -5v to -3v output from negative op-amp
  // ***OR*** 2.0833v to 1.250v from Mega Pin
  // 0.2v step size out of op-amp
  for (float dutyCycle = dutyMaxNeg; dutyCycle > dutyCritNeg; dutyCycle = dutyCycle - bigStepSize){
    Serial.print("Duty cycle = ");
    Serial.println(dutyCycle);
    Timer4.pwm(Timer4_A_PIN, (dutyCycle / 100) * 1023);
    delay(2000);
  }

  delay(5000);
  // Should be -3v to 0v output from negative op-amp
  // ***OR*** 1.250v to 0v from Mega Pin
  // 0.1v step size out of op-amp
  for (float dutyCycle = dutyCritNeg; dutyCycle > 0; dutyCycle = dutyCycle - criticalStepSize){
    Serial.print("Duty cycle = ");
    Serial.println(dutyCycle);
    Timer4.pwm(Timer4_A_PIN, (dutyCycle / 100) * 1023);
    delay(100);
  }

  // Disable A pin during use of B pin
  Timer4.pwm(Timer4_A_PIN, 0);
  
  // Use Timer4_B_PIN (Mega Pin 16) for positive voltages
  
  // Should be 0v to 1v from positive op-amp
  // ***OR*** 0v to .41667v from Mega Pin
  // 0.02v step size out of op amp
  for (float dutyCycle = 0; dutyCycle < dutyCritPos; dutyCycle = dutyCycle + criticalStepSize){
    Serial.print("Duty cycle = ");
    Serial.println(dutyCycle);
    Timer4.pwm(Timer4_B_PIN, (dutyCycle / 100) * 1023);
    delay(100);
  }

  delay(3000);
  
  // Should be 1v to 10v from positive op-amp
  // ***OR*** .41667v to 4.1667v from Mega Pin
  // 0.1v step size
  for (float dutyCycle = dutyCritPos; dutyCycle < dutyMaxPos; dutyCycle = dutyCycle + smallStepSize){
    Serial.print("Duty cycle = ");
    Serial.println(dutyCycle);
    Timer4.pwm(Timer4_B_PIN, (dutyCycle / 100) * 1023);
    delay(100);
  }
  delay(5000);
  // Disable B pin for switch to A pin
  Timer4.pwm(Timer4_B_PIN, 0);

  plasmaSweep = false;
}


//// Using ISR to be more accurate with how program actually runs
//// Interrupt Service Routine (ISR)
//// Toggles pin on compare
ISR(TIMER5_COMPA_vect) {
  plasmaSweep = true; // Trigger test code
  }
