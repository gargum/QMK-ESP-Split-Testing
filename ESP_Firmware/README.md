# ESP Firmware - Notes and Considerations

## Wireless Protocols
This firmware has been designed to target ESP-NOW, as the majority of commonly used ESP32 and ESP8266 microcontrollers have access to it.

If/when this functions as intended, other protocols like 2.4Ghz wireless or Bluetooth Serial may be considered.

## ESP Boards
This firmware is being tested on the ESP32-C3, the ESP32-WROOM-32D, and the ESP-12F

Variants with the capacity to behave as USB hosts like the ESP32-S3 are not being tested to eliminate this feature as a potential point of failure.

## Wiring
The basic wiring is as follows:

| QMK MCU - 1 | Wired Protocol | ESP - 1 | Wireless Protocol | ESP - 2 | Wired Protocol | QMK MCU - 2 |
| ----------- | ----------- | ----------- | ----------- | ----------- | ----------- | ----------- |
| TX | UART | RX | ESP-NOW | TX | UART | RX |
| RX | UART | TX | ESP-NOW | RX | UART | TX |

In addition, it is necessary to add this circuit comprised of two 1N5819 Schottky diodes and two 1K resistors to the TX lines of your ESP microcontrollers.

[<img src="https://user-images.githubusercontent.com/6020549/74654209-28084e80-51cd-11ea-8af5-d0c03551a484.jpg">](https://github.com/nopnop2002/ESP8266-UART-to-UART-Bridge/)
