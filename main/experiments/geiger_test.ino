void setup_geiger(){
    // Set up two interrupt pins
    // Generate interrupt on rising
    // This section shouldn't need optimized for speed
    attachInterrupt(digitalPinToInterrupt(2), geiger_0_isr, RISING); // Geiger 0
    attachInterrupt(digitalPinToInterrupt(3), geiger_1_isr, RISING); // Geiger 1
    
    // Reset Geiger counts
    Geiger[0] = 0;
    Geiger[1] = 0;
    
    geigerConfigured = true;
}

void geiger() {
  // Goal: collect particle counts from 2 detectors
  // Implementation: count pulses at 2 pins.
  // Interrupt-based
  
  // Hang out and make sure counters haven't overflown

  switch(Geiger[0]) {
    case -2147483647:
      Geiger[0] = 0;
      log_overflow(0);
      break;
    default :
      // Do nothing
      break;
  }

   switch(Geiger[1]) {
    case -2147483647:
      Geiger[0] = 0;
      log_overflow(1);
      break;
    default :
      // Do nothing
      break;
  }

}

// TODO
void store_geiger_data(){
  
}

// Called when geiger experiment is finished
void teardown_geiger(){
  // Detach the two interrupts so they are ignored
  detachInterrupt(digitalPinToInterrupt(2)); // Geiger 0
  detachInterrupt(digitalPinToInterrupt(3)); // Geiger 1
  
  geigerConfigured = false;
}


void log_overflow (int geiger_num){

  //TODO
}

// Interrupt Service Routine (ISR)
// Geiger pin 0
void geiger_0_isr() {
    Geiger[0] = Geiger[0] + 1;
}

// Interrupt Service Routine (ISR)
// Geiger pin 1
void geiger_1_isr() {
    Geiger[1] = Geiger[1] + 1;
}