/*
 * ButtonState.h
 *
 *  Created on: Mar 22, 2022
 *      Author: KA4CDN
 */

#ifndef BUTTONSTATE_H_
#define BUTTONSTATE_H_

#include <Arduino.h>

class ButtonState
{
public:
  enum BUTTON_STATE { NO_PRESS, PRESSED, SHORT_PRESS, LONG_PRESS };
  enum BUTTON_TYPE  { NORMAL, LONG_SUPPORT };

  ButtonState ( BUTTON_TYPE type );
  virtual
  ~ButtonState ();
  enum BUTTON_STATE CheckButton(byte buttonPressed );

private:
  BUTTON_TYPE   m_supportLongPress;
  byte          m_lastBtnState;
  unsigned long m_buttonLowTime = 0;
  bool          m_ignoreUp = false;

  // Debounce
  unsigned long m_stableTime = 0;
  byte          m_priorBtnState;
  bool debounced(byte buttonPress);
  void resetDebounceStart(void);

  // filtering
  float         m_avgButtonState = 1;
  byte filteredButton( byte buttonPress );
};

#endif /* BUTTONSTATE_H_ */
