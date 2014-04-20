// ************************************************************************
// *                                                                      *
// * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <stdio.h>

#include "rpi_stat.h"
#include "shell_cmd.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define VCGENCMD "/usr/bin/vcgencmd"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

rpi_stat::rpi_stat(void)
{
}

////////////////////////////////////////////////////////////////

rpi_stat::~rpi_stat(void)
{
}

////////////////////////////////////////////////////////////////

long rpi_stat::get_temperature(float &value)
{
  const string cmd = string(VCGENCMD) + string(" measure_temp");
  shell_cmd rpi_cmd;  
  
  // Execute shell command
  string output;
  if (rpi_cmd.execute(cmd, output) != SHELL_CMD_SUCCESS) {
    return RPI_STAT_CMD_FAILED;
  }

  // Extract result from command output  
  if (sscanf(output.c_str(), "temp=%f'C", &value) != 1) {
    return RPI_STAT_UNEXPECTED_RESPONSE;
  }

  return RPI_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long rpi_stat::get_voltage(RPI_STAT_VOLT_ID id,
			   float &value)
{
  string cmd = string(VCGENCMD) + string(" measure_volts");
  shell_cmd rpi_cmd;  

  // Actual voltage identifier
  switch (id) {
  case RPI_STAT_VOLT_ID_CORE:
    cmd.append(" core");
    break;
  case RPI_STAT_VOLT_ID_SDRAM_C:
    cmd.append(" sdram_c");
    break;
  case RPI_STAT_VOLT_ID_SDRAM_I:
    cmd.append(" sdram_i");
    break;
  case RPI_STAT_VOLT_ID_SDRAM_P:
    cmd.append(" sdram_p");
    break;
  }
  
  // Execute shell command
  string output;
  if (rpi_cmd.execute(cmd, output) != SHELL_CMD_SUCCESS) {
    return RPI_STAT_CMD_FAILED;
  }

  // Extract result from command output
  if (sscanf(output.c_str(), "volt=%fV", &value) != 1) {
    return RPI_STAT_UNEXPECTED_RESPONSE;
  }

  return RPI_STAT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long rpi_stat::get_frequency(RPI_STAT_FREQ_ID id,
			     unsigned &value)
{
  string cmd = string(VCGENCMD) + string(" measure_clock");
  shell_cmd rpi_cmd;  

  // Actual clock identifier
  switch (id) {
  case RPI_STAT_FREQ_ID_ARM:
    cmd.append(" arm");
    break;
  case RPI_STAT_FREQ_ID_CORE:
    cmd.append(" core");
    break;
  case RPI_STAT_FREQ_ID_H264:
    cmd.append(" h264");
    break;
  case RPI_STAT_FREQ_ID_ISP:
    cmd.append(" isp");
    break;
  case RPI_STAT_FREQ_ID_V3D:
    cmd.append(" v3d");
    break;
  case RPI_STAT_FREQ_ID_UART:
    cmd.append(" uart");
    break;
  case RPI_STAT_FREQ_ID_PWM:
    cmd.append(" pwm");
    break;
  case RPI_STAT_FREQ_ID_EMMC:
    cmd.append(" emmc");
    break;
  case RPI_STAT_FREQ_ID_PIXEL:
    cmd.append(" pixel");
    break;
  case RPI_STAT_FREQ_ID_VEC:
    cmd.append(" vec");
    break;
  case RPI_STAT_FREQ_ID_HDMI:
    cmd.append(" hdmi");
    break;
  case RPI_STAT_FREQ_ID_DPI:
    cmd.append(" dpi");
    break;
  }
  
  // Execute shell command
  string output;
  if (rpi_cmd.execute(cmd, output) != SHELL_CMD_SUCCESS) {
    return RPI_STAT_CMD_FAILED;
  }

  // Extract result from command output
  int clock_nr;
  if (sscanf(output.c_str(),
	     "frequency(%d)=%u",
	     &clock_nr, &value) != 2) {
    return RPI_STAT_UNEXPECTED_RESPONSE;
  }

  return RPI_STAT_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////
