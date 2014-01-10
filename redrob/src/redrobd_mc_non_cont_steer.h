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

#ifndef __REDROBD_MC_NON_CONT_STEER_H__
#define __REDROBD_MC_NON_CONT_STEER_H__

#include "redrobd_motor_ctrl.h"

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

// States for the state machine
typedef enum {MC_STATE_INIT,
	      MC_STATE_FORWARD,
	      MC_STATE_REVERSE,
	      MC_STATE_LEFT,
	      MC_STATE_RIGHT,
	      MC_STATE_MEMORY,
	      MC_STATE_STOP} MC_STATE;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_mc_non_cont_steer : public redrobd_motor_ctrl {

 public:
  redrobd_mc_non_cont_steer(uint8_t pin_right_motor_1,
			    uint8_t pin_right_motor_2,
			    uint8_t pin_left_motor_1,
			    uint8_t pin_left_motor_2);

  ~redrobd_mc_non_cont_steer(void); 

  // Implements pure virtual function from base class
  virtual void steer(uint16_t code);

 private:
  // Keep track of the state machine
  uint16_t m_last_steering;
  MC_STATE m_current_state;
  uint16_t m_last_memory_state_code;
  bool     m_memory_state_done;

  void init_members(void);
};

#endif // __REDROBD_MC_NON_CONT_STEER_H__
