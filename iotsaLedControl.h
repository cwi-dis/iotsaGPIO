#ifndef _IOTSALEDCONTROL_H_
#define _IOTSALEDCONTROL_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaLed.h"

class IotsaLedControlMod : public IotsaLedMod {
public:
  using IotsaLedMod::IotsaLedMod;
  void serverSetup();
  String info();
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
private:
  void handler();
};

#endif
