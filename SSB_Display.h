#ifndef SSB_Display_h
#define SSB_Display_h

/*
 * SSB_Display.h
 * KK4DAS, Dean Souleles, KK4DAS@gmail.com
 * May 30, 2020
 * 
 * Constants and function prototypes used in the Display Routines
 * This file needs to be in included in the main sketch
 * 
 */
#include "RadioControl.h"



// ===========================Function Prototypes==================================

extern void displaySMeter(byte level);
extern void displayBanner(String s);
extern void displayActVFO(uint32_t freq);
extern void displayAltVFO(uint32_t freq);
extern void displayVFOAB(int vfo);
extern void displayTxRx(int tx_rx);
extern void displayMode(int mode);
extern void displayIncr(uint32_t increment);
extern void displayTune(boolean on_off);
extern void displaySplit(boolean splt); 
extern void displaySetup(String banner,
                  uint32_t vfoActfreq, uint32_t vfoAltfreq,
                  uint32_t activeVFO,
                  int tx_rx,
                  int sideband,
                  boolean split,
                  uint32_t increment,
                  byte s_meter);

#ifdef DISPLAY_NEXTION
extern void CheckTouch();
#endif
                  
#endif
