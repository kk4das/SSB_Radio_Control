/*
   SSB_Radio_Control
   Dean Souleles, KK4DAS, contact kk4das@gmail.com
 
   This sketch implement s basic SSB radio control panel with the following features:
       Dual VFO A/B
       Rotary encoder for tuning, push button to change tuning increment
       SI5351 Clock generator for the LO and BFO
       CAT Control (emulates an ICOM IC-746)
       Split mode support (Split from CAT, manual split requres Nextion touch screen) 
       Settings memory (last frequency and mode are saved)
       Optional S-Meter

       Controls
         * Rotary encoder to change frequence
         * Rotary encode button changes tuning increment
         * VFO A/B Select toggle
         * Mode select USB/LSB toggle
         * Tune button (emits 10 second pulsed tone at 800Hz for tuning)
         * MOX toggle - puts rig in to Tx/Rx
         * Optional dual-band support 20/40

       Modules for different display types
         * 20x4 LCD
         * 320x240 TFT color display
         * 2.8" Nextion Touch Screen

       Display features
         * Dual VFOS
         * Mode indicator  SSB/LSB
         * Tx/Rx ndicator
         * TuningStep Inidicator
         * Optional S Meter
         * Banner including callsign

       Additional controls with the Nextion display
         * Continuous scanning
         * Split mode


   Version 2.0 Release
   July 7, 2025
       Merged mode with sideband logic - with assist from Claude.ai
  
   January 22, 2023
       Fix Mode change logic including CAT control
       
   March 27, 2022
       Bring Nextion Display up to date for long press and CW modes
   March 24, 2022
       Introduce concept of modes in addition to sideband
       Implement CW mode of operation
       Added long press on encoder to decrement tuning step size
       Added checksum to the EEprom saved memory setting

   Version 1.5
   December 11 2021
       Modified S-Meter display for simple LCD display

   Version 1.4
   March 9 2021
       Rstored LCD Display Option
       Compile time selection of S-Meter and Dual Band mods
       Refactoring of encoder handling
       
   Version 1.3
   Jan 25, 2021
       Changed CAT module to IC-746

   Version 1.2
   Dec 14, 2020
       Dual Band 20/40 Support
       S-Meter

   Version 1.1
   Dec 13. 2020
       Ported to Nano Every for more sweet SRAM
 *        * Updated interrupt handling in Encoder.cpp to work with Nano Every (as well as UNO/Nano)
 *        * Replaced SoftwareSerial connections to Nextion with Hardware Serial - Serial1

   Version 1.0
   Aug 13, 2020
   Adapted from SimpleSSB Sketch by N6QW, Pete Juliano and others


   NOTE TO BUILDERS
   This is a reference implementation of an SSB radio control program for a homebrew SSB transceiver.
   It is a fully functioning SSB radio control program. While it has been built for my particular hardware
   configuration every attempt has been made to make it modular in design so that the builder can swap out
   modules at will.  It should be fairly straight forward to adapt to the builder's hardware selection
   of switches, buttons and knobs or even alternate displays.

*/

#include "RadioControl.h"

#ifdef DEBUG
char debugmsg[25];
#endif

//=============== Globals ============================================

//////////////////////////////////////////////////////////////////////
//                                                                  //
//            si5351 Clock Module                                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////
Si5351 si5351;

// Calibration offset - adjust for variability in the SI5351 module
// crystal - must be set for the particular SI5351 installed
#define CALIBRATION_OFFSET 906  // Calibration for the SI-5351 - KK4DAS
//#define CALIBRATION_OFFSET 1360  // KA4CDN



//////////////////////////////////////////////////////////////////////
//                                                                  //
//            BFO and VFO Constants and Variables                   //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#ifdef BFO12MHZ
const uint32_t USB_BFO = 12001600L;
const uint32_t LSB_BFO = 11998600L;
#endif

#ifdef BFO9MHZ
//const uint32_t USB_BFO = 9001500L;
//const uint32_t LSB_BFO = 8998500L;
#define CENTER_FREQ 8999640L  //KK4DAS
#define BFO_OFFSET 1500L
const uint32_t USB_BFO = CENTER_FREQ+BFO_OFFSET;
const uint32_t LSB_BFO = CENTER_FREQ-BFO_OFFSET;
#endif

uint32_t bfo = LSB_BFO;                      // Startup BFO frequency
const uint32_t BFO_DELTA = USB_BFO - LSB_BFO; // Difference between USB and LSB for BFO change

