// ************************************************************************
// *                                                                      *
// * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __HDC1008_IO_H__
#define __HDC1008_IO_H__

#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define HDC1008_IO_SUCCESS                 0
#define HDC1008_IO_FILE_OPERATION_FAILED  -1
#define HDC1008_IO_UNEXPECTED_STATE       -2
#define HDC1008_IO_I2C_OPERATION_FAILED   -3

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////
typedef struct {
  union {
    struct {
      uint8_t res1 : 8;  // Reserved
      uint8_t hres : 2;  // Humidity measurement resolution
      uint8_t tres : 1;  // Temperature measurement resolution
      uint8_t btst : 1;  // Battery status
      uint8_t mode : 1;  // Mode of acquisition
      uint8_t heat : 1;  // Heater
      uint8_t res2 : 1;  // Reserved
      uint8_t rst  : 1;  // Software reset
    } bs;
    uint16_t wd;
  };
} __attribute__((packed)) HDC1008_IO_CFG_REG;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class hdc1008_io {
  
 public:
  hdc1008_io(uint8_t i2c_address,
	    string i2c_dev);
  ~hdc1008_io(void);

  long initialize(void);
  long finalize(void);

  long read_config(HDC1008_IO_CFG_REG &reg);
  long write_config(HDC1008_IO_CFG_REG reg);

  long read_temperature(float &value);
  long read_humidity(float &value);

 private:  
  uint8_t  m_i2c_address;
  string   m_i2c_dev;
  int      m_i2c_fd;

  long read_register(uint8_t addr,
		     uint16_t &value);

  long write_register(uint8_t addr,
		      uint16_t value);

  long single_measurement(uint8_t addr,
			  uint16_t &reg_value);

  long reset_chip(void);

  long to_temperature(uint16_t reg_value,
		      float &temp_value);

  long to_humidity(uint16_t reg_value,
		   float &hum_value);
};

#endif // __HDC1008_IO_H__
