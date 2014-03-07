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
#define REDROBD_RC_STEER_NONE     0x00
#define REDROBD_RC_STEER_FORWARD  0x01
#define REDROBD_RC_STEER_REVERSE  0x02
#define REDROBD_RC_STEER_RIGHT    0x04
#define REDROBD_RC_STEER_LEFT     0x08

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_remote_ctrl {
  
 public:
  redrobd_remote_ctrl(void);

  ~redrobd_remote_ctrl(void);

  bool is_active(void);

  virtual void initialize(void) = 0;       // Pure virtual function
  virtual void finalize(void) = 0;         // Pure virtual function
  virtual uint16_t get_steering(void) = 0; // Pure virtual function

 protected:
  void set_active(bool value);

 private:
  bool m_is_active;

  void init_members(void);
};

#endif // __REDROBD_REMOTE_CTRL_H__
