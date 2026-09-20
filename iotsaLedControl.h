#ifndef _IOTSALEDCONTROL_H_
#define _IOTSALEDCONTROL_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaLed.h"

//
// LED module: lets an external party trigger iotsaStatus's status-pulse channel
// (cwi-dis/iotsa#176) -- color, on/off duration, total duration -- over the
// web/REST API. A pulse is transient by design: it always decays back to the
// normal status display on its own, so there's nothing to read back or cancel
// (cwi-dis/iotsa#256).
//
class IotsaLedControlMod : public IotsaLedMod {
public:
  using IotsaLedMod::IotsaLedMod;
  void lateSetup() override;
  String info() override;
protected:
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
private:
  void webHandler() override;
};

#endif
