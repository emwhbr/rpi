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

#ifndef __REDROBD_CORE_H__
#define __REDROBD_CORE_H__

#include <pthread.h>
#include <memory>

#include "redrobd.h"
#include "redrobd_ctrl_thread.h"
#include "excep.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_core {

public:
  redrobd_core(void);
  ~redrobd_core(void);

  long get_prod_info(REDROBD_PROD_INFO *prod_info);

  long get_config(REDROBD_CONFIG *config);

  long get_last_error(REDROBD_STATUS *status);

  long check_run_status(void);

  long initialize(string logfile,
		  bool log_stdout,
		  double ctrl_thread_frequency,
		  bool verbose);

  long finalize(void);

private:
  // Error handling information
  REDROBD_ERROR_SOURCE  m_error_source;
  long                  m_error_code;
  bool                  m_last_error_read;
  pthread_mutex_t       m_error_mutex;

  // Keep track of initialization
  bool             m_initialized;
  pthread_mutex_t  m_init_mutex;
  bool             m_led_initialized;

  // The cyclic control thread object
  auto_ptr<redrobd_ctrl_thread> m_ctrl_thread_auto;

  // Private member functions
  long set_error(excep exp);
  long update_error(excep exp);

  long internal_get_prod_info(REDROBD_PROD_INFO *prod_info);

  long internal_get_config(REDROBD_CONFIG *config);

  void internal_check_run_status(void);

  void internal_initialize(string logfile,
			   bool log_stdout,
			   double ctrl_thread_frequency,
			   bool verbose);

  void internal_finalize(void);  
};

#endif // __REDROBD_CORE_H__
