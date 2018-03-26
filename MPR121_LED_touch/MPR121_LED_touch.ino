#include <Wire.h>
#include <Adafruit_MPR121.h>

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9}; // LED pins
int ledCnt = 8; // LED COUNT

void setup() {
  Serial.begin(9600);

  while (!Serial) { // needed to keep leonardo/micro from starting too fast!
    delay(10);
  }

  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");


  for(int p=0; p<ledCnt; p++) {
    pinMode(ledPins[p], OUTPUT); // Set the mode to OUTPUT
  }
}

void loop() {

  // Get the currently touched pads
  // cap.touched will return 16 bits (one byte), with each bit (from 0 - 12) representing the 
  // corrosponding touch pad
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<8; i++) {

    // BUTTON TOUCHED
    // if it *is* touched and *wasnt* touched before, alert!
    if ( (currtouched & _BV(i) ) && !( lasttouched & _BV(i) ) ) {
      Serial.print(i); Serial.print(" touched :: "); Serial.println(i, BIN);
      
      // toggle digital pin state based on previous state
      digitalWrite(ledPins[i], !digitalRead(ledPins[i]));
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


//    1 <<  0  ==    1
//    1 <<  1  ==    2
//    1 <<  2  ==    4
//    1 <<  3  ==    8
//    1 <<  4  ==    16
//    1 <<  5  ==    32
//    1 <<  6  ==    64
//    1 <<  7  ==    128

//You can read all at once with
//
// cap.touched()
//
// Which returns a 16 bit value. Each of the bottom 12 bits refers to one sensor. So if you want to test if the #4 is touched, you can use
//
// if (cap.touched() & (1 << 4)) { do something }

// You can check its not touched with:
//
// if (! (cap.touched() & (1 << 4)) ) { do something }



