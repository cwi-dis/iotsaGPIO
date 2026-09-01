//
// iotsaGPIO: a small networked bench device that exposes 3.3V hardware signals
// (digital in/out, analog in, PWM out, timed pulse out) over REST and a web
// form. Put it next to your breadboard and drive your electronics from the
// computer, without worrying about GPIO interfaces on the computer itself.
// See readme.md.
//

#include <Esp.h>
#include "iotsa.h"
#include "iotsaWifi.h"
#include "iotsaOta.h"
#include "iotsaFiles.h"
#include "iotsaFilesUpload.h"
#include "iotsaFilesBackup.h"
#include "iotsaIOPort.h"
#include "iotsaLedControl.h"

#define NEOPIXEL_PIN 15

IotsaApplication application("Iotsa GPIO Server");
IotsaWifiMod wifiMod(application);

IotsaIOPortMod gpioMod(application);

IotsaOtaMod otaMod(application);                      // OTA firmware updates
IotsaFilesMod filesMod(application);                  // static file webserver
IotsaFilesUploadMod filesUploadMod(application);      // upload of static files
IotsaFilesBackupMod filesBackupMod(application);      // backup of config + files
IotsaLedControlMod ledMod(application, NEOPIXEL_PIN); // status LED, also REST-controllable

void setup(void){
  application.setup();
  application.lateSetup();
#ifndef ESP32
  ESP.wdtEnable(WDTO_120MS);
#endif
}

void loop(void){
  application.loop();
}
