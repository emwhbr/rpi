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

#include "redrobd.h"
#include "redrobd_core.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static redrobd_core g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long redrobd_get_prod_info(REDROBD_PROD_INFO *prod_info)
{
  return g_object.get_prod_info(prod_info);
}

////////////////////////////////////////////////////////////////

long redrobd_get_config(REDROBD_CONFIG *config)
{
  return g_object.get_config(config);
}

////////////////////////////////////////////////////////////////

long redrobd_get_last_error(REDROBD_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long redrobd_check_run_status(void)
{
  return g_object.check_run_status();
}

////////////////////////////////////////////////////////////////

long redrobd_initialize(const char *logfile,
			bool log_stdout,
			double ctrl_thread_frequency,
			bool verbose)
{
  return g_object.initialize(logfile,
			     log_stdout,
			     ctrl_thread_frequency,
			     verbose);
}

////////////////////////////////////////////////////////////////

long redrobd_finalize(void)
{
  return g_object.finalize();
}
