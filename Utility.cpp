/* Utility Functions
 *  
 *  Delay(interval) - non-blocking delahy
 */

#include "Arduino.h"
#include "Utility.h"

/*
 *  "Delay" is a non-blocking delay function. The standard Arduino "delay" function
 *  turns off interrupts, which prevents the Serial I/O functions from working. That
 *  sometimes causes characters to be missed and junk commands to show up.
 */

void Delay ( unsigned long interval )
{
unsigned long currentTime = 0UL;

unsigned long startTime = millis();

  while ( startTime + interval > currentTime )
      currentTime = millis();
}


/*
 * ToggleLED - toggles built-in LED On/Off - useful for debug
 */
byte LEDState=LOW;
void ToggleLED(){
  if (LEDState == LOW){
    LEDState = HIGH;
  } else {
    LEDState = LOW;
  }
  digitalWrite(LED_BUILTIN, LEDState);
}
