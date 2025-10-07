/*
* This file exists to configure the RP2040.
* It is also used to turn on features such as increased USB power consumption, and the use of PIO to connect the two halves of the Iyada.
*/

#pragma once

#include_next <mcuconf.h>

// for the bootloader reset behaviour configuration
    #define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
    #define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
    #define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP17

// for the serial connection between the two keyboard halves
    #undef RP_SERIAL_USE_USART0
    #define RP_SERIAL_USE_USART0 TRUE
    #undef SERIAL_USART_TX_PIN
    #define SERIAL_USART_TX_PIN GP0
    #undef SERIAL_USART_RX_PIN
    #define SERIAL_USART_RX_PIN GP1
    #undef SERIAL_USART_FULL_DUPLEX
    #define SERIAL_USART_FULL_DUPLEX TRUE
    #undef SERIAL_USART_TIMEOUT
    #define SERIAL_USART_TIMEOUT 20
    #undef USB_SUSPEND_WAKEUP_DELAY
    #define USB_SUSPEND_WAKEUP_DELAY 2000
