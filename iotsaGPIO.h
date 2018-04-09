#ifndef _IOTSASIMPLEIO_H_
#define _IOTSASIMPLEIO_H_
#include "iotsa.h"
#include "iotsaApi.h"

#define PWM_OUTPUT OUTPUT + 42
#define PULSE_OUTPUT OUTPUT + 43

class GPIOPort {
public:
  GPIOPort(const char* _name, int _pin)
  : name(_name),
    pin(_pin),
    mode(INPUT),
    value(false),
    changed(false)
  {}
  virtual bool setMode(int _mode) = 0;
  virtual void setValue(int _value) = 0;
  virtual int getValue() = 0;
  virtual void loop() {}
  int getMode() {
    return mode;
  }
  String name;

protected:
  int pin;
  int mode;
  int value;
  bool changed;
};

class DigitalPort : public GPIOPort {
public:
  DigitalPort(const char* _name, int _pin) : GPIOPort(_name, _pin) {}
  
  virtual bool setMode(int _mode) {
    if (mode == _mode) return false;
    mode = _mode;
    if (_mode == PWM_OUTPUT || _mode == PULSE_OUTPUT) _mode = OUTPUT;
    pinMode(pin, _mode);
    return true;
  };
  virtual void setValue(int _value) {
    value = _value;
    if (mode == OUTPUT)
      digitalWrite(pin, value ? HIGH : LOW);
#ifndef ESP32
     else if (mode == PWM_OUTPUT)
      analogWrite(pin, value);
#endif
    else if (mode == PULSE_OUTPUT) {
        if (value > 0) {
          digitalWrite(pin, HIGH);
          pulseEndTime = millis() + value;
        }
        else {
          digitalWrite(pin, LOW);
          pulseEndTime = 0;
        }
    }
  };
  virtual int getValue() {
    if (mode == OUTPUT || mode == PWM_OUTPUT) return value;
    if (mode == PULSE_OUTPUT){
      if (pulseEndTime == 0) return 0;
      return pulseEndTime - millis();
    }
    return digitalRead(pin);
  };

  virtual void loop() {
    if (pulseEndTime < millis()) {
      digitalWrite(pin, LOW);
      pulseEndTime = 0;
    }
  };

protected: 
  unsigned long pulseEndTime;

};



class AnalogInput : public GPIOPort {
public:
  AnalogInput(const char *name, int _pin) : GPIOPort(name, _pin) {}
  virtual bool setMode(int _mode) { 
    if (_mode < 0 || _mode == INPUT) { 
      mode = _mode; return true;
    } 
    return false; 
  }
  virtual void setValue(int _value) {}
  virtual int getValue() {
    return analogRead(pin);
  };
};

class TimestampInput : public GPIOPort {
public:
  TimestampInput() : GPIOPort("timestamp", -1) {}
  virtual bool setMode(int _mode) { 
    if (_mode < 0 || _mode == INPUT) { 
      mode = _mode; return true;
    } 
    return false; 
  }
  virtual void setValue(int _value) {}
  virtual int getValue() {
    return millis();
  };
};

class IotsaGPIOMod : public IotsaApiMod {
public:
  using IotsaApiMod::IotsaApiMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
};

#endif
