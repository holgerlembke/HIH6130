/*
  HIH6130.cpp - Library for reading RHT data from the Honeywell HIH6130.
  Created by David H Hagan, November 22, 2014.

  Use as you like. MIT license.

  hl:
    - removed the wire.begin(), shall be done in setup() due to different calls on different platforms
    - made begin() bool, check for sensor communication
    - made readRHT() bool to reflect succsessful reads
    - removed a lock situation in readRHT() if bus errors occure by adding a timer  
    - made status public
    - fixed data request concept (need to wait until measurement is done)

    Fun fact: Honeywell distributes "Honeywell HumidIcon ™ Digital Humidity/Temperature Sensors
                   HIH9000 Series ±1.7 %RH Accuracy | ±0.3 °C Temperature Accuracy"
                  (filename: honeywell-sensing-humidicon-hih9000-series-product-sheet-009076-7-en2.pdf)
                  with a wrong calculation formula (2^14 - 2) instead of (2^14 - 1)
*/

#include <Arduino.h>
#include "HIH6130.h"
#include <Wire.h>

HIH6130::HIH6130(uint8_t address)
{
  _address = address;
  _humidity_lo = 0;
  _humidity_hi = 0;
  _temp_hi = 0;
  _temp_lo = 0;
  status = 0;
}

bool HIH6130::begin() {
  Wire.beginTransmission(_address);
  return Wire.endTransmission() == 0;
}

bool HIH6130::readRHT() {
  // reads data from the sensor and stores them in temporary variables that
  // are then accessed via public variables
  Wire.beginTransmission(_address);
  int res = Wire.endTransmission();

  if (res == 0) {
    delay(37); // active wait for measurement been done hih-6130 doc states 36.65 ms typical

    Wire.requestFrom( (int) _address, (int) 4);

    unsigned long ticker = millis();
    while ( (Wire.available() != 4) && (millis() - ticker < 1000) ) ;

    if (Wire.available() == 4) {
      _humidity_hi = Wire.read();
      _humidity_lo = Wire.read();
      _temp_hi = Wire.read();
      _temp_lo = Wire.read();

      Wire.endTransmission();

      // Get the status (first two bits of _humidity_hi_)
      status = (_humidity_hi >> 6); // should give 0 to indicate read after measurement

      // Calculate Relative Humidity
      humidity = (double)(((unsigned int) (_humidity_hi & 0x3f) << 8) | _humidity_lo)
                 * 100.0 / ((1 << 14) - 1.0);

      // Calculate Temperature
      temperature = (double) (((unsigned int) (_temp_hi << 6) + (_temp_lo >> 2))
                              / ((1 << 14) - 1.0) * 165.0 - 40.0);
      return true;
    }

    return false;
  }

  return false;

}





