/*
    This code is for the bird houses sensing for Ecocampus.

    Circuit:
    ========
        * SD card attached to SPI bus as follows:
            ** MOSI - pin 11
            ** MISO - pin 12
            ** CLK - pin 13
            ** CS (== SS) - pin 10

        * Sensor attached to pin specified by sensor_pin (A5 == PC5 == pin 28 on ATmega layout)

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
#include <avr/sleep.h>
#include <avr/power.h>

// If set to true, debug is possible on Serial
const bool debug = true;
// Note: this is Arduino's numbering, not Atmel datasheet's
const int sensor_pin = A4;
int old_val = HIGH;
char filename[50];

void log(String log) {
    Serial.println(log);
}

// Sleep functions
void setup_watchdog(uint8_t prescalar) {
    prescalar = min(9, prescalar);
    uint8_t wdtcsr = prescalar & 7;
    if (prescalar & 8) {
        wdtcsr |= _BV(WDP3);
    }
    MCUSR &= ~_BV(WDRF);
    WDTCSR = _BV(WDCE) | _BV(WDE);
    WDTCSR = _BV(WDCE) | wdtcsr | _BV(WDIE);
}

ISR(WDT_vect) {
}

void do_sleep(void) {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
    sleep_enable();
    sleep_mode(); // System sleeps here
    sleep_disable(); // System continues execution here when watchdog timed out
}

typedef enum { wdt_16ms = 0, wdt_32ms, wdt_64ms, wdt_128ms, wdt_250ms, wdt_500ms, wdt_1s, wdt_2s, wdt_4s, wdt_8s } wdt_prescalar_e;


void setup() {
    if (debug) {
        Serial.begin(9600);
    }
    log("[INFO] Initializing SD card…");

    // Make sure that the default chip select pin is set to output
    pinMode(8, OUTPUT);
    pinMode(10, OUTPUT);
    log("[INFO] CS pin set as output.");

    pinMode(sensor_pin, INPUT_PULLUP);
    log("[INFO] Sensor pin set as input with pull-up resistor.");

    //setup_watchdog(wtd_250ms);
    //log("[INFO] Set up sleep mode to 250ms.");

    if (!SD.begin(10)) {
        log("[ERROR] Unable to initialize the SD card (ptet que t'as oublié de mettre la carte sd ?).");
        return;
    }
    log("[INFO] SD card ready to be used.");

    int i = 0;
    sprintf(filename, "data%d.csv", 0);
    for (i = 0; SD.exists(filename); ++i) {
        sprintf(filename, "data%d.csv", i);
    }
}

void loop() {
    String dataString = "";

    // Read the sensor value
    int val = digitalRead(sensor_pin);
    if (val == LOW && old_val == HIGH) {
        log("[INFO] IR beam sensor was triggered.");
        dataString = String((int) (millis() / 1000));

        File dataFile = SD.open(filename, FILE_WRITE);

        if (dataFile) {
            dataFile.println(dataString);
            dataFile.close();

            String to_log = "[INFO] Wrote data to ";
            to_log += filename;
            to_log += dataString;
            log(to_log);
        }
        else {
            String to_log = "[ERROR] Error opening ";
            to_log += filename;
            to_log += " file.";
            log(to_log);
        }
    }
    old_val = val;
    digitalWrite(8, val);

    delay(300);

    // delay(100);  // 100 ms for the MCU to settle
    // do_sleep();
    // delay(100);  // 100 ms for the MCU to settle

}
