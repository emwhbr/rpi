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

#ifndef __REDROBD_RC_RF_H__
#define __REDROBD_RC_RF_H__

#include "redrobd_remote_ctrl.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_rc_rf : public redrobd_remote_ctrl {
  
 public:
  redrobd_rc_rf(uint8_t pin_forward,
		uint8_t pin_reverse,
		uint8_t pin_right,
		uint8_t pin_left);

  ~redrobd_rc_rf(void);

  // Implements pure virtual functions from base class
  virtual void initialize(void);
  virtual void finalize(void);
  virtual uint16_t get_steering(void);

 private:
  // GPIO pins
  uint8_t m_pin_forward;
  uint8_t m_pin_reverse;
  uint8_t m_pin_right;
  uint8_t m_pin_left;

  // Keep track of old pin functions
  uint8_t m_pin_func_forward;
  uint8_t m_pin_func_reverse;
  uint8_t m_pin_func_right;
  uint8_t m_pin_func_left;

  void init_members(void);
};

#endif // __REDROBD_RC_RF_H__
