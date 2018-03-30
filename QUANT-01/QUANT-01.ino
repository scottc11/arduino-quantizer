
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_MPR121.h>

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

Adafruit_MCP4725 dac;

// TONICS
int I = 819;
int II = 955.5;
int III = 1092;
int IV = 1160.25;
int V = 1296.75;
int VI = 1433.25;
int VII = 1569.75;
int VIII = 1638;

// VOLTAGE INPUT
int VinPin = A0;    // Voltage Input Pin
int Vin = 0;        // Variable to store value of VinPin
int Vout = 0;       // quantized voltage output

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

int arrLength = 8; // length of arrays

int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // LED pins
int quantizedVoltages[] = {I, II, III, IV, V, VI, VII, VIII}; // not actual voltage, but int conversion
bool activeTonics[] = {0, 0, 0, 0, 0, 0, 0, 0}; // true == 1, false == 0

int activeCount = 0;
int activeNotes[8];
int thresholdArray[8];


// SET ACTIVE NOTES FOR Vout
void setActiveNotes() {
  activeCount = 0;
  for (int i=0; i<arrLength; i++) {
    if (activeTonics[i] == true) {
      activeNotes[activeCount] = quantizedVoltages[i];
      activeCount += 1;
    }
  }
}

void setActiveVoltageThresholds(int count) {
  int threshold = 1023 / activeCount;
  for (int i=0; i<count; i++) {
    thresholdArray[i] = threshold * (i + 1);
  }
}

void setVoltageOut() {
  Vin = analogRead(VinPin);
  for (int i=0; i<activeCount; i++) {
    if (Vin < thresholdArray[i]) {
      Vout = activeNotes[i];
      
      // Set quantized voltage output
      dac.setVoltage(Vout, false);
    }
  }
}


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
  for(int p=0; p<arrLength; p++) {
    pinMode(ledPins[p], OUTPUT); // Set the mode to OUTPUT
  }
}

void loop() {

  // Get the currently touched pads
  // cap.touched will return 16 bits (one byte), with each bit (from 0 - 12) representing the 
  // corrosponding touch pad
  currtouched = cap.touched();
  
  
  
  // Iterate over first 8 touch sensors
  for (uint8_t i=0; i<arrLength; i++) {

    // BUTTON TOUCHED
    // if it *is* touched and *wasnt* touched before, alert!
    if ( (currtouched & _BV(i) ) && !( lasttouched & _BV(i) ) ) {
      Serial.print(i); Serial.print(" touched :: "); Serial.println(i, BIN);
      
      // activate / deactivate tonic
      activeTonics[i] = !activeTonics[i];
      
      // toggle digital pin state based on tonic state
      digitalWrite(ledPins[i], activeTonics[i]);

      setActiveNotes();
      setActiveVoltageThresholds(activeCount);
      Serial.print("active tonics count: "); Serial.println(activeCount);
      Serial.print("threshold: "); Serial.println(thresholdArray[0]);
      Serial.print("first active note: "); Serial.println(activeNotes[0]);
      
    }



    // BUTTON RELEASED
    //  if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      
      Serial.print(i); Serial.println(" released");
    }
  }
  
  // reset our state
  lasttouched = currtouched;

  // apply Vout based on Vin
  setVoltageOut();
}


