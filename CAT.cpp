/**************************************************************
   F40 CAT Control
    1/24/2021 - KK4DAS Version 1.3
    Changed to IC746 CAT Library

 ***************************************************************/
#include "RadioControl.h"
#include <IC746.h>

IC746 radio = IC746();

//#define CAT_DEBUG

// radio modes
#define MODE_LSB 00
#define MODE_USB 01
#define MODE_CW 02

// TX Rx State
//     FT857D combine TX and RX status
//     0b abcdefgh
//      a = 0 = PTT on
//      a = 1 = PTT off
//      b = 0 = HI SWR off
//      b = 1 = HI SWR on
//      c = 0 = split on
//      c = 1 = split off
//      d = dummy data
//      efgh = SWR meter data ??




// function to run when we must put radio on TX/RX
// If PTT requests transmit and the rig is not already transmitting, start TX
// If PTT request stop transmit and the current trasnmit source is PTT_CAT then stop TX
//
void catGoPtt(boolean pttf) {

#ifdef CAT_DEBUG
  String msg = F("CatGoPtt ");
  msg += pttf;
  displayBanner(msg);
#endif

  if ((TxRxState == TX) && (txSource != PTT_CAT)) {
    return;
  }

  if (pttf) {
    // displayDebug("CAT PTT ON ");
    if (TxRxState == RX) {
      startTx(PTT_CAT);
    }
  } else {

    if (txSource == PTT_CAT) {
      // displayDebug("CAT PTT OFF");
      stopTx();
    }
  }
}

boolean catGetPtt() {
#if defined (DEBUG)
  String msg = "GetPTT: ";
  if (ptt == PTT_TX) {
    msg += "Tx";
  } else {
    msg += "Rx";
  }
  displayPrintln(msg);
#endif

  if (TxRxState == TX) {
    return true;
  } else {
    return false;
  }
}

void catGoSplit(boolean cat_split) {

#ifdef CAT_DEBUG
  String msg = F("CatGoSplit ");
  msg += cat_split;
  displayBanner(msg);
#endif
  if (cat_split) {
    startSplit();
  } else {
    stopSplit();
  }
}

// function to run when VFOs A/B are toggled
void catSwapVfo() {
  SwapVFO();

#ifdef CAT_DEBUG
  String msg = F("catGoToggleVFOs");
  displayBanner(msg);
#endif

}

// function to set a freq from CAT
void catSetFreq(long f) {

  if (f == 0) return; // ignore spurious command

  //
  // Change the frequency of the current active VFO
  // Clock frequency is the operating frequecy plus the BFO
  //
  if (active_vfo == VFOA) {
    vfoAfreq = f;
    setVFO(vfoAfreq);
    displayActVFO(vfoAfreq);
  } else {
    vfoBfreq = f;
    setVFO(vfoBfreq);
    displayActVFO(vfoBfreq);
  }

#ifdef CAT_DEBUG
  String msg = F("catsetFreq ");
  msg += f;
  displayBanner(msg);
#endif
  startSettingsTimer();
}

// function to set the mode(LSB or USB) from the cat command
void catSetMode(byte m) {
  // If the new mode is different from the current sideband, then swap USB/LSB
  switch (sideband) {
    case USB:
      if (m == MODE_LSB) SwapSB();
      break;
    case LSB:
      if (m == MODE_USB) SwapSB();
      break;
  }

#ifdef CAT_DEBUG
  String msg = F("CatSetMode ");
  if (sideband == USB ) {
    msg += F("USB ");
  } else {
    msg += F("LSB ");
  }
  msg += m;
  displayBanner(msg);
#endif
}

// function to pass the freq to the cat library
long catGetFreq() {
  // this must return the freq as an unsigned long in Hz, you must prepare it before
  long freq;

  //   displayPrintln("catGetFreq called");
  if (active_vfo == VFOA) {
    freq = vfoAfreq;
  } else {
    freq = vfoBfreq;
  }

#ifdef CAT_DEBUG
  String msg = F("catGetFreq ");
  msg += freq;
  displayBanner(msg);
#endif

  return freq;
}

// function to pass the mode to the cat library
byte catGetMode() {
  // this must return the mode in the wat the CAT protocol expect it
  byte mode;

  if (sideband == USB) {
    mode = MODE_USB;
  } else {
    mode = MODE_LSB;
  }

#ifdef CAT_DEBUG
  String msg = F("CatGetMode ");
  if (sideband == USB ) {
    msg += F("USB ");
  } else {
    msg += F("LSB ");
  }
  msg += mode;
  displayBanner(msg);
#endif

  return mode;
}

// function to pass the smeter reading in RX mode
byte catGetSMeter() {
  // this must return a byte in with the 4 LSB are the S meter data
  // so this procedure must take care of convert your S meter and scale it
  // up to just 4 bits

#ifdef CAT_DEBUG
  String msg = F("CatGetSMeter ");
  msg += smeter;
  displayBanner(msg);
#endif

  return smeter;
}


// Function to select the active VFO from the cat command
// If requested VFO is not already active then swap active and alternate
void catSetVFO(byte catVfo) {
  if ( ((catVfo == CAT_VFO_A) && (active_vfo == VFOB)) ||
       ((catVfo == CAT_VFO_B) && (active_vfo == VFOA))) {
    SwapVFO();
  }

#if defined (DEBUG)
  String msg = "SetVFO: ";
  if (v == CAT_VFO_A) {
    msg += "VFO-A";
  } else {
    msg += "VFO-B";
  }
  displayPrintln(msg);
#endif
}

// Function to make VFOS the same
void catVfoAtoB() {

  if (active_vfo == VFOA) {
    vfoBfreq = vfoAfreq;
    vfoBSideband = vfoASideband;
    displayAltVFO(vfoBfreq);
  } else {
    vfoAfreq = vfoBfreq;
    vfoASideband = vfoBSideband;
    displayAltVFO(vfoAfreq);
  }

#if defined (DEBUG)
  String msg = "VfoAtoB";
  displayPrintln(msg);
#endif
}


void setupCat() {

  // Setup the CAT control command handlers
  radio.addCATPtt(catGoPtt);
  radio.addCATGetPtt(catGetPtt);
  radio.addCATAtoB(catVfoAtoB);
  radio.addCATSwapVfo(catSwapVfo);
  radio.addCATsplit(catGoSplit);
  radio.addCATFSet(catSetFreq);
  radio.addCATMSet(catSetMode);
  radio.addCATVSet(catSetVFO);
  radio.addCATGetFreq(catGetFreq);
  radio.addCATGetMode(catGetMode);
  radio.addCATSMeter(catGetSMeter);

  // now we activate the library

  radio.begin(19200, SERIAL_8N1);

#ifdef CAT_DEBUG
  displayBanner(String(F("setupCAT")));
#endif

}


void CheckCat() {
  radio.check();
}
