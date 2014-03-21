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

#ifndef __REDROBD_RC_NET_H__
#define __REDROBD_RC_NET_H__

#include <string>
#include <stdint.h>
#include <memory>

#include "redrobd_remote_ctrl.h"
#include "redrobd_rc_net_server_thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Camera codes
#define REDROBD_RC_CAMERA_NONE          0x00
#define REDROBD_RC_CAMERA_STOP_STREAM   0x01
#define REDROBD_RC_CAMERA_START_STREAM  0x02

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_rc_net : public redrobd_remote_ctrl {
  
 public:
  redrobd_rc_net(string server_ip_address,
		 uint16_t server_port);

  ~redrobd_rc_net(void);

  // Implements pure virtual functions from base class
  virtual void initialize(void);
  virtual void finalize(void);
  virtual uint16_t get_steering(void);

  void set_voltage(float value);

  uint16_t get_camera_code(void);

  void set_sys_stat(uint8_t cpu_load,
		    uint32_t mem_used,
		    uint16_t irq,
		    uint32_t uptime);

  void server_thread_check(void);

 private:
  string   m_server_ip_address;
  uint16_t m_server_port;

  // The server thread object
  auto_ptr<redrobd_rc_net_server_thread> m_server_thread_auto;

  void init_members(void);
};

#endif // __REDROBD_RC_NET_H__
