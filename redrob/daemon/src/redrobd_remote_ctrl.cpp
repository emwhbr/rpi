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

redrobd_remote_ctrl::redrobd_remote_ctrl(uint8_t pin_forward,
					 uint8_t pin_reverse,
					 uint8_t pin_right,
					 uint8_t pin_left)
{
  m_pin_forward = pin_forward;
  m_pin_reverse = pin_reverse;
  m_pin_right   = pin_right;
  m_pin_left    = pin_left;

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
  redrobd_gpio_set_function_inp(m_pin_forward, m_pin_func_forward);
  redrobd_gpio_set_function_inp(m_pin_reverse, m_pin_func_reverse);
  redrobd_gpio_set_function_inp(m_pin_right,   m_pin_func_right);
  redrobd_gpio_set_function_inp(m_pin_left,    m_pin_func_left);
}

////////////////////////////////////////////////////////////////

void redrobd_remote_ctrl::finalize(void)
{
  // Restore all pins
  redrobd_gpio_set_function(m_pin_forward, m_pin_func_forward);
  redrobd_gpio_set_function(m_pin_reverse, m_pin_func_reverse);
  redrobd_gpio_set_function(m_pin_right,   m_pin_func_right);
  redrobd_gpio_set_function(m_pin_left,    m_pin_func_left);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_remote_ctrl::get_steering(void)
{
  uint16_t steering = REDROBD_RC_NONE;

  if ( redrobd_gpio_get_pin(m_pin_forward) ) {
    steering |= REDROBD_RC_FORWARD;
  }

  if ( redrobd_gpio_get_pin(m_pin_reverse) ) {
    steering |= REDROBD_RC_REVERSE;
  }

  if ( redrobd_gpio_get_pin(m_pin_right) ) {
    steering |= REDROBD_RC_RIGHT;
  }

  if ( redrobd_gpio_get_pin(m_pin_left) ) {
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
  m_pin_func_forward = 0;
  m_pin_func_reverse = 0;
  m_pin_func_right   = 0;
  m_pin_func_left    = 0;
}
