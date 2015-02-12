/*
    This code is for the bird houses sensing for Ecocampus.

    Circuit:
    ========
        * SD card attached to SPI bus as follows:
            ** MOSI - pin 11
            ** MISO - pin 12
            ** CLK - pin 13
            ** CS (== SS) - pin 10

        * Sensor attached to pin specified by sensor_pin.

    Usage:
    =======
        It will save timestamps at which the infrared beam was broke in a `data.csv` file on the SD card. The file is plaintext with one timestamp per line. As we do not have hardware clock, the timestamp is the number of seconds elapsed since the last reset of the ATmega.

        Debug is possible through Serial, if `debug` constant is set to `true`.


    Note:
    =====
        This code is in the public domain.
        Written by Phyks on 12/02/2015.
*/

#include <SPI.h>
#include <SD.h>

// If set to true, debug is possible on Serial
const bool debug = true;
// Note: this is Arduino's numbering, not Atmel datasheet's
const int sensor_pin = 13;

void log(String log) {
    Serial.println(log);
}

void setup() {
    if (debug) {
        Serial.begin(9600);
    }
    log("[INFO] Initializing SD card…");

    // Make sure that the default chip select pin is set to output
    pinMode(10, OUTPUT);
    log("[INFO] CS pin set as output.");

    pinMode(sensor_pin, INPUT_PULLUP);
    log("[INFO] Sensor pin set as input with pull-up resistor.");

    if (!SD.begin()) {
        log("[ERROR] Unable to initialize the SD card.");
        return;
    }
    log("[INFO] SD card ready to be used.");
}

void loop() {
    String dataString = "";

    // Read the sensor value
    int val = digitalRead(sensor_pin);
    if (val == LOW) {
        log("[INFO] IR beam sensor was triggered.");
        dataString = String((int) (millis() / 1000));
    }

    File dataFile = SD.open("data.csv", FILE_WRITE);

    if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();

        log("[INFO] Wrote data to data.csv: " + dataString);
    }
    else {
        log("[ERROR] Error opening data.csv file.");
    }

    delay(200);  // Wait for 200ms before doing a new measurement
    /* TODO:
        * Are 200ms ok ?
        * Use the watchdog to sleep during the 200ms and reduce power consumption.
    */
}
