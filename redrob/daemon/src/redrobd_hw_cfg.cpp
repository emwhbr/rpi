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

#include "redrobd_hw_cfg.h"

// Implementation notes:
// 1. All configuration (input) channels for the MCP3008
//    shall have the following values:
//    - High : pull-up(3.3V)
//    - Low  : pull-down(0V). 
//
// 2. The definition of true/false for each channel is
//    done separately. But the default condition shall be
//    considered to be pull-up(3.3V).
//
// 3. Assumes the MCP3008 interface already initialized.
//

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define MIN_HIGH_VOLTAGE  2.0 // Volt

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_hw_cfg::redrobd_hw_cfg(mcp3008_io *mcp3008_io_ptr,
			       MCP3008_IO_CHANNEL mcp3008_io_chn_shutdown,
			       MCP3008_IO_CHANNEL mcp3008_io_chn_cont_steer)
{
  m_mcp3008_io_ptr = mcp3008_io_ptr;
  m_mcp3008_io_chn_shutdown   = mcp3008_io_chn_shutdown;
  m_mcp3008_io_chn_cont_steer = mcp3008_io_chn_cont_steer;

  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_hw_cfg::~redrobd_hw_cfg(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_hw_cfg::initialize(void)
{
}

////////////////////////////////////////////////////////////////

void redrobd_hw_cfg::finalize(void)
{
}

////////////////////////////////////////////////////////////////

bool redrobd_hw_cfg::select_shutdown(void)
{
  // Shutdown: Pull-up   : False
  //           Pull-down : True
  if (adc_channel_high(m_mcp3008_io_chn_shutdown)) {
    return false;
  }
  else {
    return true;
  }
}

////////////////////////////////////////////////////////////////

bool redrobd_hw_cfg::select_continuous_steering(void)
{
  // Continuous: Pull-up   : True
  //             Pull-down : False
  if (adc_channel_high(m_mcp3008_io_chn_cont_steer)) {
    return true;
  }
  else {
    return false;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_hw_cfg::init_members(void)
{
}

////////////////////////////////////////////////////////////////

bool redrobd_hw_cfg::adc_channel_high(MCP3008_IO_CHANNEL chn)
{
  uint16_t adc_value;
  float v_chn;

  // Get voltage from analog input
  m_mcp3008_io_ptr->read_single(chn, adc_value);
  v_chn = m_mcp3008_io_ptr->to_voltage(adc_value);

  // Check voltage above level for 'logic 1'
  if (v_chn > MIN_HIGH_VOLTAGE) {
    return true;
  }
  else {
    return false;
  }
}
