# QMK Firmware - Notes and Considerations

## Microcontrollers
This firmware is targeting QMK running on the RP2040 microcontroller, and all testing is performed on this platform.

Only if/when this is polished will firmware be added or modified to accomodate other QMK-compatible microcontrollers.

## Split Transport
Currently, the split transport method being targeted is full duplex USART using the SIO driver.

This can be defined explicitly in the rules.mk file.

`SERIAL_DRIVER = usart`

If one is using the SIO serial driver on HAL/ChibiOS, one should also enable the SIO subsystem in the halconf.h file.

```
// halfconf.h
#pragma once

#define HAL_USE_SIO TRUE

#include_next <halconf.h>
```

## Baud rates
The ESP32 and ESP8266 firmware is designed to accommodate USART communication at a baud rate of 115200.
This means the soft serial speed should be set to set appropriately in the config.h file.

`#define SELECT_SOFT_SERIAL_SPEED 2`
