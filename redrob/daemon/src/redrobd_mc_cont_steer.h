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

#ifndef __REDROBD_MC_CONT_STEER_H__
#define __REDROBD_MC_CONT_STEER_H__

#include "redrobd_motor_ctrl.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_mc_cont_steer : public redrobd_motor_ctrl {

 public:
  redrobd_mc_cont_steer(uint8_t pin_right_motor_1,
			uint8_t pin_right_motor_2,
			uint8_t pin_left_motor_1,
			uint8_t pin_left_motor_2);

  ~redrobd_mc_cont_steer(void); 

  // Implements pure virtual function from base class
  virtual void steer(uint16_t code);
};

#endif // __REDROBD_MC_CONT_STEER_H__
