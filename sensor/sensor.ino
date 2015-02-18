/*
    This code is for the bird houses sensing for Ecocampus.

    Circuit:
    ========
        * SD card attached to SPI bus as follows:
            ** MOSI - pin 11
            ** MISO - pin 12
            ** CLK - pin 13
            ** CS (== SS) - pin 10

        * Sensor output plug to pin specified by sensor_pin (A4 == PC4 == pin 27 on ATmega layout)
        
        * Sensor power plug to pin specified by sensor_power_pin (7 == PD7 == pin 13 on ATmega layout)

    Usage:
    =======
        It will save timestamps at which the infrared beam was broke in a `data.csv` file on the SD card.
        The file is plaintext with one timestamp per line. As we do not have hardware clock, the timestamp
        is the number of seconds elapsed since the last reset of the ATmega.

        Debug is possible through Serial, if `debug` constant is set to `true`.


    Note:
    =====
        This code is in the public domain.
        Written by hackEns on 12/02/2015.
*/

#include <SPI.h>
#include <SD.h>
#include <avr/sleep.h>
#include <avr/power.h>

// If set to true, debug is possible on Serial
const bool debug = true;

// Note: this is Arduino's numbering, not Atmel datasheet's
const int sensor_pin = A4; // Sensor output
const int sensor_power_pin = 7; // Sensor power
const int cs_pin = 10; // SPI CS pin

int old_val = HIGH;
char filename[50];

void log(String msg) {
  Serial.println(msg);
}

void setup() {
  if (debug) {
      Serial.begin(9600);
  }
  log("[INFO] Initializing SD card…");

  pinMode(sensor_power_pin, OUTPUT);
  log("[INFO] Sensor power pin (" + String(sensor_power_pin) + ") set as output.");
  
  pinMode(sensor_pin, INPUT_PULLUP);
  log("[INFO] Sensor pin (" + String(sensor_pin) + ") set as input with pull-up resistor.");

  pinMode(cs_pin, OUTPUT);
  log("[INFO] CS pin (" + String(cs_pin) + ") set as output.");

  if (!SD.begin(cs_pin)) {
      log("[ERROR] Unable to initialize the SD card (Did you plug the SD card?).");
      return;
  }
  log("[INFO] SD card ready to be used.");
  
  // Select first available (i.e. non existent) dataXX.csv file
  int i = 0;
  sprintf(filename, "data%d.csv", 0);
  for (i = 0; SD.exists(filename); ++i) {
      sprintf(filename, "data%d.csv", i);
  }
}

void loop() {
  int timestamp;

  digitalWrite(sensor_power_pin, HIGH); // Power the sensor on
  delay(10); // Let the sensor "boot"

  int val = digitalRead(sensor_pin); // Read sensor value
  
  if (val == LOW && old_val == HIGH) {
    timestamp = (int) (millis() / 1000);
    log("[INFO] IR beam sensor was triggered.");

    File dataFile = SD.open(filename, FILE_WRITE);

    if (dataFile) {
      dataFile.println(String(timestamp));
      dataFile.close();

      log("[INFO] Wrote data to " + String(filename) + ": " + String(timestamp));
    }
    else {
      log("[ERROR] Error opening file " + String(filename));
    }
  }
  
  old_val = val;
  
  digitalWrite(sensor_power_pin, LOW); // Power the sensor off

  delay(300);  // 100 ms for the MCU to settle
}
