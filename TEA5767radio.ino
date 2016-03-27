/*
 * TEA5767radio - Arduino TEA5767 single chip stereo FM receiver.  
 * 
 * Paul Johnson 27 Mar2016 v002
 */

#include <LiquidCrystal.h>
#include <math.h>
#include <EEPROM.h>
#include "TEA5767.h"
#include "DFR_Key.h"

/*
 * TEA5767 wiring
 * SCA to Arduino SDA
 * SDL to Arduino SDL
 * VCC to 5V
 * GND to GND
 * 
 * LCD wiring
 * R/W (pin 1) to GND
 * VCC (pin 2) to 5V
 * V0 (pin 3) to variable resistor (contrast)
 * RS (pin 4) to Arduino digital pin 
 * E  (pin 6) to Arduino digital pin 9
 * D4 (pin 11) to Arduino digital pin 4
 * D5 (pin 12) to Arduino digital pin 5
 * D6 (pin 13) to Arduino digital pin 6
 * D7 (pin 14) to Arduino digital pin 7
 *  
 */

// define radio modes
#define SIGNAL_MODE 1
#define MANUAL_MODE 2
#define SCAN_MODE 3
#define PRESET_MODE 4
#define PROGRAMME_MODE 5

int mode = SIGNAL_MODE;   // currently selected radio mode 
int preset = 1;           // currently selected preset
boolean scanning=false;   // is the radio scanning

// define key pad
DFR_Key keypad;
int keyPress = NO_KEY;  // currently selected key press


// LCD
LiquidCrystal lcd(8,9,4,5,6,7); // rs, enable, d4, d5, d6, d7

// TEA Radio
TEA5767 radio;

/*
 * Setup
 * 
 */
void setup() {
  
  // setup LCD display
  lcd.begin(16,2); // configure LCD with 16 columns 2 rows
  lcd.clear();    

  // setup key pad
  pinMode(A0, INPUT);  // ensure A0 pin is configured as an input
  keypad.setRate(10);  // set the keypad read rate to 10

  // setup serial 
  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  
  // setup radio
  radio.init(100.0);

  Serial.println("Starting...");
}

/*
 * * Update the top line LCD display
 */
void updateLcdTopLine() {
  // display FM frequency
  lcd.setCursor(0, 0);
  lcd.print("FM ");
  lcd.print( radio.getFrequencyAvailable());
  lcd.setCursor(8, 0);
  lcd.print(" ");

  // indicate mode - scanning, mono or stereo
  lcd.setCursor(10,0);
  if (!radio.ready()) {
    lcd.print("SCAN  ");    
  } else {
      if (radio.stereo()) {
        lcd.print("Stereo");
      } else {
        lcd.print("Mono  ");
      }
  }
}


/*
 * Update the bottom line LCD display
 */
void updateLcdBottonLine() {
  // int i;
  
  lcd.setCursor(0,1);
  switch (mode) {
    case SIGNAL_MODE : // volume
      lcd.print("Signal:"); 
      for (int i=0; i < (radio.getLevel() >> 1); i++) { // print level/2 '*'s
        lcd.print("*");  
      }
      for (int i=(7 + (radio.getLevel() >> 1)); i < 16; i++) { // print ' ' to end of lcd line
        lcd.print(" ");
      }
    break;

    case MANUAL_MODE : // Tune
      lcd.print("Manual tune     ");
    break;

    case SCAN_MODE : // scan
      lcd.print("Scan            ");
    break;

    case PRESET_MODE : // Select Preset
      lcd.print("Select preset: ");
      lcd.print(preset);
    break;

    case PROGRAMME_MODE : // Programme Preset
      lcd.print("Set preset: ");
      lcd.print(preset);
      lcd.print("   ");
    break;

  }
}

/*
 * Store the current frequency as a radio preset
 */
void setPreset()
{
  int eeAddress = preset * sizeof(double);
  EEPROM.put(eeAddress, radio.getFrequencyAvailable());
}

/*
 * Load a radio preset frequency
 */
void loadPreset()
{
  int eeAddress = preset * sizeof(double);
  double freq;
  
  EEPROM.get(eeAddress, freq);

  radio.setFrequency(freq);
}


/*
 * keyPressed
 */
void keyPressed() {
  if (keyPress == UP_KEY) {
    mode++;
    if (mode == 6) {
      mode = 1;
    }
  } else if (keyPress == DOWN_KEY) {
    mode--;
    if (mode == 0) {
      mode = 5;
    }
  } else if (keyPress == LEFT_KEY) {
    if (mode == SIGNAL_MODE ) { // signal mode
      // do nothing
    } else if (mode == MANUAL_MODE) { // manual tune mode
        radio.stepDown();
    } else if (mode == SCAN_MODE) { // scan mose
        radio.searchDown(); 
    } else if (mode == PRESET_MODE) { // preset mode
        preset--;
        if (preset == 0)
        {
          preset = 9;
        }
    } else if (mode == PROGRAMME_MODE) { // programme mode
        preset--;
        if (preset == 0)
        {
          preset = 9;
        }
    }
  } else if (keyPress == RIGHT_KEY) {
    if (mode == SIGNAL_MODE ) { // signal mode
      // do nothing
    } else if (mode == MANUAL_MODE) { // manual tune mode
      radio.stepUp();
    } else if (mode == SCAN_MODE) { // scan
        radio.searchUp();
    } else if (mode == PRESET_MODE) { // select preset
        preset++;
        if (preset == 10)
        {
          preset = 1;
        }
    } else if (mode == PROGRAMME_MODE) { // programme preset
        preset++;
        if (preset == 10)
        {
          preset = 1;
        }
    }
  } else if (keyPress == SELECT_KEY) {
    if (mode == PRESET_MODE) {
      // TO DO load preset
      loadPreset();
      mode = SIGNAL_MODE;
    } else if (mode == PROGRAMME_MODE) {
      // TO DO set preset
      setPreset();
      mode = SIGNAL_MODE;
    }
  }
}

void checkKeyPress() {

  keyPress = keypad.getKey();
}

void loop() {
  radio.read();
 
  checkKeyPress();

  if (keyPress > 0) {
    keyPressed();
    keyPress=NO_KEY;
  }

  updateLcdTopLine();
  updateLcdBottonLine();

  delay(100);
}

