// Called when radio experiment is being turned on
// Setup internal clock timer
void setup_radio() {
  radioSample = false; // Make sure sample flag is false
  
  // Disable interrupts
  cli();
  // Clear Timer1 registers
  TCCR1A = 0;
  TCCR1B = 0;
  // Set OCR1A (Top): 16MHz/500Hz /2 = 16000 steps 
  // Divide 16000 by 2 = 8000 because waveforms are centered around OCR1A (because toggle)
  OCR1A = 8000;
  // Configure Timer/Counter 1
  // Select P & F Correct PWM Mode, and OCR1A as TOP. 
  
  TCCR1B = _BV(WGM12) // CTC Mode with OCR1A as Top 
         | _BV(CS10); // Enable base clock without pre-scale
  
  TIMSK1 = _BV(OCIE1A); // Enable OCRA Compare ISR
  
  sei(); // Enable interrupts
  
  radioConfigured = true;
}

// Called by Timer1A ISR at a preset frequency
void radio() {
  // Goal: measure the power of 3 radio frequencies. 
  // Implementation: measure the voltage at 3 input pins.
  // - TODO *** Time-tag on first conversion of struct
  // - Select channel
  // - Trigger conversion
  // - Wait for conversion to finish
  // - Store result
  // - Transmit result
  // - Continue

  ADMUX = (ADMUX & ~0x0F) | (0x00 & 0x0F); // Set ADMUX to ch. 0
  ADCSRA |= _BV(ADSC); // Start single conversion
  while(ADCSRA & _BV(ADSC)); // Wait for ADSC to be set to 0, then continue
  store_radio_data(radioData, 0);
  
  ADMUX = (ADMUX & ~0x0F) | (0x01 & 0x0F); // Set ADMUX to ch. 1
  ADCSRA |= _BV(ADSC); // Start single conversion
  while(ADCSRA & _BV(ADSC)); // Wait for ADSC to be set to 0, then continue
  store_radio_data(radioData, 1);
  
  ADMUX = (ADMUX & ~0x0F) | (0x02 & 0x0F); // Set ADMUX to ch. 2
  ADCSRA |= _BV(ADSC); // Start single conversion
  while(ADCSRA & _BV(ADSC)); // Wait for ADSC to be set to 0, then continue
  store_radio_data(radioData, 2);

  // Write the data over I2C
  // SDA & SCL: Board pins 20 & 21 - 2560 pins 44 & 43
  // Wire.write((uint8_t *)&radioData, sizeof(radioData));
}

// Is passed the channel a reading was taken from
// Then stores reads from ADCH & ADCL and stores as appropriate
void store_radio_data(radioStruct radioData, int channel) {

  radioData.bigEndian = 1;
  radioData.lilEndian = 0;

  switch (channel) {
    case 0:
        radioData.chan0 = 0x03FF & ((ADCH << 8) | (ADCL));
        break;
    case 1:
        radioData.chan1 = 0x03FF & ((ADCH << 8) | (ADCL));
        break;
    case 2:
        radioData.chan2 = 0x03FF & ((ADCH << 8) | (ADCL));
        break;
  }
}

// Called when VLF experiment is terminated
void teardown_radio() {
    TCCR1B = ~_BV(WGM12) // CTC Mode with OCR1A as Top 
         | ~_BV(CS10);   // Disable clock-source
    
    TIMSK1 = ~_BV(OCIE1A); // Disable OCRA Compare ISR
    
    radioConfigured = false;
}

// Interrupt Service Routine (ISR)
// Sets flag to sample radio channels
ISR(TIMER1_COMPA_vect) {
  if ( radioSample == false)
    radioSample = true;
  else
    Serial.print("Flag sent to true and attempted to sample again - flag unchanged.");
}