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

#ifndef __TMP102_IO_H__
#define __TMP102_IO_H__

#include <stdint.h>
#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define TMP102_IO_SUCCESS                 0
#define TMP102_IO_FILE_OPERATION_FAILED  -1
#define TMP102_IO_UNEXPECTED_STATE       -2
#define TMP102_IO_I2C_OPERATION_FAILED   -3

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

typedef struct {
  union {
    struct {
      uint8_t spare : 4; // Not used
      uint8_t em    : 1; // Extended mode
      uint8_t al    : 1; // Alert
      uint8_t cr0   : 1; // Conversion rate
      uint8_t cr1   : 1;
      uint8_t sd    : 1; // Shutdown
      uint8_t tm    : 1; // Thermostat
      uint8_t pol   : 1; // Polarity
      uint8_t f0    : 1; // Fault queue
      uint8_t f1    : 1;
      uint8_t r0    : 1; // Converter resolution
      uint8_t r1    : 1;
      uint8_t os    : 1; // One shot ready
    } bs;
    uint16_t wd;
  };
} __attribute__((packed)) TMP102_IO_CFG_REG;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class tmp102_io {
  
 public:
  tmp102_io(uint8_t i2c_address,
	    string i2c_dev);
  ~tmp102_io(void);

  long initialize(bool extended_mode);
  long finalize(void);

  long read_config(TMP102_IO_CFG_REG &reg);
  long write_config(TMP102_IO_CFG_REG reg);

  long read_temperature(float &value);

 private:  
  uint8_t  m_i2c_address;
  string   m_i2c_dev;
  bool     m_extended_mode;
  int      m_i2c_fd;

  void init_members(void);

  long read_register(uint8_t addr,
		     uint16_t &value);

  long write_register(uint8_t addr,
		      uint16_t value);

  long reset_chip(void);

  long to_temperature(uint16_t reg_value,
		      float &temp_value);
};

#endif // __TMP102_IO_H__
