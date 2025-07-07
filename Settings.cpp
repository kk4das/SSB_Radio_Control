#include "RadioControl.h"
#include <EEPROM.h>

#define EE_MAGIC_ADDR 0
#define EE_SETTINGS_ADDR 4
#define SETTINGS_TIMER 3000  // time to wait after a settings change before writing to EEPROM

const byte magic[4] = {'4', 'D', 'A', 'S'};

struct Settings_struct {
//  byte sumCheckDbg;
  uint32_t vfoAfreq;
  uint32_t vfoBfreq;
  uint32_t vfoAmode;      // Changed from byte to uint32_t for consistency
  uint32_t vfoBmode;      // Changed from byte to uint32_t for consistency
  uint32_t increment;
  byte     active_vfo;
  byte     mode;          // Current active mode
  // Band specific memory - keep for backward compatibility
  uint32_t band20Freq;
  uint32_t band40Freq;
  byte     band20Sideband;  // Keep for backward compatibility
  byte     band40Sideband;  // Keep for backward compatibility
};

// Settings Change
unsigned long settings_time;
bool settings_changed;

uint16_t calcCheckSum( const byte *p, size_t len) {
  uint16_t sum = 0;
  for ( int i = 0; i < len; i++ )
    sum += *p++;
  return sum;
}

bool readSettings() {
  Settings_struct settings;
  uint16_t tailCheck, sum;
  bool readOk = true;

  EEPROM.get(EE_SETTINGS_ADDR, settings);
  EEPROM.get(EE_SETTINGS_ADDR+sizeof(Settings_struct), tailCheck);

  sum = calcCheckSum((const byte *)&settings, sizeof(settings));
  if (tailCheck != sum) {
    initSettings();
    readOk = false;
  } else {
    vfoAfreq       = settings.vfoAfreq;
    vfoBfreq       = settings.vfoBfreq;
    vfoAmode       = settings.vfoAmode;
    vfoBmode       = settings.vfoBmode;
    increment      = settings.increment;
    active_vfo     = settings.active_vfo;
    mode           = settings.mode;
    band20Freq     = settings.band20Freq;
    band40Freq     = settings.band40Freq;
    band20Sideband = settings.band20Sideband;
    band40Sideband = settings.band40Sideband;
  }

  // Set sideband based on current mode
  sideband = GetSidebandFromMode(mode);
  bfo = (sideband == LSB) ? LSB_BFO : USB_BFO;

  return readOk;
}

void writeSettings() {
  Settings_struct settings;
  uint16_t wsum;

  settings.vfoAfreq       = vfoAfreq;
  settings.vfoBfreq       = vfoBfreq;
  settings.vfoAmode       = vfoAmode;
  settings.vfoBmode       = vfoBmode;
  settings.increment      = increment;
  settings.active_vfo     = active_vfo;
  settings.mode           = mode;
  settings.band20Freq     = band20Freq;
  settings.band40Freq     = band40Freq;
  settings.band20Sideband = band20Sideband;
  settings.band40Sideband = band40Sideband;
  
  EEPROM.put(EE_SETTINGS_ADDR, settings);
  wsum = calcCheckSum((const byte *) &settings, sizeof(settings));
  EEPROM.put(EE_SETTINGS_ADDR+sizeof(Settings_struct), wsum);
}

void initSettings() {
  EEPROM.put(EE_MAGIC_ADDR, magic);
  writeSettings();
}

bool setupSettings() {
  byte buff[4];
  bool magic_ok = true;
  bool readOk = false;

  EEPROM.get(EE_MAGIC_ADDR, buff);

  for (int i = 0; i < 4; i++) {
    if (buff[i] != magic[i]) {
      magic_ok = false;
      break;
    }
  }

  if (magic_ok) {
    readOk = readSettings();
  } else {
    initSettings();
  }

  settings_time = millis();
  settings_changed = false;
  return readOk;
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
