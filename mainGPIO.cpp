//
// iotsaGPIO: a small networked bench device that exposes 3.3V hardware signals
// (digital in/out, analog in, PWM out, timed pulse out) over REST and a web
// form. Put it next to your breadboard and drive your electronics from the
// computer, without worrying about GPIO interfaces on the computer itself.
// See readme.md.
//

#include <Esp.h>
#include "iotsa.h"
#include "iotsaFiles.h"
#include "iotsaFilesUpload.h"
#include "iotsaFilesBackup.h"
#include "iotsaIOPort.h"
#include "iotsaLedControl.h"

IotsaApplication application("Iotsa GPIO Server");

IotsaIOPortMod gpioMod(application);
IotsaFilesMod filesMod(application);                  // static file webserver
IotsaFilesUploadMod filesUploadMod(application);      // upload of static files
IotsaFilesBackupMod filesBackupMod(application);      // backup of config + files
// Status LED, also REST-controllable: the board's own (NeoPixel or plain LED,
// from its IOTSA_PIN_LED/IOTSA_LED_* defines), else a NeoPixel on GPIO15.
#ifdef IOTSA_PIN_LED
IotsaLedControlMod ledMod(application);
#else
IotsaLedControlMod ledMod(application, 15);
#endif

void setup(void){
  application.setup();
  application.lateSetup();
}

void loop(void){
  application.loop();
}
