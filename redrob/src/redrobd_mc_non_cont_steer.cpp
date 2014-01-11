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

#include "redrobd_mc_non_cont_steer.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_mc_non_cont_steer::
redrobd_mc_non_cont_steer(uint8_t pin_right_motor_1,
			  uint8_t pin_right_motor_2,
			  uint8_t pin_left_motor_1,
			  uint8_t pin_left_motor_2) : redrobd_motor_ctrl(pin_right_motor_1,
									 pin_right_motor_2,
									 pin_left_motor_1,
									 pin_left_motor_2)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_mc_non_cont_steer::~redrobd_mc_non_cont_steer(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_mc_non_cont_steer::steer(uint16_t code)
{ 
  // Check of allowed codes
  if ( !check_steer_code(code) ) {
    steer_stop();
    m_current_state = MC_STATE_STOP;
    return;
  }

  // Non-continuous mode remembers last steering
  // No steering equals continue with last steering
  // Stop is achieved by generating same steer code
  // after a period of no steering

  MC_STATE next_state;
  
  switch (m_current_state) {
    /////////////////////////////////////////////////////
  case MC_STATE_INIT:
    ////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_MC_NONE:
      next_state = MC_STATE_INIT;
      break;
    case REDROBD_MC_STOP:
      steer_stop();
      next_state = MC_STATE_STOP;
      break;
    case REDROBD_MC_FORWARD:
      steer_forward();
      next_state = MC_STATE_FORWARD;
      break;
    case REDROBD_MC_REVERSE:
      steer_reverse();
      next_state = MC_STATE_REVERSE;
      break;
    case REDROBD_MC_RIGHT:
      steer_right();
      next_state = MC_STATE_RIGHT;
      break;
    case REDROBD_MC_LEFT:
      steer_left();
      next_state = MC_STATE_LEFT;
    }
    break;
    /////////////////////////////////////////////////////
  case MC_STATE_FORWARD:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_MC_NONE:
      m_memory_steer = true;
      next_state = MC_STATE_FORWARD;
      break;
    case REDROBD_MC_STOP:
      steer_stop();
      m_memory_steer = false;
      next_state = MC_STATE_STOP;
      break;
    case REDROBD_MC_FORWARD:
      if (m_memory_steer) {
	steer_stop();
	m_last_steering = code;
	next_state = MC_STATE_STOP;
      }
      else {
	steer_forward();
	next_state = MC_STATE_FORWARD;
      }
      break;
    case REDROBD_MC_REVERSE:
      steer_reverse();
      m_memory_steer = false;
      next_state = MC_STATE_REVERSE;
      break;
    case REDROBD_MC_RIGHT:
      steer_right();
      m_memory_steer = false;
      next_state = MC_STATE_RIGHT;
      break;
    case REDROBD_MC_LEFT:
      steer_left();
      m_memory_steer = false;
      next_state = MC_STATE_LEFT;
    }
    break;
    /////////////////////////////////////////////////////
  case MC_STATE_REVERSE:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_MC_NONE:
      m_memory_steer = true;
      next_state = MC_STATE_REVERSE;
      break;
    case REDROBD_MC_STOP:
      steer_stop();
      m_memory_steer = false;
      next_state = MC_STATE_STOP;
      break;
    case REDROBD_MC_FORWARD:
      steer_forward();
      m_memory_steer = false;
      next_state = MC_STATE_FORWARD;
      break;
    case REDROBD_MC_REVERSE:
      if (m_memory_steer) {
	steer_stop();
	m_last_steering = code;
	next_state = MC_STATE_STOP;
      }
      else {
	steer_reverse();
	next_state = MC_STATE_REVERSE;
      }
      break;
    case REDROBD_MC_RIGHT:
      steer_right();
      m_memory_steer = false;
      next_state = MC_STATE_RIGHT;
      break;
    case REDROBD_MC_LEFT:
      steer_left();
      m_memory_steer = false;
      next_state = MC_STATE_LEFT;
    }
    break;
    /////////////////////////////////////////////////////
  case MC_STATE_LEFT:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_MC_NONE:
      m_memory_steer = true;
      next_state = MC_STATE_LEFT;
      break;
    case REDROBD_MC_STOP:
      steer_stop();
      m_memory_steer = false;
      next_state = MC_STATE_STOP;
      break;
    case REDROBD_MC_FORWARD:
      steer_forward();
      m_memory_steer = false;
      next_state = MC_STATE_FORWARD;
      break;
    case REDROBD_MC_REVERSE:
      steer_reverse();
      m_memory_steer = false;
      next_state = MC_STATE_REVERSE;
      break;
    case REDROBD_MC_RIGHT:
      steer_right();
      m_memory_steer = false;
      next_state = MC_STATE_RIGHT;
      break;
    case REDROBD_MC_LEFT:
      if (m_memory_steer) {
	steer_stop();
	m_last_steering = code;
	next_state = MC_STATE_STOP;
      }
      else {
	steer_left();
	next_state = MC_STATE_LEFT;
      }
    }    
    break;
    /////////////////////////////////////////////////////
  case MC_STATE_RIGHT:
    /////////////////////////////////////////////////////
    switch (code) {
    case REDROBD_MC_NONE:
      m_memory_steer = true;
      next_state = MC_STATE_RIGHT;
      break;
    case REDROBD_MC_STOP:
      steer_stop();
      m_memory_steer = false;
      next_state = MC_STATE_STOP;
      break;
    case REDROBD_MC_FORWARD:
      steer_forward();
      m_memory_steer = false;
      next_state = MC_STATE_FORWARD;
      break;
    case REDROBD_MC_REVERSE:
      steer_reverse();
      m_memory_steer = false;
      next_state = MC_STATE_REVERSE;
      break;
    case REDROBD_MC_RIGHT:
      if (m_memory_steer) {
	steer_stop();
	m_last_steering = code;
	next_state = MC_STATE_STOP;
      }
      else {
	steer_right();
	next_state = MC_STATE_RIGHT;
      }
      break;
    case REDROBD_MC_LEFT:
      steer_left();
      m_memory_steer = false;
      next_state = MC_STATE_LEFT;
    }    
    break;
    /////////////////////////////////////////////////////
  case MC_STATE_STOP:
    /////////////////////////////////////////////////////
    if (m_memory_steer && (code == m_last_steering)) {      
      next_state = MC_STATE_STOP;
    }
    else {
      m_memory_steer = false;
      switch (code) {
      case REDROBD_MC_NONE:
      case REDROBD_MC_STOP:
	next_state = MC_STATE_STOP;
	break;
      case REDROBD_MC_FORWARD:
	steer_forward();
	next_state = MC_STATE_FORWARD;
	break;
      case REDROBD_MC_REVERSE:
	steer_reverse();
	next_state = MC_STATE_REVERSE;
	break;
      case REDROBD_MC_RIGHT:
	steer_right();
	next_state = MC_STATE_RIGHT;
	break;
      case REDROBD_MC_LEFT:
	steer_left();
	next_state = MC_STATE_LEFT;
      }
    }
    break;
  }

  m_current_state = next_state;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_mc_non_cont_steer::init_members(void)
{  
  m_current_state = MC_STATE_INIT;
  m_last_steering = REDROBD_MC_NONE;
  m_memory_steer = false;
}
