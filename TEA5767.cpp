/*
 * TEA5767.cpp - Arduino library to support the TEA5767 single chip stereo FM receiver.  
 * 
 * This library works with the TES5767 chip in I2C mode. 
 * BUSMODE pin should be connected to the ground to operate in I2C mode. 
 * NB The IC address is 11000001
 * 
 * Paul Johnson 27Mar2016 v002
 */
 
#include <Wire.h>

#include "Arduino.h"
#include "TEA5767.h"

/* Constructor
    NB does nothing
*/
TEA5767::TEA5767() {
  
}


/* Initialise the TEA5767
 * Must be called first before using any other functions
 */
void TEA5767::init(double freq) {
  Wire.begin();
  setFrequency(freq);
}

/* Set the Frequency data bytes
 *  - freq is the frequency in MHz e.g 98.50
 */
void TEA5767::setFrequency(double freq) {

  unsigned int frequencyB;
  byte frequencyH=0;     // TEA5767 data byte 1
  byte frequencyL=0;     // TEA5767 data byte 2
  byte preserveMuteAndSearchMode;

  if ( (freq >= _lowFreq) && (freq <= _highFreq) ) {

    // calculate PPL word for high side injection
    // PPL = 4 * freq(Hz) + 225kHz / clock(Hz)
    frequencyB=round( ( 4*( (freq*1000000)+225000) ) /32768); //calculating PLL word

    frequencyH=frequencyB>>8;
    frequencyL=frequencyB&0XFF;

    preserveMuteAndSearchMode = _wdata1 & B11000000; // preserve the mute and search mode bits in _wdata1

    _wdata1 = frequencyH | preserveMuteAndSearchMode;
    _wdata2 = frequencyL;

    write();
  }
}

/* Set the search mode to scan up
 */
void TEA5767::setSearchModeUp() {
  _wdata3 = _wdata3 | B10000000;
  write();
}

/* Set the search mode to scan down
 * Follow with a call to write() to write the updated configuration to the TEA5767
 */
void TEA5767::setSearchModeDown() {
  _wdata3 = _wdata3 & B01111111;
  write();
}

/* Set mute
 */
void TEA5767::setMute() {
  _wdata1 = _wdata1 | B10000000;
  write();
}

/* Unset mute
 */
void TEA5767::unsetMute() {
  _wdata1 = _wdata1 & B01111111;
  write();
}

/* Initiate searchmode 
 */
void TEA5767::setSearchMode() {
  _wdata1 = _wdata1 | B01000000;
  write();
}

/* Unset searchmode 
 */
void TEA5767::unsetSearchMode() {
  _wdata1 = _wdata1 & B10111111;
  write();
}

/* step down 0.1Hz
 */
void TEA5767::stepDown() {
  _curFreq-=0.1;
  if (_curFreq < _lowFreq) {
    _curFreq = _highFreq;
  }
  setFrequency(_curFreq);
}

/* step up 0.1Hz
 */
void TEA5767::stepUp() {
  _curFreq+=0.1;
  if (_curFreq > _highFreq) {
    _curFreq = _lowFreq;
  }
  setFrequency(_curFreq);
}


/* Start searching up
 */
void TEA5767::searchUp() {
  read();
  setFrequency(getFrequencyAvailable()+0.1);
  setMute();
  setSearchModeUp();
  setSearchMode();
 
  delay(10);
  read();

  while (!ready()) {
    delay(10);
    read();
  }
  
  if (bandLimit()) {
    setFrequency(_lowFreq);
  } 

  unsetMute();
}

/* Start searching down
 */
void TEA5767::searchDown() {
  read();
  setFrequency(getFrequencyAvailable()-0.1);
  setMute();
  setSearchModeDown();
  setSearchMode();

  read();
  while (!ready()) {
    delay(10);
    read();
  }
  
  if (bandLimit()) {
    setFrequency(_highFreq);
  } 
  unsetMute();
}

/* Returns the last read frequency in MHz e.g. 98.50
 */
double TEA5767::getFrequencyAvailable() {
  return _curFreq;
}
/* Returns the last read volume level (0 to 16)
 */
byte TEA5767::getLevel() {
  return _level;
}

/* Returns the last read ready status
 */
byte TEA5767::ready() {
  return _readyFlag;
}

/* Returns the last read ready status
 */
byte TEA5767::bandLimit() {
  return _bandLimitFlag;
}

/* Returns the last read ready status
 */
byte TEA5767::stereo() {
  return _stereoFlag;
}

/* Returns the last read ready status
 */
byte TEA5767::getIfCounterResult() {
  return  _buffer[2] & B01111111;
}

/* Write data bytes to TEA5767
 */
void TEA5767::write() {

  Wire.beginTransmission(0x60);   //writing TEA5767

  Wire.write(_wdata1);
  Wire.write(_wdata2);
  Wire.write(_wdata3);
  Wire.write(_wdata4);
  Wire.write(_wdata5);
  Wire.endTransmission();

}

/* Read the TEA5767 status
 */
void TEA5767::read() {
  double freqAvailable;
  
  Wire.requestFrom(0x60,5); //reading TEA5767

  if (Wire.available()) 
  {
    for (int i=0; i<5; i++) {
      _buffer[i]= Wire.read();
    }
  }

  freqAvailable = (((_buffer[0]&0x3F)<<8)+_buffer[1])*32768/4-225000;
  _curFreq = (floor((freqAvailable/100000) + 0.5))/10; // TEA5767 current tuned frequency
  _readyFlag = _buffer[0] & B10000000;        // TEA5767 ready flag
  _stereoFlag = _buffer[2] & B10000000;       // TEA5767 stereo mono flag
  _level = (_buffer[3]>>4);                   // TEA5767 signal level
  _bandLimitFlag = _buffer[0] & B01000000; // TEA5767 reached band limit flag
}
