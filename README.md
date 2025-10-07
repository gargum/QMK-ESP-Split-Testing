# QMK ESP Serial Bridge - Tests

This repo exists to test the viability of using ESP32 or ESP8266 microcontrollers to trick QMK into thinking a wireless connection between two halves of a split keyboard is in fact a standard wired UART connection.

Thus far, successful wirelesss communication between two ESP microcontrollers has been achieved and successful transmission of data from an RP2040 microcontroller running QMK to an ESP microcontroller have been achieved.

Currently, the goal is to polish this code enough for some wireless communication protocol available on an ESP32 or ESP8266 to successfully trick QMK into believing both keyboard halves are connected.

Demonstrations alongside explanations regarding issues like appropriate wiring will be added as they are discovered.
