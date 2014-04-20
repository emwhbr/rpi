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
#include "socket_support.h"

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
#define BAT_MON_THREAD_FREQUENCY        4.0  // Hz
#define BAT_MON_THREAD_START_TIMEOUT    1.0  // Seconds
#define BAT_MON_THREAD_EXECUTE_TIMEOUT  0.5  // Seconds
#define BAT_MON_THREAD_STOP_TIMEOUT     1.25 // Seconds
                                             // Period time + one extra second

#define BAT_MIN_ALLOWED_VOLTAGE  6.9 // Volt

#define SYS_STAT_CHECK_FREQUENCY  1.0 // Hz

#define RC_NET_SERVER_IP    ANY_IP_ADDRESS
#define RC_NET_SERVER_PORT  52022

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_ctrl_thread::
redrobd_ctrl_thread(string thread_name,
		    double frequency,
		    bool verbose) : cyclic_thread(thread_name,
						  frequency)
{
  m_verbose = verbose;

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

    /////////////////////////////////
    //  INITIALIZE ALIVE THREAD
    /////////////////////////////////

    // Create the cyclic alive thread object with garbage collector
    redrobd_alive_thread *thread_ptr1 =
      new redrobd_alive_thread(ALIVE_THREAD_NAME,
			       ALIVE_THREAD_FREQUENCY);    
    m_alive_thread_auto =
      auto_ptr<redrobd_alive_thread>(thread_ptr1);
    
    redrobd_log_writeln("About to initialize alive thread");

    // Take back ownership from auto_ptr
    thread_ptr1 = m_alive_thread_auto.release();
    
    try {
       // Initialize cyclic alive thread object
      redrobd_thread_initialize((thread *)thread_ptr1,
				ALIVE_THREAD_START_TIMEOUT,
				ALIVE_THREAD_EXECUTE_TIMEOUT);
    }
    catch (...) {
      m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);
      throw;
    }

    // Give back ownership to auto_ptr
    m_alive_thread_auto = auto_ptr<redrobd_alive_thread>(thread_ptr1);

    /////////////////////////////////
    //  INITIALIZE A/D Converter
    /////////////////////////////////

    // Create the A/D Converter object
    m_mcp3008_io_ptr = new mcp3008_io(MCP3008_SPI_DEV,
				      MCP3008_REF_VOLTAGE);
    
    // Initialize A/D Converter
    m_mcp3008_io_ptr->initialize(MCP3008_SPI_SPEED);
    
    /////////////////////////////////
    //  INITIALIZE HW configuration
    /////////////////////////////////

    // Create the hardware configuration object object with garbage collector
    redrobd_hw_cfg *redrobd_hw_cfg_ptr =
      new redrobd_hw_cfg(m_mcp3008_io_ptr,
			 (MCP3008_IO_CHANNEL)MCP3008_CHN_SHUTDOWN,
			 (MCP3008_IO_CHANNEL)MCP3008_CHN_CONT_STEER);

    m_hw_cfg_auto = auto_ptr<redrobd_hw_cfg>(redrobd_hw_cfg_ptr);

    // Initialize hardware configuration
    m_hw_cfg_auto->initialize();

    /////////////////////////////////
    //  INITIALIZE remote control
    /////////////////////////////////

    // Create the remote control object with garbage collector (RF, Radio)
    redrobd_rc_rf *rc_rf_ptr =
      new redrobd_rc_rf(PIN_RF_IN_3,  // Forward
			PIN_RF_IN_2,  // Reverse
			PIN_RF_IN_0,  // Right
			PIN_RF_IN_1); // Left

    m_rc_rf_auto = auto_ptr<redrobd_rc_rf>(rc_rf_ptr);

    // Initialize remote control (RF, Radio)
    m_rc_rf_auto->initialize();

    // Create the remote control object with garbage collector (NET, Sockets)
    redrobd_rc_net *rc_net_ptr =
      new redrobd_rc_net(RC_NET_SERVER_IP,    // Server local IP address
			 RC_NET_SERVER_PORT); // Server local port

    m_rc_net_auto = auto_ptr<redrobd_rc_net>(rc_net_ptr);

    // Initialize remote control (NET, Sockets)
    m_rc_net_auto->initialize();
    m_rc_net_auto->set_voltage(0.0);

    /////////////////////////////////
    //  INITIALIZE camera control
    /////////////////////////////////

    // Create the camera control object with garbage collector
    redrobd_camera_ctrl *cc_ptr = new redrobd_camera_ctrl();
    m_cc_auto = auto_ptr<redrobd_camera_ctrl>(cc_ptr);

    // Initialize camera control
    m_cc_auto->initialize();
    
    /////////////////////////////////
    //  INITIALIZE motor control
    /////////////////////////////////

    // Check if continuous steering was selected (DIP-switch)
    m_cont_steering = m_hw_cfg_auto->select_continuous_steering();
    if (m_cont_steering) {
      redrobd_log_writeln(get_name() + " : Continuous steering selected");      
    }
    else {
      redrobd_log_writeln(get_name() + " : Non-continuous steering selected");
    }

    // Create the motor control object with garbage collector
    if (m_cont_steering) {
      redrobd_mc_cont_steer *mc_ptr =
	new redrobd_mc_cont_steer(PIN_L293D_1A,  // Right motor
				  PIN_L293D_2A,
				  PIN_L293D_3A,  // Left motor
				  PIN_L293D_4A);
      
      m_mc_cont_steer_auto = auto_ptr<redrobd_mc_cont_steer>(mc_ptr);
    }
    else {
      redrobd_mc_non_cont_steer *mc_ptr =
	new redrobd_mc_non_cont_steer(PIN_L293D_1A,  // Right motor
				      PIN_L293D_2A,
				      PIN_L293D_3A,  // Left motor
				      PIN_L293D_4A);
      
      m_mc_non_cont_steer_auto = auto_ptr<redrobd_mc_non_cont_steer>(mc_ptr);
    }

    // Initialize motor control
    if (m_cont_steering) {
      m_mc_cont_steer_auto->initialize();
    }
    else {
      m_mc_non_cont_steer_auto->initialize();
    }

    /////////////////////////////////
    //  INITIALIZE battery monitor
    /////////////////////////////////

    // Create the cyclic battery monitor thread object with garbage collector
    redrobd_voltage_monitor_thread *thread_ptr2 =
      new redrobd_voltage_monitor_thread(BAT_MON_THREAD_NAME,
					 BAT_MON_THREAD_FREQUENCY,
					 m_mcp3008_io_ptr,
					 (MCP3008_IO_CHANNEL)MCP3008_CHN_VBAT,
					 MCP3008_CHN_VBAT_SF);    
    m_bat_mon_thread_auto =
      auto_ptr<redrobd_voltage_monitor_thread>(thread_ptr2);

    redrobd_log_writeln("About to initialize battery monitor thread");

    // Take back ownership from auto_ptr
    thread_ptr2 = m_bat_mon_thread_auto.release();
    
    try {
      // Initialize cyclic battery monitor thread object
      redrobd_thread_initialize((thread *)thread_ptr2,
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

    // Start timer controlling when to check system stats
    if (m_sys_stat_check_timer.reset() != TIMER_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
		"Error resetting system stats check timer for thread %s",
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
    //  FINALIZE battery monitor
    ////////////////////////////////////////
    redrobd_log_writeln("About to finalize battery monitor thread");

    // Take back ownership from auto_ptr
    redrobd_voltage_monitor_thread *thread_ptr2 =
      m_bat_mon_thread_auto.release();
    
    try {
      // Finalize the cyclic battery monitor thread object
      redrobd_thread_finalize((thread *)thread_ptr2,
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

    ////////////////////////////////////////
    //  FINALIZE motor control
    ////////////////////////////////////////

    // Finalize and delete the motor control object
    if (m_cont_steering) {
      m_mc_cont_steer_auto->finalize();
      m_mc_cont_steer_auto.reset();
    }
    else {
      m_mc_non_cont_steer_auto->finalize();
      m_mc_non_cont_steer_auto.reset();
    }

    ////////////////////////////////////////
    //  FINALIZE camera control
    ////////////////////////////////////////

    // Finalize and delete the camera control object
    m_cc_auto->finalize();
    m_cc_auto.reset();

    ////////////////////////////////////////
    //  FINALIZE remote control
    ////////////////////////////////////////

    // Finalize and delete the remote control objects
    m_rc_net_auto->finalize();
    m_rc_net_auto.reset();

    m_rc_rf_auto->finalize();   
    m_rc_rf_auto.reset();

    
    ////////////////////////////////////////
    //  FINALIZE HW configuration
    ////////////////////////////////////////    

    // Finalize and delete the hardware configuration object
    m_hw_cfg_auto->finalize();
    m_hw_cfg_auto.reset();

    ////////////////////////////////////////
    //  FINALIZE A/D Converter
    ////////////////////////////////////////

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
      redrobd_thread_finalize((thread *)thread_ptr1,
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
    if ( !battery_voltage_ok() ) {
      redrobd_led_bat_low(true);   // Turn status LED on
    }
    else {
      redrobd_led_bat_low(false);  // Turn status LED off
    }

    // Check system stats
    check_system_stats();

    // Check state and status of created threads
    check_thread_run_status();

    // Check remote control steering
    uint16_t steering = get_remote_steering();

    // Do motor control
    switch (steering) {
    case REDROBD_RC_STEER_NONE:
      motor_control(REDROBD_MC_NONE);
      break;
    case REDROBD_RC_STEER_FORWARD:
      if (m_verbose) {
	redrobd_log_writeln(get_name() + " : steer forward");
      }
      motor_control(REDROBD_MC_FORWARD);
      break;
    case REDROBD_RC_STEER_REVERSE:
      if (m_verbose) {
	redrobd_log_writeln(get_name() + " : steer reverse");
      }
      motor_control(REDROBD_MC_REVERSE);
      break; 
    case REDROBD_RC_STEER_RIGHT:      
      if (m_verbose) {
	redrobd_log_writeln(get_name() + " : steer right");
      }
      motor_control(REDROBD_MC_RIGHT);
      break;
    case REDROBD_RC_STEER_LEFT:
      if (m_verbose) {
	redrobd_log_writeln(get_name() + " : steer left");
      }
      motor_control(REDROBD_MC_LEFT);
      break;
    default:
      // All other steerings are ignored for now
       ostringstream oss_msg;
       oss_msg << get_name()
	       << " : Got undefined steering = 0x"
	       << hex << setw(4) << setfill('0') << (unsigned)steering;
       redrobd_log_writeln(oss_msg.str());
       oss_msg.str("");

       motor_control(REDROBD_MC_STOP); 
    }

    // Check remote control camera code
    uint16_t camera_code = m_rc_net_auto->get_camera_code();

    // Do camera control
    switch (camera_code) {
    case REDROBD_RC_CAMERA_NONE:
      camera_control(REDROBD_CC_NONE);
      break;
    case REDROBD_RC_CAMERA_STOP_STREAM:
      camera_control(REDROBD_CC_STOP_STREAM);
      break;
    case REDROBD_RC_CAMERA_START_STREAM:
      camera_control(REDROBD_CC_START_STREAM);
      break; 
    default:
      // All other camera codes are ignored for now
       ostringstream oss_msg;
       oss_msg << get_name()
	       << " : Got undefined camera code = 0x"
	       << hex << setw(4) << setfill('0') << (unsigned)steering;
       redrobd_log_writeln(oss_msg.str());
       oss_msg.str("");
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
  m_alive_thread_auto.reset();
  m_bat_mon_thread_auto.reset();
  m_rc_rf_auto.reset();
  m_rc_net_auto.reset();
  m_cc_auto.reset();
  m_mc_cont_steer_auto.reset();
  m_mc_non_cont_steer_auto.reset();

  m_mcp3008_io_ptr = NULL;

  m_hw_cfg_auto.reset();

  m_battery_check_allowed = false;

  m_shutdown_select = false;

  m_cont_steering = false;
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

    // Update voltage for remote control (NET, Sockets)
    m_rc_net_auto->set_voltage(0.0);
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

    // Update voltage for remote control (NET, Sockets)
    m_rc_net_auto->set_voltage(v_bat.v_in);
  }

  return battery_ok;
}

////////////////////////////////////////////////////////////////

void redrobd_ctrl_thread::check_system_stats(void)
{
  if ( m_sys_stat_check_timer.get_elapsed_time() <
	 (1.0/SYS_STAT_CHECK_FREQUENCY) ) {
    return;
  }

  long rc;

  // Get system stats (Linux)
  float cpu_load;
  unsigned mem_used;
  unsigned irq;
  unsigned uptime;
  
  rc = m_sys_stat.get_interval_cpu_load(cpu_load);
  if (rc != SYS_STAT_SUCCESS) {      
    cpu_load = 0;
  }
  
  rc = m_sys_stat.get_mem_used_kb(mem_used);
  if (rc != SYS_STAT_SUCCESS) {
    mem_used = 0;
  }
  
  rc = m_sys_stat.get_interval_irq(irq);
  if (rc != SYS_STAT_SUCCESS) {
    irq = 0;
  }
  
  rc = m_sys_stat.get_uptime_sec(uptime);
  if (rc != SYS_STAT_SUCCESS) {
    uptime = 0;
  }
  
  // Get system stats (Raspberry Pi)
  float cpu_temp;
  float cpu_voltage;
  unsigned cpu_freq;
  
  rc = m_rpi_stat.get_temperature(cpu_temp);
  if (rc != RPI_STAT_SUCCESS) {      
    cpu_temp = 0.0;
  }
  
  rc = m_rpi_stat.get_voltage(RPI_STAT_VOLT_ID_CORE,
			      cpu_voltage);
  if (rc != RPI_STAT_SUCCESS) {      
    cpu_voltage = 0.0;
  }
  
  rc = m_rpi_stat.get_frequency(RPI_STAT_FREQ_ID_ARM,
				cpu_freq);
  if (rc != RPI_STAT_SUCCESS) {      
    cpu_freq = 0;
  }
  
  // Reset system stats interval
  rc = m_sys_stat.reset_interval_cpu_load();
  if (rc != SYS_STAT_SUCCESS) {
    if (rc != SYS_STAT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SYS_STAT_OPERATION_FAILED,
		"Error resetting system stats interval(cpu_load) for thread %s",
		get_name().c_str());  
    }
  }
  
  rc = m_sys_stat.reset_interval_irq();
  if (rc != SYS_STAT_SUCCESS) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SYS_STAT_OPERATION_FAILED,
	      "Error resetting system stats interval(irq) for thread %s",
	      get_name().c_str());  
  }
  
  // Update system stats for remote control (NET, Sockets)
  m_rc_net_auto->set_sys_stat((uint8_t)cpu_load,
			      (uint32_t)mem_used,
			      (uint16_t)irq,
			      (uint32_t)uptime,
			      (uint32_t)(cpu_temp * 1000.0),
			      (uint16_t)(cpu_voltage * 1000.0),
			      (uint16_t)((float)(cpu_freq) / 1000000.0));        
  // Reset timer
  if (m_sys_stat_check_timer.reset() != TIMER_SUCCESS) {
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_TIME_ERROR,
	      "Error resetting system stats check timer for thread %s",
	      get_name().c_str());   
  }
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
    redrobd_thread_check((thread *)thread_ptr1);
  }
  catch (...) {
    m_alive_thread_auto =
      auto_ptr<redrobd_alive_thread>(thread_ptr1);
    throw;
  }

  // Give back ownership to auto_ptr
  m_alive_thread_auto =
    auto_ptr<redrobd_alive_thread>(thread_ptr1);

  /////////////////////////////////////////////////
  //  CHECK REMOTE CONTROL (NET, SOCKETS) THREAD
  /////////////////////////////////////////////////

  // Check state and status of server thread object
  m_rc_net_auto->server_thread_check();

  ///////////////////////////////////
  //  CHECK BATTERY MONITOR THREAD   
  ///////////////////////////////////

  // Take back ownership from auto_ptr
  redrobd_voltage_monitor_thread *thread_ptr2 =
    m_bat_mon_thread_auto.release();

  try {
    // Check state and status of alive thread object
    redrobd_thread_check((thread *)thread_ptr2);
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

////////////////////////////////////////////////////////////////

uint16_t redrobd_ctrl_thread::get_remote_steering(void)
{
  uint16_t steering_rf;
  uint16_t steering_net;
  uint16_t steering;

  // Get steerings from remote control objects
  steering_rf = m_rc_rf_auto->get_steering();
  steering_net = m_rc_net_auto->get_steering();

  // (RF, Radio) has highest priority
  if (m_rc_rf_auto->is_active()) {
    steering = steering_rf;
    if (m_verbose) {
      redrobd_log_writeln(get_name() + " : remote RF active");
    }
  }
  else if (m_rc_net_auto->is_active()) {
    steering = steering_net;
    if (m_verbose) {
      redrobd_log_writeln(get_name() + " : remote NET active");
    }
  }
  else {
    steering = REDROBD_RC_STEER_NONE;
    if (m_verbose) {
      redrobd_log_writeln(get_name() + " : remote NONE active");
    }
  }

  return steering;
}

////////////////////////////////////////////////////////////////

void redrobd_ctrl_thread::motor_control(uint16_t steer_code)
{
  if (m_cont_steering) {
    m_mc_cont_steer_auto->steer(steer_code);
  }
  else {
    m_mc_non_cont_steer_auto->steer(steer_code);
  }
}

////////////////////////////////////////////////////////////////

void redrobd_ctrl_thread::camera_control(uint16_t camera_code)
{
  m_cc_auto->command(camera_code);
}
