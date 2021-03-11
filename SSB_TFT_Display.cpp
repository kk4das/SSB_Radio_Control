#include "RadioControl.h"
#ifdef DISPLAY_TFT

//
// 7/31/202 - Dean Souleles 
//   Added stubbed out Split indicator function to stay consistent with main sketch
//   TO DO:
//       ADD constants and code for Split indicator
//

/* SSB_TFT_Display
 *  KK4DAS, Dean Souleles, KK4DAS@gmail.com
 *  May 30, 2020
 *  
 *  Basic radio display panel for a SSB transsceiver
 *  Designed for a 320x240 Color TFT (non touch)
 *  Tested with an ILI9341 display
 *  Requires the following libraries be installed:
 *    Adafruit_GFX
 *    Adafruit_ILI9341
 *  
 *  Implements a basic SSB display console with the following features
 *     Dual VFO A/B
 *     Mode indicator  SSB/LSB
 *     Tx/Rx ndicator
 *     TuningStep Inidicator
 *     S Meter
 *     Banner including Call sign
 *  
 *     Fully customizable. Fast display makins use of minimal resources./
 *     Room is left on the screen for additional features
 *     There is room on the screen for another row of features
 *     
 *     Easily change colors, font sizes and layout
 *          
 *  Default Screeen Layout
 *         A 7.200.000 LSB            -- VFO A/B indicator, Active VFO Freq, LSB/USB inidicator
 *        Rx 7.048.000 100K           -- Rx/Tx indicator, Alternate VFO Freq, Tuning Increment
 *        
 *        S |_|_|_|_|_|_|_|_|_|_|_|_| -- S Meter
 *           1   3   5   7   9
 *        
 *           AGC   SPL   RIT          -- (Planned) AGC on/of, Split On/Off,  RIT On/OFF
 *        
 *        Ver   Rig Name   Call
 * 
 * This module provides the following radio console dsiplayfunctions:
 *   displaySetup     - initialize the display and displays the startup values - call once from your setup function
 *   displayBanner    - Displays a text banner across the bottom of the screen
 *   displayActVFO    - Displays the frequency of the Active VFO
 *   displayAltVFO    - Displays the frequency of the Alternate VFO
 *   displayVFOAB     - Displays the indicator which VFO is active (A or B)
 *   displayTxRx      - Displays whether the rig is in (Tx or Rx)
 *   displayMode      - Displays the whiuch sideband is selected (LSB or USB) 
 *   displayIncr      - Displays the tuning increment (10, 100, 1K 10K, 100K, 1M)
 *   displaySMeter    - Displays the S Meter (1-9 are gray, +10 +20 and +30 are red
 * 
 * This module also provides the following general purpose displauy functions:
 *   displayClearScreen     - fills the screen with the selected backgrond color
 *   displayPrintat         - prints text or nubmers on the screen at a specific location
 *   displayDrawBoundingBox - draw a box on the screen and fills it with a background color
 *   displayDrawTextBox     - displays text inside a boundig box
 * 
 * Design notes and how to use the code
 * 
 * NOTE TO BUILDERS
 * This is not a complete radio control sketch. It is the Display software only. In the spirit of modular design 
 * it is stand-alone and not dependent on using an SI-5351 or any other specific hardware, or on my particular 
 * hardware selection of switches, buttons and knobs.  The demonstration sketch shows how to update the display,  
 * but you need to provide the code to determine what the actual values should be. You will likely need other 
 * libraries like the Si5351 and a Rotary encoder library aside from the GFX and the ILI9341. 
 * 
 * DESIGN PRINCIPLES
 * Good software design principles are to use as few hard-coded numbers as possible.  
 * Wherever possible I have used #defines  for any number that will be used more than one place in the code.  
 * For example  #define DSP_VFO_ACT_X 60 defines the X coordinate (how far from the left of the screen) 
 * of the Active VFO frequency display. You will see multiple references to DSP_VFO_ACT_X  throughout the code, 
 * but I never use the hardcoded number 60 again.  Change it once – and it is changed throughout.
 * 
 * Taking the S-Meter as an example:
 * 
 * To update the S-Meter display you make a call to displaySMeter(n); where n is an integer from 1 to 12 
 * (representing S1-9, +10, +20 +30).  Your sketch will need a way of monitoring signal strength (an analog input 
 * pin on the Arduino attached to an appropriate place on your rig) and converting it to the logarithmic S scale.
 * 
 * SCREEN COORDINATES
 * Coordinates work differently on displays than a typical graph where the origin 0,0 is in the middle abd positive
 * and negative values move you away from the origin. For displays  0,0, the origin, is always upper left hand corner 
 * of the display and you only use positive numbers for the coordinates  +X is pixels from the left edge, +Y is pixels 
 * down from the top. This particular example based on a 320x240 display but should be easily portable to other 
 * display sizes – but you have to keep in mind how the coordinate system works.
 * 
 * SCREEN LAYOUT
 * Here are a few notes about how the demonstration display is laid out.  This should help you understand the design 
 * concept and allow you to begin to modify it.  
 * 
 * The VFO display is setup for a dual VFO rig.  The currently Active VFO is always on the top and the alternate VFO 
 * is just below it.  Your code will need to keep track of whether VFO A or VFO B is currently selected and call the 
 * display routines to update the display.   I’ll describe how the VFO displays are are defined and that will give you 
 * an idea how you might modify or enhance the display.
 * 
 * Active VFO - top center of the screen
 * #define DSP_VFO_ACT_X 60   // Active VFO begins 60 pixels from the left hand edge (I picked 60 by experimenting)
 * #define DSP_VFO_ACT_Y 30   // Active VFO box starts 30 pixels down from the top of the screen (Try changing it to 50 
 *                            // and see what happens)
 * #define DSP_VFO_ACT_COLOR ILI9341_GREEN   // This sets the text color for the Frequency display 
 *                                           // use whatever colors you like
 * #define DSP_VFO_ACT_BK ILI9341_BLACK      // This sets the background color for the Active VFO
 * #define DSP_VFO_ACT_SZ 3                  // This is text size from Arduino TFT, values 1-5 1 is small 5 is large 
 *                                           // (2 was too small, 4 was too large, 3 was just right)
 *                                           
 * Alternate VFO – the second VFO is placed directly below the Active VFO on the screen.  There are a couple of things of  
 * interest here.  For the X coordinate, instead of putting in a hard coded number I refer back to the #define that I used 
 * for the Active VFO (DSP_VFO_ACT).  That way, if I want to move VFO section to a different part of the screen I only need 
 * to change one number DSP_VFO_ACT_X, and the alternate VFO will move as well.  Figuring out the Y coordinate for the 
 * alternate VFO is a little more challenging.  Some math is involved.  Starting with the Y coordinate of the Active VFO 
 * I need to calculate where how far down the display I need to go to place the second VFO.  To do that I need calculate 
 * how many pixels tall the text characters in the Active VFO are and use that as an offset.  It turns out we have everything 
 * we need already defined.  CH_W and CH_H are #defines that specify the height and width of a text character in pixels for 
 * TFT font size 1.  Size 2 through 5 are even multiples of that – so font height for size 2 is 2*CH_H pixels and font width 
 * for size 4 is 4*CH_W pixels and so on.  so we have everything we need to calculate how many pixels the Active VFO takes 
 * on the screen – we multiply the font size by the character height and add 16 pixels offset. The 16 was determined by 
 * experimentation for something that looked good.  The code looks like this:
 * 
 * #define DSP_VFO_ALT_X DSP_VFO_ACT_X
 * #define DSP_VFO_ALT_Y DSP_VFO_ACT_Y + (DSP_VFO_ACT_SZ * CH_H) + 16 
 * 
 * Take a look the other sections of the display code and you will see similar references and calculations.  The VFO A//B 
 * indicator and LSB/USB mode indicator, for example are similarly “pinned” to the Active VFO display, so if you move the 
 * Active VFO display to another screen location they will move also.
 *
 * In summary - each object is on the display is defined by a set of constants that indicate the X,Y coordinates
 * of the object on the screen and various other attributes like text size and color.  The basic user interface display 
 * object is a bounded/filed text box.  You can control the text size and color, and the box fill color.  With this basic 
 * set of features you can implement a wide variety of user interface elements. The S-meter, for example, is a row of
 * filled boxes.
 *    
 * HARDWARE NOTES
 * My test sketch uses an Arduino Nano. The display is an HiLetgo 2.2 Inch ILI9341 SPI TFT LCD Display 240x320 ILI9341, 
 * but any ILI9341 display should work. There are many sources.  Please note that the Arduino has 5V logic levels,
 * but the display requires 3.3V - so you need some sort of level shifter.  I used the"HiLetgo 10pcs 4 Channels IIC I2C 
 * Logic Level Converter Bi-Directional 3.3V-5V Shifter Module for Arduino"  I used hardware SPI and the pinouts are
 * standard as follows:
 *      
 *        Arduino 
 *         Pin       TFT Pin
 *      -----------|---------    
 *          8      |  RST  - any free Arduino Pin (not used in this sketch)
 *          9      |  DC   - any free Arduino Pin
 *          10     |  CS   - any free Arduino Pin 
 *          11     |  MOSI - fixed
 *          12     |  MISO - fixed
 *          13     |  CLK  - fixed
 *          
 * That is all the wiring you need for the demonstration sketch.
 * 
 * 
 * BUILDING THE DEMONSTRATION SKETCH
 * 
 * Create a folder called SSB_TFT_Display_Demo
 * 
 * Copy all three files to that folder
 *    SSB_TFT_Display_Demo.ino
 *    SSB_TFT_Display.h
 *    SSB_TFT_Display.cpp
 *
 * Use the Arduino IDE library manager to install teh following libraries
 *    Adafruit_GFX
 *    Adafruit_ILI9341
 *    
 * Compile and upload the sketch
 ******************************************************************************************************************       
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>


// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 8
#define TFT_MISO 12

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define CH_W 6  // default TFT Character width in pixels
#define CH_H 8  // default TFT Charachter height in pixels 



#define DSP_BG_COLOR ILI9341_NAVY

// Banner - Version, Rig Name, Call sign
#define DSP_BANNER_X 10
#define DSP_BANNER_Y 220
#define DSP_BANNER_COLOR ILI9341_BLACK
#define DSP_BANNER_BK ILI9341_WHITE
#define DSP_BANNER_SZ 2


// Active VFO - top center of the screen
#define DSP_VFO_ACT_X 60
#define DSP_VFO_ACT_Y 30
#define DSP_VFO_ACT_COLOR ILI9341_GREEN
#define DSP_VFO_ACT_BK ILI9341_BLACK
#define DSP_VFO_ACT_SZ 3

// Alternate VFO - directly below the Active VFO
#define DSP_VFO_ALT_X DSP_VFO_ACT_X
#define DSP_VFO_ALT_Y DSP_VFO_ACT_Y + (DSP_VFO_ACT_SZ * CH_H) + 16
#define DSP_VFO_ALT_COLOR ILI9341_WHITE
#define DSP_VFO_ALT_BK DSP_VFO_ACT_BK
#define DSP_VFO_ALT_SZ 3

//  VFP A/B indidcator - to the left of the Active VFO
#define DSP_VFO_AB_X DSP_VFO_ACT_X-40
#define DSP_VFO_AB_Y DSP_VFO_ACT_Y + 6
#define DSP_VFO_AB_COLOR DSP_VFO_ACT_COLOR
#define DSP_VFO_AB_BK DSP_VFO_ACT_BK
#define DSP_VFO_AB_SZ 2

//  Tx/Rx Indicator - to the left of the Alternate VFO
#define DSP_RX_TX_X DSP_VFO_ACT_X-45
#define DSP_RX_TX_Y DSP_VFO_ALT_Y + 4
#define DSP_RX_COLOR DSP_VFO_ACT_COLOR
#define DSP_TX_COLOR ILI9341_RED
#define DSP_RX_TX_BK DSP_VFO_ACT_BK
#define DSP_RX_TX_SZ 2

//  Mode (LSB/USB) inidcator - to the right of the Active VFO
#define DSP_MODE_X DSP_VFO_ACT_X+205
#define DSP_MODE_Y DSP_VFO_ACT_Y + 6
#define DSP_MODE_COLOR DSP_VFO_ACT_COLOR
#define DSP_MODE_BK DSP_VFO_ACT_BK
#define DSP_MODE_SZ 2

// Tuning Increment - to the right of teh Alternate VFO
#define DSP_INCR_X DSP_VFO_ACT_X+200
#define DSP_INCR_Y DSP_VFO_ALT_Y + 4
#define DSP_INCR_COLOR DSP_VFO_ACT_COLOR
#define DSP_INCR_BK DSP_VFO_ACT_BK
#define DSP_INCR_SZ 2

// S Meter - below the the Alt VFO (1/2 way down the screen)
#define DSP_S_METER_X 56
#define DSP_S_METER_Y 120
#define DSP_S_METER_SEGMENTS 12
#define DSP_S_METER_UNIT_SIZE 20
#define DSP_S_METER_LOW_COLOR ILI9341_DARKGREY  //below S9
#define DSP_S_METER_HI_COLOR ILI9341_RED        //above S9

// S Meter Label "S" to the left of the S Meter
#define DSP_S_LABEL_X DSP_S_METER_X - 18
#define DSP_S_LABEL_Y DSP_S_METER_Y + 4
#define DSP_S_LABEL_COLOR ILI9341_BLACK
#define DSP_S_LABEL_SZ 2

// S Meter Scale immediately belwo the S Meter
#define DSP_S_METER_SCALE_X DSP_S_METER_X + 5
#define DSP_S_METER_SCALE_Y DSP_S_METER_Y + DSP_S_METER_UNIT_SIZE + 5
#define DSP_S_METER_SCALE_COLOR DSP_S_LABEL_COLOR
#define DSP_S_METER_SCALE_SZ 2


///////////////////////////////////////////////////////////////////////////
// displayPrintln(s)
// Prints one line of text at a time on the display - for debug messages
///////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
int ln=0;
void displayPrintln(String s ) { 
  if (ln==14) {
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    ln=0;
  }
  tft.println(s);
  ln++;
}
#endif

///////////////////////////////////////////////////////////////////////////
// displayClearScreen()
// Fills the screen with selected bacground color
///////////////////////////////////////////////////////////////////////////
void displayClearScreen() {
  tft.fillScreen(DSP_BG_COLOR);
}

///////////////////////////////////////////////////////////////////////////
// displayPrintat String
// Prints a string on the display
// x,y: upper left pixel coordinates
// fontsize: Arduino Font size (1-5)
// color: text color
///////////////////////////////////////////////////////////////////////////
void displayPrintat(String s, int x, int y, int fontsize, int color) {
  tft.setCursor(x,y);
  tft.setTextSize(fontsize);
  tft.setTextColor(color);
  tft.print(s);
}

///////////////////////////////////////////////////////////////////////////
// displayPrintat Integer
// Prints a whole number on the display
// x,y: upper left pixel coordinates
// fontsize: Arduino Font size (1-5)
// color: text color
///////////////////////////////////////////////////////////////////////////
void displayPrintat(int i, int x, int y, int fontsize, int color) {
  tft.setCursor(x,y);
  tft.setTextSize(fontsize);
  tft.setTextColor(color);
  tft.print(i);
}

///////////////////////////////////////////////////////////////////////////
// displayDrawBoundingBox
//    Draw and fill a bounding box for text
//      Draw a recrtanglie slightly larger than the text 
//      Fill it with the color - leave the border white  
//    
//    len - length of the box in characxters
//    x - x coord 
//    y - y coord
//    fontsiez - font size - fontsize*CH_W is the width of a character, fontsize*CH_H is the height
//    fillcolor  - color
///////////////////////////////////////////////////////////////////////////
void displayDrawBoundingBox(int len, int x, int y, int fontsize, int fillcolor) {

   tft.drawRect(x-8, y-6, ((len*fontsize*CH_W))+16, ((fontsize*CH_H)+8), ILI9341_WHITE);
   tft.fillRect(x-6, y-4, ((len*fontsize*CH_W))+12, ((fontsize*CH_H)+4), fillcolor);
}

///////////////////////////////////////////////////////////////////////////
// displayDrawTextBox
//   Display text in an outlined and filled box
//   
//   s: text to display
//   x,y: upper left pixel coordinates
//   fontsize: ARduino font size (1-5)
//   fillcolor: color to fill the box with
///////////////////////////////////////////////////////////////////////////
void displayDrawTextBox(String s, int x, int y, int fontsize, int color, int fillcolor) {
  displayDrawBoundingBox(s.length(), x, y, fontsize, fillcolor);  // draw the box
  displayPrintat(s, x, y, fontsize, color);                       // display the text
}

///////////////////////////////////////////////////////////////////////////
// displaySMeter
// Display the S meter as a line of filled squares - up to the input level (1-12) S1-S0, +10dB, +20dB, +30dB
// S 1-9 are filled with the Low color S9+ is filled with the High color
///////////////////////////////////////////////////////////////////////////
void displaySMeter(byte level) {
  int i;
  int color;

  displayPrintat(F("S"), DSP_S_LABEL_X, DSP_S_LABEL_Y, DSP_S_LABEL_SZ, DSP_S_LABEL_COLOR);
  
  for (i=0; i<DSP_S_METER_SEGMENTS; i++) {
    color = ILI9341_BLACK;
    if (i<level) {
      color = DSP_S_METER_LOW_COLOR;
      if (i>8 && level>9) {
        color = DSP_S_METER_HI_COLOR;
      }
    }
    tft.drawRect(DSP_S_METER_X+(i*DSP_S_METER_UNIT_SIZE), DSP_S_METER_Y, DSP_S_METER_UNIT_SIZE, DSP_S_METER_UNIT_SIZE, ILI9341_WHITE);
    tft.fillRect(DSP_S_METER_X+(i*DSP_S_METER_UNIT_SIZE)+2, DSP_S_METER_Y+2, DSP_S_METER_UNIT_SIZE-4, DSP_S_METER_UNIT_SIZE-4, color);

    // Print scale odd numbers up to 9
    if ((i % 2) == 0 && i<9) {
      displayPrintat(i+1, DSP_S_METER_SCALE_X+(i*DSP_S_METER_UNIT_SIZE), DSP_S_METER_SCALE_Y, DSP_S_METER_SCALE_SZ, DSP_S_METER_SCALE_COLOR);
    }
  }
}

///////////////////////////////////////////////////////////////////////////
// displayBanner
// Displays a banner acroos the bottom of the display
///////////////////////////////////////////////////////////////////////////
void displayBanner(String s) {
  displayDrawTextBox(s,DSP_BANNER_X, DSP_BANNER_Y, DSP_BANNER_SZ, DSP_BANNER_COLOR, DSP_BANNER_BK);  
}


///////////////////////////////////////////////////////////////////////////
// displayActVFO(uint32_t freq)
// displayAltVFO(uint32_t freq
// Formats and displays Active and Alternat VFO frequencies
// Legal values:  Frequency in Hz
//
// To do - comibine into one function
///////////////////////////////////////////////////////////////////////////
void displayActVFO(uint32_t freq) {
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
  snprintf(f, sizeof(f),"%2ld.%ld%ld%ld.%ld%ld%ld",mil, hund_thou, ten_thou,thou, hund, tens, ones);
  displayDrawTextBox(f,DSP_VFO_ACT_X, DSP_VFO_ACT_Y, DSP_VFO_ACT_SZ, DSP_VFO_ACT_COLOR, DSP_VFO_ACT_BK); 
}
  
void displayAltVFO(uint32_t freq) {
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
  snprintf(f, sizeof(f),"%2ld.%ld%ld%ld.%ld%ld%ld",mil, hund_thou, ten_thou,thou, hund, tens, ones);
  displayDrawTextBox(f,DSP_VFO_ALT_X, DSP_VFO_ALT_Y, DSP_VFO_ALT_SZ, DSP_VFO_ALT_COLOR, DSP_VFO_ALT_BK);  
}

///////////////////////////////////////////////////////////////////////////
// displayVFOAB(int vfo)
// Displays which VFO is currently active A or B
// Legal values are VFOA and VFOB
///////////////////////////////////////////////////////////////////////////
void displayVFOAB(int vfo) {
  String vfo_str;
  if (vfo == VFOA) {
    vfo_str = F("A");
  } else {
    vfo_str = F("B");
  }
  displayDrawTextBox(vfo_str,DSP_VFO_AB_X, DSP_VFO_AB_Y, DSP_VFO_AB_SZ, DSP_VFO_AB_COLOR, DSP_VFO_AB_BK);
}

///////////////////////////////////////////////////////////////////////////
// displayTxRx(int tx_rx
// Displays whether the radio is currenlty transmitting or receiveing
// Legal values are TX and RX
///////////////////////////////////////////////////////////////////////////
void displayTxRx(int tx_rx) {
  String tx_rx_str;
  int color;
  if (tx_rx == RX) {
    tx_rx_str = F("Rx");
    color = DSP_RX_COLOR;
  } else {
    tx_rx_str = F("Tx");
    color = DSP_TX_COLOR;
  }
  displayDrawTextBox(tx_rx_str,DSP_RX_TX_X, DSP_RX_TX_Y, DSP_RX_TX_SZ, color, DSP_RX_TX_BK); 
}

//////////////////////////////////////////////////////////////////////////
// displayMode(int mode)
// Displays whether the radio is LSB or USB
// Legal values are LSB and USB
///////////////////////////////////////////////////////////////////////////
void displayMode(int mode) {
  String mode_str;
  if (mode == LSB) {
    mode_str = F("LSB");
  } else {
    mode_str = F("USB");
  }
  displayDrawTextBox(mode_str,DSP_MODE_X, DSP_MODE_Y, DSP_MODE_SZ, DSP_MODE_COLOR, DSP_MODE_BK);
}

///////////////////////////////////////////////////////////////////////////
// displayIncr(uint32_t increment)
// Changes display of the tuning incrementindicator
// Legal values in Hz are 1, 10, 100, 1000, 10000, 100000, 1000000
///////////////////////////////////////////////////////////////////////////
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
  displayDrawTextBox(hertz,DSP_INCR_X, DSP_INCR_Y, DSP_INCR_SZ, DSP_INCR_COLOR, DSP_INCR_BK);
}

///////////////////////////////////////////////////////////////////////////
// displaySplit(boolean split)
// Turns split mode indicator on/off
///////////////////////////////////////////////////////////////////////////
void displaySplit(boolean splt) {
  if (split) {
    //  todo
  } else {
    // todo
  }
}

///////////////////////////////////////////////////////////////////////////
// displaySplit(boolean split)
// Turns split mode indicator on/off
///////////////////////////////////////////////////////////////////////////
void displayTune(boolean on_off) {
// todo
}


///////////////////////////////////////////////////////////////////////////
// displaySetup()
// Initialze and populate the display
///////////////////////////////////////////////////////////////////////////
void displaySetup(String banner,
                  uint32_t vfoActfreq, uint32_t vfoAltfreq,
                  uint32_t activeVFO,
                  int tx_rx,
                  int sideband,
                  boolean split,
                  uint32_t increment,
                  byte s_meter) {
                     
  tft.begin();          // Initialize the TFT
  tft.setRotation(1);   // Set landscape orientation
  displayClearScreen(); // Fill teh screen with the background color

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

#endif
