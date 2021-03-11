// Button handler
#include "RadioControl.h"

//#define BTN_DEBUG

//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Button Control Variables                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte TuneButtonState = 0;
byte lastTuneButtonState = 0;

// Encoder button control
byte EncButtonState = 0;
byte lastEncButtonState = 0;

//  Sideband select button control
byte SideBandButtonState = 0;
byte lastSideBandButtonState = 0;

//  Band Switch button control
byte BandButtonState = 0;
byte lastBandButtonState = 0;

// VFO select button control
byte VFOButtonState = 0;
byte lastVFOButtonState = 0;

// PTT Control
byte PTTState = 0;
byte lastPTTState = 0;


//*********************Check Band ************************************
void SwapBand() {
  
  uint32_t freq;
  
  if (band == BAND20) {          // Switch to 40 Meter Band
    band=BAND40;
    sideband=band40Sideband;
    freq=band40Freq;   
  } else {                        // Switch to 20 Meter Band
    band = BAND20;
    sideband=band20Sideband;
    freq=band20Freq;
  }

  // 
  // Make sure BFO clock tracks with band change
  //
  if (sideband == USB) {
    bfo = USB_BFO;
  } else { 
    bfo = LSB_BFO;
  }  

  //
  // Change the clock frequency to the new bfo
  //
  setBFO(bfo);

  //
  // Set the active VFO to new frequency
  //
  switch (active_vfo) {
    case VFOA:
      vfoAfreq=freq;
      setVFO(vfoAfreq);
      displayActVFO(vfoAfreq);
      break;
    case VFOB:
      vfoBfreq=freq;
      setVFO(vfoBfreq);
      displayActVFO(vfoBfreq);
      break;
  }

  //
  // Update the display to show the active mode/sideband
  //
  displayMode(sideband);
  startSettingsTimer();
}

void CheckBand() {
  BandButtonState = digitalRead(BAND_BTN);
  if (BandButtonState != lastBandButtonState) {

    //
    // On button press, change band 20/40
    //
    if (BandButtonState == LOW) { // if button pressed

#ifdef BTN_DEBUG
       ToggleLED();
       String msg = F("CheckBand");
       displayBanner(msg);
#endif      
      SwapBand();
    }

    lastBandButtonState = BandButtonState;
    Delay(50);
    BandButtonState = digitalRead(BAND_BTN);  //debounce
  }
}


//*********************Check Sideband ************************************
void SwapSB() {
  if (sideband == USB) {          // Switch to LSB
    sideband = LSB;
    bfo = LSB_BFO;
  } else {                        // Switch to USB
    sideband = USB;
    bfo = USB_BFO;
  }

  //
  // Keep track of which SSB is currently selected for current band
  //
  if (band == BAND20) {
    band20Sideband = sideband;
  } else {
    band40Sideband = sideband;
  }

  //
  // Change the clock frequency to the new bfo
  //
  setBFO(bfo);

  //
  // Set the active VFO to adjusted frequency
  //
  switch (active_vfo) {
    case VFOA:
      setVFO(vfoAfreq);
      vfoASideband=sideband;
      break;
    case VFOB:
      setVFO(vfoBfreq);
      vfoBSideband=sideband;
      break;
  }

  //
  // Update the display to show the active mode/sideband
  //
  displayMode(sideband);
  startSettingsTimer();
}

void CheckSB() {
  SideBandButtonState = digitalRead(SIDEBAND_BTN);
  if (SideBandButtonState != lastSideBandButtonState) {

    //
    // On button press, change active sideband
    //   Set the BFO as appropriate
    //
    if (SideBandButtonState == LOW) { // if button pressed
      SwapSB();
    }

    lastSideBandButtonState = SideBandButtonState;
    Delay(50);
    SideBandButtonState = digitalRead(SIDEBAND_BTN);  //debounce
  }
}

/*
//********************* Tune Button Handling ************************************
void DoTune() {
  //
  //
  // CW experiment - call doCW to turn off BFO, set LO to operating frequencey+700 
  // Then activate Tx - should generate a carrier
  //
  // Does but level is too low to drive the Tx amps
  //

  displayTune(true);

  setCW();
  
  startTx(PTT_TUNE);
  
  for (int i = 0; i < 100; i++) {
//    tone(TONE_PIN, NOTE_B5);
    Delay(50);
//    noTone(TONE_PIN);
    Delay(50);
  }
  stopTx();
  displayTune(false);
}
*/

