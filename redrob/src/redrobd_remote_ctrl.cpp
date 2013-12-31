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

#include "redrobd_remote_ctrl.h"
#include "redrobd_gpio.h"
#include "rpi_hw.h"

// Implementation notes:
// 1. Assumes GPIO interface already initialized.
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_remote_ctrl::redrobd_remote_ctrl(void)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_remote_ctrl::~redrobd_remote_ctrl(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_remote_ctrl::initialize(void)
{
  // Set all pins as inputs, save old pin functions
  redrobd_gpio_set_function_inp(PIN_RF_IN_0, m_pin_func_rf_in_0);
  redrobd_gpio_set_function_inp(PIN_RF_IN_1, m_pin_func_rf_in_1);
  redrobd_gpio_set_function_inp(PIN_RF_IN_2, m_pin_func_rf_in_2);
  redrobd_gpio_set_function_inp(PIN_RF_IN_3, m_pin_func_rf_in_3);
}

////////////////////////////////////////////////////////////////

void redrobd_remote_ctrl::finalize(void)
{
  // Restore all pins
  redrobd_gpio_set_function(PIN_RF_IN_0, m_pin_func_rf_in_0);
  redrobd_gpio_set_function(PIN_RF_IN_1, m_pin_func_rf_in_1);
  redrobd_gpio_set_function(PIN_RF_IN_2, m_pin_func_rf_in_2);
  redrobd_gpio_set_function(PIN_RF_IN_3, m_pin_func_rf_in_3);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_remote_ctrl::get_steering(void)
{
  uint16_t steering = REDROBD_RC_NONE;

  if ( redrobd_gpio_get_pin(PIN_RF_IN_0) ) {
    steering |= REDROBD_RC_FORWARD;
  }

  if ( redrobd_gpio_get_pin(PIN_RF_IN_1) ) {
    steering |= REDROBD_RC_REVERSE;
  }

  if ( redrobd_gpio_get_pin(PIN_RF_IN_2) ) {
    steering |= REDROBD_RC_RIGHT;
  }

  if ( redrobd_gpio_get_pin(PIN_RF_IN_3) ) {
    steering |= REDROBD_RC_LEFT;
  }

  return steering;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_remote_ctrl::init_members(void)
{
  m_pin_func_rf_in_0 = 0;
  m_pin_func_rf_in_1 = 0;
  m_pin_func_rf_in_2 = 0;
  m_pin_func_rf_in_3 = 0;
}
