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

#ifndef __RPI_STAT_H__
#define __RPI_STAT_H__

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define RPI_STAT_SUCCESS               0
#define RPI_STAT_CMD_FAILED           -1
#define RPI_STAT_UNEXPECTED_RESPONSE  -2

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef enum {RPI_STAT_VOLT_ID_CORE,
	      RPI_STAT_VOLT_ID_SDRAM_C,
	      RPI_STAT_VOLT_ID_SDRAM_I,
	      RPI_STAT_VOLT_ID_SDRAM_P} RPI_STAT_VOLT_ID;

typedef enum {RPI_STAT_FREQ_ID_ARM,
	      RPI_STAT_FREQ_ID_CORE,
	      RPI_STAT_FREQ_ID_H264,
	      RPI_STAT_FREQ_ID_ISP,
	      RPI_STAT_FREQ_ID_V3D,
	      RPI_STAT_FREQ_ID_UART,
	      RPI_STAT_FREQ_ID_PWM,
	      RPI_STAT_FREQ_ID_EMMC,
	      RPI_STAT_FREQ_ID_PIXEL,
	      RPI_STAT_FREQ_ID_VEC,
	      RPI_STAT_FREQ_ID_HDMI,
	      RPI_STAT_FREQ_ID_DPI} RPI_STAT_FREQ_ID;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class rpi_stat {

 public:
  rpi_stat(void);
  ~rpi_stat(void);
  
  long get_temperature(float &value);

  long get_voltage(RPI_STAT_VOLT_ID id,
		   float &value);

  long get_frequency(RPI_STAT_FREQ_ID id,
		     unsigned &value);

 private:
};

#endif // __RPI_STAT_H__
