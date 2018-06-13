### Components

- [SPDT Toggle Switch](https://www.digikey.ca/product-detail/en/e-switch/200MSP1T1B1M2QEH/EG2447-ND/378916)
- Arduino Pro Mini
- MPR121 Touch Matrix
- MCP4725 DAC
- LED x 8



# MODES

- when switching between modes, you have to somehjow preserve the previous modes active notes
- one way to do this is to store monophonic mode last touched Vout in a variable, and arpeggiator modes last count + active notes.
