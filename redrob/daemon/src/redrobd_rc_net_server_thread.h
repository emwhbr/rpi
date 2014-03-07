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

#ifndef __REDROBD_RC_NET_SERVER_THREAD_H__
#define __REDROBD_RC_NET_SERVER_THREAD_H__

#include <string>
#include <stdint.h>
#include <pthread.h>

#include "thread.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

// Client steer codes
#define CLI_STEER_NONE     0x00
#define CLI_STEER_FORWARD  0x01
#define CLI_STEER_REVERSE  0x02
#define CLI_STEER_RIGHT    0x04
#define CLI_STEER_LEFT     0x08

// Client camera codes
#define CLI_CAMERA_NONE          0x00
#define CLI_CAMERA_STOP_STREAM   0x01
#define CLI_CAMERA_START_STREAM  0x02

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_rc_net_server_thread : public thread {

 public:
  redrobd_rc_net_server_thread(string thread_name,
			       string server_ip_address,
			       uint16_t server_port);

  ~redrobd_rc_net_server_thread(void);

  uint16_t get_steer_code(void);

  void set_voltage(float value);

  uint16_t get_camera_code(void);

  void shutdown_server(void);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long execute(void *arg); // Implements pure virtual function from base class
    
 private:
  // Server ip info
  string   m_server_ip_address;
  uint16_t m_server_port;

  // Controlled server shutdown
  bool m_shutdown_requested;
  bool m_client_connected;
  bool m_server_closed;

  // Server socket
  int m_server_sd;

  // Client socket
  int m_client_sd;

  // Latest client steer code
  pthread_mutex_t m_steer_code_mutex;
  uint16_t        m_steer_code;

  // Latest voltage
  pthread_mutex_t m_voltage_mutex;
  uint16_t        m_voltage;

  // Latest client camera code
  pthread_mutex_t m_camera_code_mutex;
  uint16_t        m_camera_code;

  void init_members(void);

  void handle_clients(void);

  void recv_client(void *data,
		   unsigned nbytes);

  void send_client(void *data,
		   unsigned nbytes);
};

#endif // __REDROBD_RC_NET_SERVER_THREAD_H__
