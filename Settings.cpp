#include "RadioControl.h"
#include <EEPROM.h>

#define EE_MAGIC_ADDR 0
#define EE_SETTINGS_ADDR 4
#define SETTINGS_TIMER 3000  // time to wait after a settings change before writing to EEPROM

const byte magic[4] = {'4', 'D', 'A', 'S'};


struct Settings_struct {
  uint32_t vfoAfreq;
  uint32_t vfoBfreq;
  uint32_t increment;
  byte active_vfo;
  byte sideband;
  // Band specific memory
  uint32_t band20Freq;
  uint32_t band40Freq;
  byte band20Sideband;
  byte band40Sideband;
};

// Settings Change
unsigned long settings_time;
bool settings_changed;


void readSettings() {
  Settings_struct settings;
  EEPROM.get(EE_SETTINGS_ADDR, settings);
  vfoAfreq = settings.vfoAfreq;
  vfoBfreq = settings.vfoBfreq;
  increment = settings.increment;
  active_vfo = settings.active_vfo;
  sideband = settings.sideband;
  band20Freq = settings.band20Freq;
  band40Freq = settings.band40Freq;
  band20Sideband = settings.band20Sideband;
  band40Sideband = settings.band40Sideband;

  if (sideband == LSB) {
    bfo = LSB_BFO;
  } else {
    bfo = USB_BFO;
  }

}

void writeSettings() {
  Settings_struct settings;
  settings.vfoAfreq = vfoAfreq;
  settings.vfoBfreq = vfoBfreq;
  settings.increment = increment;
  settings.active_vfo = active_vfo;
  settings.sideband = sideband;
  settings.band20Freq = band20Freq;
  settings.band40Freq = band40Freq;
  settings.band20Sideband = band20Sideband;
  settings.band40Sideband = band40Sideband;
  EEPROM.put(EE_SETTINGS_ADDR, settings);
}

void initSettings() {
  EEPROM.put(EE_MAGIC_ADDR, magic);
  writeSettings();
}

void setupSettings() {
  byte buff[4];
  bool magic_ok = true;

  EEPROM.get(EE_MAGIC_ADDR, buff);

  for (int i = 0; i < 4; i++) {
    if (buff[i] != magic[i]) {
      magic_ok = false;
      break;
    }
  }

  if (magic_ok) {
    readSettings();
  } else {
    initSettings();
  }

  settings_time = millis();
  settings_changed = false;
}

void CheckSettings() {

  if (settings_changed && (millis() - SETTINGS_TIMER) > settings_time) {
    writeSettings();
    settings_time = millis();
    settings_changed = false;
  }
}

void startSettingsTimer() {
  settings_time = millis();
  settings_changed = true;
}
