#ifndef _IOTSALEDCONTROL_H_
#define _IOTSALEDCONTROL_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaLed.h"

class IotsaLedControlMod : public IotsaLedMod {
public:
  using IotsaLedMod::IotsaLedMod;
  void serverSetup() override;
  String info() override;
protected:
  bool getHandler(const char *path, JsonObject& reply) override;
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
private:
  void handler();
};

#endif
