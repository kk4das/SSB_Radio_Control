/*
 * ButtonState.cpp
 *
 *  Created on: Mar 22, 2022
 *      Author: KA4CDN
 */

#include "ButtonState.h"

#define DEPRESSED     0
#define NOT_DEPRESSED 1

#define LONG_PRESS_TIME 1500 // msec
#define DEBOUNCE_TIME     50 // msec

#define FILTER_DEPTH    25  // Number of button presses to average
#define BTN_PRESS_THRSH 0.3  // Threshold to consider an average of 1/0 to be pressed

ButtonState::ButtonState (ButtonState::BUTTON_TYPE longPressSupport)
{
  m_supportLongPress = longPressSupport;
  m_lastBtnState = m_priorBtnState = NOT_DEPRESSED;
}

ButtonState::~ButtonState ()
{
  // TODO Auto-generated destructor stub
}

/*
 * Check the button state. Pass in the current read state of the button and this
 * method debounces and returns a single shot indication of the button press.
 *
 * If the button was constructed as supporting long presses then the type of
 * press, short or long, is returned.
 *
 * In:  byte buttonPressed. Pass 0 if button depressed and 1 if not depressed
 * Out: ButtonState::BUTTON_STATE
 *          NO_PRESS : button not pressed
 *          PRESSED  : button pressed (only if button does not support long presses)
 *          SHORT_PRESS : button pressed and release (long press support)
 *          LONG_PRESS  : button pressed and held, a long press
 */
enum ButtonState::BUTTON_STATE ButtonState::CheckButton(byte rawButtonPressed )
{
  BUTTON_STATE retState = NO_PRESS;

  // Filter the button high freq. bouces
  byte buttonPressed = filteredButton( rawButtonPressed );

  // Check for low freq. bounce stability
  if ( debounced(buttonPressed) )
  {
    if ( buttonPressed != m_lastBtnState )
    {
        if ( buttonPressed == DEPRESSED )
        {
          m_buttonLowTime = millis();  // Only used for long press checks
          m_ignoreUp = false;
        }
        else if (m_ignoreUp == false) // ignore button up after long press
        {
          retState = m_supportLongPress == LONG_SUPPORT ? SHORT_PRESS : PRESSED;
        }
    }
    else if ( m_supportLongPress == LONG_SUPPORT )
    {
        if ( (buttonPressed == DEPRESSED)
            && (m_ignoreUp == false) // ignore button up after long press
            && ((millis() - m_buttonLowTime) > LONG_PRESS_TIME) )
        {
          retState = LONG_PRESS;
          m_ignoreUp = true;
          m_buttonLowTime = millis();
        }
    }
  }

  // Reset the debounce timer after a valid press detect.
  if ( retState != NO_PRESS )
    resetDebounceStart();

  m_lastBtnState = buttonPressed;
  return retState;
}

byte ButtonState::filteredButton( byte buttonPress )
{
  m_avgButtonState = m_avgButtonState * ( FILTER_DEPTH - 1 ) / FILTER_DEPTH + (float) buttonPress / FILTER_DEPTH;

  if ( m_avgButtonState < BTN_PRESS_THRSH )
    return DEPRESSED;
  else
    return NOT_DEPRESSED;
}


bool ButtonState::debounced(byte buttonPress)
{
  bool debounced = true;

  if ( millis() < m_stableTime + DEBOUNCE_TIME )
  {
    debounced = false;
    if ( buttonPress != m_priorBtnState )
    {
        m_stableTime = millis();
        m_priorBtnState = buttonPress;
    }
  }
  return debounced;
}

void ButtonState::resetDebounceStart(void)
{
  m_stableTime = millis();
}
