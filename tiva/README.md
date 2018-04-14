# I2C EEPROM emulator (slave)
This is a very simple I2C slave that emulates 256 byte (2kbit) EEPROM.
Protocol is based on Microchip's 24LC02B datasheet.

Write is not supported, except address write.
Only continuous reading from 0 to end has been tested.

Pins used:
PA6 is clock (SCL)
PA7 is data (SDA)

Pull-up is not implemented or tested, on my circuit there was an external pull-up.

## More info
This was used to emulate EEPROM of KTmicro's KT0922 based FM/AM radio.
Pressing SW1 or SW2 during boot changed the second FM band of the radio.
The radio reads full memory content on startup.
