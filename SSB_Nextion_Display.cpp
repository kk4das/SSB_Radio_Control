#include "RadioControl.h"
#ifdef DISPLAY_NEXTION

//#define NEX_DEBUG   // sends debug messages to the banner (bottom of screen)
//#define NEX_DEBUG1

// Dean Souleles, KK4DAS
// 7/25/2020
//
// Nextion Display Module for SSB_Radio_Contrl
//  Nextion 2.8 Inch display
//  Tested on Arduino Nano
//
// Arduino Nano Every
// PINS / WIRING
// ----------------------
// Arduino | Nextion
// --------|-------------
// 1 (RX)  | TX  (Blu)     -- pins 0 and 1 are for Serial1
// 0 (TX)  | RX  (Yel)     
// +5      | +5  (Red)     
// Gnd     | Gnd (Blk)     
//------------------------
//
// Arduino Nano // deprecated - runs out of memory / stack crash
// PINS / WIRING
// ----------------------
// Arduino | Nextion
// --------|-------------
// 8 (RX)  | TX  (Blu)     -- pins 8 and 9 are requuired by AltSoftSerial
// 9 (TX)  | RX  (Yel)     -- to use other pins you can use SofwareSerial
// +5      | +5  (Red)     -- but that will conflict with the interrupt
// Gnd     | Gnd (Blk)     -- used by the Rotary Encoder
//------------------------


////////////////////////////////////////////////////////////////////////////////////////////
// 
// We use the AltSoftSerial library to pins (8 and 9) rather than default 0,1
// AltSoftSerial is used rather than SoftwareSerial to free up the interrupt vector
// required for the digital encoder 
// NOTE:
//     AltSoftSerial uses only fixed pins 8,9 on an Uno or Nano
//     Pin 10 cannot be used for PWM
//     For other boards see: https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html
//
// First, apply the serial fixes from Ray Livingston to the Nextion Libarary:
//      https://forum.arduino.cc/index.php?topic=620821.0
//      Replace files in the ../libraries/ITEADLIB_Arduino_Nextion folder:
//         NexConfig.h
//         NexHardware.h
//         NexHardware.cpp
//      Plus one more patch to fix a bug that was including SoftwareSerial when it should not have
//         NexUpload.cpp  (change by me, Dean Souleles)
//
// Next we edit NexConfig.h in the ../libraries/ITEADLIB_Arduino_Nextion folder as follows:  
//     Comment out this line:
//     #define nexSerial Serial
//    
//     Add these three lines:
//     #include <AltSoftSerial.h>
//     extern AltSoftSerial HMISerial;
//     #define nexSerial HMISerial
//
// Add the following to your sketch
//     #include <AltSoftSerial.h>
//     AltSoftSerial HMISerial;  //RX TX  - connect to Nextion TX RX - Must use Pins 9,10 on Uno/Nano       
//
////////////////////////////////////////////////////////////////////////////////////////////////

#include <Nextion.h>     // https://github.com/itead/ITEADLIB_Arduino_Nextion

//#include <SoftwareSerial.h>
//SoftwareSerial HMISerial(8,9);  //RX TX  - connect to Nextion TX RX - Pins 8,9 on Uno/Nano

#if _BOARDTYPE == Nano
#include <AltSoftSerial.h>
AltSoftSerial HMISerial;  //RX TX  - connect to Nextion TX RX - Must use Pins 8,9 on Uno/Nano
#endif

//
// Declare  Nextion objects 
// Use the page ID, component id and component name from the Nextion IDE
//

// Nextion PAGE
// Current design only uses one page of the display
#define PAGE 0



// Nextion Component IDs for buttons and text displays
// These ID's must match the ID's Nextion IDE
#define VFO_ID 2
#define SIDEBAND_ID 5
#define TUNE_ID 7
#define INCREMENT_ID 6
#define ACT_VFO_ID 3
#define ALT_VFO_ID 4
#define TX_RX_ID 8
#define SMETER_ID 10
#define BANNER_ID 9
#define TUNE_PLUS_ID 11
#define TUNE_MINUS_ID 12
#define TUNE_STATE_ID 15
#define ATOB_ID 18          // VFO A copy to B button 
#define SPLIT_ID 19
#define VAB_ID 20           // VFO Inidicator

