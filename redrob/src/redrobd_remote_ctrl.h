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

#ifndef __REDROBD_REMOTE_CTRL_H__
#define __REDROBD_REMOTE_CTRL_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Steer codes
#define REDROBD_RC_NONE     0x00
#define REDROBD_RC_FORWARD  0x01
#define REDROBD_RC_REVERSE  0x02
#define REDROBD_RC_RIGHT    0x04
#define REDROBD_RC_LEFT     0x08

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_remote_ctrl {
  
 public:
  redrobd_remote_ctrl(uint8_t pin_forward,
		      uint8_t pin_reverse,
		      uint8_t pin_right,
		      uint8_t pin_left);

  ~redrobd_remote_ctrl(void);

  void initialize(void);
  void finalize(void);

  uint16_t get_steering(void);

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

#endif // __REDROBD_REMOTE_CTRL_H__
