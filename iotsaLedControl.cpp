#include "iotsaLedControl.h"

//
// A "Led" control module, the same as iotsa's examples/Led: the outside world
// can flash the status LED (iotsaStatus.setStatusPulse(), cwi-dis/iotsa#176),
// a colour with on and off times, for a while. Nothing is saved and there is
// nothing to read back, so PUT but no GET. Open to everyone on the network.
//

void IotsaLedControlMod::setup() {
  IotsaLedMod::setup();
#ifdef IOTSA_WITH_BLE
  // Our BLE service, and its one characteristic. The description tells BLE
  // clients what to write: a colour as a 32-bit number.
  bleApi.setup(serviceUUID, this);
  bleApi.addCharacteristic(rgbUUID, bleApi.BLE_WRITE, NimBLE2904::FORMAT_UINT32, 0x2700, "RGBx color (solid pulse)");
#endif
}

void IotsaLedControlMod::lateSetup() {
  IotsaLedMod::lateSetup();
  // PUT only: there is nothing to GET.
  api.setup("led", false, true);
}

// A PUT starts a flash. Everything is optional: {} flashes white for a second.
// rgb is a number here, 0xRRGGBB (for example 16711680 is red), because JSON
// has no hexadecimal.
// Read all fields first, and only then act: if a field is wrong (say
// "onDuration": "fast"), ctx.failed() is true and the request is refused
// without flashing.
void IotsaLedControlMod::putHandler(IotsaApiContext& ctx) {
  uint32_t rgb = 0xffffff;
  uint32_t onDuration = 0;
  uint32_t offDuration = 0;
  uint32_t durationMs = 1000;
  ctx.arg("rgb", rgb, IotsaApiContext::Transient);
  ctx.arg("onDuration", onDuration, IotsaApiContext::Transient);
  ctx.arg("offDuration", offDuration, IotsaApiContext::Transient);
  ctx.arg("durationMs", durationMs, IotsaApiContext::Transient);
  if (ctx.failed()) return;
  iotsaStatus.setStatusPulse(rgb, onDuration, offDuration, durationMs, "REST led control");
}

#ifdef IOTSA_WITH_BLE
// The BLE equivalent of putHandler(): a client wrote one of our
// characteristics. One write carries one value, so over BLE there is only a
// colour, shown steadily for a few seconds.
bool IotsaLedControlMod::blePutHandler(UUIDstring charUUID) {
  if (charUUID == rgbUUID) {
    uint32_t rgb = bleApi.getAsInt(rgbUUID);
    iotsaStatus.setStatusPulse(rgb, 0, 0, bleSolidDurationMs, "BLE led control");
    return true;
  }
  return false;
}
#endif // IOTSA_WITH_BLE

#ifdef IOTSA_WITH_WEB
// The page has a form with the flash's parameters and a "Pulse" button. Only
// when that button is pressed do we read the fields and flash: ctx.argAction()
// makes something happen now, it doesn't change a setting.
void IotsaLedControlMod::formHandler_args(IotsaWebContext& ctx) {
  if (!ctx.argAction("pulse", IotsaWebContext::Transient)) return;
  String rgb = "ffffff";
  uint32_t onDuration = 0;
  uint32_t offDuration = 0;
  uint32_t durationMs = 1000;
  ctx.arg("rgb", rgb, IotsaWebContext::Transient);
  ctx.arg("onDuration", onDuration, IotsaWebContext::Transient);
  ctx.arg("offDuration", offDuration, IotsaWebContext::Transient);
  ctx.arg("durationMs", durationMs, IotsaWebContext::Transient);
  if (ctx.failed()) return;
  iotsaStatus.setStatusPulse(strtol(rgb.c_str(), 0, 16), onDuration, offDuration, durationMs, "web led control");
  ctx.feedback("Pulsing.");
}

void IotsaLedControlMod::formHandler_fields(IotsaWebContext& ctx) {
  if (!ctx.includesEveryday()) return;
  IotsaWebContext::Form form(ctx, "Pulse", "Status LED pulse", "pulse");
  ctx.showField("Color", "rgb", "ffffff", IotsaWebContext::Transient, "hex rrggbb");
  ctx.showField("On time", "onDuration", 0, IotsaWebContext::Transient, "ms, 0: steady");
  ctx.showField("Off time", "offDuration", 0, IotsaWebContext::Transient, "ms");
  ctx.showField("Total duration", "durationMs", 1000, IotsaWebContext::Transient, "ms");
}

String IotsaLedControlMod::info() {
  String rv = "<p>See <a href=\"/led\">/led</a> to flash the LED.";
#ifdef IOTSA_HAS_RESTSERVER
  rv += " Or use PUT on <code>/api/led</code>.";
#endif
#ifdef IOTSA_WITH_BLE
  rv += " Or use BLE service " + String(serviceUUID) + " on device " + iotsaConfig.hostName + " for a solid-color pulse.";
#endif
  rv += "</p>";
  return rv;
}
#endif // IOTSA_WITH_WEB
