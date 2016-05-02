void plasma_probe() {
  // Goal: sweep a bias voltage V_b and measure a current I (i.e. a voltage V_shunt = R_shunt*I)
  // Implementation: increment the voltage V_b and apply to an output pin.
  // Read the V_shunt at an input pin.
  // PWM- or loop-based.

  // Notes: Voltage will be bi-directional so IO pins will need alternated
  // Define as (R)everse (Voltage): from PF6 to PF5
  //           (F)orward (Voltage): from PF5 to PF6

  // Sweep voltage from -5V to 0V - SLOW DOWN FOR CRITICAL REGION
  // Change 'direction'
  // Sweep voltage from 0V to 5V

  // Temp: 
  CRIT_START = XXX;
  CRIT_END = YYY;

  // (R)everse section of current flow

  
//  int x = 1;
//   for (int i = 0; i < 255; i = i + 1){
//      analogWrite(plasmaPin_Out, i);
//      delay(10); // Need a delay?
//      X = analogRead(plasmaPin_In) // To be handled
//   } 

}

// Called when plasma experiment is being turned on
// Setup internal clock timer
// Consider hard  coding steps to control potential overflow conditions
void setup_plasma(int hertz, int main_step, int crit_step) {

  // Disable interrupts
  cli();
  // Clear Timer3 registers
  TCCR3A = 0;
  TCCR3B = 0;
  // Set OCR3A (Top): 16MHz/500Hz /2 = 16000 steps 
  // Divide 16000 by 2 = 8000 because waveforms are centered around OCR1A (because toggle)
  OCR3A = (16000000/hertz) / 2;
  // Configure Timer/Counter 3
  // Select P & F Correct PWM Mode, and OCR3A as TOP. 
  
  TCCR3B = _BV(WGM12) // CTC Mode with OCR3A as Top 
         | _BV(CS10); // Enable base clock without pre-scale
  
  TIMSK3 = _BV(OCIE3A); // Enable OCRA Compare ISR
  
  sei(); // Enable interrupts
  
  plasmaConfigured = true;
}

// Interrupt Service Routine (ISR)
// Sets flag to start another round of plasma sampling
ISR(TIMER3_COMPA_vect) {
  if ( plasmaSample == false)
    plasmaSample = true;
  else
    Serial.print("Plasma flag sent to true and attempted to sample again - flag unchanged.");
}