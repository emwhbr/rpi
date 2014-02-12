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

#include "redrobd_mc_cont_steer.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_mc_cont_steer::
redrobd_mc_cont_steer(uint8_t pin_right_motor_1,
		      uint8_t pin_right_motor_2,
		      uint8_t pin_left_motor_1,
		      uint8_t pin_left_motor_2) : redrobd_motor_ctrl(pin_right_motor_1,
								     pin_right_motor_2,
								     pin_left_motor_1,
								     pin_left_motor_2)
{
}

////////////////////////////////////////////////////////////////

redrobd_mc_cont_steer::~redrobd_mc_cont_steer(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_mc_cont_steer::steer(uint16_t code)
{
  // Check of allowed codes
  if ( !check_steer_code(code) ) {
    steer_stop();
    return;
  }

  // Continuous mode requires a flow of steer codes
  // No steering equals stop

  switch (code) {
  case REDROBD_MC_NONE:
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
  }
}
