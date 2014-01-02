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

#include <string.h>

#include "redrobd_core.h"
#include "redrobd_log.h"
#include "redrobd_cfg_file.h"
#include "redrobd_error_utility.h"
#include "redrobd_thread_utility.h"
#include "redrobd_gpio.h"
#include "redrobd_led.h"
#include "daemon_utility.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PRODUCT_NUMBER   "REDROBD"
#define RSTATE           "R1A02"

#ifndef REDROBD_CFG_FILE
#define CFG_FILE "/tmp/"REDROBD_NAME".cfg"
#endif

#define CTRL_THREAD_NAME             "REDROBD_CTRL"
#define CTRL_THREAD_START_TIMEOUT     3.0  // Seconds
#define CTRL_THREAD_EXECUTE_TIMEOUT   0.5  // Seconds
#define CTRL_THREAD_STOP_TIMEOUT     10.0  // Seconds
                                           // Long enough for controller and
                                           // its child threads to complete

#define MUTEX_LOCK(mutex) \
  ({ if (pthread_mutex_lock(&mutex)) { \
      return REDROBD_MUTEX_FAILURE; \
    } })

#define MUTEX_UNLOCK(mutex) \
  ({ if (pthread_mutex_unlock(&mutex)) { \
      return REDROBD_MUTEX_FAILURE; \
    } })

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

redrobd_core::redrobd_core(void)
{
  m_error_source    = REDROBD_INTERNAL_ERROR;
  m_error_code      = REDROBD_NO_ERROR;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  m_led_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL); // Use default mutex attributes
}

/////////////////////////////////////////////////////////////////////////////

redrobd_core::~redrobd_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::get_prod_info(REDROBD_PROD_INFO *prod_info)
{
  try {
    // Do the actual work
    return internal_get_prod_info(prod_info);
  }
  catch (...) {
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::get_config(REDROBD_CONFIG *config)
{
  try {
    // Do the actual work
    return internal_get_config(config);
  }
  catch (excep &exp) {
    return set_error(exp);
  }
  catch (...) {
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::get_last_error(REDROBD_STATUS *status)
{
  try {
    MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = REDROBD_INTERNAL_ERROR;
    m_error_code      = REDROBD_NO_ERROR;
    m_last_error_read = true;
    MUTEX_UNLOCK(m_error_mutex);
    return REDROBD_SUCCESS;
  }
  catch (...) {
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::check_run_status(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    internal_check_run_status();

    // Check completed
    MUTEX_UNLOCK(m_init_mutex);

    return REDROBD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::initialize(string logfile,
			      bool log_stdout,
			      double ctrl_thread_frequency)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if already initialized
    if (m_initialized) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_ALREADY_INITIALIZED,
		"Already initialized");
    }

    // Check input values
    if (ctrl_thread_frequency < 0.0) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_BAD_ARGUMENT,
		"Illegal ctrl thread frequency (%f)",
		ctrl_thread_frequency);
    }

    // Do the actual initialization
    internal_initialize(logfile,
			log_stdout,
			ctrl_thread_frequency);

    // Initialization completed
    m_initialized = true;
    MUTEX_UNLOCK(m_init_mutex);

    return REDROBD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::finalize(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual finalization
    internal_finalize();

    // Finalization completed
    m_initialized = false;
    MUTEX_UNLOCK(m_init_mutex);

    return REDROBD_SUCCESS;
  }
  catch (excep &exp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(exp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(EXP(REDROBD_INTERNAL_ERROR, REDROBD_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::set_error(excep exp)
{
  // Turn status LED on
  if (m_led_initialized) {
    redrobd_led_sysfail(true);
  }

  // Get syslog friendly error string
  string error_string = redrobd_error_syslog_string(exp);
  
  // Print all info
  syslog_error(error_string.c_str());

  // Update internal error information
  return update_error(exp);
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::update_error(excep exp)
{
  MUTEX_LOCK(m_error_mutex);
  if (m_last_error_read) {
    m_error_source    = (REDROBD_ERROR_SOURCE)exp.get_source();
    m_error_code      = exp.get_code();
    m_last_error_read = false; // Latch last error until read
  }
  MUTEX_UNLOCK(m_error_mutex);

  return REDROBD_FAILURE;
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::internal_get_prod_info(REDROBD_PROD_INFO *prod_info)
{
  long rc = REDROBD_SUCCESS;
 
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((REDROBD_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((REDROBD_PROD_INFO *)0)->rstate));

  return rc;
}

/////////////////////////////////////////////////////////////////////////////

long redrobd_core::internal_get_config(REDROBD_CONFIG *config)
{
  long rc;
  redrobd_cfg_file *cfg_f = new redrobd_cfg_file(CFG_FILE);

  // Parse configuration file
  rc = cfg_f->parse();
  if ( (rc != CFG_FILE_SUCCESS) &&         // Use values from file
       (rc != CFG_FILE_FILE_NOT_FOUND) ) { // Use default values
    delete cfg_f;
    switch(rc) {
    case CFG_FILE_FILE_IO_ERROR:
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_FILE_OPERATION_FAILED,
		"I/O error parsing config file %s",
		CFG_FILE);
      break;
    case CFG_FILE_BAD_FILE_FORMAT:
    case CFG_FILE_BAD_VALUE_FORMAT:
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_BAD_FORMAT,
		"Bad format parsing config file %s",
		CFG_FILE);
      break;
    default:
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
		"Unexpected error(%ld) parsing config file %s",
		rc, CFG_FILE);
    }
  }

  // Get configuration values
  bool daemonize;
  rc = cfg_f->get_daemonize(daemonize);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_daemonize", rc);
  }
  string user;
  rc = cfg_f->get_user_name(user);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_user_name", rc);
  }
  string work_dir;
  rc = cfg_f->get_work_dir(work_dir);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_work_dir", rc);
  }
  string lock_file;
  rc = cfg_f->get_lock_file(lock_file);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_lock_file", rc);
  }
  string log_file;
  rc = cfg_f->get_log_file(log_file);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_log_file", rc);
  }
  bool log_stdout;
  rc = cfg_f->get_log_stdout(log_stdout);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_log_stdout", rc);
  }
  double s_freq;
  rc = cfg_f->get_supervision_freq(s_freq);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_supervison_freq", rc);
  }
  double wt_freq;
  rc = cfg_f->get_ctrl_thread_freq(wt_freq);
  if (rc != CFG_FILE_SUCCESS) {
    delete cfg_f;
    THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_CFG_FILE_UNEXCPECTED_ERROR,
	      "Unexpected error(%ld) get_ctrl_thread_freq", rc);
  }
  
  // Copy configuration values to caller
  config->daemonize = daemonize;
  strncpy(config->user,      user.c_str(),      sizeof(REDROBD_STRING));
  strncpy(config->work_dir,  work_dir.c_str(),  sizeof(REDROBD_STRING));
  strncpy(config->lock_file, lock_file.c_str(), sizeof(REDROBD_STRING));
  strncpy(config->log_file,  log_file.c_str(),  sizeof(REDROBD_STRING));
  config->log_stdout = log_stdout;
  config->supervision_freq = s_freq;
  config->ctrl_thread_freq = wt_freq;
  
  delete cfg_f;

  return REDROBD_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

