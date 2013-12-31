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

#include "redrobd_alive_thread.h"
#include "redrobd_log.h"
#include "redrobd_error_utility.h"
#include "redrobd_led.h"
#include "daemon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_alive_thread::
redrobd_alive_thread(string thread_name,
		     double frequency) : cyclic_thread(thread_name,
						       frequency)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_alive_thread::~redrobd_alive_thread(void)
{
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long redrobd_alive_thread::setup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : setup");

    init_members();

    redrobd_led_alive(true); // Turn status LED on
            
    return THREAD_SUCCESS;    
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_alive_thread::setup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_alive_thread::cleanup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : cleanup");

    redrobd_led_alive(false); // Turn status LED off
    
    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_alive_thread::cleanup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_alive_thread::cyclic_execute(void)
{
  try {
    redrobd_led_alive(m_led_alive_activate); // Toggle status LED on/off
    m_led_alive_activate = !m_led_alive_activate;
        
    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_alive_thread::cyclic_execute->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_alive_thread::init_members(void)
{
  m_led_alive_activate = false;
}
