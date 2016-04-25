#include <Arduino.h>
#include <Pins_Arduino.h>
#include <Wire.h>

// Arduino code based on RockSat 2012, 2013, 2014
//
// Last update - April 10 2016

//int sdaPin = 20 // PD1/SDA, Mega Pin 44
//int sclPin = 21 // PD0/SCL, Mega Pin 43

typedef struct {
  int bigEndian:1;
  int chan0:10;
  int chan1:10;
  int chan2:10;
  int lilEndian:1;
}radioStruct;


// Struct to hold digitized radio channel data
radioStruct radioData;
// Bool used to determine if radio is currently configured
bool radioConfigured;
// Bool triggered by timer ISR to read radio channels
bool radioSample;

// Geiger data
volatile long Geiger[2];  // Max values of 2147483647
// Bool to determine if Geiger is configured
bool geigerConfigured;

void setup() {
  setup_general();

  setup_geiger();

//  setup_plasma_probe();

  setup_adc();

}

// TODO - State Machine
// Remember - switch cases are faster
void loop() {
//  general();

//  geiger();

//  plasma_probe();

//  radio();

//  output_data();
  
}


// Initialize IO Pins    
// Note: Atmega pins default to inputs
// redefined solely for clarity.
void setup_general() {
        
    // Three pins for Radio Channel Inputs
    DDRF = ~_BV(PF2)  // Pin PF2 - Mega pin 95
         | ~_BV(PF1)  // Pin PF1 - Mega pin 96
         | ~_BV(PF0); // Pin PF0 = Mega pin 97
    // Disable pullups and make it tri-state
    PORTF = 0x07;

    // Plasma probe pins
    // Input (Voltage to read)
    // AND'd with radio pins above
    DDRF &= ~_BV(PF7); // Pin PF7, Mega pin 90
    // Output (Sweep Voltage)
    DDRK = _BV(PF6); // Pin PF6 - Mega pin 91 // Originally tried PK0, pin 89
    
    // Two pins for Geiger counter inputs
    DDRE = ~_BV(PE5)  // PE5/INT5, Mega Pin 7
         | ~_BV(PE4); // PE4/INT4, Mega Pin 6
    
    // TODO - I2C Pins
    DDRD = 0x00; // temp - set to correct, depends on i2c implementation
    PORTD = 0x00; //temp - set to correct, depends on i2c implementation
}

void setup_adc() {
  
  // Initialize the ADC first
  
  // Setup ADC multiplexer selection register (ADMUX) with these settings:
  // Zero out the register
  ADMUX = 0x00;
  
  // ADMUX 7:6 - Reference selection bits
  // Currently selects internal 5V Ref - requires external cap at AREF pin
  ADMUX |= ~_BV(REFS1) | _BV(REFS0);

  // ADMUX 5 - ADC left adjust result
  // Stays as zero
  // ADMUX 4:0 - Analog channel and gain selection
  // Leave as zeros, will be adjusted to select input to read

  // Setup ADC status register A (ADCSRA) with these settings:
  
  // Zero out the register.
  ADCSRA = 0x00;
  
  // ADCSRA 7:6 - Enable Bit, Single Conversion
  // TODO: Probably need to clear ADEN when not running experiment to save power
  // otherwise ADC consumes energy
  
  // Single conversion is used here to give ADC time to initialize
  ADCSRA |= _BV(ADEN) | _BV(ADSC);

  // ADCSRA 5:3 - Auto Trigger, Interrupt Flag, Interrupt Enable
  // Stay as zeros

  // ADCSRA 2:0 - Prescaler Select Bits
  // Prescale = 128, gives ADC Clock of 125 kHz using 16 mHz base clock
  // Needs to be in range of 50 kHz to 200 kHz for best accuracy
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
  
  // Setup ADC status register B (ADCSRB) with these settings:
  
  // Zero out the register.
  ADCSRB = 0x00;

  // ADCSRB 7 - Reserved bit
  // Stays as zero
  // ADCSRB 6:3 - Stay as zeros (for now)
  // ADCSRB 2:0 - Auto trigger source
  
}

void teardown_adc(){
  //TODO
}

void plasma_probe() {
  // Goal: sweep a bias voltage V_b and measure a current I (i.e. a voltage V_shunt = R_shunt*I)
  // Implementation: increment the voltage V_b and apply to an output pin.
  // Read the V_shunt at an input pin.
  // PWM- or loop-based.
  
//  int x = 1;
//   for (int i = 0; i < 255; i = i + 1){
//      analogWrite(plasmaPin_Out, i);
//      delay(10); // Need a delay?
//      X = analogRead(plasmaPin_In) // To be handled
//   } 

}
