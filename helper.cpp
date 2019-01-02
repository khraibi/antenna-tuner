/*
 * Helper functions 
 *
 * This source file is part of the Magnetic Loop Antenna Tuner Arduino firmware
 * found under http://www.github.com/microfarad-de/antenna-tuner
 * 
 * Please visit:
 *   http://www.microfarad.de
 *   http://www.github.com/microfarad-de
 * 
 * Copyright (C) 2019 Karim Hraibi (khraibi at gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "helper.h"
#include <EEPROM.h>


#define BUTTON_LONG_PRESS_TIMEOUT (500)


/*#######################################################################################*/

void LedClass::initialize (uint8_t ledPin) {
  this->ledPin = ledPin;
  pinMode (ledPin, OUTPUT);
  digitalWrite (ledPin, LOW);
  powerOn = false;
  blinking = false;
  initialized = true;
}

void LedClass::loopHandler (void) {
  uint32_t ts;
  
  if (!initialized || !blinking ) return;

  ts = millis ();
  
  if ( (blinkOn && ts - blinkTs > tOn) || (!blinkOn && ts - blinkTs > tOff) ) {
    blinkOn = !blinkOn;
    digitalWrite (ledPin, blinkOn);
    blinkTs = ts;
    count--;
    if (count <= 0) {
      blinking = false;
      digitalWrite (ledPin, powerOn);
    }
  }
}

void LedClass::turnOn (void) {
  if (!initialized) return;
  powerOn = true;
  digitalWrite (ledPin, powerOn);
}

void LedClass::turnOff (void) {
  if (!initialized) return;
  powerOn = false;
  digitalWrite (ledPin, powerOn);
}

void LedClass::toggle (void) {
  if (!initialized) return;
  powerOn = !powerOn;
  digitalWrite (ledPin, powerOn);
}

void LedClass::blink (int32_t count, uint32_t tOn, uint32_t tOff) {
  if (!initialized || blinking || count <= 0) return;
  this->blinking = true;
  this->count = 2 * count;
  this->tOn = tOn;
  this->tOff = tOff;
  this->blinkOn = !powerOn;
  digitalWrite (ledPin, blinkOn);  
  blinkTs = millis ();
}

void LedClass::blinkBlocking (int32_t count, uint32_t tOn, uint32_t tOff) {
  blink (count, tOn, tOff);
  while (blinking) loopHandler ();
}

/*#######################################################################################*/

void ButtonClass::press (void) {
  wasPressed = pressed;
  pressed = true;
}

void ButtonClass::release (void) {
  wasPressed = pressed;
  pressed = false;
}

bool ButtonClass::rising (void) {
  bool rv = false;
  if (pressed && !wasPressed) {
    wasPressed = pressed;
    longPressTs = millis ();
    longPressed = true;
    wasLongPressed = false;
    rv = true;
  }
  return rv;
}

bool ButtonClass::falling (void) {
  bool rv = false;
  if (!pressed && wasPressed && !wasLongPressed) {
    wasPressed = pressed;
    rv = true;
  }
  return rv;  
}

bool ButtonClass::fallingLongPress (void) {
  bool rv = false;
  if (!pressed && wasPressed && wasLongPressed) {
    wasPressed = pressed;
    wasLongPressed = false;
    rv = true;
  }
  return rv;  
}

bool ButtonClass::fallingContinuous (void) {
  return !pressed && wasPressed; 
}

bool ButtonClass::longPress (void) {
  bool rv = false;
  if (pressed && longPressed && millis () - longPressTs > BUTTON_LONG_PRESS_TIMEOUT) {
    longPressed = false;
    wasLongPressed = true;
    rv = true;
  }
  return rv;
}

bool ButtonClass::longPressContinuous (void) {
  bool rv = false;
  if (pressed && millis () - longPressTs > BUTTON_LONG_PRESS_TIMEOUT) {
    longPressed = false;
    wasLongPressed = true;
    rv = true;
  }
  return rv;
}


/*#######################################################################################*/


void FirFilterClass::initialize (int16_t *memory, uint16_t size) {
  this->memory = memory;
  this->size = size;
  this->index = 0;
  this->initialized = true;
}

int16_t FirFilterClass::process (int16_t input) {
  int16_t i;
  int32_t output = 0;
  if (!initialized) return 0;
  
  memory[index] = input;
  index++;
  if (index >= size) index = 0;

  for (i = 0; i < size; i++) {
    output = output + memory[i];
  }

  return (int16_t)(output / size);
}

/*#######################################################################################*/


int16_t IirFilterClass::process (int16_t input, uint16_t size) {
  output = ((size - 1) * output + (int32_t)input) / size;
  return (int16_t)output;
}

/*#######################################################################################*/



void eepromWrite (uint16_t addr, uint8_t *buf, uint16_t bufSize) {
  uint8_t i, v;

  for (i = 0; i < bufSize && i < EEPROM.length (); i++) {
    v = EEPROM.read (addr + i);
    if ( buf[i] != v) EEPROM.write (addr + i, buf[i]);
  }
}



void eepromRead (uint16_t addr, uint8_t *buf, uint16_t bufSize) {
  uint8_t i;

  for (i = 0; i < bufSize; i++) {
    buf[i] = EEPROM.read (addr + i);
  }
}


/*#######################################################################################*/


int8_t sgn (int val) {
 if (val < 0) return -1;
 if (val == 0) return 0;
 return 1;
}
