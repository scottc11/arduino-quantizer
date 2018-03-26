# Arduino Programming Practices

#### Things to Note:
- Arduino code is essentially C/C++
- using dictionaries is pretty much out of the question due to memory limits

---

## [I2C](https://learn.sparkfun.com/tutorials/i2c)

- I2C requires a mere two wires, like asynchronous serial, but those two wires can support up to 1008 slave devices.
- most I2C devices can communicate at 100kHz or 400kHz.
- It can be fairly trivially implemented in software.

#### Signals
- Each I2C bus consists of two signals: SCL and SDA. SCL is the clock signal, and SDA is the data signal.

---

## [Bitwise Operators](https://www.arduino.cc/reference/en/language/structure/bitwise-operators/bitwiseand/)
- One of the most common uses of bitwise AND is to select a particular bit (or bits) from an integer value, often called masking.
- [BitMath Tutorial](http://playground.arduino.cc/Code/BitMath)

In Arduino, the type int is a 16-bit value, so using & between two int expressions causes 16 simultaneous AND operations to occur.


```
int a =  92;    // in binary: 0000000001011100
int b = 101;    // in binary: 0000000001100101
int c = a & b;  // result:    0000000001000100, or 68 in decimal.
```
Each of the 16 bits in `a` and `b` are processed by using the bitwise AND, and all 16 resulting bits are stored in `c`, resulting in the value `01000100` in binary, which is `68` in decimal.

#### Common Bitwise Uses
- Saving memory by packing up to 8 true/false data values in a single byte.
- Turning on/off individual bits in a control register or hardware port register.
- Performing certain arithmetic operations involving multiplying or dividing by powers of 2.

###### Bitwise NOT operator

Bitwise NOT changes each bit to its opposite: 0 becomes 1, and 1 becomes 0. For example:
```
int a = 103;    // binary:  0000000001100111
int b = ~a;     // binary:  1111111110011000 = -104
```

###### Bit Shift Operators --> `>>` or `<<`
There are two bit shift operators in C++: the left shift operator `<<` and the right shift operator `>>`. These operators cause the bits in the left operand to be shifted left or right by the number of positions specified by the right operand. For example:
```
int a = 5;        // binary: 0000000000000101
int b = a << 3;   // binary: 0000000000101000, or 40 in decimal
int c = b >> 3;   // binary: 0000000000000101, or back to 5 like we started with
```
When you shift a value x by y bits (x << y), the leftmost y bits in x are lost, literally shifted out of existence.



### Port registers of the Atmega8

```
void setup()
{
  // set pins 1 (serial transmit) and 2..7 as output,
  // but leave pin 0 (serial receive) as input
  // (otherwise serial port will stop working!) ...
  DDRD = B11111110;  // digital pins 7,6,5,4,3,2,1,0

  // set pins 8..13 as output...
  DDRB = B00111111;  // digital pins -,-,13,12,11,10,9,8

  // Turn off digital output pins 2..7 ...
  PORTD &= B00000011;   // turns off 2..7, but leaves pins 0 and 1 alone

  // Write simultaneously to pins 8..13...
  PORTB = B00111000;   // turns on 13,12,11; turns off 10,9,8
}
```

Sometimes you might need to set multiple output pins at exactly the same time. Calling `digitalWrite(10,HIGH);` followed by `digitalWrite(11,HIGH);` will cause pin 10 to go `HIGH` several microseconds before pin 11, which may confuse certain time-sensitive external digital circuits you have hooked up. Alternatively, you could set both pins high at exactly the same moment in time using `PORTB |= B1100;`

------

### Button Logic

##### Turn LED On/Off with push button

```
int previousButtonState = LOW;

void loop() {
  int currentButtonState = cap.touched() & (1 << 0);

  if (currentButtonState == HIGH && oldButtonState == LOW) {
    // Switch the state of the output
    digitalWrite(ledPins[0], !digitalRead(ledPins[0]) );
  }

  previousButtonState = currentButtonState;
}
```

---
## LIBRARIES

#### [Adafruit MPR121](https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/overview)
