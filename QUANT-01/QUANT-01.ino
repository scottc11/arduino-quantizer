
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
int VinPin = A1;    // Voltage Input Pin
int Vin = 0;        // Variable to store value of VinPin
int Vout = 0;       // quantized voltage output

// MODE
int MODE_PIN = A0;   // toggles QUANTIZER_MODE variable true/false
bool newModeSwitchState = 0;
bool oldModeSwitchState = 0;
bool QUANTIZER_MODE = false;

int GATE_PIN = A2;
int OCTAVE_UP_PIN = 9;
int OCTAVE_DOWN_PIN = 8;

// Keeps track of the last pins touched
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

// Toggle Switches
int switchPins[] = {0, 0, 10, 0, 11, 12, 13, 0};

// variables to hold bitmask values of switch states. These will essentially be the equivalent of an array containing 
// 8 integers with the values of 0 or 1. Use bitRead(newSwitchStates, index) to get the value of a bit
byte newSwitchStates = 0;
byte oldSwitchStates = 0;


int ledPins[] = {9, 8, 7, 6, 5, 4, 3, 2}; // LED pins: they are backwards order because I'm a goof.
int OCTAVE = 0;
int OCTAVE_VALUES[] = {0, 819, 1638, 2457, 3276};

int quantizedVoltages[8][2] = {
    { 0, 0 }, // I
    { 136.5, 0 },
    { 204.75, 273 },   // min3, maj3
    { 341.25, 409.5 }, // per4, aug4
    { 409.5, 477.75 }, // dim5, per5
    { 546, 614.25 },   // min6, maj6
    { 682.5, 750.75 }, // min7, maj7
    { 819, 0 } // VIII
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


// -----------------------------------
// SET ACTIVE NOTES FOR Vout
// -----------------------------------
void setActiveVoltages() {
  activeCount = 0;
  for (int i=0; i<LENGTH; i++) {                              
    if (activeQuantizedNotes[i] == true) {
      uint8_t state = bitRead(newSwitchStates, i);
      activeVoltages[activeCount] = quantizedVoltages[i][state];
      activeCount += 1;
    }
  }
  setActiveVoltageThresholds(activeCount);
}


// -----------------------------------
// SET VOLTAGE OUT
//   - final stage determining which voltage to set the DAC too
// -----------------------------------
void setVoltageOut(int index) {
  if (QUANTIZER_MODE) {
    Vin = analogRead(VinPin);
    for (int i=0; i<activeCount; i++) {
      if (Vin < thresholdArray[i]) {
        Vout = activeVoltages[i] + OCTAVE_VALUES[OCTAVE]; // state not needed here, see --> setActiveVoltages()
  
        // Set quantized voltage output
        dac.setVoltage(Vout, false);
        break;
      }
      delay(1);
    }
  } else {
    uint8_t state = bitRead(newSwitchStates, index);
    Vout = quantizedVoltages[index][state] + OCTAVE_VALUES[OCTAVE];
    dac.setVoltage(Vout, false); // Set quantized voltage output
  }
}


// -----------------------------------
// SET ACTIVE NOTES
// - based on current mode of quantizer, set the 'active' notes based on user selection via touch pads
// note: this is not setting the voltage!
// -----------------------------------
void setActiveNotes(int index) {
  if (QUANTIZER_MODE) {
    // activate / deactivate notes
    // activeNotes[index] = !activeNotes[index];

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



// -----------------------------------
// SET/TOGGLE MODE
// -----------------------------------
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



// -----------------------------------
// SET OCTAVE
// -----------------------------------
void setOctave(int BUTTON) {
  if (BUTTON == OCTAVE_UP_PIN) {
    if (OCTAVE < 4) {
      OCTAVE += 1;
    }
  }
  else if (BUTTON == OCTAVE_DOWN_PIN) {
    if (OCTAVE > 0) {
      OCTAVE -= 1;
    }
  }
  
}





// ==============================================================================================================
// SETUP
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
  pinMode(13, INPUT);
  pinMode(12, INPUT);
  pinMode(11, INPUT);
  pinMode(10, INPUT);```````````````````````````````````````````````````````````
  
  // A1 acting as digitalInput
  pinMode(MODE_PIN, INPUT_PULLUP);
  toggleMode(digitalRead(MODE_PIN));

  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, HIGH);

}






// ==============================================================================================================
// LOOP
// ==============================================================================================================

void loop() {
  
  // Get the currently touched pads
  // cap.touched will return 16 bits (one byte), with each bit (from 0 - 12) representing the corrosponding touch pad
  currtouched = cap.touched();

  newModeSwitchState = digitalRead(MODE_PIN);

  if (newModeSwitchState != oldModeSwitchState) {
    Serial.print("Switching MODE to: "); Serial.println(newModeSwitchState);
    oldModeSwitchState = newModeSwitchState;
    toggleMode(newModeSwitchState);
  }



  // get switch states of each scale step
  for (uint8_t i=0; i < 8; i++) {
    if (i == 2 || i == 4 || i == 5 || i == 6) {
      uint8_t state = digitalRead(switchPins[i]);
      bitWrite(newSwitchStates, i, state);
    }
  }
  
  if ( newSwitchStates != oldSwitchStates ) {
    oldSwitchStates = newSwitchStates;
    setActiveVoltages();
    if (!QUANTIZER_MODE) {
      setVoltageOut(lastTouchedIndex);
    }
  }
  


  // Iterate over touch sensors
  for (uint8_t i=0; i<12; i++) {

    // BUTTON TOUCHED
    // if it *is* touched and *wasnt* touched before, alert!
    if ( (currtouched & _BV(i) ) && !( lasttouched & _BV(i) ) ) {
      Serial.print(i); Serial.print(" touched :: "); Serial.println(i, BIN);
      
      if (i < 8) {
        digitalWrite(GATE_PIN, HIGH); // SET GATE HIGH (opposite via schmitt trigger)
        setActiveNotes(i); 
      }
      else if (i > 7) {
        setOctave(i);
      }

    }



    // BUTTON RELEASED
    //  if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {

      
      digitalWrite(GATE_PIN, LOW); // SET GATE LOW

      Serial.print(" .             OCTAVE -->   ");
      Serial.println(OCTAVE);
      
      Serial.print(" .    active voltages -->   ");
      for (int i=0; i<LENGTH; i++) {
        Serial.print(activeVoltages[i]); Serial.print(" : ");
      }

      Serial.println("");
      Serial.print("Voltage IN thresholds -->   ");
      for (int i=0; i<LENGTH; i++) {
        Serial.print(thresholdArray[i]); Serial.print(" : ");
      }

      Serial.println("");
      Serial.print("  active tonics count -->   "); Serial.println(activeCount);

//      Serial.print(i); Serial.println(" released");
    }
  }

  // reset touch sensors state
  lasttouched = currtouched;

  // if in quantizer mode apply Vout based on Vin
  if (QUANTIZER_MODE) {
    setVoltageOut(0);
  }
}
