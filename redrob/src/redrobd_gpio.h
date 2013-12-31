// ************************************************************************
// *                                                                      *
// * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __REDROBD_GPIO_H__
#define __REDROBD_GPIO_H__

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////
extern void redrobd_gpio_initialize(void);
extern void redrobd_gpio_finalize(void);

extern void redrobd_gpio_set_function_inp(uint8_t pin,
					  uint8_t &old_func);

extern void redrobd_gpio_set_function_out(uint8_t pin,
					  uint8_t &old_func);

extern void redrobd_gpio_set_function(uint8_t pin,
				      uint8_t func);

extern void redrobd_gpio_set_pin_high(uint8_t pin);
extern void redrobd_gpio_set_pin_low(uint8_t pin);

extern uint8_t redrobd_gpio_get_pin(uint8_t pin);

#endif // __REDROBD_GPIO_H__
