/*
 * TEA5767.h - Arduino library to support the TEA5767 single chip stereo FM receiver.  
 * 
 * This library works with the TES5767 chip in I2C mode. 
 * BUSMODE pin should be connected to the ground to operate in I2C mode. 
 * NB The IC address is 11000001
 * Fxtal is 32.768kHz
 * 
 * Paul Johnson 27Mar2016 v002
 */

#ifndef TEA5767_h
#define TEA5767_h

#include <Arduino.h>

class TEA5767
{
  public:
    TEA5767();
    void init(double freq);
    void setFrequency(double freq);
    void setSearchModeUp();
    void setSearchModeDown();
    void setMute();
    void unsetMute();
    void setSearchMode();
    void unsetSearchMode();  
    void searchUp();
    void searchDown();
    void stepDown();    
    void stepUp();        
    void write();
    void read();
    double getFrequencyAvailable();
    byte getLevel();
    byte ready();
    byte bandLimit();
    byte stereo();
    byte getIfCounterResult(); 

  private:
    byte _wdata1 = 0;        // write data byte 1 (frequency high)
    byte _wdata2 = 0;        // write data byte 2 (frequency low)
    byte _wdata3 = 0xF0;     // write data byte 3 (xF0 = search up, search stop level 11 (high), HiLo side injection)
    byte _wdata4 = 0x10;     // write data byte 4 (x10 = Fxtal 32.768kHz)
    byte _wdata5 = 0x00;     // write data byte 5
    byte _buffer[5];         // buffer to read data from the TEA5767

    double _lowFreq=87.5;
    double _highFreq=108.0;

    double _curFreq = 100.0; // currently selected frequency
    byte _readyFlag;         // TEA5767 ready flag
    byte _stereoFlag;         // TEA5767 stereo mono flag
    byte _level;             // TEA5767 signal level
    byte _bandLimitFlag;     // TEA5767 reached band limit flag
    
};

#endif
