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

#ifndef __REDROBD_CAMERA_CTRL_H__
#define __REDROBD_CAMERA_CTRL_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Camera codes
#define REDROBD_CC_NONE          0x00
#define REDROBD_CC_STOP_STREAM   0x01
#define REDROBD_CC_START_STREAM  0x02

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
// States for the state machine
typedef enum {CC_STATE_INIT,
	      CC_STATE_ACTIVE,
	      CC_STATE_DEACTIVE} CC_STATE;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_camera_ctrl {
  
 public:
  redrobd_camera_ctrl(void);

  ~redrobd_camera_ctrl(void);

  void initialize(void);
  void finalize(void);

  void command(uint16_t code);

 private:
  // Keep track of the state machine  
  CC_STATE m_current_state;

  void init_members(void);

  bool check_camera_code(uint16_t code);

  void stop_stream(void);
  void start_stream(void);  
};

#endif // __REDROBD_CAMERA_CTRL_H__
