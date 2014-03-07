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

#include "redrobd_rc_net.h"
#include "redrobd_log.h"
#include "redrobd_thread_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define RC_NET_SERVER_THREAD_NAME             "REDROBD_RC_NET_SERVER"
#define RC_NET_SERVER_THREAD_START_TIMEOUT    1.0 // Seconds
#define RC_NET_SERVER_THREAD_EXECUTE_TIMEOUT  0.5 // Seconds
#define RC_NET_SERVER_THREAD_STOP_TIMEOUT     1.5 // Seconds

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_rc_net::redrobd_rc_net(string server_ip_address,
			       uint16_t server_port) : redrobd_remote_ctrl()
{
  m_server_ip_address = server_ip_address;
  m_server_port = server_port;

  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_rc_net::~redrobd_rc_net(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net::initialize(void)
{
  // Not yet active
  set_active(false);

  // Create the server thread object with garbage collector
  redrobd_rc_net_server_thread *thread_ptr =
    new redrobd_rc_net_server_thread(RC_NET_SERVER_THREAD_NAME,
				     m_server_ip_address,
				     m_server_port);
  m_server_thread_auto =
    auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);

  redrobd_log_writeln("About to initialize rc net server thread");

  // Take back ownership from auto_ptr
  thread_ptr = m_server_thread_auto.release();
  
  try {
    // Initialize server thread object
    redrobd_thread_initialize((thread *)thread_ptr,
			      RC_NET_SERVER_THREAD_START_TIMEOUT,
			      RC_NET_SERVER_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_server_thread_auto = auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_server_thread_auto = auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net::finalize(void)
{
  redrobd_log_writeln("About to finalize rc net server thread");

  // Take back ownership from auto_ptr
  redrobd_rc_net_server_thread *thread_ptr = m_server_thread_auto.release(); 

  try {
    // Shutdown server
    thread_ptr->shutdown_server();

    // Finalize server thread object
    redrobd_thread_finalize((thread *)thread_ptr,
			    RC_NET_SERVER_THREAD_STOP_TIMEOUT);
  }
  catch (...) {
    m_server_thread_auto = auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_server_thread_auto = auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);

  // Delete the server thread object
  m_server_thread_auto.reset();

  // Not active anymore
  set_active(false);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_rc_net::get_steering(void)
{
  uint16_t steering;

  // Get latest steer code from client
  // We know that steer codes can be directly
  // translated to steerings.
  steering = m_server_thread_auto->get_steer_code();

  // It requires at least one steering to be considered activated
  if ( (!is_active()) && (steering != REDROBD_RC_STEER_NONE) ) {
    set_active(true);
  }

  return steering;
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net::set_voltage(float value)
{
  m_server_thread_auto->set_voltage(value);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_rc_net::get_camera_code(void)
{
  // Get latest camera code from client
  // We know that camera codes can be directly translated
  return m_server_thread_auto->get_camera_code();
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net::server_thread_check(void)
{
  // Take back ownership from auto_ptr
  redrobd_rc_net_server_thread *thread_ptr =
    m_server_thread_auto.release();

  try {
    // Check state and status of server thread object
    redrobd_thread_check((thread *)thread_ptr);
  }
  catch (...) {
    m_server_thread_auto =
      auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);
    throw;
  }

  // Give back ownership to auto_ptr
  m_server_thread_auto =
    auto_ptr<redrobd_rc_net_server_thread>(thread_ptr);
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_rc_net::init_members(void)
{
  m_server_thread_auto.reset();
}
