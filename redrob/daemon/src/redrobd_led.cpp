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

#include <stdint.h>

#include "redrobd_led.h"
#include "redrobd_gpio.h"
#include "rpi_hw.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static uint8_t g_pin_func_sysfail;
static uint8_t g_pin_func_alive;
static uint8_t g_pin_func_bat_low;

static bool g_sysfail_active = false;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_led_initialize(void)
{
  // Note! Assumes GPIO already initialized

  // Set all LED pins as outputs, save old pin functions
  redrobd_gpio_set_function_out(PIN_SYSFAIL, g_pin_func_sysfail);
  redrobd_gpio_set_function_out(PIN_ALIVE,   g_pin_func_alive);
  redrobd_gpio_set_function_out(PIN_BAT_LOW, g_pin_func_bat_low);

  // Set intial state for all LEDs
  redrobd_led_sysfail(false);
  redrobd_led_alive(false);
  redrobd_led_bat_low(false);
}

////////////////////////////////////////////////////////////////

void redrobd_led_finalize(void)
{
  // Note! Assumes GPIO already initialized

  // Restore all LED pins
  // SYSFAIL pin is not restored if activated
  if (!g_sysfail_active) {
    redrobd_gpio_set_function(PIN_SYSFAIL, g_pin_func_sysfail);
  }
  redrobd_gpio_set_function(PIN_ALIVE,   g_pin_func_alive);
  redrobd_gpio_set_function(PIN_BAT_LOW, g_pin_func_bat_low);
}

////////////////////////////////////////////////////////////////

void redrobd_led_sysfail(bool activate)
{
  // Note! Assumes GPIO already initialized

  if (activate) {
    redrobd_gpio_set_pin_high(PIN_SYSFAIL);
    g_sysfail_active = true;
  }
  else {
    redrobd_gpio_set_pin_low(PIN_SYSFAIL);
    g_sysfail_active = false;
  }
}

////////////////////////////////////////////////////////////////

void redrobd_led_alive(bool activate)
{
  // Note! Assumes GPIO already initialized

  if (activate) {
    redrobd_gpio_set_pin_high(PIN_ALIVE);
  }
  else {
    redrobd_gpio_set_pin_low(PIN_ALIVE);
  }
}

////////////////////////////////////////////////////////////////

void redrobd_led_bat_low(bool activate)
{
  // Note! Assumes GPIO already initialized

  if (activate) {
    redrobd_gpio_set_pin_high(PIN_BAT_LOW);
  }
  else {
    redrobd_gpio_set_pin_low(PIN_BAT_LOW);
  }
}
