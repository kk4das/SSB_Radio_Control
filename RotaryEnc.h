/*
 * Rotary encoder library for Arduino.
 * 
 * Revision: 2020 Dean Souleles, KK4DAS, Licensed under the GNU GPL Version 3.
 * Contact: kk4das@gmail.com 
 * Added default pins and button handling and setup for clean interrupt handling
 * 
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 */

#ifndef RotaryEnc_h
#define RotaryEnc_h


#include "Arduino.h"

// Enable this to emit codes twice per step.
//#define HALF_STEP

// Enable weak pullups
#define ENABLE_PULLUPS

// Values returned by 'process'
// No complete step yet.
#define DIR_NONE 0x0
// Clockwise step.
#define DIR_CW 0x10
// Counter-clockwise step.
#define DIR_CCW 0x20


//
// Default Encoder wiring  Pins D2 and D3 for the encoder, A3 for the pushbutton
//
#ifndef ENCODER_A
#define ENCODER_A    2                    // Encoder pin A  D2 (interrupt pin)
#endif

#ifndef ENCODER_B
#define ENCODER_B    3                    // Encoder pin B  D3 (interrupt pin)
#endif


#ifndef ENCODER_BTN
#define ENCODER_BTN  A3                   // Encoder push buttonh
#endif


class Rotary
{
  public:
    Rotary(char, char, char);
    Rotary();
    unsigned char process();
    byte buttonState();
  private:
    unsigned char state;
    unsigned char pin1;
    unsigned char pin2;
    unsigned char btn;
    void Constructor_Common();
};

#endif
 