// Nextion Component names
// These names must match the names in the Nextion IDE
#define VFO_NAME "bVFO"
#define SIDEBAND_NAME "bSideband"
#define TUNE_NAME "bTune"
#define INCREMENT_NAME "bIncr"
#define ACT_VFO_NAME "tActVFO"
#define ALT_VFO_NAME "tAltVFO"
#define BANNER_NAME "tBanner"
#define TX_RX_NAME "tTxRx"
#define SMETER_NAME "pSmeter"
#define TUNE_PLUS_NAME "bPlus"
#define TUNE_MINUS_NAME "bMinus"
#define TUNE_STATE_NAME "tTuneState"
#define ATOB_NAME "bAtoB"
#define SPLIT_NAME "btSplit"
#define VAB_NAME "tVAB"

// Nextion Buttons
// Each user interface object that the user touches needs to be defined here
// 
NexButton bVFO = NexButton(PAGE, VFO_ID, VFO_NAME);
NexButton bSideband = NexButton(PAGE, SIDEBAND_ID, SIDEBAND_NAME);
NexButton bTune = NexButton(PAGE, TUNE_ID, TUNE_NAME);
NexButton bIncrement = NexButton(PAGE, INCREMENT_ID, INCREMENT_NAME);
NexButton bTunePlus = NexButton(PAGE, TUNE_PLUS_ID, TUNE_PLUS_NAME);
NexButton bTuneMinus = NexButton(PAGE, TUNE_MINUS_ID, TUNE_MINUS_NAME);
NexButton bAtoB = NexButton(PAGE, ATOB_ID, ATOB_NAME); 
NexDSButton btSplit = NexDSButton(PAGE, SPLIT_ID, SPLIT_NAME);


//
// Setup a list of objects to respond to a touch event
//
NexTouch *nex_listen_list[] = {
  &bVFO,
  &bSideband,
  &bTune,
  &bIncrement,
  &bTunePlus,
  &bTuneMinus,
  &bAtoB,
  &btSplit,
  NULL
};

///////////////////////////////////////////////////////////////////////////
// HMI_send_command(cmd)
// Sends one command to the Nextion Display
//
// Example usage:  
//  String cmd;
//  cmd = F("vis ");
//  cmd = cmd + F(TUNE_STATE_NAME);
//  cmd = cmd + F(",");
//  if (on_off) {
//    cmd = cmd + F("1");
//  } else {
//    cmd = cmd + F("0");
//  }
//  HMI_send_command(cmd.c_str());
//  
///////////////////////////////////////////////////////////////////////////
void HMI_send_command(char* cmd) {

/*
// Send the command 
  HMISerial.print(cmd);
  
// Send end-of-message per Nextion protocol
  HMISerial.write(0xff);
  HMISerial.write(0xff);
  HMISerial.write(0xff);

*/
// Send the command to the Nextion
  nexSerial.print(cmd);
  
// Send end-of-message per Nextion protocol
  nexSerial.write(0xff);
  nexSerial.write(0xff);
  nexSerial.write(0xff);
  
}

///////////////////////////////////////////////////////////////////////////
// displayActVFO(uint32_t freq)
// Formats and displays Active and Alternat VFO frequencies
// Legal values:  Frequency in Hz
//
// To do - comibine into one function
///////////////////////////////////////////////////////////////////////////

void displayActVFO(uint32_t freq) {
  String cmd;
  String fmt;
  char f[11];
  uint32_t mil, hund_thou, ten_thou, thou, hund, tens, ones;

// Format number as nn.nnn.nnn
  mil = freq / 1000000;
  hund_thou = (freq/100000)%10;
  ten_thou =  (freq/10000)%10;
  thou =      (freq/1000)%10;
  hund =      (freq/100)%10;
  tens =      (freq/10)%10;
  ones =       freq%10;
  fmt=F("%2ld.%ld%ld%ld.%ld%ld%ld");
  snprintf(f, sizeof(f),fmt.c_str(),mil, hund_thou, ten_thou,thou, hund, tens, ones);

  cmd = F(ACT_VFO_NAME);
  cmd += F(".txt=\"");
  cmd += f;
  cmd += F("\"");
  HMI_send_command(cmd.c_str());

}

