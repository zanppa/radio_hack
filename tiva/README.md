# I2C EEPROM emulator (slave)
This is a very simple I2C slave that emulates 256 byte (2kbit) EEPROM.
Protocol is based on Microchip's 24LC02B datasheet.

Write is not supported, except address write.
Only continuous reading from 0 to end has been tested.

Pins used:
PA6 is clock (SCL)
PA7 is data (SDA)

Pull-up is not implemented or tested, on my circuit there was an external pull-up.

## Compiling
This is written with Energia 15, http://energia.nu/
The .ino is the main file used in Energia but it is not needed and since left empty.
It is necessary to modify the main.cpp in Energia's tivac library, in my case the directory is
  `%LOCALAPPDATA%\Energia15\packages\energia\hardware\tivac\1.0.3\cores\tivac`.
Change init from
`void _init(void)`
to
`void __attribute__((weak)) _init(void)`
and main from
`int main(void)`
to
`int __attribute__((weak)) main(void)`
This way we can override them in our own main and get rid off all "Arduino  style" initializations!

## More info
This was used to emulate EEPROM of KTmicro's KT0922 based FM/AM radio.
Pressing SW1 or SW2 during boot changed the second FM band of the radio.
The radio reads full memory content on startup.
