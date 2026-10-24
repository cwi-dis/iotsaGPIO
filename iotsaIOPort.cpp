#include "iotsaIOPort.h"
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

// -2 for a name that isn't a mode (-1 is "unused").
static int name2mode(const String &name) {
  for(unsigned int i=0; i<nPinModeNames; i++) {
    if (name == pinModeNames[i].modeName) return pinModeNames[i].mode;
  }
  return -2;
}

static int mode2index(int mode) {
  for(unsigned int i=0; i<nPinModeNames; i++) {
    if (mode == pinModeNames[i].mode) return i;
  }
  return 0;
}

static bool isOutput(int mode) {
  return mode == OUTPUT || mode == PWM_OUTPUT || mode == PULSE_OUTPUT;
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

IOPortPort *ports[] = {
  &io4, &io5, &io12, &io13, &io14, &io16, &a0, &timePort
};
#define nPorts (sizeof(ports)/sizeof(ports[0]))

// This device sits next to a breadboard: changing a pin's mode and setting
// its value are everyday use, so everything here is open to everyone on the
// network. The modes are saved, so they survive a reboot.

#ifdef IOTSA_WITH_WEB
static const char *modeOptions[nPinModeNames];

// Per pin, scoped "<pin>.": mode (a choice), value and set (an output's Set
// button, so a form never writes values back that weren't meant to be set:
// for a pulse output that would fire it again).
void IotsaIOPortMod::formHandler_args(IotsaWebContext& ctx) {
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    IotsaWebContext::Scope s(ctx, p->name);
    int idx = mode2index(p->getMode());
    if (ctx.argChoice("mode", idx, nPinModeNames, IotsaContext::Saved)) {
      if (!p->setMode(pinModeNames[idx].mode)) {
        ctx.error(p->name + ": can't be " + pinModeNames[idx].modeName + ".");
        return;
      }
    }
    if (ctx.argAction("set", IotsaContext::Transient)) {
      int value = 0;
      ctx.arg("value", value, IotsaContext::Transient);
      if (ctx.failed()) return;
      if (!isOutput(p->getMode())) {
        ctx.error(p->name + " is not an output.");
        return;
      }
      p->setValue(value);
    }
  }
}

void IotsaIOPortMod::formHandler_fields(IotsaWebContext& ctx) {
  if (!ctx.includesEveryday()) return;
  for (unsigned int i=0; i<nPinModeNames; i++) modeOptions[i] = pinModeNames[i].modeName.c_str();
  ctx << "<p><a href='/ioconfig'>Reload</a></p>";
  {
    IotsaWebContext::Form form(ctx, "Save", "Pin modes");
    for (unsigned int pi=0; pi<nPorts; pi++) {
      IOPortPort *p = ports[pi];
      IotsaWebContext::Scope s(ctx, p->name);
      ctx.showChoice(p->name, "mode", mode2index(p->getMode()), modeOptions, nPinModeNames, IotsaContext::Saved);
    }
  }
  ctx << "<h3>Values</h3>\n";
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    int mode = p->getMode();
    if (mode == -1) continue;
    IotsaWebContext::Scope s(ctx, p->name);
    if (!isOutput(mode)) {
      ctx.showStatus(p->name, "value", String(p->getValue()));
      continue;
    }
    IotsaWebContext::Form form(ctx, "Set", "", "set");
    const char *hint = mode == PULSE_OUTPUT ? "pulse length in ms, 0 stops it" : mode == PWM_OUTPUT ? "0 to 255" : "0 or 1";
    ctx.showField(p->name, "value", mode == PULSE_OUTPUT ? 0 : p->getValue(), IotsaContext::Transient, hint);
  }
}
#endif // IOTSA_WITH_WEB

String IotsaIOPortMod::info() {
  String message = "<p>Built with IOPort module. See <a href=\"/ioconfig\">/ioconfig</a> to examine GPIO pin configuration and values, ";
  message += "<a href=\"/api/io\">/api/io</a> for REST access to values, <a href=\"/api/ioconfig\">/api/ioconfig</a> to configure using REST.</p>";
  return message;
}

// /api/io: the value of every pin in use (for a pulse output the ms left).
// /api/ioconfig: the mode of every pin.
void IotsaIOPortMod::getHandler(IotsaApiContext& ctx) {
  bool config = ctx.path() == "/api/ioconfig";
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    if (config) {
      ctx.show(p->name.c_str(), mode2name(p->getMode()));
    } else if (p->getMode() != -1) {
      ctx.show(p->name.c_str(), p->getValue());
    }
  }
}

// /api/io: set outputs, {"io4": 1}. /api/ioconfig: set modes, {"io4": "output"}.
void IotsaIOPortMod::putHandler(IotsaApiContext& ctx) {
  bool config = ctx.path() == "/api/ioconfig";
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    if (config) {
      String modeName = mode2name(p->getMode());
      if (!ctx.arg(p->name.c_str(), modeName, IotsaContext::Saved)) continue;
      int mode = name2mode(modeName);
      if (mode == -2 || !p->setMode(mode)) {
        ctx.error(400, p->name + ": can't be " + modeName);
        return;
      }
    } else {
      // A command, not a field: setting a pulse to the same length fires it again.
      if (!ctx.argAction(p->name.c_str(), IotsaContext::Transient)) continue;
      JsonVariant v = ctx.request()[p->name];
      if (!v.is<int>()) {
        ctx.error(400, p->name + ": a number");
        return;
      }
      int value = v.as<int>();
      if (!isOutput(p->getMode())) {
        ctx.error(400, p->name + ": not an output");
        return;
      }
      p->setValue(value);
    }
  }
}

void IotsaIOPortMod::setup() {
  configLoad();
}

void IotsaIOPortMod::lateSetup() {
  // "io" carries live input values (GET) and output writes (PUT); it deliberately
  // has no web page of its own -- "ioconfig" is the single page for both.
  api.setup("io", true, true, false, false);
  api.setup("ioconfig", true, true);
  // name must match the *config* endpoint ("ioconfig"), not the live-value one
  // ("io"), so that iotsa backup/restore (which only ever queries /api/<name>)
  // captures the persisted pin-mode configuration instead of a live input snapshot.
  name = "ioconfig";
}

void IotsaIOPortMod::configLoad() {
  IotsaConfigFileLoad cf("/config/GPIO.cfg");
  // xxxjack should use object interface
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    int mode;
    cf.get(p->name+"mode", mode, INPUT);
    p->setMode(mode);
  }
}

void IotsaIOPortMod::configSave() {
  IotsaConfigFileSave cf("/config/GPIO.cfg");
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    cf.put(p->name+"mode", p->getMode());
  }
}

void IotsaIOPortMod::loop() {
  for (unsigned int pi=0; pi<nPorts; pi++) {
    IOPortPort *p = ports[pi];
    p->loop();
  }
}