void displayAltVFO(uint32_t freq) {
  String cmd;
  String fmt;
  char f[11];
  uint32_t mil, hund_thou, ten_thou, thou, hund, tens, ones;

// Format number as nn.nnn.nnn
  mil = freq / 1000000;
  hund_thou = (freq/100000)%10;
  ten_thou =  (freq/10000)%10;  
  thou =      (freq/1000)%10;
  hund =      (freq/100)%10;
  tens =      (freq/10)%10;
  ones =       freq%10;
  fmt=F("%2ld.%ld%ld%ld.%ld%ld%ld");
  snprintf(f, sizeof(f),fmt.c_str(),mil, hund_thou, ten_thou,thou, hund, tens, ones);
 
  cmd = F(ALT_VFO_NAME);
  cmd += F(".txt=\"");
  cmd += f;
  cmd += F("\"");
  HMI_send_command(cmd.c_str());
  
}

void displaySMeter(byte level) {
// Nextion dipslay bar graph is set by integer percent 0-100
// Convert the S level into a % and send to display

  String cmd;
  float pct;
  int scaled_level;
  pct = (float(level)/13.0)*100.0;
  scaled_level=pct;

  cmd = F(SMETER_NAME);
  cmd += F(".val=");
  cmd += scaled_level;

  HMI_send_command(cmd.c_str());

#ifdef NEX_DEBUG1
  String msg = F("displaySMeter: ");
  msg = msg + level;
  msg = msg + F(" ");
  msg = msg + scaled_level;
  displayBanner(msg);
#endif

}

void displayBanner(String s) {
  String cmd;
  cmd = F(BANNER_NAME);
  cmd += F(".txt=\"");
  cmd += s;
  cmd += F("\"");
  HMI_send_command(cmd.c_str());
}

void displayVFOAB(int vfo) {
  String cmd;
  cmd = F(VAB_NAME);
  cmd += F(".txt=\"");
  if (vfo == VFOA) {
    cmd += F("A");
  } else {
    cmd += F("B");
  }
  cmd += F("\"");
  HMI_send_command(cmd.c_str());

#ifdef NEX_DEBUG
  String msg = F("displayVFOAB: ");
  msg = msg + vfo;
  displayBanner(msg);
#endif

}

void displayTxRx(int tx_rx) {
  String cmd;
  cmd = F(TX_RX_NAME);
  cmd += F(".txt=\"");
  if (tx_rx == TX) {
    cmd += F("TX");
  } else {
    cmd += F("RX");
  }
  cmd += F("\"");
  HMI_send_command(cmd.c_str());

#ifdef NEX_DEBUG
  String msg = F("displayTxRx: ");
  msg = msg + tx_rx;
  displayBanner(msg);
#endif

}

void displayMode(int mode) {
  String modeString;
  if (mode == USB) {
    modeString = F("USB");
  } else {
    modeString = F("LSB");
  }
  bSideband.setText(modeString.c_str());

#ifdef NEX_DEBUG
  String msg = F("displayMode: ");
  msg = msg + modeString;
  displayBanner(msg);
#endif

}

void displayIncr(uint32_t increment) {
  String hertz;
  switch (increment) {
    case 1:      hertz = F("   1"); break;
    case 10:     hertz = F("  10"); break;
    case 100:    hertz = F(" 100"); break;
    case 1000:   hertz = F("  1K"); break;
    case 10000:  hertz = F(" 10K"); break;
    case 100000: hertz=  F("100K"); break;
    case 1000000:hertz = F("  1M");
  }
  bIncrement.setText(hertz.c_str());

#ifdef NEX_DEBUG
  String msg = F("displayIncr: ");
  msg = msg + hertz;
  displayBanner(msg);
#endif

}

