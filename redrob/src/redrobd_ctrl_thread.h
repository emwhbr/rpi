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

#ifndef __REDROBD_CTRL_THREAD_H__
#define __REDROBD_CTRL_THREAD_H__

#include <memory>

#include "cyclic_thread.h"
#include "redrobd_alive_thread.h"
#include "redrobd_voltage_monitor_thread.h"
#include "redrobd_remote_ctrl.h"
#include "redrobd_motor_ctrl.h"
#include "mcp3008_io.h"
#include "redrobd_hw_cfg.h"
#include "timer.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_ctrl_thread : public cyclic_thread {

 public:
  redrobd_ctrl_thread(string thread_name,
		      double frequency);
  ~redrobd_ctrl_thread(void);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long cyclic_execute(void); // Implements pure virtual function from base class
    
 private:
  // The alive thread object
  auto_ptr<redrobd_alive_thread> m_alive_thread_auto;

  // The battery monitor thread object
  auto_ptr<redrobd_voltage_monitor_thread> m_bat_mon_thread_auto;

  // Remote control object
  redrobd_remote_ctrl m_remote_ctrl;

  // Motor control object
  redrobd_motor_ctrl m_motor_ctrl;

  // A/D Converter object pointer
  mcp3008_io *m_mcp3008_io_ptr;

  // Hardware configuration object
  auto_ptr<redrobd_hw_cfg> m_hw_cfg_auto;

  // Controls battery check
  timer m_battery_check_timer;
  bool  m_battery_check_allowed;
  bool  m_battery_low_detected;

  // Controls shutdown
  bool m_shutdown_select;

  void init_members(void);

  bool battery_voltage_ok(void);

  void check_thread_run_status(void);
};

#endif // __REDROBD_CTRL_THREAD_H__