//********************* Tune Button Handling ************************************
void DoTune() {
  //
  // Temp code - 30 second full duty tone for debugging circuits - cheap tone generator.
  //      Delay(12);
  //      tone(TONE_PIN, NOTE_B5);
  //      Delay(30000);
  //      noTone(TONE_PIN);

  displayTune(true);
  startTx(PTT_TUNE);
  
  for (int i = 0; i < 100; i++) {
    tone(TONE_PIN, NOTE_B5);
    Delay(50);
    noTone(TONE_PIN);
    Delay(50);
  }
  stopTx();
  displayTune(false);
}



void CheckTune() {
  TuneButtonState = digitalRead(TUNE_BTN); // creates a 10 second tuning pulse trani 50% duty cycle and makes TUNE appear on the screen
  if (TuneButtonState != lastTuneButtonState) {
    if (TuneButtonState == LOW) {
      DoTune();
    }
    lastTuneButtonState = TuneButtonState;
    Delay(50);
  }
}

//*********************VFO switch******* VFO A or B ****************
void SwapVFO() {
  
  if (active_vfo == VFOA) {
    active_vfo = VFOB;                // Make VFOB Active
    sideband=vfoBSideband;
  } else {
    active_vfo = VFOA;                // Make VFOA Active
    sideband=vfoASideband;
  }

  //
  // Adjust BFO in case sideband has changed
  //
  if (sideband == USB) {          // Switch to LSB
    bfo = USB_BFO;
  } else {                        // Switch to USB
    bfo = LSB_BFO;
  }
  setBFO(bfo);
  displayMode(sideband);               // Change sideband indicator

  
#ifdef BTN_DEBUG
  ToggleLED();
  String msg = F("SwapVFO: active_vfo=");
  msg += active_vfo;
  displayBanner(msg);
#endif
 
  //
  // Swap Active/Alternate frequency displays
  // 
  switch (active_vfo) {
    case VFOA:
      setVFO(vfoAfreq);
      displayActVFO(vfoAfreq);
      displayAltVFO(vfoBfreq);
      break;
    case VFOB:
      setVFO(vfoBfreq);
      displayActVFO(vfoBfreq);
      displayAltVFO(vfoAfreq);
      break;
  }
  displayVFOAB(active_vfo);            // Change the A/B indicator
  displayMode(sideband);               // Change sideband indicator
  
  startSettingsTimer();
}


void CheckVFO() {

  VFOButtonState = digitalRead(VFO_BTN);
  if (VFOButtonState != lastVFOButtonState) {
    if (VFOButtonState == LOW) {       // button pressed
      SwapVFO();
    }
    lastVFOButtonState = VFOButtonState;
    Delay(50);
    VFOButtonState = digitalRead(VFO_BTN);  //debounce
  }
}


// startSplit
// Turn on split mode and update the display
void startSplit() {
  split = true;
  displaySplit(split);
}

// Turnb off split mode and update the display
void stopSplit() {
  split = false;
  displaySplit(split);
}

// startTx
// if split mode is on swap Act and Alt VFO
// Put the rig in to TX by triggering the PTT pin
// Update the display to Tx
void startTx(byte PTT_source) {
  if (split) SwapVFO();
  digitalWrite(PTT,HIGH);
  displayTxRx(TX);
  TxRxState = TX;
  txSource = PTT_source;
}

// stopTx
// Return the rig to Rx by lowering the PTT pin
// If split mode is on swap Act and Alt VFO
// Update the display to Tx
void stopTx() {
  digitalWrite(PTT,LOW);
  if (split) SwapVFO();
  displayTxRx(RX);
  TxRxState = RX;
}

//********************* PTT ****************************

void CheckPTT(){ 

  if ((TxRxState==TX) && (txSource != PTT_MIC)) {
    return;
  }
  
  TxRxState = digitalRead(PTT_SENSE); 

  if(TxRxState != lastTxRxState){

    if(TxRxState == TX){
      startTx(PTT_MIC); 
    } else {
      if (txSource == PTT_MIC) {
        stopTx();
      }
    } 
    lastTxRxState = TxRxState;
    Delay(50);
  }
  
}