//
// Startup VFO A/B frequencies
//
uint32_t vfoAfreq = 7200000L;   //  7.200.000
uint32_t vfoBfreq = 7074000L;   //  FT-8 7.074.000
//
// Startup VFO sidebands
//
byte vfoASideband = LSB;
byte vfoBSideband = USB;
//
// Startup VFO modes
//
byte vfoAmode = L_SSB;
byte vfoBmode = U_SSB;

uint32_t increment = 1000;                //  startup VFO tuning increment in HZ.


//////////////////////////////////////////////////////////////////////
//                                                                  //
//            Active VFO                                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte active_vfo = VFOA;  // startup on VFOA


//////////////////////////////////////////////////////////////////////
//                                                                  //
//           Active Sideband and Mode Selection                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte sideband = LSB;      // startup in LSB
byte mode     = L_SSB;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//           Band Selection                                         //
//                                                                  //
/////////////////////////////////////////////////////////////////////
uint32_t band20Freq = 14200000L;  // 14.200.000
uint32_t band40Freq = 7200000L;   //  7.200.000
byte band20Sideband = USB;
byte band40Sideband = LSB;
byte band = BAND40;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//           Transmit / Receive Indicators                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte TxRxState = RX;          //  startup in RX
byte lastTxRxState = RX;      //  previous TxRxState
byte txSource = PTT_MIC;      //  transmit source - Mic, Tune, CAT
byte CwTxRxState = RX;
byte lastCwTxRxState = RX;

//////////////////////////////////////////////////////////////////////
//                                                                  //
//           S Meter 0-9 +10, +20 +30                               //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte smeter = 0;             //  startup s_meter reading (requires SMETER)


//////////////////////////////////////////////////////////////////////
//                                                                  //
//           Split Mode On/Off                                      //
//                                                                  //
//////////////////////////////////////////////////////////////////////
bool split = false;




///////////////////////////////////////////////////////////
//  setBandFilters(band)                                 //
//    For 20 meters turn relay ON   (NO)                 //
//    For 40 meters turn relay OFF  (NC)                 //
///////////////////////////////////////////////////////////
void setBandFilters(int band) {
#ifdef DUAL_BAND
  switch (band) {
    case BAND20:
      digitalWrite(BAND_PIN, HIGH);
      break;
    case BAND40:
      digitalWrite(BAND_PIN, LOW);
      break;
  }
#endif
}

#ifdef CW
//
// setCW()
//
//

void setCW( bool on, uint32_t tone) {
  if ( on ) {
      uint32_t ifInjectFreq = sideband == USB ? bfo - tone : bfo + tone;
      si5351.clock_enable(SI5351_CLK2, false);
      si5351.set_freq(ifInjectFreq, 0, SI5351_CLK1);
      si5351.clock_enable(SI5351_CLK1, true);
  } else {
    si5351.clock_enable(SI5351_CLK1,false);
    setBFO( bfo );
    si5351.clock_enable(SI5351_CLK2, true);
  }
}
#endif

//********************setVFO******************************************
void setVFO(uint32_t freq) {
  //
  //  Set CLK0 to the to input frequency adjusted for the current BFO frequency

#ifdef DUAL_BAND
  //
  // Set filters for the band based on frequency
  // Save frequency and sideband for band switching
  //
  if (freq >= BAND20_EDGE) {
    setBandFilters(BAND20);
    band20Freq = freq;
    band20Sideband = sideband;
    band = BAND20;
  } else if (freq >= BAND40_EDGE) {
    setBandFilters(BAND40);
    band40Freq = freq;
    band40Sideband = sideband;
    band=BAND40;
  }
#endif

  uint32_t listen_freq = freq + bfo;
#ifdef CW
  if ( mode == L_CW)
    listen_freq += CW_TONE;
  else if ( mode == U_CW)
        listen_freq -= CW_TONE;
#endif

//  displayDebug("vfo="+String(listen_freq));

  si5351.set_freq(listen_freq, SI5351_PLL_FIXED, SI5351_CLK0);
  startSettingsTimer();  // start timer to save current settings
}

void setBFO(uint32_t freq) {
  //
  //  Set CLK2 to the to input frequency
  //

  si5351.set_freq(freq, 0, SI5351_CLK2);
}


//*********************Setup Arduino Pins*****************************

