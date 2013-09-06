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

#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <unistd.h>

#include "tmp102_io.h"

// Implementation notes:
// 1. I2C Linux
//    https://www.kernel.org/doc/Documentation/i2c/dev-interface
//
// 2. I2C specification
//    http://www.i2c-bus.org/references/
// 
// 3. TMP102 datasheet, Document: SBOS397C (October 2012)
//    http://www.ti.com/

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

// Pointer register values (addresses) of available registers
#define TEMP_REG_ADDR  0x00
#define CFG_REG_ADDR   0x01
#define TLOW_REG_ADDR  0x02
#define THI_REG_ADDR   0x03

#define CFG_REG_PWRUP     0x60a0 // Config register, power-up value
#define TEMP_REG_MASK_EM  0x01   // Temperature register, bit0

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

tmp102_io::tmp102_io(uint8_t i2c_address,
		     string i2c_dev)
{
  m_i2c_address = i2c_address;
  m_i2c_dev     = i2c_dev;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

tmp102_io::~tmp102_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::initialize(bool extended_mode)
{
  int rc;
  long lrc;
  TMP102_IO_CFG_REG cfg_reg;

  m_extended_mode = extended_mode;

  // Open I2C adapter
  rc = open(m_i2c_dev.c_str(), O_RDWR);
  if (rc == -1) {
    return TMP102_IO_FILE_OPERATION_FAILED;
  }
  m_i2c_fd = rc;

  // Reset registers to power-up values
  lrc = reset_chip();
  if (lrc != TMP102_IO_SUCCESS) {
    return lrc;
  }

  // Check expected power-up state
  lrc = read_config(cfg_reg);
  if (lrc != TMP102_IO_SUCCESS) {
    return lrc;
  }
  if (cfg_reg.wd != CFG_REG_PWRUP) {
    return TMP102_IO_UNEXPECTED_STATE;
  }

  // If specified, change to extended mode
  // Default at power-up is normal mode
  if (m_extended_mode) {
    cfg_reg.bs.em = 1;               // Modify
    lrc = write_config(cfg_reg);     // Write
    if (lrc != TMP102_IO_SUCCESS) {
      return lrc;
    }   
  }

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::finalize(void)
{
  int rc;

  // Close I2C device
  rc = close(m_i2c_fd);
  if (rc == -1) {
    return TMP102_IO_FILE_OPERATION_FAILED;
  }

  init_members();

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::read_config(TMP102_IO_CFG_REG &reg)
{
  long rc;
  uint16_t cfg_reg;

  rc = read_register(CFG_REG_ADDR, cfg_reg);
  if (rc != TMP102_IO_SUCCESS) {
    return rc;
  }
  
  reg.wd = cfg_reg;

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::write_config(TMP102_IO_CFG_REG reg)
{
  long rc;
  uint16_t cfg_reg;

  cfg_reg = reg.wd;

  rc = write_register(CFG_REG_ADDR, cfg_reg);
  if (rc != TMP102_IO_SUCCESS) {
    return rc;
  }

  // Update expected value of extended mode
  if (reg.bs.em) {
    m_extended_mode = true;
  }

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::read_temperature(float &value)
{
  long rc;
  uint16_t temp_reg;

  // Read temperature register
  rc = read_register(TEMP_REG_ADDR, temp_reg);
  if (rc != TMP102_IO_SUCCESS) {
    return rc;
  }

  // Convert register value to temperature
  rc = to_temperature(temp_reg, value);
  if (rc != TMP102_IO_SUCCESS) {
    return rc;
  }

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void tmp102_io::init_members(void)
{
  m_extended_mode = false;
  m_i2c_fd        = 0;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::read_register(uint8_t addr,
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
    return TMP102_IO_I2C_OPERATION_FAILED;
  }

  // Step 4: MSB was sent first from chip
  value = (the_buffer[0] << 8) | (the_buffer[1]);

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::write_register(uint8_t addr,
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
    return TMP102_IO_I2C_OPERATION_FAILED;
  }

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::reset_chip(void)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[1];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 1;

  uint8_t the_buffer = 0x06; // Reset command

  // Step 1 : Set reset command
  i2c_msg_list[0].addr  = 0x0000;   // General call address
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = 1;
  i2c_msg_list[0].buf   = &the_buffer;

  // Step 2 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return TMP102_IO_I2C_OPERATION_FAILED;
  }

  return TMP102_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long tmp102_io::to_temperature(uint16_t reg_value,
			       float &temp_value)
{
  bool is_negative;

  // Check mode and sign bit
  if (m_extended_mode) {
    // Extended mode, 13 bits valid
    if ( !(reg_value & TEMP_REG_MASK_EM) ) {
      return TMP102_IO_UNEXPECTED_STATE;
    }
    reg_value = (reg_value >> 3) & 0x1fff;
    is_negative = ((reg_value & 0x1000) != 0);
  }
  else {
    // Normal mode, 12 bits valid
    if ( reg_value & TEMP_REG_MASK_EM ) {
      return TMP102_IO_UNEXPECTED_STATE;
    } 
    reg_value = (reg_value >> 4) & 0x0fff;
    is_negative = ((reg_value & 0x0800) != 0);
  }

  // Convert from register value to physical temperature
  if (!is_negative) {
    // Non-negative
    if (m_extended_mode) {
      temp_value = (reg_value & 0x0fff) * 0.0625;
    }
    else {
      temp_value = (reg_value & 0x07ff) * 0.0625;
    }
  }
  else {
    // Binary two complements format used
    if (m_extended_mode) {
      // 1 sign bit, 12 value bits ==> 13 bits
      temp_value = ( (~(reg_value & 0x1fff) + 1) & (0x1fff) ) * -0.0625;
    }
    else {
      // 1 sign bit, 11 value bits ==> 12 bits
      temp_value = ( (~(reg_value & 0x0fff) + 1) & (0x0fff) ) * -0.0625;
    }
  }
  
  return TMP102_IO_SUCCESS;
}
