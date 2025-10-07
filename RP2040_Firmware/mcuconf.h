#pragma once

#include_next <mcuconf.h>

// for the serial connection between the two keyboard halves
    #undef RP_SERIAL_USE_USART0
    #define RP_SERIAL_USE_USART0 TRUE
