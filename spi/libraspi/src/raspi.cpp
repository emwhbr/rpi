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

#include "raspi.h"
#include "raspi_core.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static raspi_core g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long raspi_get_last_error(RASPI_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long raspi_get_error_string(long error_code,
			    RASPI_ERROR_STRING error_string)
{
  return g_object.get_error_string(error_code, error_string);  
}

////////////////////////////////////////////////////////////////

long raspi_initialize(RASPI_CE ce,
		      RASPI_MODE mode,
		      RASPI_BPW bpw,
		      uint32_t speed)
{
  return g_object.initialize(ce,
			     mode,
			     bpw,
			     speed);
}

////////////////////////////////////////////////////////////////

long raspi_finalize(RASPI_CE ce)
{
  return g_object.finalize(ce);
}

////////////////////////////////////////////////////////////////

long raspi_xfer(RASPI_CE ce,
		const void *tx_buf,
		void *rx_buf,
		uint32_t nbytes)
{
  return g_object.xfer(ce,
		       tx_buf,
		       rx_buf,
		       nbytes);
}

////////////////////////////////////////////////////////////////

long raspi_test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info)
{
  return g_object.test_get_lib_prod_info(prod_info);
}
