# iotsaGPIO - web server that interfaces to hardware signals

![build-platformio](https://github.com/cwi-dis/iotsaGPIO/workflows/build-platformio/badge.svg)
![build-arduino](https://github.com/cwi-dis/iotsaGPIO/workflows/build-arduino/badge.svg)

This is a wifi http server that allows access to some 3.3v hardware signals (analog input, digital input, digital output, PWM output). It is built using the iotsa framework.

Home page is <https://github.com/cwi-dis/iotsaGPIO>.
This software is licensed under the [MIT license](LICENSE.txt) by the   CWI DIS group, <http://www.dis.cwi.nl>.

As distributed the service allows access to 6 digital ports (which can be configured as input, input with pullup, output or PWM-output) and one analog input. In addition there is a readonly port `timestamp` which returns the number of milliseconds since boot.

## Software requirements

* PlatformIO (recommended). `pio run` builds every environment in `platformio.ini`.
* Or the Arduino IDE, with the iotsa framework from <https://github.com/cwi-dis/iotsa>.

## Hardware requirements

* a iotsa board (ESP8266). Alternatively any other ESP8266 board, or an ESP32 /
  ESP32-C3 board (see the `esp32c3supermini` environment) -- but then you may have
  to adapt the available I/O pins.

## REST API

`GET /api/io` will return the current value of all ports in use (inputs and outputs; for a `pulse_output` the milliseconds left).

`PUT /api/io` expects an object that can contain `name : value` pairs for any ports configured as _output_, _pwm\_output_ or _pulse\_output_. Setting a pulse output fires the pulse, also when the value is the same as before.

`GET /api/ioconfig` will return an object with name-value pairs containing the current configuration of each port:

- `'input'` for input ports (digital, analog or timestamp),
- `'input_pullup'` for digital input ports with the internall pullup resistor enabled (so open inputs read as 1 in stead of a random undefined value).
- `'output'` for digital output ports.
- `'pwm_output'` for digital output ports that present a pseudo-analog output value using pulse-width modulation (ESP8266 only).
- `'pulse_output'` for digital output ports that emit a single HIGH pulse (value = length in ms) and then return LOW on their own.
- `'unused'` for ports that are not in use.

`PUT /api/ioconfig` allows to set the configuration for each port, using the same values as above. An unknown mode, or one the port can't do, is an error (400).

The device is meant to sit next to a breadboard, so changing modes and values is everyday use: nothing needs configuration mode or a login. The modes are saved, so they survive a reboot.