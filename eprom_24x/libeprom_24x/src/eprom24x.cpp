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

#include "eprom24x.h"
#include "eprom24x_core.h"

/////////////////////////////////////////////////////////////////////////////
//               Module global variables
/////////////////////////////////////////////////////////////////////////////
static eprom24x_core g_object;

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long eprom24x_get_last_error(EPROM24x_STATUS *status)
{
  return g_object.get_last_error(status);
}

////////////////////////////////////////////////////////////////

long eprom24x_get_error_string(long error_code,
			       EPROM24x_ERROR_STRING error_string)
{
  return g_object.get_error_string(error_code, error_string);  
}

////////////////////////////////////////////////////////////////

long eprom24x_initialize(EPROM24x_DEVICE eprom_device,
			 uint8_t i2c_address,
			 const char *i2c_dev)
{
  return g_object.initialize(eprom_device,
			     i2c_address,
			     i2c_dev);
}

////////////////////////////////////////////////////////////////

long eprom24x_finalize(void)
{
  return g_object.finalize();
}

////////////////////////////////////////////////////////////////

long eprom24x_read_u8(uint32_t addr, uint8_t *value)
{
  return g_object.read_u8(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_read_u16(uint32_t addr, uint16_t *value)
{
  return g_object.read_u16(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_read_u32(uint32_t addr, uint32_t *value)
{
  return g_object.read_u32(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_read(uint32_t addr, void *data, uint16_t len)
{
  return g_object.read(addr, data, len);
}

////////////////////////////////////////////////////////////////

long eprom24x_write_u8(uint32_t addr, uint8_t value)
{
  return g_object.write_u8(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_write_u16(uint32_t addr, uint16_t value)
{
  return g_object.write_u16(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_write_u32(uint32_t addr, uint32_t value)
{
  return g_object.write_u32(addr, value);
}

////////////////////////////////////////////////////////////////

long eprom24x_write(uint32_t addr, const void *data, uint16_t len)
{
  return g_object.write(addr, data, len);
}

////////////////////////////////////////////////////////////////

long eprom24x_test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info)
{
  return g_object.test_get_lib_prod_info(prod_info);
}
