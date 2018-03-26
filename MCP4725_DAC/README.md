
## [MCP4725 DAC](https://www.adafruit.com/product/935)

- [tutorial](https://learn.adafruit.com/mcp4725-12-bit-dac-tutorial)
- The output voltage is rail-to-rail and proportional to the power pin so if you run it from 3.3V, the output range is 0-3.3V. If you run it from 5V the output range is 0-5V.

#### Wiring

- connect `VDD` to your microcontroller power pin (3-5V), `GND` to ground, `SDA` to I2C Data (on the Arduino Uno, this is `A4`), `SCL` to I2C Clock(on the Arduino Uno, this is `A5`) and listen on `VOUT`.

- `A0` allows you to change the I2C address. By default (nothing attached to A0) the address is hex `0x62`. If `A0` is connected to `VDD` the address is `0x63`.
- `VOUT` is the voltage out from the DAC! The voltage will range from 0V (when the DAC value is 0) to VDD (when the DAC 'value' is the max 12-bit number: `0xFFF`)

---

#### [Adafruit MCP4725 Library](https://github.com/adafruit/Adafruit_MCP4725)

There is only one function that comes with the Adafruit MCP4725 Library, and it is pretty strait forward. In works with Arduino's `Wire` library so you must also import that at the top.

Essentially the function accepts a number between `0` and `4095` which gets evaluated into the Voltage out.  So passing `2500` will give you exactly `3.14V` on the `Vout` pin of the DAC.

```
/**************************************************************************/
/*!
   @brief  Sets the output voltage to a fraction of source vref.  (Value
           can be 0..4095)
   @param[in]  output
               The 12-bit value representing the relationship between
               the DAC's input voltage and its output voltage.
   @param[in]  writeEEPROM
               If this value is true, 'output' will also be written
               to the MCP4725's internal non-volatile memory, meaning
               that the DAC will retain the current voltage output
               after power-down or reset.
*/
/**************************************************************************/

void Adafruit_MCP4725::setVoltage( uint16_t output, bool writeEEPROM )
{
#ifdef TWBR
 uint8_t twbrback = TWBR;
 TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
#endif
 Wire.beginTransmission(_i2caddr);
 if (writeEEPROM)
 {
   Wire.write(MCP4726_CMD_WRITEDACEEPROM);
 }
 else
 {
   Wire.write(MCP4726_CMD_WRITEDAC);
 }
 Wire.write(output / 16);                   // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
 Wire.write((output % 16) << 4);            // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
 Wire.endTransmission();
#ifdef TWBR
 TWBR = twbrback;
#endif
}
```
