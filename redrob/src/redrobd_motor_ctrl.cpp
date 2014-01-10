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

#include <sstream>
#include <iomanip>

#include "redrobd_motor_ctrl.h"
#include "redrobd_gpio.h"
#include "redrobd_log.h"

// Implementation notes:
// 1. Skid steering (differential drive, tank style).
//    Turn right : Left motor forward, right motor backward
//    Turn left  : Right motor forward, left motor backward
//
// 2. Motor driver hardware is L293D H-bridge using
//    bidirectional DC-motor control of right and left motor.
//
// 3. Assumes GPIO interface already initialized.
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_motor_ctrl::redrobd_motor_ctrl(uint8_t pin_right_motor_1,
				       uint8_t pin_right_motor_2,
				       uint8_t pin_left_motor_1,
				       uint8_t pin_left_motor_2)
{
  m_pin_rm_1 = pin_right_motor_1;
  m_pin_rm_2 = pin_right_motor_2;
  m_pin_lm_1 = pin_left_motor_1;
  m_pin_lm_2 = pin_left_motor_2;

  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_motor_ctrl::~redrobd_motor_ctrl(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::initialize(void)
{
  // Set all pins as outputs, save old pin functions
  redrobd_gpio_set_function_out(m_pin_rm_1, m_pin_func_rm_1);
  redrobd_gpio_set_function_out(m_pin_rm_2, m_pin_func_rm_2);
  redrobd_gpio_set_function_out(m_pin_lm_1, m_pin_func_lm_1);
  redrobd_gpio_set_function_out(m_pin_lm_2, m_pin_func_lm_2);

  // Stop all motors
  steer_stop();
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::finalize(void)
{
  // Stop all motors
  steer_stop();

  // Restore all pins
  redrobd_gpio_set_function(m_pin_rm_1, m_pin_func_rm_1);
  redrobd_gpio_set_function(m_pin_rm_2, m_pin_func_rm_2);
  redrobd_gpio_set_function(m_pin_lm_1, m_pin_func_lm_1);
  redrobd_gpio_set_function(m_pin_lm_2, m_pin_func_lm_2);
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

bool redrobd_motor_ctrl::check_steer_code(uint16_t code)
{
  bool steer_code_ok = false;

  switch (code) {
  case REDROBD_MC_NONE:
  case REDROBD_MC_STOP:
  case REDROBD_MC_FORWARD:
  case REDROBD_MC_REVERSE:
  case REDROBD_MC_RIGHT:
  case REDROBD_MC_LEFT:
    steer_code_ok = true;
    break;
  default:
    // All other steer codes are ignored for now
    ostringstream oss_msg;
    oss_msg << "Motor control : Got undefined steer code = 0x"
	    << hex << setw(4) << setfill('0') << (unsigned)code;
    redrobd_log_writeln(oss_msg.str());

    steer_code_ok = false;
  }

  return steer_code_ok;
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_none(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_stop(void)
{
  // Right motor - stop
  steer_motor(REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_DIR_STOP);

  // Left motor - stop
  steer_motor(REDROBD_MC_MOTOR_ID_LEFT,
	      REDROBD_MC_MOTOR_DIR_STOP);
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_forward(void)
{
  // Right motor - forward
  steer_motor(REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_DIR_FORWARD);

  // Left motor - forward
  steer_motor(REDROBD_MC_MOTOR_ID_LEFT,
	      REDROBD_MC_MOTOR_DIR_FORWARD);
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_reverse(void)
{
  // Right motor - reverse
  steer_motor(REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_DIR_REVERSE);

  // Left motor - reverse
  steer_motor(REDROBD_MC_MOTOR_ID_LEFT,
	      REDROBD_MC_MOTOR_DIR_REVERSE);
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_right(void)
{
  // Left motor - forward
  steer_motor(REDROBD_MC_MOTOR_ID_LEFT,
	      REDROBD_MC_MOTOR_DIR_FORWARD);

  // Right motor - reverse
  steer_motor(REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_DIR_REVERSE);
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_left(void)
{
  // Right motor - forward
  steer_motor(REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_DIR_FORWARD);

  // Left motor - reverse
  steer_motor(REDROBD_MC_MOTOR_ID_LEFT,
	      REDROBD_MC_MOTOR_DIR_REVERSE);
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::init_members(void)
{
  m_pin_func_rm_1 = 0;
  m_pin_func_rm_2 = 0;
  m_pin_func_lm_1 = 0;
  m_pin_func_lm_2 = 0;
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_motor(REDROBD_MC_MOTOR_ID motor_id,
				     REDROBD_MC_MOTOR_DIR motor_dir)
{
  uint8_t l293d_inp1 = 0;
  uint8_t l293d_inp2 = 0;

  switch (motor_id) {
  case REDROBD_MC_MOTOR_ID_RIGHT:
    l293d_inp1 = m_pin_rm_1;
    l293d_inp2 = m_pin_rm_2;
    break;
  case REDROBD_MC_MOTOR_ID_LEFT:
    l293d_inp1 = m_pin_lm_1;
    l293d_inp2 = m_pin_lm_2;
    break;
  }

  switch (motor_dir) {
  case REDROBD_MC_MOTOR_DIR_STOP:
    redrobd_gpio_set_pin_low(l293d_inp1);
    redrobd_gpio_set_pin_low(l293d_inp2);
    break;
  case REDROBD_MC_MOTOR_DIR_FORWARD:
    redrobd_gpio_set_pin_high(l293d_inp1);
    redrobd_gpio_set_pin_low(l293d_inp2);
    break;
  case REDROBD_MC_MOTOR_DIR_REVERSE:
    redrobd_gpio_set_pin_low(l293d_inp1);
    redrobd_gpio_set_pin_high(l293d_inp2);
    break;
  }
}
