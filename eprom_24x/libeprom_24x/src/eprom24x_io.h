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

#ifndef __EPROM24x_IO_H__
#define __EPROM24x_IO_H__

#include <pthread.h>
#include <string>

#include "eprom24x.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class eprom24x_io {

public:
  eprom24x_io(EPROM24x_DEVICE eprom_device,
	      uint8_t i2c_address,
	      const char *i2c_dev);
  ~eprom24x_io(void);

  void initialize(void);
  void finalize(void);

  long read(uint32_t addr, void *data, uint16_t len);
  long write(uint32_t addr, const void *data, uint16_t len);

private:
  bool     	   m_eprom_supported;
  uint8_t  	   m_nr_address_bytes;
  uint32_t 	   m_eprom_size_in_bytes;
  double   	   m_page_write_time;
  uint32_t 	   m_page_size_in_bytes;
  uint8_t  	   *m_page_write_buffer;
  pthread_mutex_t  m_rw_mutex;
  uint8_t  	   m_i2c_address;
  string   	   m_i2c_dev;
  int      	   m_i2c_fd;

  void init_members(void);

  void check_valid_address(uint32_t addr, uint16_t bytes_to_access);

  void read_page_data(uint32_t addr, uint8_t *data, uint16_t len);

  void write_page(uint32_t addr, const uint8_t *data, uint16_t len);
  void write_page_data(uint32_t addr, const uint8_t *data, uint16_t len);
  void write_byte(uint32_t addr, const uint8_t *data, uint16_t len);
  void write_byte_data(uint32_t addr, const uint8_t *value);

  bool eprom_ready(void);
  void wait_eprom_ready(double timeout_in_sec,
			double poll_interval_in_sec);
};

#endif // __EPROM24x_IO_H__
