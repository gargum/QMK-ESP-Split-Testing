#pragma once

#define HAL_USE_SIO TRUE
#define HAL_USE_USB TRUE
#define USB_USE_WAIT TRUE
#define HAL_USE_SERIAL_USB TRUE
#define HAL_USE_PAL TRUE
#define PAL_USE_CALLBACKS TRUE
#define PAL_USE_WAIT TRUE
#define HAL_USE_I2C TRUE
#define HAL_USE_SPI TRUE
#define SPI_USE_WAIT TRUE
#define SPI_SELECT_MODE SPI_SELECT_MODE_PAD
#define HAL_USE_PWM TRUE

#include_next <halconf.h>
