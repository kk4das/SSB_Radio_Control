   SSB_Radio_Control
   Dean Souleles, KK4DAS, contact kk4das@gmail.com
 
   This sketch implement s basic SSB radio control panel with the following features:
       Dual VFO A/B
       Rotary encoder for tuning, push button to change tuning increment
       SI5351 Clock generator for the LO and BFO
       CAT Control (emulates an ICOM IC-746)
       Split mode support (Split from CAT, manual split requres Nextion touch screen) 
       Settings memory (last frequency and mode are saved)
       Optional S-Meter

       Controls
         * Rotary encoder to change frequence
         * Rotary encode button changes tuning increment
         * VFO A/B Select toggle
         * Mode select USB/LSB toggle
         * Tune button (emits 10 second pulsed tone at 800Hz for tuning)
         * MOX toggle - puts rig in to Tx/Rx
         * Optional dual-band support 20/40

       Modules for different display types
         * 20x4 LCD
         * 320x240 TFT color display
         * 2.8" Nextion Touch Screen

       Display features
         * Dual VFOS
         * Mode indicator  SSB/LSB
         * Tx/Rx ndicator
         * TuningStep Inidicator
         * Optional S Meter
         * Banner including callsign

       Additional controls with the Nextion display
         * Continuous scanning
         * Split mode

   Version 1.4
   March 9 2021
       Rstored LCD Display Option
       Compile time selection of S-Meter and Dual Band mods
       Refactoring of encoder handling
       
   Version 1.3
   Jan 25, 2021
       Changed CAT module to IC-746

   Version 1.2
   Dec 14, 2020
       Dual Band 20/40 Support
       S-Meter

   Version 1.1
   Dec 13. 2020
       Ported to Nano Every for more sweet SRAM
 *        * Updated interrupt handling in Encoder.cpp to work with Nano Every (as well as UNO/Nano)
 *        * Replaced SoftwareSerial connections to Nextion with Hardware Serial - Serial1

   Version 1.0
   Aug 13, 2020
   Adapted from SimpleSSB Sketch by N6QW, Pete Juliano and others


   NOTE TO BUILDERS
   This is a reference implementation of an SSB radio control program for a homebrew SSB transceiver.
   It is a fully functioning SSB radio control program. While it has been built for my particular hardware
   configuration every attempt has been made to make it modular in design so that the builder can swap out
   modules at will.  It should be fairly straight forward to adapt to the builder's hardware selection
   of switches, buttons and knobs or even alternate displays.
