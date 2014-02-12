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

#ifndef __MCP3008_IO_H__
#define __MCP3008_IO_H__

#include <pthread.h>

#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef enum {MCP3008_IO_CH0,
	      MCP3008_IO_CH1,
	      MCP3008_IO_CH2,
	      MCP3008_IO_CH3,
	      MCP3008_IO_CH4,
	      MCP3008_IO_CH5,
	      MCP3008_IO_CH6,
	      MCP3008_IO_CH7} MCP3008_IO_CHANNEL;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class mcp3008_io {
  
 public:
  mcp3008_io(string spi_dev, float vref);
  ~mcp3008_io(void);

  void initialize(uint32_t speed);
  void finalize(void);

  void read_single(MCP3008_IO_CHANNEL channel,
		   uint16_t &value);

  float to_voltage(uint16_t value);

 private:  
  string          m_spi_dev;
  int             m_spi_fd;
  pthread_mutex_t m_xfer_mutex;
  float           m_vref;

  void init_members(void);

  void spi_xfer(const void *tx_buf,
		void *rx_buf,
		uint32_t nbytes);
};

#endif // __MCP3008_IO_H__
