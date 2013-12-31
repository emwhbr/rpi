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

#include <sstream>
#include <iomanip>

#include "redrobd_voltage_monitor_thread.h"
#include "redrobd_log.h"
#include "redrobd_error_utility.h"
#include "daemon_utility.h"

// Implementation notes:
// 1. Assumes the MCP3008 interface already initialized.
//

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_voltage_monitor_thread::
redrobd_voltage_monitor_thread(string thread_name,
			       double frequency,
			       mcp3008_io *mcp3008_io_ptr,
			       MCP3008_IO_CHANNEL mcp3008_io_chn,
			       float voltage_sf) : cyclic_thread(thread_name,
								 frequency)
{
  pthread_mutex_init(&m_voltage_mutex, NULL); // Use default mutex attributes

  m_mcp3008_io_ptr = mcp3008_io_ptr;
  m_mcp3008_io_chn = mcp3008_io_chn;
  m_voltage_sf     = voltage_sf;

  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_voltage_monitor_thread::~redrobd_voltage_monitor_thread(void)
{
  pthread_mutex_destroy(&m_voltage_mutex);
}

////////////////////////////////////////////////////////////////

void redrobd_voltage_monitor_thread::get_voltage(REDROBD_VOLTAGE &value)
{
  // Lockdown read operation
  pthread_mutex_lock(&m_voltage_mutex);

  value.v_mon = m_voltage.v_mon;
  value.v_in  = m_voltage.v_in;

  // Lockup read operation
  pthread_mutex_unlock(&m_voltage_mutex);
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long redrobd_voltage_monitor_thread::setup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : setup");

    init_members();
            
    return THREAD_SUCCESS;    
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_voltage_monitor_thread::setup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_voltage_monitor_thread::cleanup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : cleanup");
    
    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_voltage_monitor_thread::cleanup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_voltage_monitor_thread::cyclic_execute(void)
{
  try {
    uint16_t adc_value;
    float v_mon;
    float v_in;

    // Get voltage from analog input
    m_mcp3008_io_ptr->read_single(m_mcp3008_io_chn, adc_value);
    v_mon = m_mcp3008_io_ptr->to_voltage(adc_value);

    // Scale back to input voltage
    v_in = v_mon / m_voltage_sf;

    // Lockdown read operation
    pthread_mutex_lock(&m_voltage_mutex);
    
    m_voltage.v_mon = v_mon;
    m_voltage.v_in  = v_in;
    
    // Lockup read operation
    pthread_mutex_unlock(&m_voltage_mutex);

    // Log voltages
    ostringstream oss_msg;
    oss_msg.precision(3);
    oss_msg << fixed;
    oss_msg << get_name() << " : Vmon=" << v_mon << ", Vin=" << v_in;

    redrobd_log_writeln(oss_msg.str());

    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_voltage_monitor_thread::cyclic_execute->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_voltage_monitor_thread::init_members(void)
{
  m_voltage.v_mon = 0.0;
  m_voltage.v_in  = 0.0;
}