void displayTune(boolean on_off) {
// Send visibility command to Nextion
// Format:  vis <object_name),<0/1>  
  String cmd;
  cmd = F("vis ");
  cmd += F(TUNE_STATE_NAME);
  cmd += F(",");
  cmd += on_off;
  HMI_send_command(cmd.c_str());

#ifdef NEX_DEBUG
  displayBanner(cmd);
#endif
}


void displaySplit(boolean split) {
// The split button is a dual state button
//     State 0 = blue =  ff
//     State 1 = green = on
// To change the state and color set the val property to 0/1
// This mimics a button press on the display

  String cmd =F(SPLIT_NAME);
  cmd += F(".val=");
  if (split) {
    cmd += 1;
  } else {
    cmd += 0;
  }
  HMI_send_command(cmd.c_str());

#ifdef NEX_DEBUG
  String msg = F("displaySplit: ");
  msg = msg + split;
  displayBanner(msg);
#endif
}

//
// Button Callback Functions
// Called whenever a button is pressed and released
//

// VFO A/B Button
void bVFOPopCallback(void *ptr) {
 SwapVFO();  // call the VFO switch button handler
}

// LSB/USB Button
void bSidebandPopCallback(void *ptr) {
 SwapSB();
}

// Tune Plus (increase VFO)
void bTunePlusPushCallback(void *ptr) {
  AdjustVFO(increment);
}

// Tune Minus (decrease VFO)
void bTuneMinusPushCallback(void *ptr) {
  AdjustVFO(-1 * increment);
}

// Tune Tone
void bTunePopCallback(void *ptr) {
 DoTune();
}

// Tuning Increment Change
void bIncrementPopCallback(void *ptr) {
  AdvanceIncrement();
}

// Split on/off
void btSplitPopCallback(void *ptr) {
  uint32_t split_val;

  btSplit.getValue(&split_val);
  if (split_val==1) {
    startSplit();
  } else {
    stopSplit();
  }

#ifdef NEX_DEBUG
  String msg = F("btSplitPopCallback: ");
  msg = msg + split_val;
  displayBanner(msg);
#endif
}

//   Make Act and Alt VFO the same
void bAtoBPopCallback(void *ptr) {
  
  if (active_vfo == VFOA) {
    vfoBfreq = vfoAfreq;
  } else {
    vfoAfreq = vfoBfreq;
  }
  displayAltVFO(vfoAfreq);  // update the Alt VFO display

}

//
// Setup
// Called once at startup
//
void displaySetup(String banner,
                  uint32_t vfoActfreq, uint32_t vfoAltfreq,
                  uint32_t activeVFO,
                  int tx_rx,
                  int sideband,
                  boolean split,
                  uint32_t increment,
                  byte s_meter) {
                    
  nexInit(9600);   // Initialize the Nextion library
 
  //
  // Attach callback routines for each button
  // attachPop will set the library to invoke the specified funtion each time a button is released
  //
  bVFO.attachPop(bVFOPopCallback, &bVFO);                     // VFO A/B button
  bSideband.attachPop(bSidebandPopCallback, &bSideband);      // LSB/USB button
  bTune.attachPop(bTunePopCallback, &bTune);                  // Tune 
  bIncrement.attachPop(bIncrementPopCallback, &bIncrement);   // Change Incrmement
  btSplit.attachPop(btSplitPopCallback, &btSplit);            // Split On/Off
  bAtoB.attachPop(bAtoBPopCallback, &bAtoB);                  // Make Alt VFO = Act VFO

  bTunePlus.attachPush(bTunePlusPushCallback, &bTunePlus);    // Tune up
  bTuneMinus.attachPush(bTuneMinusPushCallback, &bTuneMinus); // Tune down
  
  //
  // Display the intitial values
  //
  displayBanner(banner);
  displayActVFO(vfoActfreq);
  displayAltVFO(vfoAltfreq);
  displayVFOAB(activeVFO);
  displayTxRx(tx_rx);
  displayMode(sideband);
  displaySplit(split);
  displayIncr(increment);
  displaySMeter(s_meter);
}



void CheckTouch() {
  // Call the Nextion check function to look for activites on your listen list
  nexLoop(nex_listen_list);
}


#endif
