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
#include "redrobd_rc_rf.h"
#include "redrobd_rc_net.h"
#include "redrobd_mc_cont_steer.h"
#include "redrobd_mc_non_cont_steer.h"
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
		      double frequency,
		      bool verbose);
  ~redrobd_ctrl_thread(void);

 protected:
  virtual long setup(void);   // Implements pure virtual function from base class
  virtual long cleanup(void); // Implements pure virtual function from base class

  virtual long cyclic_execute(void); // Implements pure virtual function from base class
    
 private:
  // Full verbose logging
  bool m_verbose;

  // The alive thread object
  auto_ptr<redrobd_alive_thread> m_alive_thread_auto;

  // The battery monitor thread object
  auto_ptr<redrobd_voltage_monitor_thread> m_bat_mon_thread_auto;

  // Remote control object (RF, Radio)
  auto_ptr<redrobd_rc_rf> m_rc_rf_auto;

  // Remote control object (NET, Sockets)
  auto_ptr<redrobd_rc_net> m_rc_net_auto;

  // Motor control object (continuous steer)
  auto_ptr<redrobd_mc_cont_steer> m_mc_cont_steer_auto;

  // Motor control object (non-continuous steer)
  auto_ptr<redrobd_mc_non_cont_steer> m_mc_non_cont_steer_auto;

  // A/D Converter object pointer
  mcp3008_io *m_mcp3008_io_ptr;

  // Hardware configuration object
  auto_ptr<redrobd_hw_cfg> m_hw_cfg_auto;

  // Controls battery check
  timer m_battery_check_timer;
  bool  m_battery_check_allowed;

  // Controls shutdown
  bool m_shutdown_select;

  // Controls type of motor control
  bool m_cont_steering;

  void init_members(void);

  bool battery_voltage_ok(void);

  void check_thread_run_status(void);

  uint16_t get_remote_steering(void);

  void motor_control(uint16_t steer_code);
};

#endif // __REDROBD_CTRL_THREAD_H__
