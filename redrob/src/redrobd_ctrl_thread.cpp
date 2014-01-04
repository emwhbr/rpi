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

#include "redrobd_ctrl_thread.h"
#include "redrobd.h"
#include "redrobd_log.h"
#include "redrobd_error_utility.h"
#include "redrobd_thread_utility.h"
#include "redrobd_led.h"
#include "rpi_hw.h"
#include "daemon_utility.h"
#include "excep.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define ALIVE_THREAD_NAME             "REDROBD_ALIVE"
#define ALIVE_THREAD_FREQUENCY        2.0 // Hz
#define ALIVE_THREAD_START_TIMEOUT    1.0 // Seconds
#define ALIVE_THREAD_EXECUTE_TIMEOUT  0.5 // Seconds
#define ALIVE_THREAD_STOP_TIMEOUT     1.5 // Seconds
                                          // Period time + one extra second

#define BAT_MON_THREAD_NAME             "REDROBD_BAT_MON"
#define BAT_MON_THREAD_FREQUENCY        0.2 // Hz
#define BAT_MON_THREAD_START_TIMEOUT    1.0 // Seconds
#define BAT_MON_THREAD_EXECUTE_TIMEOUT  0.5 // Seconds
#define BAT_MON_THREAD_STOP_TIMEOUT     6.0 // Seconds
                                            // Period time + one extra second

