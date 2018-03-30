
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_MPR121.h>

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

Adafruit_MCP4725 dac;

// TONICS
int I = 0;
int II = 0;
int III = 0;
int IV = 0;
int V = 0;
int VI = 0;
int VII = 0;
int VIII = 0;


// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // LED pins
int ledCnt = 8; // LED COUNT
int quantizedVoltages[] = {0, 0, 0, 0, 0, 0, 0, 0};

String MODE = "mono"

void setup() {
  Serial.begin(9600);

  // Connect MPR121 touch sensors @ I2C address 0x5A (default)
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  
  // Connect MCP4725A1 DAC @ I2C address 0x62 (default)
  dac.begin(0x62);
  
  // Set pinouts for LEDs
  for(int p=0; p<ledCnt; p++) {
    pinMode(ledPins[p], OUTPUT); // Set the mode to OUTPUT
  }
}

void loop() {

  // Get the currently touched pads
  // cap.touched will return 16 bits (one byte), with each bit (from 0 - 12) representing the 
  // corrosponding touch pad
  currtouched = cap.touched();
  
  // Iterate over first 8 touch sensors
  for (uint8_t i=0; i<8; i++) {

    // BUTTON TOUCHED
    // if it *is* touched and *wasnt* touched before, alert!
    if ( (currtouched & _BV(i) ) && !( lasttouched & _BV(i) ) ) {
      Serial.print(i); Serial.print(" touched :: "); Serial.println(i, BIN);
      
      // toggle digital pin state based on previous state
      digitalWrite(ledPins[i], !digitalRead(ledPins[i]));
      
      // Set quantized voltage output
      quantizedVoltages[i] = i * 68.25;
      dac.setVoltage(quantizedVoltages[i], false);
    }



    // BUTTON RELEASED
    //  if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.print(" released :: "); Serial.println(i, BIN);

    }
  }
  
  // reset our state
  lasttouched = currtouched;
}


