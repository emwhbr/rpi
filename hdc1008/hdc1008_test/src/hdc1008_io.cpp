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

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <unistd.h>

#include "hdc1008_io.h"

// Implementation notes:
// 1. I2C Linux
//    https://www.kernel.org/doc/Documentation/i2c/dev-interface
//
// 2. I2C specification
//    http://www.i2c-bus.org/references/
// 
// 3. HDC1008 datasheet, Document: SNAS649B (October 2014)
//    http://www.ti.com/

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
// Pointer register values (addresses) of available registers
#define HDC1008_REG_TEMPERATURE     0x00
#define HDC1008_REG_HUMIDITY        0x01
#define HDC1008_REG_CONFIGURATION   0x02

// Configuration register bit masks
#define HDC1008_CONFIGURATION_RST    (1 << 15)
#define HDC1008_CONFIGURATION_HEAT   (1 << 13)
#define HDC1008_CONFIGURATION_MODE   (1 << 12)
#define HDC1008_CONFIGURATION_BTST   (1 << 11)

// Configuration register, reset value
#define HDC1008_CONFIGURATION_RESET_VALUE  0x1000

// According to HDC1008 data sheet, device start-up time is max 15ms
#define HDC1008_RESET_TIME_US  20000  // 20ms (added safety factor)

// According to HDC1008 data sheet, conversion time (14 bit resolution):
// Temperature : 6.35ms, Humidity: 6.50ms
#define HDC1008_CONVERSION_TIME_US  10000  // 10ms (added safety factor)

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

hdc1008_io::hdc1008_io(uint8_t i2c_address,
		       string i2c_dev)
{
  m_i2c_address = i2c_address;
  m_i2c_dev     = i2c_dev;
  m_i2c_fd      = 0;
}

/////////////////////////////////////////////////////////////////////////////

hdc1008_io::~hdc1008_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::initialize(void)
{
  int rc;
  long lrc;
  HDC1008_IO_CFG_REG cfg_reg;

  // Open I2C adapter
  rc = open(m_i2c_dev.c_str(), O_RDWR);
  if (rc == -1) {
    return HDC1008_IO_FILE_OPERATION_FAILED;
  }
  m_i2c_fd = rc;

  // Reset chip
  lrc = this->reset_chip();
  if (lrc != HDC1008_IO_SUCCESS) {
    return lrc;
  }

  // Check expected reset state
  lrc = this->read_config(cfg_reg);
  if (lrc != HDC1008_IO_SUCCESS) {
    return lrc;
  }
  if (cfg_reg.wd != HDC1008_CONFIGURATION_RESET_VALUE) {
    return HDC1008_IO_UNEXPECTED_STATE;
  }

  // Set mode to temperature OR humidity acquisition
  cfg_reg.bs.mode = 0;
  lrc = this->write_config(cfg_reg);
  if (lrc != HDC1008_IO_SUCCESS) {
    return lrc;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::finalize(void)
{
  int rc;

  // Close I2C device
  rc = close(m_i2c_fd);
  if (rc == -1) {
    return HDC1008_IO_FILE_OPERATION_FAILED;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::read_config(HDC1008_IO_CFG_REG &reg)
{
  long rc;
  uint16_t cfg_reg;

  rc = this->read_register(HDC1008_REG_CONFIGURATION, cfg_reg);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }
  
  reg.wd = cfg_reg;

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::write_config(HDC1008_IO_CFG_REG reg)
{
  long rc;
  uint16_t cfg_reg;

  cfg_reg = reg.wd;

  rc = this->write_register(HDC1008_REG_CONFIGURATION, cfg_reg);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::read_temperature(float &value)
{
  long rc;
  uint16_t reg_val;

  // Perform temperature measurement
  rc = this->single_measurement(HDC1008_REG_TEMPERATURE, reg_val);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  // Convert register value to temperature
  rc = to_temperature(reg_val, value);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::read_humidity(float &value)
{
  long rc;
  uint16_t reg_val;

  // Perform humidity measurement
  rc = this->single_measurement(HDC1008_REG_HUMIDITY, reg_val);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  // Convert register value to relative humidity
  rc = to_humidity(reg_val, value);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::read_register(uint8_t addr,
			       uint16_t &value)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[2];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 2;

  uint8_t the_buffer[2];

  // Step 1 : Set pointer register byte
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = 1;
  i2c_msg_list[0].buf   = &addr;

  // Step 2 : Set destination data
  i2c_msg_list[1].addr  = m_i2c_address;
  i2c_msg_list[1].flags = I2C_M_RD; // Read operation
  i2c_msg_list[1].len   = 2;
  i2c_msg_list[1].buf   = the_buffer;

  // Step 3 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return HDC1008_IO_I2C_OPERATION_FAILED;
  }

  // Step 4: MSB was sent first from chip
  value = (the_buffer[0] << 8) | (the_buffer[1]);

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::write_register(uint8_t addr,
				uint16_t value)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[1];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 1;

  uint8_t the_buffer[3];

  // Step 1 : Set pointer register byte
  the_buffer[0] = addr;

  // Step 2 : Copy data to transmission buffer
  //          MSB is sent first to chip
  the_buffer[1] = (uint8_t)((value >> 8) & 0xff);
  the_buffer[2] = (uint8_t)(value & 0xff);

  // Step 3 : Set pointer register byte and data
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = 3;
  i2c_msg_list[0].buf   = the_buffer;

  // Step 4 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return HDC1008_IO_I2C_OPERATION_FAILED;
  }

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::single_measurement(uint8_t addr,
				    uint16_t &reg_value)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[1];

  rdwr_msg.msgs  = i2c_msg_list;  
  rdwr_msg.nmsgs = 1;

  uint8_t the_buffer[2];

  //////////////////////////////
  //   (A) Trigger measurement
  //////////////////////////////

  // Step 1 : Set pointer register byte
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = 1;
  i2c_msg_list[0].buf   = &addr;

  // Step 2 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return HDC1008_IO_I2C_OPERATION_FAILED;
  }

  ///////////////////////////////////////////////
  //   (B) Wait for the measurement to complete
  ///////////////////////////////////////////////

  usleep(HDC1008_CONVERSION_TIME_US);

  ////////////////////////////////////////////////////
  //   (C) Retrieve the completed measurement result
  ////////////////////////////////////////////////////

  // Step 1 : Set destination data
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = I2C_M_RD; // Read operation
  i2c_msg_list[0].len   = 2;
  i2c_msg_list[0].buf   = the_buffer;

  // Step 2 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return HDC1008_IO_I2C_OPERATION_FAILED;
  }

  // Step 3: MSB was sent first from chip
  reg_value = (the_buffer[0] << 8) | (the_buffer[1]);
  
  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::reset_chip(void)
{
  long rc;

  // Set software reset bit
  rc = this->write_register(HDC1008_REG_CONFIGURATION,
			    HDC1008_CONFIGURATION_RST);
  if (rc != HDC1008_IO_SUCCESS) {
    return rc;
  }

  usleep(HDC1008_RESET_TIME_US); // Wait for device to start-up

  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::to_temperature(uint16_t reg_value,
				float &temp_value)
{
  // Convert from register value to physical temperature value
  temp_value = (reg_value / 65536.0) * 165.0 - 40.0;
  
  return HDC1008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long hdc1008_io::to_humidity(uint16_t reg_value,
			     float &hum_value)
{
  // Convert from register value to physical relative humidity value
  hum_value = (reg_value / 65536.0) * 100.0;
  
  return HDC1008_IO_SUCCESS;
}
