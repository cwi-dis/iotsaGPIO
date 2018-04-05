# iotsaGPIO - web server that interfaces to hardware signals

This is a wifi http server that allows access to some 3.3v hardware signals (analog input, digital input, digital output, PWM output). It is built using the iotsa framework.

Home page is <https://github.com/cwi-dis/iotsaGPIO>.
This software is licensed under the [MIT license](LICENSE.txt) by the   CWI DIS group, <http://www.dis.cwi.nl>.

As distributed the service allows access to 6 digital ports (which can be configured as input, output or PWM-output) and one analog input.

## Software requirements

* Arduino IDE, v1.6 or later.
* The iotsa framework, download from <https://github.com/cwi-dis/iotsa>.

Or you can build using PlatformIO.

## Hardware requirements

* a iotsa board. Alternatively you can use any other esp8266 board, but then you may have to adapt the available I/O pins.