#define BAT_MIN_ALLOWED_VOLTAGE  6.9 // Volt

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_ctrl_thread::
redrobd_ctrl_thread(string thread_name,
		    double frequency) : cyclic_thread(thread_name,
						      frequency)
{
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_ctrl_thread::~redrobd_ctrl_thread(void)
{
  delete m_mcp3008_io_ptr;
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long redrobd_ctrl_thread::setup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : setup started");

    init_members();

    // Create the cyclic alive thread object with garbage collector
    redrobd_alive_thread *thread_ptr1 =
      new redrobd_alive_thread(ALIVE_THREAD_NAME,
			       ALIVE_THREAD_FREQUENCY);    
    m_alive_thread_auto =
      auto_ptr<redrobd_alive_thread>(thread_ptr1);

    /////////////////////////////////
    //  INITIALIZE ALIVE THREAD
    /////////////////////////////////
    redrobd_log_writeln("About to initialize alive thread");

    // Take back ownership from auto_ptr
    thread_ptr1 = m_alive_thread_auto.release();
    
    try {
       // Initialize cyclic alive thread object
      redrobd_thread_initialize_cyclic((cyclic_thread *)thread_ptr1,
				       ALIVE_THREAD_START_TIMEOUT,
				       ALIVE_THREAD_EXECUTE_TIMEOUT);
    }
    catch (...) {
      m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);
      throw;
    }

    // Give back ownership to auto_ptr
    m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);

    // Create the A/D Converter object
    m_mcp3008_io_ptr = new mcp3008_io(MCP3008_SPI_DEV,
				      MCP3008_REF_VOLTAGE);
    
    // Initialize A/D Converter
    m_mcp3008_io_ptr->initialize(MCP3008_SPI_SPEED);
    
    // Create the hardware configuration object object with garbage collector
    redrobd_hw_cfg *redrobd_hw_cfg_ptr =
      new redrobd_hw_cfg(m_mcp3008_io_ptr,
			 (MCP3008_IO_CHANNEL)MCP3008_CHN_SHUTDOWN,
			 (MCP3008_IO_CHANNEL)MCP3008_CHN_CONT_STEER);

    m_hw_cfg_auto = auto_ptr<redrobd_hw_cfg>(redrobd_hw_cfg_ptr);

    // Initialize hardware configuration
    m_hw_cfg_auto->initialize();

    // Create the remote control object with garbage collector
    redrobd_remote_ctrl *rc_ptr =
      new redrobd_remote_ctrl(PIN_RF_IN_0,  // Forward
			      PIN_RF_IN_1,  // Reverse
			      PIN_RF_IN_2,  // Right
			      PIN_RF_IN_3); // Left

    m_remote_ctrl_auto = auto_ptr<redrobd_remote_ctrl>(rc_ptr);

    // Initialize remote control
    m_remote_ctrl_auto->initialize();

    // Check if continuous steering was selected (DIP-switch)
    bool cont_steering = m_hw_cfg_auto->select_continuous_steering();
    if (cont_steering) {
      redrobd_log_writeln(get_name() + " : Continuous steering selected");      
    }
    else {
      redrobd_log_writeln(get_name() + " : Non-continuous steering selected");
    }

    // Create the motor control object with garbage collector
    redrobd_motor_ctrl *mc_ptr =
      new redrobd_motor_ctrl(PIN_L293D_1A,  // Right motor
			     PIN_L293D_2A,
			     PIN_L293D_3A,  // Left motor
			     PIN_L293D_4A);

    m_motor_ctrl_auto = auto_ptr<redrobd_motor_ctrl>(mc_ptr);

    // Initialize motor control
    m_motor_ctrl_auto->initialize(cont_steering);

    // Create the cyclic battery monitor thread object with garbage collector
    redrobd_voltage_monitor_thread *thread_ptr2 =
      new redrobd_voltage_monitor_thread(BAT_MON_THREAD_NAME,
					 BAT_MON_THREAD_FREQUENCY,
					 m_mcp3008_io_ptr,
					 (MCP3008_IO_CHANNEL)MCP3008_CHN_VBAT,
					 MCP3008_CHN_VBAT_SF);    
    m_bat_mon_thread_auto =
      auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);

    ////////////////////////////////////////
    //  INITIALIZE BATTERY MONITOR THREAD
    ////////////////////////////////////////
    redrobd_log_writeln("About to initialize battery monitor thread");

    // Take back ownership from auto_ptr
    thread_ptr2 = m_bat_mon_thread_auto.release();
    
    try {
      // Initialize cyclic battery monitor thread object
      redrobd_thread_initialize_cyclic((cyclic_thread *)thread_ptr2,
				       BAT_MON_THREAD_START_TIMEOUT,
				       BAT_MON_THREAD_EXECUTE_TIMEOUT);
    }
    catch (...) {
      m_bat_mon_thread_auto =
	auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);
      throw;
    }
    
    // Give back ownership to auto_ptr
    m_bat_mon_thread_auto =
      auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);

    // Start timer controlling when to check battery
    if (m_battery_check_timer.reset() != TIMER_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
		"Error resetting battery check timer for thread %s",
		get_name().c_str());      
    }

    redrobd_log_writeln(get_name() + " : setup done");

    return THREAD_SUCCESS;    
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_ctrl_thread::setup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_ctrl_thread::cleanup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : cleanup started");

    ////////////////////////////////////////
    //  FINALIZE BATTERY MONITOR THREAD
    ////////////////////////////////////////
    redrobd_log_writeln("About to finalize battery monitor thread");

    // Take back ownership from auto_ptr
    redrobd_voltage_monitor_thread *thread_ptr2 =
      m_bat_mon_thread_auto.release();
    
    try {
      // Finalize the cyclic battery monitor thread object
      redrobd_thread_finalize_cyclic((cyclic_thread *)thread_ptr2,
				     BAT_MON_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_bat_mon_thread_auto =
	auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);
      throw;
    }
    
    // Give back ownership to auto_ptr
    m_bat_mon_thread_auto =
      auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);

    // Delete the cyclic battery monitor thread object
    m_bat_mon_thread_auto.reset();

    // Finalize and delete the motor control object
    m_motor_ctrl_auto->finalize();
    m_motor_ctrl_auto.reset();

    // Finalize and delete the remote control object
    m_remote_ctrl_auto->finalize();
    m_remote_ctrl_auto.reset();

    // Finalize and delete the hardware configuration object
    m_hw_cfg_auto->finalize();
    m_hw_cfg_auto.reset();

    // Finalize and delete A/D Converter
    m_mcp3008_io_ptr->finalize();
    delete m_mcp3008_io_ptr;
    m_mcp3008_io_ptr = NULL;

    /////////////////////////////////
    //  FINALIZE ALIVE THREAD
    /////////////////////////////////
    redrobd_log_writeln("About to finalize alive thread");

    // Take back ownership from auto_ptr
    redrobd_alive_thread *thread_ptr1 = m_alive_thread_auto.release();
    
    try {
      // Finalize the cyclic alive thread object
      redrobd_thread_finalize_cyclic((cyclic_thread *)thread_ptr1,
				     ALIVE_THREAD_STOP_TIMEOUT);
    }
    catch (...) {
      m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);
      throw;
    }

    // Give back ownership to auto_ptr
    m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);
    
    // Delete the cyclic alive thread object
    m_alive_thread_auto.reset();
    
    redrobd_log_writeln(get_name() + " : cleanup done");

    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_ctrl_thread::cleanup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_ctrl_thread::cyclic_execute(void)
{
  try {
    // Check if shutdown was selected (DIP-switch)
    if ( (!m_shutdown_select) && (m_hw_cfg_auto->select_shutdown()) ) {
      redrobd_log_writeln(get_name() + " : Shutdown selected");
      // Send signal to main process
      if (send_signal_self(SIG_TERMINATE_DAEMON) != DAEMON_SUCCESS) {
	THROW_EXP(REDROBD_LINUX_ERROR, REDROBD_SIGNAL_OPERATION_FAILED,
		  "Error sending shutdown signal for thread %s",
		  get_name().c_str());   
      }
      m_shutdown_select = true; // Only signal once
    }

    // Check battery voltage
    if ( (!m_battery_low_detected) && (!battery_voltage_ok()) ) {
      redrobd_log_writeln(get_name() + " : Low battery voltage detected");
      redrobd_led_bat_low(true);     // Turn status LED on     
      m_battery_low_detected = true; // Only signal once
    }

    // Check state and status of created threads
    check_thread_run_status();

    // Check steering from remote control
    uint16_t steering = m_remote_ctrl_auto->get_steering();

    // Do motor control
    switch (steering) {
    case REDROBD_RC_NONE:
       m_motor_ctrl_auto->steer(REDROBD_MC_NONE);
      break;
    case REDROBD_RC_FORWARD:
      redrobd_log_writeln(get_name() + " : steer forward");
      m_motor_ctrl_auto->steer(REDROBD_MC_FORWARD);
      break;
    case REDROBD_RC_REVERSE:
      redrobd_log_writeln(get_name() + " : steer reverse");
      m_motor_ctrl_auto->steer(REDROBD_MC_REVERSE);
      break; 
    case REDROBD_RC_RIGHT:
      redrobd_log_writeln(get_name() + " : steer right");
      m_motor_ctrl_auto->steer(REDROBD_MC_RIGHT);
      break;
    case REDROBD_RC_LEFT:
      redrobd_log_writeln(get_name() + " : steer left");
      m_motor_ctrl_auto->steer(REDROBD_MC_LEFT);
      break;
    default:
      // All other steerings are ignored for now
       ostringstream oss_msg;
       oss_msg << get_name()
	       << " : Got undefined steering = 0x"
	       << hex << setw(4) << setfill('0') << (unsigned)steering;
       redrobd_log_writeln(oss_msg.str());

       m_motor_ctrl_auto->steer(REDROBD_MC_STOP);
    }

    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_ctrl_thread::cyclic_execute->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_ctrl_thread::init_members(void)
{
  m_mcp3008_io_ptr = NULL;

  m_battery_check_allowed = false;
  m_battery_low_detected  = false;

  m_shutdown_select = false;
}

////////////////////////////////////////////////////////////////

bool redrobd_ctrl_thread::battery_voltage_ok(void)
{
  bool battery_ok = true;

  // Allow battery monitor thread to run at least one period
  if (!m_battery_check_allowed) {
    if ( m_battery_check_timer.get_elapsed_time() > 
	 (1.0/BAT_MON_THREAD_FREQUENCY) ) {      
      m_battery_check_allowed = true;
    }
  }
  else {
    // Safe to check battery voltage now
    REDROBD_VOLTAGE v_bat;
    
    // Get latest monitored value
    m_bat_mon_thread_auto->get_voltage(v_bat);
    
    // Check if value to low
    if (v_bat.v_in < BAT_MIN_ALLOWED_VOLTAGE) {
      battery_ok = false;
    }
  }

  return battery_ok;
}

////////////////////////////////////////////////////////////////

void redrobd_ctrl_thread::check_thread_run_status(void)
{
  //////////////////////////
  //  CHECK ALIVE THREAD   
  //////////////////////////

  // Take back ownership from auto_ptr
  redrobd_alive_thread *thread_ptr1 =
    m_alive_thread_auto.release();

  try {
    // Check state and status of alive thread object
    redrobd_thread_check_cyclic((cyclic_thread *)thread_ptr1);
  }
  catch (...) {
    m_alive_thread_auto =
      auto_ptr<redrobd_alive_thread>(thread_ptr1);
    throw;
  }

  // Give back ownership to auto_ptr
  m_alive_thread_auto =
    auto_ptr<redrobd_alive_thread>(thread_ptr1);

  ///////////////////////////////////
  //  CHECK BATTERY MONITOR THREAD   
  ///////////////////////////////////

  // Take back ownership from auto_ptr
  redrobd_voltage_monitor_thread *thread_ptr2 =
    m_bat_mon_thread_auto.release();

  try {
    // Check state and status of alive thread object
    redrobd_thread_check_cyclic((cyclic_thread *)thread_ptr2);
  }
  catch (...) {
    m_bat_mon_thread_auto =
      auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);
    throw;
  }

  // Give back ownership to auto_ptr
  m_bat_mon_thread_auto =
    auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);
}