void setupPins() {
  //
  // Set the control buttons to INPUT_PULLUP
  // Button state will be HIGH when open and LOW when pressed
  //
  pinMode(TUNE_BTN, INPUT_PULLUP);                           // Tune - momentary button
  pinMode(VFO_BTN, INPUT_PULLUP);                            // VFO A/B Select - momentary button
  pinMode(SIDEBAND_BTN, INPUT_PULLUP);                       // Upper/lower SB Select - momentary button
  pinMode(BAND_BTN, INPUT_PULLUP);                           // Band Switch 20/40 - momentary button
  pinMode(PTT_SENSE, INPUT_PULLUP);                          // Mic PTT switch
  pinMode(PTT, OUTPUT);  digitalWrite(PTT, LOW);             // HIGH to enable TX
  pinMode(BAND_PIN, OUTPUT); digitalWrite(BAND_PIN, LOW);    // Band Switch Relay (LOW = NC = 40m,  HIGH = NO = 20m)
#ifdef CW
  pinMode(KEY_IN, INPUT_PULLUP);                             // CW key input
  pinMode(CW_OUT, OUTPUT); digitalWrite(CW_OUT, LOW);        // CW Relay (enable audio amp on Tx w/ tune tone as input)
#endif
  pinMode(LED_BUILTIN, OUTPUT);

}


//**********************Initialize SI5351******************************

void setupSI5351() {
  si5351.init(SI5351_CRYSTAL_LOAD_8PF);
  si5351.set_correction(CALIBRATION_OFFSET);           // calibration offset
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA); // Higher Drive since it is a ADE-1 DBM
#ifdef CW
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
#endif
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
  si5351.set_freq(bfo, 0, SI5351_CLK2);                // Initialize the bfo

  //
  // Initialize the VFO
  //
  switch (active_vfo) {
    case VFOA:
      setVFO(vfoAfreq);
      break;
    case VFOB:
      setVFO(vfoBfreq);
      break;
  }
}


//////////////////////////////////////////////////////////////////////
//                         Setup                                    //
//                                                                  //
// Called once by the Arduino operating system at startup           //
// All initialization code goes here                                //
//                                                                  //
//////////////////////////////////////////////////////////////////////
void setup() {

  uint32_t vfoActfreq;
  uint32_t vfoAltfreq;
  bool eeprom_ok;

#ifdef DEBUG
  Serial.begin(57600);
#endif

  setupPins();              // Initialize arduino pins
  setupEncoder();           // Initialize interrupt service for rotary encoder
  eeprom_ok = setupSettings();  // Retrieve settings from EEPROM
  setupSI5351();

 

  if (active_vfo == VFOA) {
    vfoActfreq = vfoAfreq;
    vfoAltfreq = vfoBfreq;
  } else {
    vfoActfreq = vfoBfreq;
    vfoAltfreq = vfoAfreq;
  }

  
  //  Initialize the display with startup values
  Delay(500);  // short delay to let the display initialize - needed for Nextion

  // Construct banner for TFT or Nextion display
  // "Vx.x   RIGNAME  CALLSIGN"
  String banner;
  banner = F("V");
  banner += F(VERSION);
  banner += F("   ");
  banner += F(RIGNAME);
  banner += F("   ");
  banner += F(CALLSIGN);

  displaySetup(banner,              // version number. call sign
               vfoActfreq, vfoAltfreq,  // Initial active and alternate VFO
               active_vfo,          // VFO A/B indicator
               TxRxState,           // TX/RX indicator
               sideband,            // LSB/USB,
               split,               // Split mode on/off
               increment,           // Tuning increment
               smeter);            // S Meter

  if ( eeprom_ok == false)
    displayDebug("Memory Reset");

  setupCat();

//  SetMode(L_SSB);  // fix mode XXXXX
}

void loop() {

  CheckEncoder();   // VFO frequency changes
  CheckIncrement(); // Encoder Button

#ifdef DUAL_BAND
  CheckBand();      // Band Switch 20/40
#endif

  CheckVFO();       // VFO A/B change
  CheckMode();      // USB/LSB/U_CW/L_CW change
  CheckPTT();       // Check for Mic PTT
#ifdef CW
  CheckCW();        // Check for CW key down
#endif
  CheckTune();      // Check for Tune button press

#ifdef SMETER
  CheckSmeter();    // Signal strength
#endif

  CheckCat();       // CAT Control
  CheckSettings();  // Update EEPROM on Settings Change

#ifdef DISPLAY_NEXTION
  CheckTouch();     // Check for touch screen action
#endif

}
