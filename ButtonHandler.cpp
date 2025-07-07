// Button handler
#include "RadioControl.h"
#include "ButtonState.h"

//#define BTN_DEBUG

//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Button Control Variables                            //
//                                                                  //
//////////////////////////////////////////////////////////////////////
byte TuneButtonState = 0;
byte lastTuneButtonState = 0;

// Encoder button control
ButtonState EncButton(ButtonState::LONG_SUPPORT);

//  Sideband select button control
ButtonState SideBandButton(ButtonState::NORMAL);

//  Band Switch button control
ButtonState BandButton(ButtonState::NORMAL);

// VFO select button control
ButtonState VfoButton(ButtonState::NORMAL);


// PTT Control
byte PTTState = 0;
byte lastPTTState = 0;

// CW Control
#define CW_TX_OFF_TIMER    500 // milliseconds before releasing TX after cw tx
unsigned long cw_tx_time = 0;

// Helper function to get sideband from mode
byte GetSidebandFromMode(byte mode_val) {
  switch (mode_val) {
    case L_SSB:
#ifdef CW
    case L_CW:
#endif
      return LSB;
    case U_SSB:
#ifdef CW
    case U_CW:
#endif
      return USB;
    default:
      return USB; // Default fallback
  }
}

//
//********************* Set a new mode *************************************
// new mode select the mode and sets the proper sideband.
//
void SetMode(byte new_mode)
{
  // Save new mode
  mode = new_mode;
  
  // Get sideband directly from mode
  byte new_sideband = GetSidebandFromMode(new_mode);
  
  // Apply the sideband settings (calls SetSB internally)
  SetSB(new_sideband);
  
  // Save the mode for the current VFO
  switch (active_vfo) {
    case VFOA:
      vfoAmode = mode;
      break;
    case VFOB:
      vfoBmode = mode;
      break;
  }
  
  // Update the display to show the active mode
  displayMode(mode);
}

//*********************Check Band ************************************
void SwapBand() {
  
  uint32_t freq;
  byte band_mode;
  
  if (band == BAND20) {          // Switch to 40 Meter Band
    band = BAND40;
    freq = band40Freq;
    // Determine mode from stored sideband (for backward compatibility)
    band_mode = (band40Sideband == LSB) ? L_SSB : U_SSB;
  } else {                        // Switch to 20 Meter Band
    band = BAND20;
    freq = band20Freq;
    // Determine mode from stored sideband (for backward compatibility)
    band_mode = (band20Sideband == LSB) ? L_SSB : U_SSB;
  }

  // Set the active VFO to new frequency
  switch (active_vfo) {
    case VFOA:
      vfoAfreq = freq;
      // Keep the VFO's current mode if it matches the band's sideband
      if (GetSidebandFromMode(vfoAmode) == GetSidebandFromMode(band_mode)) {
        SetMode(vfoAmode);
      } else {
        SetMode(band_mode);
      }
      displayActVFO(vfoAfreq);
      break;
    case VFOB:
      vfoBfreq = freq;
      // Keep the VFO's current mode if it matches the band's sideband
      if (GetSidebandFromMode(vfoBmode) == GetSidebandFromMode(band_mode)) {
        SetMode(vfoBmode);
      } else {
        SetMode(band_mode);
      }
      displayActVFO(vfoBfreq);
      break;
  }

  displayMode(mode);
  startSettingsTimer();
}

void CheckBand() {
  if ( BandButton.CheckButton(digitalRead(BAND_BTN)) == ButtonState::PRESSED )
  {
#ifdef BTN_DEBUG
      ToggleLED();
      String msg = F("CheckBand");
      displayBanner(msg);
#endif      
     SwapBand();
  }
}

//*********************Change Modes *************************************

void ChangeMode() {

  switch (mode) {
#ifndef CW
    case L_SSB:
      SetMode(U_SSB);
      break;
    case U_SSB:
      SetMode(L_SSB);
      break;
#else
    case L_SSB:
      SetMode(U_SSB);
      break;
    case U_SSB:
      SetMode(L_CW);
      break;
    case L_CW:
      SetMode(U_CW);
      break;
    case U_CW:
      SetMode(L_SSB);
      break;
#endif
  }
}


//*********************Set a new sideband ************************************
void SetSB(byte sb) {

  sideband = sb;  // Keep this for hardware compatibility
  bfo = (sb == USB ? USB_BFO : LSB_BFO);

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
      break;
    case VFOB:
      setVFO(vfoBfreq);
      break;
  }
//  displayDebug("save mode="+String(mode));
  startSettingsTimer();
}

void CheckMode() {
  if ( SideBandButton.CheckButton(digitalRead(SIDEBAND_BTN)) == ButtonState::PRESSED )
    ChangeMode();
}


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
//    noTone(TONE_PIN); // Uncomment for pulsed output
    Delay(50);
  }
  
  noTone(TONE_PIN);
  stopTx();
  displayTune(false);
}


//*********************Check if Tune Button pressed ****************
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
  
  // Save current VFO mode before switching
  switch (active_vfo) {
    case VFOA:
      vfoAmode = mode;
      break;
    case VFOB:
      vfoBmode = mode;
      break;
  }
  
  // Switch to the other VFO and restore its mode
  if (active_vfo == VFOA) {
    active_vfo = VFOB;
    SetMode(vfoBmode);  // This will set mode, sideband, BFO, etc.
  } else {
    active_vfo = VFOA;
    SetMode(vfoAmode);  // This will set mode, sideband, BFO, etc.
  }

#ifdef BTN_DEBUG
  ToggleLED();
  String msg = F("SwapVFO: active_vfo=");
  msg += active_vfo;
  msg += F(" mode=");
  msg += mode;
  displayBanner(msg);
#endif
 
  //
  // Swap Active/Alternate frequency displays
  // 
  switch (active_vfo) {
    case VFOA:
      displayActVFO(vfoAfreq);
      displayAltVFO(vfoBfreq);
      break;
    case VFOB:
      displayActVFO(vfoBfreq);
      displayAltVFO(vfoAfreq);
      break;
  }
  displayVFOAB(active_vfo);            // Change the A/B indicator
  
  startSettingsTimer();
}


//********************* Check if Swap VFO pressed ****************
void CheckVFO() {
  if ( VfoButton.CheckButton(digitalRead(VFO_BTN)) == ButtonState::PRESSED )
      SwapVFO();
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

#ifdef CW
//********************* CW key down ****************************
void CheckCW() {
  if ( mode != L_CW && mode != U_CW )
    return;

  CwTxRxState = digitalRead(KEY_IN);

  // Keep the PA up between CW keydowns
  if ((TxRxState==TX) && (txSource == PTT_CW) && (CwTxRxState != TX)) {
      if ((millis() - CW_TX_OFF_TIMER) > cw_tx_time) {
//          displayDebug("");
          stopTx();
          digitalWrite(CW_OUT,LOW);
          setCW(false, CW_TONE);
      }
  }

  // Transition states between key up and key down.
  if(CwTxRxState != lastCwTxRxState){
      if (CwTxRxState == TX) {
//          displayDebug("CW TX");
          setCW(true, CW_TONE);
          startTx(PTT_CW);            // key the transmitter
          digitalWrite(CW_OUT,HIGH);  // power the audio amp, switch tune tone as input
          cw_tx_time = millis();      // reset the PA tx timer.
          tone(TONE_PIN, CW_TONE);
      } else {
          noTone(TONE_PIN);
      }
      lastCwTxRxState = CwTxRxState;
      Delay(50);
  }
}
#endif
