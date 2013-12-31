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

#ifndef __REDROBD_MOTOR_CTRL_H__
#define __REDROBD_MOTOR_CTRL_H__

#include <stdint.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Steer codes
#define REDROBD_MC_NONE     0x00
#define REDROBD_MC_STOP     0x01
#define REDROBD_MC_FORWARD  0x02
#define REDROBD_MC_REVERSE  0x04
#define REDROBD_MC_RIGHT    0x08
#define REDROBD_MC_LEFT     0x10

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
// Motor identifiers
typedef enum {REDROBD_MC_MOTOR_ID_RIGHT,
	      REDROBD_MC_MOTOR_ID_LEFT} REDROBD_MC_MOTOR_ID;

// Motor directions
typedef enum {REDROBD_MC_MOTOR_DIR_STOP,
	      REDROBD_MC_MOTOR_DIR_FORWARD,
	      REDROBD_MC_MOTOR_DIR_REVERSE} REDROBD_MC_MOTOR_DIR;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_motor_ctrl {
  
 public:
  redrobd_motor_ctrl(void);
  ~redrobd_motor_ctrl(void);

  void initialize(bool continuous_steer);

  void finalize(void);

  void steer(uint16_t code);

 private:
  // Keep track of old pin functions
  uint8_t m_pin_func_l293d_1a;
  uint8_t m_pin_func_l293d_2a;
  uint8_t m_pin_func_l293d_3a;
  uint8_t m_pin_func_l293d_4a;

  // Steering mode
  bool     m_continuous_steer;
  uint16_t m_previous_code;
  uint16_t m_last_steering;

  void init_members(void);

  void steer_none(void);
  void steer_stop(void);
  void steer_forward(void);
  void steer_reverse(void);
  void steer_right(void);
  void steer_left(void);

  void steer_motor(REDROBD_MC_MOTOR_ID motor_id,
		   REDROBD_MC_MOTOR_DIR motor_dir);
};

#endif // __REDROBD_MOTOR_CTRL_H__
