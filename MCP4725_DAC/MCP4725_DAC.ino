/*!
 * Modular synths tend to follow a scheme were a specific change in voltage – 
 * such as 1.00 volts – results in a precisely one octave change in pitch. In a 
 * 1 volt per octave scheme, 1/12 of a volt change results in a semitone change in pitch.
 * 
 * 5.00v == 5 octaves
 * 1.00v == 1 octave
 * 0.08333V == 1 step
 * 
 * MCP4725 Library
 * input == 0 .. 4095
 * 
 * 1 octave == 819
 * 1 step = 68.25 
 * 
*/



#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

void setup() {
  Serial.begin(9600);

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  dac.begin(0x62);
  
}

void loop() {

  uint16_t output = 819;

  for (uint16_t i=68.25; i<4095; i=i+68.25) {
    Serial.println(i);
    dac.setVoltage(i, false);
    delay(1000);
  }
}
