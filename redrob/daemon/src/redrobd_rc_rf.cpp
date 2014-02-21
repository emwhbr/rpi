// ************************************************************************
// *                                                                      *
// * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include "redrobd_rc_rf.h"
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

redrobd_rc_rf::redrobd_rc_rf(uint8_t pin_forward,
			     uint8_t pin_reverse,
			     uint8_t pin_right,
			     uint8_t pin_left) : redrobd_remote_ctrl()
{
  m_pin_forward = pin_forward;
  m_pin_reverse = pin_reverse;
  m_pin_right   = pin_right;
  m_pin_left    = pin_left;

  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_rc_rf::~redrobd_rc_rf(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_rc_rf::initialize(void)
{
  // Not yet active
  set_active(false);

  // Set all pins as inputs, save old pin functions
  redrobd_gpio_set_function_inp(m_pin_forward, m_pin_func_forward);
  redrobd_gpio_set_function_inp(m_pin_reverse, m_pin_func_reverse);
  redrobd_gpio_set_function_inp(m_pin_right,   m_pin_func_right);
  redrobd_gpio_set_function_inp(m_pin_left,    m_pin_func_left);
}

////////////////////////////////////////////////////////////////

void redrobd_rc_rf::finalize(void)
{
  // Restore all pins
  redrobd_gpio_set_function(m_pin_forward, m_pin_func_forward);
  redrobd_gpio_set_function(m_pin_reverse, m_pin_func_reverse);
  redrobd_gpio_set_function(m_pin_right,   m_pin_func_right);
  redrobd_gpio_set_function(m_pin_left,    m_pin_func_left);

  // Not active anymore
  set_active(false);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_rc_rf::get_steering(void)
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

  // It requires at least one steering to be considered activated
  if ( (!is_active()) && (steering != REDROBD_RC_NONE) ) {
    set_active(true);
  }

  return steering;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_rc_rf::init_members(void)
{
  m_pin_func_forward = 0;
  m_pin_func_reverse = 0;
  m_pin_func_right   = 0;
  m_pin_func_left    = 0;
}