void redrobd_core::internal_check_run_status(void)
{
  //////////////////////////////
  //  CHECK CONTROL THREAD   
  //////////////////////////////

  // Take back ownership from auto_ptr
  redrobd_ctrl_thread *thread_ptr = m_ctrl_thread_auto.release();

  try {
    // Check state and status of cyclic control thread object
    redrobd_thread_check_cyclic((cyclic_thread *)thread_ptr);
  }
  catch (...) {
    m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);
}

/////////////////////////////////////////////////////////////////////////////

void redrobd_core::internal_initialize(string logfile,
				       bool log_stdout,
				       double ctrl_thread_frequency)
{
  // Initialize the logfile singleton object
  redrobd_log_initialize(logfile, log_stdout);

  // Initialize GPIO
  redrobd_gpio_initialize();

  // Initialize LEDs
  // Note! This must be done after initialization of GPIO
  redrobd_led_initialize();
  m_led_initialized = true;

  // Create the cyclic control thread object with garbage collector
  redrobd_ctrl_thread *thread_ptr = 
    new redrobd_ctrl_thread(CTRL_THREAD_NAME,
			    ctrl_thread_frequency);
  m_ctrl_thread_auto =
    auto_ptr<redrobd_ctrl_thread>(thread_ptr);

  /////////////////////////////////////
  //  INITIALIZE CONTROL THREAD
  /////////////////////////////////////
  redrobd_log_writeln("About to initialize ctrl thread");

  // Take back ownership from auto_ptr
  thread_ptr = m_ctrl_thread_auto.release();

  try {
    // Initialize cyclic control thread object
    redrobd_thread_initialize_cyclic((cyclic_thread *)thread_ptr,
				     CTRL_THREAD_START_TIMEOUT,
				     CTRL_THREAD_EXECUTE_TIMEOUT);
  }
  catch (...) {
    m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);
    throw;
  }
  
  // Give back ownership to auto_ptr
  m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);
}

/////////////////////////////////////////////////////////////////////////////

void redrobd_core::internal_finalize(void)
{
  /////////////////////////////////////
  //  FINALIZE CONTROL THREAD
  /////////////////////////////////////
  redrobd_log_writeln("About to finalize ctrl thread");

  // Take back ownership from auto_ptr
  redrobd_ctrl_thread *thread_ptr = m_ctrl_thread_auto.release();

  try {
    // Finalize the cyclic control thread object
    redrobd_thread_finalize_cyclic((cyclic_thread *)thread_ptr,
				   CTRL_THREAD_STOP_TIMEOUT);
  }
  catch (...) {
    m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);
    throw;
  }

  // Give back ownership to auto_ptr
  m_ctrl_thread_auto = auto_ptr<redrobd_ctrl_thread>(thread_ptr);

  // Delete the cyclic control thread object
  m_ctrl_thread_auto.reset();

  // Finalize LEDs
  // Note! This must be done before finalization of GPIO
  redrobd_led_finalize();
  m_led_initialized = false;

  // Finalize GPIO
  redrobd_gpio_finalize(); 

  // Finalize the logfile singleton object
  redrobd_log_finalize();
}
