//
// A "Led" control module based on the Led module.
//
#include "iotsaLedControl.h"

void
IotsaLedControlMod::webHandler() {
  IotsaWebServer *server = api.webService->server;
  // Handles the page that is specific to the Led module: triggers a transient
  // status-LED pulse (cwi-dis/iotsa#176) with the requested color/timing. It
  // always decays back to the normal status display on its own.
  bool anyChanged = false;
  uint32_t _rgb = 0xffffff;
  uint32_t _onDuration = 0;
  uint32_t _offDuration = 0;
  uint32_t _durationMs = 1000;
  if( server->hasArg("rgb")) {
    _rgb = strtol(server->arg("rgb").c_str(), 0, 16);
    anyChanged = true;
  }
  if( server->hasArg("onDuration")) {
    _onDuration = server->arg("onDuration").toInt();
    anyChanged = true;
  }
  if( server->hasArg("offDuration")) {
    _offDuration = server->arg("offDuration").toInt();
    anyChanged = true;
  }
  if( server->hasArg("durationMs")) {
    _durationMs = server->arg("durationMs").toInt();
    anyChanged = true;
  }
  if (anyChanged) iotsaStatus.setStatusPulse(_rgb, _onDuration, _offDuration, _durationMs, "web led control");

  String message = "<html><head><title>Led Server</title></head><body><h1>Led Server</h1>";
  message += "<p>Triggers a transient status-LED pulse; it decays back to the normal status display automatically.</p>";
  message += "<form method='get'>";
  message += "Color (hex rrggbb): <input type='text' name='rgb'><br>";
  message += "On time (ms): <input type='text' name='onDuration'><br>";
  message += "Off time (ms): <input type='text' name='offDuration'><br>";
  message += "Total duration (ms): <input type='text' name='durationMs' value='1000'><br>";
  message += "<input type='submit'></form></body></html>";
  server->send(200, "text/html", message);
}

String IotsaLedControlMod::info() {
  // Return some information about this module, for the main page of the web server.
  String rv = "<p>See <a href=\"/led\">/led</a> for triggering a status-LED pulse.</p>";
  return rv;
}

bool IotsaLedControlMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  uint32_t _rgb = request["rgb"]|0xffffff;
  uint32_t _onDuration = request["onDuration"]|0;
  uint32_t _offDuration = request["offDuration"]|0;
  uint32_t _durationMs = request["durationMs"]|1000;
  iotsaStatus.setStatusPulse(_rgb, _onDuration, _offDuration, _durationMs, "REST led control");
  return true;
}

void IotsaLedControlMod::lateSetup() {
  // PUT + web page; no REST GET, since a pulse is transient -- there's
  // nothing meaningful to read back (cwi-dis/iotsa#256).
  api.setup("led", false, true);
  name = "led";
}
