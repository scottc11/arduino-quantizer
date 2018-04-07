
#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_MPR121.h>

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

Adafruit_MCP4725 dac;

// CONSTANTS
int LENGTH = 8; // length of arrays


// TONICS


// VOLTAGE INPUT/OUTPUT
int VinPin = A0;    // Voltage Input Pin
int Vin = 0;        // Variable to store value of VinPin
int Vout = 0;       // quantized voltage output

// MODE
int ModePin = A1;   // toggles QUANTIZER_MODE variable true/false
bool newModeSwitchState = 0;
bool oldModeSwitchState = 0;
bool QUANTIZER_MODE = false;

// Keeps track of the last pins touched
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// Toggle Switches
int switchPins[] = {10, 11, 12, 13, 0, 0, 0, 0};
bool newSwitchStates[] = {0, 0, 0, 0, 0, 0, 0, 0};
bool oldSwitchStates[] = {0, 0, 0, 0, 0, 0, 0, 0};


int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // LED pins
int quantizedVoltages[8][2] = {
    {0, 0}, // I
    {136.5, 0},
    {204.75, 273},
    {341.25, 0},
    {477.75, 0},
    {614.25, 0},
    {750.75, 0},
    {819, 0} // VIII
  };

bool activeQuantizedNotes[] = {0, 0, 0, 0, 0, 0, 0, 0};
bool activeMonophonicNotes[] = {0, 0, 0, 0, 0, 0, 0, 0};
int lastTouchedIndex = 0;  // Needed specifically for monophonic notes when toggleing tonic switches

int activeCount = 0;   // how many notes are active/selected (0 === 1 active note)
int activeVoltages[8]; // active quantized voltages (from low to high)
int thresholdArray[8]; // int representation of Vin mapped to activeVoltages



// mapping Vin to a Vout in activeVoltages
void setActiveVoltageThresholds(int count) {
  int threshold = 1023 / count;
  for (int i=0; i<count; i++) {
    thresholdArray[i] = threshold * (i + 1);
  }
}



// SET ACTIVE NOTES FOR Vout
void setActiveVoltages() {
  activeCount = 0;
  for (int i=0; i<LENGTH; i++) {
    if (activeQuantizedNotes[i] == true) {
      int state = newSwitchStates[i];
      activeVoltages[activeCount] = quantizedVoltages[i][state];
      activeCount += 1;
    }
  }
  setActiveVoltageThresholds(activeCount);
}



void setVoltageOut(int index) {
  if (QUANTIZER_MODE) {
    Vin = analogRead(VinPin);
    for (int i=0; i<activeCount; i++) {
      if (Vin < thresholdArray[i]) {
        Vout = activeVoltages[i];
  
        // Set quantized voltage output
        dac.setVoltage(Vout, false);
        break;
      }
      delay(1);
    }
  } else {
    int state = newSwitchStates[index];
    Vout = quantizedVoltages[index][state];
    // Set quantized voltage output
    dac.setVoltage(Vout, false);
  }
}



void setActiveNotes(int index) {
  if (QUANTIZER_MODE) {
    // activate / deactivate notes
//    activeNotes[index] = !activeNotes[index];

    activeQuantizedNotes[index] = !activeQuantizedNotes[index];
    
    // toggle digital pin state (LEDs) based on active/inactive notes
    digitalWrite(ledPins[index], activeQuantizedNotes[index]);

    setActiveVoltages();
  }
  else {
    // set last pressed note HIGH and reset all others to LOW
    for (int i=0; i<LENGTH; i++) {
      if (i == index) {
        activeMonophonicNotes[i] = HIGH;
        lastTouchedIndex = i;
        setVoltageOut(lastTouchedIndex);
      } else {
        activeMonophonicNotes[i] = LOW;
      }
      digitalWrite(ledPins[i], activeMonophonicNotes[i]);
    }
  }
  
}


// SET MODE
void toggleMode(bool switchState) {
  QUANTIZER_MODE = switchState;

  if (QUANTIZER_MODE) {
    for (int i=0; i<LENGTH; i++) {
      // toggle digital pin state (LEDs) based on active/inactive notes
      digitalWrite(ledPins[i], activeQuantizedNotes[i]);
      setActiveVoltages();
    }
    
  } else {
    for (int i=0; i<LENGTH; i++) {
      // toggle digital pin state (LEDs) based on active/inactive notes
      digitalWrite(ledPins[i], activeMonophonicNotes[i]);
      if (activeMonophonicNotes[i] == HIGH) {
        setVoltageOut(i);
      }
    }
  }
}


// ==============================================================================================================

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
  for(int p=0; p<LENGTH; p++) {
    pinMode(ledPins[p], OUTPUT); // Set the mode to OUTPUT
  }
  // Set pinouts for toggle Switches
  pinMode(12, INPUT);

  // A1 acting as digitalInput
  pinMode(A1, INPUT_PULLUP);
  toggleMode(digitalRead(A1));
}







void loop() {

  // Get the currently touched pads
  // cap.touched will return 16 bits (one byte), with each bit (from 0 - 12) representing the corrosponding touch pad
  currtouched = cap.touched();

  newModeSwitchState = digitalRead(A1);

  if (newModeSwitchState != oldModeSwitchState) {
    Serial.print("Switching MODE to: "); Serial.println(newModeSwitchState);
    oldModeSwitchState = newModeSwitchState;
    toggleMode(newModeSwitchState);
  }

  
  newSwitchStates[2] = digitalRead(12);

  if ( newSwitchStates[2] != oldSwitchStates[2] ) {
    Serial.print("switched: "); Serial.println(newSwitchStates[2]);
    oldSwitchStates[2] = newSwitchStates[2];
    setActiveVoltages();
    if (!QUANTIZER_MODE) {
      setVoltageOut(lastTouchedIndex);
    }
  }

  // Iterate over first 8 touch sensors
  for (uint8_t i=0; i<LENGTH; i++) {

    // BUTTON TOUCHED
    // if it *is* touched and *wasnt* touched before, alert!
    if ( (currtouched & _BV(i) ) && !( lasttouched & _BV(i) ) ) {
      Serial.print(i); Serial.print(" touched :: "); Serial.println(i, BIN);

      setActiveNotes(i);
    }



    // BUTTON RELEASED
    //  if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {

      Serial.print("active voltages -->   ");
      for (int i=0; i<LENGTH; i++) {
        Serial.print(activeVoltages[i]); Serial.print(" : ");
      }

      Serial.println("");
      for (int i=0; i<LENGTH; i++) {
        Serial.print(thresholdArray[i]); Serial.print(" : ");
//        if (!activeNotes[i]) {
//          digitalWrite(ledPins[i], LOW);
//        }
      }

      Serial.println("");
      Serial.print("active tonics count: "); Serial.println(activeCount);

      Serial.print(i); Serial.println(" released");
    }
  }

  // reset touch sensors state
  lasttouched = currtouched;

  // if in quantizer mode apply Vout based on Vin
  if (QUANTIZER_MODE) {
    setVoltageOut(0);
  }
}
