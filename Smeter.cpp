//
//  S Meter
//
#include "RadioControl.h"


//#define S_DEBUG

//const int slevels[MAXSLEVELS] = {100, 200, 375, 500, 575, 650, 775, 900, 950, 975, 1000, 1025};// KA4CDN

//                                1   2   3   4   5   6   7     8    9   10    20    30
//const int slevels[MAXSLEVELS] = {20, 30, 35,  40 , 55 , 80, 140, 420, 730, 850,  975, 1000};
//const int slevels[MAXSLEVELS] =  { 20, 35, 70, 140, 280, 400, 500, 620, 730, 875, 975, 1000};  // Mikes
const int slevels[MAXSLEVELS] =  { 2, 4, 5, 9, 20, 40, 50, 62, 90, 100, 125, 160}; // KK4DAS
//byte smeter = 0;
unsigned long smeter_raw = 0;
unsigned long smeter_sample_count = 0;
unsigned long smeter_time = 0;

//*****************************************
void CheckSmeter()
{

  int i;
  int smeter_avg;

  if (TxRxState == TX)
  {
    displaySMeter(MAXSLEVELS);
    return;
  }
  smeter_raw = smeter_raw + analogRead(SMETER_PIN);
  smeter_sample_count++;

  if ((millis() - SMETER_SAMPLE_TIMER) > smeter_time)
  {

    smeter_avg = int(smeter_raw / smeter_sample_count);

#ifdef S_DEBUG
    Serial.println(smeter_avg);
#endif

    smeter = 0;
    for (i = 0; i < MAXSLEVELS; i++)
    {
      if (smeter_avg >= slevels[i])
      {
        smeter = byte(i);
      }
    }
    displaySMeter(smeter);

    smeter_raw = 0;
    smeter_sample_count = 0;
    smeter_time = millis();
  }
}
