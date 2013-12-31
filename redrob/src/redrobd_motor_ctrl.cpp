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
#include "rpi_hw.h"

// Implementation notes:
// 1. Skid steering (differential drive, tank style).
//    Turn right : Left motor forward, right motor backward
//    Turn left  : Right motor forward, left motor backward
//
// 2. Right motor is controlled by L293D-1A,2A
//    Left motor is controlled by  L293D-3A,4A
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

redrobd_motor_ctrl::redrobd_motor_ctrl(void)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_motor_ctrl::~redrobd_motor_ctrl(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::initialize(bool continuous_steer)
{
  // Set all pins as outputs, save old pin functions
  redrobd_gpio_set_function_out(PIN_L293D_1A, m_pin_func_l293d_1a);
  redrobd_gpio_set_function_out(PIN_L293D_2A, m_pin_func_l293d_2a);
  redrobd_gpio_set_function_out(PIN_L293D_3A, m_pin_func_l293d_3a);
  redrobd_gpio_set_function_out(PIN_L293D_4A, m_pin_func_l293d_4a);

  // Set steer mode
  m_continuous_steer = continuous_steer;
  m_previous_code = REDROBD_MC_NONE;
  m_last_steering = REDROBD_MC_STOP;

  // Stop all motors
  steer_stop();
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::finalize(void)
{
  // Stop all motors
  steer_stop();

  // Restore all pins
  redrobd_gpio_set_function(PIN_L293D_1A, m_pin_func_l293d_1a);
  redrobd_gpio_set_function(PIN_L293D_2A, m_pin_func_l293d_2a);
  redrobd_gpio_set_function(PIN_L293D_3A, m_pin_func_l293d_3a);
  redrobd_gpio_set_function(PIN_L293D_4A, m_pin_func_l293d_4a);
}

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer(uint16_t code)
{
  if (m_continuous_steer) {
    // Continuous mode requires a flow of steer codes
    // No steering equals stop
    if (code == REDROBD_MC_NONE) {
      code = REDROBD_MC_STOP;
    }
  }
  else {
    // Non-continuous mode remembers last steering
    // No steering equals continue with last steering
    // Stop is achieved by generating same steer code
    // after a period of no steering
    if ( (code != REDROBD_MC_NONE) && 
	 (code == m_last_steering) &&
	 (m_previous_code == REDROBD_MC_NONE) ) {
      code = REDROBD_MC_STOP;
    }
    else {
      if (code != REDROBD_MC_NONE) {
	m_last_steering = code;
      }
      m_previous_code = code;
    }
  }

  switch (code) {
  case REDROBD_MC_NONE:
    steer_none();
    break;
  case REDROBD_MC_STOP:
    steer_stop();
    break;
  case REDROBD_MC_FORWARD:
    steer_forward();
    break;
  case REDROBD_MC_REVERSE:
    steer_reverse();
    break; 
  case REDROBD_MC_RIGHT:
    steer_right();
    break;
  case REDROBD_MC_LEFT:
    steer_left();
    break;
  default:
    // All other steer codes are ignored for now
    ostringstream oss_msg;
    oss_msg << "Motor control : Got undefined steer code = 0x"
	    << hex << setw(4) << setfill('0') << (unsigned)code;
    redrobd_log_writeln(oss_msg.str());

    m_previous_code = REDROBD_MC_NONE;
    m_last_steering = REDROBD_MC_STOP;

    steer_stop();
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::init_members(void)
{
  m_pin_func_l293d_1a = 0;
  m_pin_func_l293d_2a = 0;
  m_pin_func_l293d_3a = 0;
  m_pin_func_l293d_4a = 0;
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

////////////////////////////////////////////////////////////////

void redrobd_motor_ctrl::steer_motor(REDROBD_MC_MOTOR_ID motor_id,
				     REDROBD_MC_MOTOR_DIR motor_dir)
{
  uint8_t l293d_inp1 = 0;
  uint8_t l293d_inp2 = 0;

  switch (motor_id) {
  case REDROBD_MC_MOTOR_ID_RIGHT:
    l293d_inp1 = PIN_L293D_1A;
    l293d_inp2 = PIN_L293D_2A;
    break;
  case REDROBD_MC_MOTOR_ID_LEFT:
    l293d_inp1 = PIN_L293D_3A;
    l293d_inp2 = PIN_L293D_4A;
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
