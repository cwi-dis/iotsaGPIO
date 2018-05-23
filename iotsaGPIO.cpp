#include "iotsaGPIO.h"
#include "iotsaConfigFile.h"

struct pinModeNames {
   int mode;
   String modeName;
};



struct pinModeNames pinModeNames[] = {
  {INPUT, "input"},
  {INPUT_PULLUP, "input_pullup"},
  {OUTPUT, "output"},
  {PWM_OUTPUT, "pwm_output"},
  {PULSE_OUTPUT, "pulse_output"},
  {-1, "unused"},
};

#define nPinModeNames (sizeof(pinModeNames)/sizeof(pinModeNames[0]))

static int name2mode(const String &name) {
  for(unsigned int i=0; i<nPinModeNames; i++) {
    if (name == pinModeNames[i].modeName) return pinModeNames[i].mode;
  }
  return pinModeNames[0].mode;
}

static String& mode2name(int mode) {
  for(unsigned int i=0; i<nPinModeNames; i++) {
    if (mode == pinModeNames[i].mode) return pinModeNames[i].modeName;
  }
  return pinModeNames[0].modeName;
}

DigitalPort io4("io4", 4);
DigitalPort io5("io5", 5);
DigitalPort io12("io12", 12);
DigitalPort io13("io13", 13);
DigitalPort io14("io14", 14);
DigitalPort io16("io16", 16);
AnalogInput a0("a0", A0); // GPIO 36 on most esp32 boards, sometimes 26 or something else
TimestampInput timePort;

GPIOPort *ports[] = {
  &io4, &io5, &io12, &io13, &io14, &io16, &a0, &timePort
};
#define nPorts (sizeof(ports)/sizeof(ports[0]))

void
IotsaGPIOMod::handler() {
  // First check configuration changes
  bool anyChanged = false;
  for (unsigned int pi=0; pi<nPorts; pi++) {
    GPIOPort *p = ports[pi];
    String argName = p->name + "mode";
    if (server.hasArg(argName)) {
      int mode = name2mode(server.arg(argName));
      if (mode != p->getMode()) {
        if (!iotsaConfig.inConfigurationMode()) {
          server.send(401, "text/plain", "401 Unauthorized, not in configuration mode");
          return;
        }
        if (p->setMode(mode)) {
          anyChanged = true;
        }
      }
    }
  }
  if (anyChanged) configSave();

  // Now set values
  for (unsigned int pi=0; pi<nPorts; pi++) {
    GPIOPort *p = ports[pi];
    String argName = p->name + "value";
    if (server.hasArg(argName)) {
      int value = server.arg(argName).toInt();
      p->setValue(value);
    }
  }
  String message = "<html><head><title>Simple GPIO module</title></head><body><h1>GPIO Configuration</h1>";
  message += "<form method='get'><table><tr><th>Pin</th><th>Mode</th><th>Value</th></tr>";
  for (GPIOPort **pp=ports; pp < &ports[nPorts]; pp++) {
    GPIOPort *p = *pp;
    message += "<tr><td>" + p->name + "</td>";
    int thisMode = p->getMode();
    message += "<td><select name='" + p->name + "mode'>";
    for(unsigned int i=0; i<nPinModeNames; i++) {
      message += "<option value='" + pinModeNames[i].modeName + "'";
      if (pinModeNames[i].mode == thisMode) message += " selected";
      message += ">"+pinModeNames[i].modeName+"</option>";
    }
    message += "</select></td>";
    message += "<td><input name='" + p->name + "value' value='" + String(p->getValue()) + "'></td>";
    message += "</tr>";
  }
  message += "</table><input type='submit'></form>";
  server.send(200, "text/html", message);
}

bool IotsaGPIOMod::getHandler(const char *path, JsonObject& reply) {
  if (strcmp(path, "/api/io") == 0) {
    for (unsigned int pi=0; pi<nPorts; pi++) {
      GPIOPort *p = ports[pi];
      if (p->getMode() == INPUT || p->getMode() == INPUT_PULLUP) {
        reply[p->name] = p->getValue();
      }
    }
    return true;
  } else if (strcmp(path, "/api/ioconfig") == 0) {
    for (unsigned int pi=0; pi<nPorts; pi++) {
      GPIOPort *p = ports[pi];
      String modeName = mode2name(p->getMode());
      reply[p->name] = modeName;
    }
    return true;
  }
  return false;
}

bool IotsaGPIOMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  JsonObject &args = request.as<JsonObject>();
  bool anyDone = false;
  if (strcmp(path, "/api/io") == 0) {
    for (unsigned int pi=0; pi<nPorts; pi++) {
      GPIOPort *p = ports[pi];
      if (args.containsKey(p->name)) {
        int m = p->getMode();
        if (m == OUTPUT || m == PWM_OUTPUT || m == PULSE_OUTPUT) {
          p->setValue(args[p->name].as<int>());
          anyDone = true;
        } else {
          IFDEBUG IotsaSerial.print("Attempt to set input port");
          return false;
        }
      }
    }
    return anyDone;
  } else if (strcmp(path, "/api/ioconfig") == 0) {
    if (!iotsaConfig.inConfigurationMode()) return false;
    for (unsigned int pi=0; pi<nPorts; pi++) {
      GPIOPort *p = ports[pi];
      if (args.containsKey(p->name)) {
        int mode = name2mode(args[p->name].as<String>());
        if (!p->setMode(mode)) return false;
        anyDone = true;
      }
    }
    if (anyDone) configSave();
    return anyDone;
  }
  return false;
}

void IotsaGPIOMod::setup() {
  configLoad();
}

void IotsaGPIOMod::serverSetup() {
  server.on("/ioconfig", std::bind(&IotsaGPIOMod::handler, this));
  api.setup("/api/io", true, true);
  api.setup("/api/ioconfig", true, true);
  name = "io";
}

void IotsaGPIOMod::configLoad() {
  IotsaConfigFileLoad cf("/config/GPIO.cfg");
  for (unsigned int pi=0; pi<nPorts; pi++) {
    GPIOPort *p = ports[pi];
    int mode;
    cf.get(p->name+"mode", mode, INPUT);
    p->setMode(mode);
  }
}

void IotsaGPIOMod::configSave() {
  IotsaConfigFileSave cf("/config/GPIO.cfg");
  for (unsigned int pi=0; pi<nPorts; pi++) {
    GPIOPort *p = ports[pi];
    cf.put(p->name+"mode", p->getMode());
  }
}

void IotsaGPIOMod::loop() {
  for (unsigned int pi=0; pi<nPorts; pi++) {
    GPIOPort *p = ports[pi];
    p->loop();
  }
}

String IotsaGPIOMod::info() {
  String message = "<p>Built with GPIO module. See <a href=\"/ioconfig\">/ioconfig</a> to examine GPIO pin configuration and values, ";
  message += "<a href=\"/api/io\">/api/io</a> for REST access to values, <a href=\"/api/ioconfig\">/api/ioconfig</a> to configure using REST.</p>";
  return message;
}
