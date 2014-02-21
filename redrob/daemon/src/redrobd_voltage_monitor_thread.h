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

#ifndef __REDROBD_VOLTAGE_MONITOR_THREAD_H__
#define __REDROBD_VOLTAGE_MONITOR_THREAD_H__

#include <pthread.h>

#include "cyclic_thread.h"
#include "mcp3008_io.h"
#include "timer.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  float v_mon;
  float v_in;  
} REDROBD_VOLTAGE;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_voltage_monitor_thread : public cyclic_thread {

 public:
  redrobd_voltage_monitor_thread(string thread_name,
				 double frequency,
				 mcp3008_io *mcp3008_io_ptr,
				 MCP3008_IO_CHANNEL mcp3008_io_chn,
				 float voltage_sf);

  ~redrobd_voltage_monitor_thread(void);

  void get_voltage(REDROBD_VOLTAGE &value);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long cyclic_execute(void); // Implements pure virtual function from base class
    
 private:
  // Latest monitored value
  pthread_mutex_t m_voltage_mutex;
  REDROBD_VOLTAGE m_voltage;

  // A/D Converter object pointer
  mcp3008_io *m_mcp3008_io_ptr;

  // A/D Converter channel to monitor
  MCP3008_IO_CHANNEL m_mcp3008_io_chn;

  // Scale factor used by voltage divider (v_mon = v_in * sf)
  float m_voltage_sf;

  // Controls when to log voltages
  timer m_voltage_log_timer;

  void init_members(void);
};

#endif // __REDROBD_VOLTAGE_MONITOR_THREAD_H__
