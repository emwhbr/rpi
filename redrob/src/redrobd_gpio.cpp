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

#include "redrobd_gpio.h"
#include "rpi_gpio.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static rpi_gpio g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_gpio_initialize(void)
{
  g_object.initialize();
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_finalize(void)
{
  g_object.finalize();
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_set_function_inp(uint8_t pin,
				   uint8_t &old_func)
{
  // Get current GPIO function for pin
  old_func = (uint8_t)g_object.get_function(pin);

  // Set pin as input
  g_object.set_function(pin, RPI_GPIO_FUNC_INP);
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_set_function_out(uint8_t pin,
				   uint8_t &old_func)
{
  // Get current GPIO function for pin
  old_func = (uint8_t)g_object.get_function(pin);

  // Set pin as output
  g_object.set_function(pin, RPI_GPIO_FUNC_OUT);
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_set_function(uint8_t pin,
			       uint8_t func)
{
  g_object.set_function(pin, (RPI_GPIO_FUNCTION)func);
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_set_pin_high(uint8_t pin)
{
  g_object.set_pin_high(pin);
}

////////////////////////////////////////////////////////////////

void redrobd_gpio_set_pin_low(uint8_t pin)
{
  g_object.set_pin_low(pin);
}

////////////////////////////////////////////////////////////////

uint8_t redrobd_gpio_get_pin(uint8_t pin)
{
  return g_object.get_pin(pin);
}
