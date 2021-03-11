//
//  S Meter
//
#include "RadioControl.h"


//#define S_DEBUG

const int slevels[MAXSLEVELS] = {25, 100, 200, 375, 500, 575, 650, 775, 900, 950, 975, 1000};

//byte smeter = 0;
unsigned long smeter_raw = 0;
unsigned long smeter_sample_count = 0;
unsigned long smeter_time = 0;

//*****************************************
void CheckSmeter() {

  int i;
  int smeter_avg;

  if (TxRxState == TX) {
    return;
  }
  smeter_raw = smeter_raw + analogRead(SMETER_PIN);
  smeter_sample_count++;


  if ((millis() - SMETER_SAMPLE_TIMER) > smeter_time) {
    /*
        lcd.setCursor(12, 1);
        lcd.print("        ");
        lcd.setCursor(12, 1);
        lcd.print(smeter_raw);
    */

    smeter_avg = int(smeter_raw / smeter_sample_count);
 
    #ifdef S_DEBUG
       String msg = F("S raw:");
       msg+=smeter_avg;
       displayBanner(msg);
    #endif  
    
    smeter = 0;
    for (i = 0; i < MAXSLEVELS; i++) {
      if (smeter_avg >= slevels[i]) {
        smeter = byte(i);
      }

    }
    displaySMeter(smeter);
    smeter_raw = 0;
    smeter_sample_count = 0;
    smeter_time = millis();
  }
}
