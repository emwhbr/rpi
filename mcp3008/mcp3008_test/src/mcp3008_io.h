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

#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define MCP3008_IO_SUCCESS                 0
#define MCP3008_IO_FILE_OPERATION_FAILED  -1
#define MCP3008_IO_UNEXPECTED_STATE       -2
#define MCP3008_IO_SPI_OPERATION_FAILED   -3

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
  mcp3008_io(string spi_dev,
	     float vref);
  ~mcp3008_io(void);

  long initialize(uint32_t speed);
  long finalize(void);

  long read_single(MCP3008_IO_CHANNEL channel,
		   uint16_t &value);

  float to_voltage(uint16_t value);

 private:  
  string m_spi_dev;
  int    m_spi_fd;
  float  m_vref;

  void init_members(void);

  long spi_xfer(const void *tx_buf,
		void *rx_buf,
		uint32_t nbytes);
};

#endif // __MCP3008_IO_H__
