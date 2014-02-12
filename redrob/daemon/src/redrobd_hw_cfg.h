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

#ifndef __REDROBD_HW_CFG_H__
#define __REDROBD_HW_CFG_H__

#include "mcp3008_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class redrobd_hw_cfg {

 public:
  redrobd_hw_cfg(mcp3008_io *mcp3008_io_ptr,
		 MCP3008_IO_CHANNEL mcp3008_io_chn_shutdown,
		 MCP3008_IO_CHANNEL mcp3008_io_chn_cont_steer);

  ~redrobd_hw_cfg(void);

  void initialize(void);
  void finalize(void);

  bool select_shutdown(void);
  bool select_continuous_steering(void);
    
 private:
  // A/D Converter object pointer
  mcp3008_io *m_mcp3008_io_ptr;

  // A/D Converter channels
  MCP3008_IO_CHANNEL m_mcp3008_io_chn_shutdown;
  MCP3008_IO_CHANNEL m_mcp3008_io_chn_cont_steer;

  void init_members(void);

  bool adc_channel_high(MCP3008_IO_CHANNEL chn);
};

#endif // __REDROBD_HW_CFG_H__
