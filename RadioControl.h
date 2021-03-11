#ifndef RadioControl_h
#define RadioControl_h

#define CALLSIGN "KK4DAS"
#define VERSION "1.4"
#define RIGNAME "SimpleSSB"
  

//#define DEBUG
#ifdef DEBUG
 extern char debugmsg[];
#endif


//=============================== FEATURE SELECTION =========================
// Each flag below enables optional features
// DISPLAY_X  - uncomment one line depending on the display that is attached
// BFOxXMHS   - IF filter center frequency - eitherr 9MHz or 12MHZ
// SMNETER    - Uncomment if S-meter sensor circuit is installed
// DUAL_BAND  - Uncomment to enable band switching 20/40 if installed
// CW         - Uncomment if CW mod installed (future)

//=============================== DISPLAY TYPE ==============================
#define DISPLAY_LCD         //uncomment for 20x4 LCD 
//#define DISPLAY_TFT         // uncomment for 320x240 TFT
//#define DISPLAY_NEXTION     //uncomment for 2.8" Nextion

//=============================== IF FILTER FREQ ============================
#define BFO9MHZ              // uncomment for 9.0 MHz  IF
//#define BFO12MHZ          // uncomment for 12.0 MHz  IF

//=============================== S-METER INSTALLED =========================
//#define SMETER              // uncomment if SMETER mod installed

//=============================== DUAL BAND MOD INSTALLED ===================
//#define DUAL_BAND           // uncomment if dual band mod installed 20/40 

//=============================== DISPLAY TYPE ==============================
//#define CW                 // uncomment if CW enabled (not complete)




//============================== BOARD TYPE (Nano, Every) =====================
#define Nano 0
#define Every 1
#define Uno 2

#if defined(ARDUINO_AVR_NANO)
#define _BOARDTYPE Nano
#endif
#if defined(ARDUINO_AVR_NANO_EVERY)
#define _BOARDTYPE Every
#endif
#if defined (ARDUINO_AVR_UNO)
#define _BOARDTYPE Uno
#endif

//============================= INCLUDES =====================================
#include <Arduino.h>
#include "Utility.h"         // General purpose common functions
#include "SSB_Display.h"     // Display handling
#include "RotaryEnc.h"       // Rotary encoder handling
#include "si5351.h"          // SI 5351 clock
#include "CAT.h"             // CAT Control handling
#include "Settings.h"        // Get/Store Settings in EEPROM
#include "Smeter.h"          // Smeter definitions


//============================== Symbolic constants ==========================


//////////////////////////////////////////////////////////////////////
//                                                                  //
//           Arduino Pin Definitions                                //
//                                                                  //
//////////////////////////////////////////////////////////////////////

#define ENCODER_A      2                  // Rotary Lib Default - Encoder pin A  D2 (interrupt pin)
#define ENCODER_B      3                  // Rotary Lib Default - Encoder pin B  D3 (interrupt pin)
#define PTT_SENSE      4                  // Detect Mic PT
#define PTT            5                  // LOW=Rx, HIGH=Tx

// Dual Band Pins (requires dual band mod)
#define BAND_BTN       6                  // Band Switch momentary button
#define BAND_PIN       7                  // Band Switch Relay -  LOW = Band A (40M), HIGH = Band B (20M)

#define VFO_BTN        A0                 // VFO A/B button
#define SIDEBAND_BTN   A1                 // USB/LSB button
#define TUNE_BTN       A2                 // Tune Button
#define ENCODER_BTN    A3                 // Rotary Lib Default - Encoder push button
#define I2C_SDA        A4                 // I2C SDA Pin
#define I2C_SCL        A5                 // I2C SCL Pin
#define TONE_PIN       A6                 // Audio out for tune tone

//  Smeter
#define SMETER_PIN     A7                 // Requires Signal Strength Sensor


//
// For Nextion / Nano Every Only
//     Pin 0,1  Serial RX/TX


//  For color TFT Only   
//        Arduino 
//         Pin       TFT Pin
//      -----------|---------    
//          8      |  RST  - any free Arduino Pin (not used in this sketch)
//          9      |  DC   - any free Arduino Pin
//          10     |  CS   - any free Arduino Pin 
//          11     |  MOSI - fixed
//          12     |  MISO - fixed
//          13     |  CLK  - fixed
//


// Tune Tone          
#define NOTE_B5      988                  // Tune tone

//
// Dual Band Mode Constants (requires DUAL_BAND)
//
#define BAND20  1
#define BAND40  2
#define BAND20_EDGE  14000000
#define BAND40_EDGE  7000000

// VFO, Sideband and TX state are used both here and in the display
// Define here if not already defined 

// VFO selection
#define VFOA 0 
#define VFOB 1

// Sideband selection
#define USB 0
#define LSB 1

// Transmit state
#define TX LOW      // TX is on when PTT switched to ground
#define RX HIGH


// PTT Source
#define PTT_MIC 0
#define PTT_CAT 1
#define PTT_TUNE 2

//=============== Globals ============================================

// Rotary Encoder
extern Rotary encoder;

// BFO
extern const uint32_t USB_BFO;
extern const uint32_t LSB_BFO;
extern uint32_t bfo;                     // Startup BFO frequency
extern const uint32_t BFO_DELTA;         // Difference between USB and LSB for BFO change

// VFO A/B frequencies
extern uint32_t vfoAfreq; 
extern uint32_t vfoBfreq; 
extern byte vfoASideband;
extern byte vfoBSideband;

// Tuning increment
extern uint32_t increment;

// Active VFO indicator
extern byte active_vfo;


// Active sideband (USB or LSB)
extern byte sideband;


// Band specific memory - requires DUAL_BAND
extern uint32_t band20Freq;
extern uint32_t band40Freq;
extern byte band20Sideband;
extern byte band40Sideband;
extern byte band;


// Transmit state
extern byte TxRxState;
extern byte lastTxRxState; 

// Transmoit source (mic, CAT)
extern byte txSource;

// S Meter
extern byte smeter;

// Split mode
extern bool split;


//=============== Function Prototypes ============================================

extern void setupEncoder();
extern void setVFO(uint32_t freq);
extern void setBFO(uint32_t freq);
extern void CheckIncrement();
extern void AdvanceIncrement();
extern void CheckEncoder();
extern void AdjustVFO(long delta);

extern void CheckSB();
extern void SwapSB();
extern void CheckTune();
extern void DoTune();
extern void CheckVFO();
extern void SwapVFO();
extern void CheckPTT();
extern void setupPins();
extern void startTx(byte PTT_source);
extern void stopTx();
extern void startSplit();
extern void stopSplit();
extern void CheckBand();    // Only called if DUAL_BAND enabled
extern void CheckSmeter();  // Only called if SMETER enabled

#ifdef CW
extern void setCW();
#endif

#endif
