//
//  Encoder fuctions
//
#include "RadioControl.h"

//#DEFINE DEBUG_ENC

//////////////////////////////////////////////////////////////////////
//                 Rotary Enconder                                  //
//////////////////////////////////////////////////////////////////////
Rotary encoder = Rotary(ENCODER_A, ENCODER_B, ENCODER_BTN); // Setup Encoder

//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Rotary Encoder Variables                            //
//   Number of clockwise and counterclockwise ticks                 //
//   Delta between successive measurements                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////
volatile int encoder_count = 0; // count of encoder clicks +1 for CW, -1 for CCW  (volatile since used in ISR)
int prev_encoder_count = 0;     // used to measure changes over time
int encoder_delta = 0;          // differrnce between successive checks of encoder count

bool incrementChanged = false;

// when multiplied by tuning increment tells what the frequency change on
// on the active VFO will be
// Encoder button control
extern byte EncButtonState;
extern byte lastEncButtonState;

///////////////////////////////////////////////////////////
//      ************* ISR ****************               //
// Interrupt service routine, called on encoder movement //
// Only interested in completed clicks                   //
//  +1 for Clocwwise                                     //
//  -1 for Counter Clockwise                             //
//  Ignore intermediate values                           //
///////////////////////////////////////////////////////////

#if _BOARDTYPE != Every
ISR(PCINT2_vect) {
  unsigned char result = encoder.process();
  if (result == DIR_CW) {
    encoder_count++;
  } else if (result == DIR_CCW) {
    encoder_count--;
  }
}

#else
#define PA0_INTERRUPT PORTA.INTFLAGS & PIN0_bm
#define PA0_CLEAR_INTERRUPT_FLAG PORTA.INTFLAGS &= PIN0_bm
#define PF5_INTERRUPT PORTF.INTFLAGS & PIN5_bm
#define PF5_CLEAR_INTERRUPT_FLAG PORTF.INTFLAGS &= PIN5_bm

ISR(PORTA_PORT_vect) {
  unsigned char result = encoder.process();

  if (PA0_INTERRUPT)
    PA0_CLEAR_INTERRUPT_FLAG;

  if (result == DIR_CW) {
    encoder_count++;
  } else if (result == DIR_CCW) {
    encoder_count--;
  }
}

ISR(PORTF_PORT_vect) {
  unsigned char result = encoder.process();

  if (PF5_INTERRUPT)
    PF5_CLEAR_INTERRUPT_FLAG;

  if (result == DIR_CW) {
    encoder_count++;
  } else if (result == DIR_CCW) {
    encoder_count--;
  }

}
#endif


//*******************Setup Interrupt Service Routine********************

void setupEncoderISR() {
  cli();
  // Set up for the rotary encoder interrupts
#if _BOARDTYPE != Every
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
#endif
#if _BOARDTYPE == Every
  PORTA.PIN0CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
  PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
#endif
  sei();
}


//********************CheckEncoder*******************************************

// roundedF - Calculates new frequency
// After an increment change rounds up or down to the next even increment
// Otherwise just changes increments or decrements the frequency
//
uint32_t roundedF(uint32_t freq, int32_t delta) {
  uint32_t f = freq;

  /*  if (incrementChanged) {
      if (f % increment) {
        if (delta > 0 ) {
          f = f + increment - (f % increment);
        } else {
          f = f - (f % increment);
        }
      }
      incrementChanged = false;
    } else {
      f = f + delta;
    }
  */
  if (f % increment) {
    if (delta > 0 ) {
      f = f + increment - (f % increment);
    } else {
      f = f - (f % increment);
    }
  } else {
    f = f + delta;
  }
  return f;
}

void AdjustVFO(long delta) {

  switch (active_vfo) {
    case VFOA:
      //      vfoAfreq = vfoAfreq + delta;
      vfoAfreq = roundedF(vfoAfreq, delta);
      setVFO(vfoAfreq);
      displayActVFO(vfoAfreq);
      break;
    case VFOB:
      //      vfoBfreq = vfoBfreq + delta;
      vfoBfreq = roundedF(vfoBfreq, delta);
      setVFO(vfoBfreq);
      displayActVFO(vfoBfreq);
      break;
  }
  startSettingsTimer();
}

void CheckEncoder() {
  int current_count = encoder_count;          // grab the current encoder_count
  int32_t encoder_delta = 0;

  if (current_count != prev_encoder_count) {  // if there is any change in the encoder coount

#ifdef DEBUG_ENC
    sprintf(debugmsg, "VFOA: %ld", vfoAfreq);
    Serial.println(debugmsg);
#endif

    //
    //  Calculate the delta (how many click positive or negaitve)
    //
    encoder_delta = current_count - prev_encoder_count;

    //
    //  Adjust the currently selected VFO
    //
    AdjustVFO(encoder_delta * increment);


#ifdef DEBUG
    sprintf(debugmsg, "current_count: %d, New VFOA: %ld", current_count, vfoAfreq);
    Serial.println(debugmsg);
#endif

    prev_encoder_count = current_count;  // save the current_count for next time around

  }
}

//********************CheckIncrement*******************************************
// Cycle through tuning increment values on button press
// 10, 100, 1K, 10K, 100K, 1M
//********************CheckIncrement*******************************************
void AdvanceIncrement() {
  if (increment == 10) {
    increment = 100;
  }
  else if (increment == 100) {
    increment = 1000;
  }
  else if (increment == 1000) {
    increment = 10000;
  }
  else if (increment == 10000) {
    increment = 100000;
  }
  else if (increment == 100000) {
    increment = 1000000;
  }
  else {
    increment = 10;
  }
  displayIncr(increment);
  incrementChanged = true;
  startSettingsTimer();
}

void CheckIncrement () {

  EncButtonState = encoder.buttonState();
  //EncButtonState = digitalRead(ENCODER_BTN);
#ifdef DEBUG
  sprintf(debugmsg, "Encoder button state: %d", EncButtonState);
  Serial.println(debugmsg);
  Delay(1000);
#endif
  if (EncButtonState != lastEncButtonState) {
#ifdef DEBUG
    sprintf(debugmsg, "Encoder button state: %d", EncButtonState);
    Serial.println(debugmsg);
#endif
    if (EncButtonState == LOW) {
      AdvanceIncrement();

    }
    lastEncButtonState = EncButtonState;
    Delay(50);
    EncButtonState = encoder.buttonState();  //debounce
    //EncButtonState = digitalRead(ENCODER_BTN);
  }
}

void setupEncoder() {
  setupEncoderISR();
}
