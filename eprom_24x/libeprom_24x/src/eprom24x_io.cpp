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

#include <stdio.h> // JOE : debug printouts

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <fcntl.h>
#include <unistd.h>

#include "eprom24x_io.h"
#include "eprom24x_timer.h"
#include "eprom24x_delay.h"
#include "eprom24x_exception.h"

// Implementation notes:
// https://www.kernel.org/doc/Documentation/i2c/dev-interface

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define LOW_BYTE(x)     ((uint8_t)((x) & 0xFF))
#define HIGH_BYTE(x)    ((uint8_t)(((x)>>8) & 0xFF))
#define LOW_WORD(x)     ((uint16_t)((x) & 0xFFFF))
#define HIGH_WORD(x)    ((uint16_t)(((x)>>16) & 0xFFFF))

#define WRITE_POLL_INTERVAL  0.001  // EPROMs got page write time > 1 ms

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

eprom24x_io::eprom24x_io(EPROM24x_DEVICE eprom_device,
			 uint8_t i2c_address,
			 const char *i2c_dev)
{
  switch (eprom_device) {
  case EPROM24x_128bit:
    m_eprom_size_in_bytes = 16;     // Data:16 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_write_time     = 0.004;
    m_page_size_in_bytes  = 0;      // Page write not available (byte only)
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_1Kbit:
    m_eprom_size_in_bytes = 128;    // Data:128 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_write_time     = 0.010;
    m_page_size_in_bytes  = 8;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_2Kbit:
    m_eprom_size_in_bytes = 256;    // Data:256 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_write_time     = 0.010;
    m_page_size_in_bytes  = 8;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_4Kbit:
    m_eprom_size_in_bytes = 512;    // Data:512 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_size_in_bytes  = 16;
    m_page_write_time     = 0.005;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_8Kbit:
    m_eprom_size_in_bytes = 1024;   // Data:1 KB, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 16;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_16Kbit:
    m_eprom_size_in_bytes = 2048;   // Data:2 KB, Address bytes:1
    m_nr_address_bytes    = 1;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 16;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_32Kbit:
    m_eprom_size_in_bytes = 4096;   // Data: 4 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 32;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_64Kbit:
    m_eprom_size_in_bytes = 8192;   // Data:8 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 32;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_128Kbit:
    m_eprom_size_in_bytes = 16384;  // Data:16 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 64;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_256Kbit:
    m_eprom_size_in_bytes = 32768;  // Data:32 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 64;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_512Kbit:
    m_eprom_size_in_bytes = 65536;  // Data:64 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.005;
    m_page_size_in_bytes  = 128;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_1Mbit:
    m_eprom_size_in_bytes = 131072; // Data:128 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_page_write_time     = 0.003;
    m_page_size_in_bytes  = 128;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  }

  pthread_mutex_init(&m_rw_mutex, NULL); // Use default mutex attributes

  m_i2c_address = i2c_address;
  m_i2c_dev     = i2c_dev;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

eprom24x_io::~eprom24x_io(void)
{
  pthread_mutex_destroy(&m_rw_mutex);
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::initialize(void)
{
  int rc;

  // Check if EPROM is supported (We don't handle block addressing yet)
  if (!m_eprom_supported) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_EPROM_NOT_SUPPORTED,
	      "EPROM uses block addressing, not yet supported");
  }

  // Open I2C adapter
  rc = open(m_i2c_dev.c_str(), O_RDWR);
  if (rc == -1) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_FILE_OPERATION_FAILED,
	      "open failed, device (%s)", m_i2c_dev.c_str());
  }
  m_i2c_fd = rc;

  // Specify with what device address you want to communicate
  if ( ioctl(m_i2c_fd, I2C_SLAVE, m_i2c_address) < 0 ) {

    close(m_i2c_fd);

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Failed to set slave addr(0x%x), device(%s)",
	      m_i2c_address, m_i2c_dev.c_str());
  }

  // Set I2C timeout (in units of 10 ms)
  if ( ioctl(m_i2c_fd, I2C_TIMEOUT, 20) < 0 ) { // 200 ms

    close(m_i2c_fd);

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Failed to set timeout, device(%s)",
	      m_i2c_address, m_i2c_dev.c_str());
  }

  // Check if EPROM is present
  if ( !eprom_ready() ) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_EPROM_NOT_RESPONDING,
	      "Failed to probe addr(0x%x), device(%s)",
	      m_i2c_address, m_i2c_dev.c_str());
  }

  // Create page write buffer if page write is supported by chip
  // This buffer will be used for page write with address and data
  if (m_page_size_in_bytes) {
    m_page_write_buffer = new uint8_t[m_nr_address_bytes + m_page_size_in_bytes];
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::finalize(void)
{
  int rc;

  // Free any created page write buffer
  delete [] m_page_write_buffer;

  // Close I2C device
  rc = close(m_i2c_fd);
  if (rc == -1) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_FILE_OPERATION_FAILED,
	      "close failed for device file %s", m_i2c_dev.c_str());
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_io::read(uint32_t addr, void *data, uint16_t len)
{
  try {
    // Lockdown the read operation
    pthread_mutex_lock(&m_rw_mutex);

    // Check that there is enough room for data on chip
    check_valid_address(addr, len);
    
    // All chips support sequential read using pages
    read_page_data(addr, (uint8_t *)data, len);

    pthread_mutex_unlock(&m_rw_mutex);
    
    return EPROM24x_SUCCESS;
  }
  catch (...) {
    pthread_mutex_unlock(&m_rw_mutex);
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_io::write(uint32_t addr, const void *data, uint16_t len)
{
  try {
    // Lockdown the write operation
    pthread_mutex_lock(&m_rw_mutex);

    // Check that there is enough room for data on chip
    check_valid_address(addr, len);
    
    // Use page write if supported by chip
    if (m_page_size_in_bytes) {
      write_page(addr, (const uint8_t *)data, len);
    }
    else {
      write_byte(addr, (const uint8_t *)data, len);
    }
    
    pthread_mutex_unlock(&m_rw_mutex);

    return EPROM24x_SUCCESS;
  }
  catch (...) {
    pthread_mutex_unlock(&m_rw_mutex);
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::init_members(void)
{
  m_page_write_buffer = NULL;
  m_i2c_fd            = 0;
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::check_valid_address(uint32_t addr, uint16_t bytes_to_access)
{
  // Check that address is valid for specified access
  if ( addr > (m_eprom_size_in_bytes-bytes_to_access) ) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
	      "Address(0x%x) too high, max address(0x%x) for %d byte(s)",
	      addr, m_eprom_size_in_bytes-bytes_to_access, bytes_to_access);
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::read_page_data(uint32_t addr, uint8_t *data, uint16_t len)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[2];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 2;

  // High address byte shall be sent first
  uint8_t the_addr[2];

  if (m_nr_address_bytes == 1) {
    the_addr[0] = LOW_BYTE(addr);
  } else {
    the_addr[0] = HIGH_BYTE(addr);
    the_addr[1] = LOW_BYTE(addr);
  }

  // Step 1 : Set address  
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = m_nr_address_bytes;
  i2c_msg_list[0].buf   = the_addr;

  // Step 2 : Set destination data
  i2c_msg_list[1].addr  = m_i2c_address;
  i2c_msg_list[1].flags = I2C_M_RD; // Read operation
  i2c_msg_list[1].len   = len;
  i2c_msg_list[1].buf   = data;

  // Step 3 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Page read failed to get bytes(%d) from addr(0x%x), device(%s)",
	      len, addr, m_i2c_dev.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::write_page(uint32_t addr, const uint8_t *data, uint16_t len)
{
  unsigned start_page = addr / m_page_size_in_bytes;
  unsigned stop_page  = (addr + len - 1) / m_page_size_in_bytes;

  uint32_t current_addr = addr;
  uint16_t bytes_left   = len;
  uint16_t bytes_to_write;

  // Handle all pages in specified address range
  for (unsigned page_cnt=0; (page_cnt < (stop_page - start_page + 1)) &&
                            bytes_left; page_cnt++) {

    unsigned page_mod_bytes = (current_addr % m_page_size_in_bytes);

    // Calculate number of bytes to write in current page
    if ( (m_page_size_in_bytes - page_mod_bytes) > bytes_left ) {
      bytes_to_write = bytes_left;
    }
    else {
      bytes_to_write = m_page_size_in_bytes - page_mod_bytes;
    }

    printf("+++ JOE:(WRITE_PAGE) current_addr(0x%x), bytes_to_write(%d), bytes_left(%d)\n",
	   current_addr, bytes_to_write, bytes_left);

    // Write bytes in current page
    write_page_data(current_addr, data + len - bytes_left, bytes_to_write);

    // Prepare for next page
    current_addr += bytes_to_write;
    bytes_left -= bytes_to_write;
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::write_page_data(uint32_t addr, const uint8_t *data, uint16_t len)
{
  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[1];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 1;
  
  // Step 1 : Set address
  //          High address byte shall be sent first
  if (m_nr_address_bytes == 1) {
    m_page_write_buffer[0] = LOW_BYTE(addr);
  } else {
    m_page_write_buffer[0] = HIGH_BYTE(addr);
    m_page_write_buffer[1] = LOW_BYTE(addr);
  }

  // Step 2 : Copy user data to page write buffer
  memcpy((void *)&m_page_write_buffer[m_nr_address_bytes], data, len);

  // Step 3 : Set address and data 
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;       // Write operation
  i2c_msg_list[0].len   = m_nr_address_bytes + len;
  i2c_msg_list[0].buf   = m_page_write_buffer;

  // Step 4 : Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Page write failed to set bytes(%d) to addr(0x%x), device(%s)",
	      len, addr, m_i2c_dev.c_str());
  }

  // Step 5 : Wait for completition with device specific timeout
  wait_eprom_ready(m_page_write_time, WRITE_POLL_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::write_byte(uint32_t addr, const uint8_t *data, uint16_t len)
{
  // Handle all bytes in specified address range
  for (unsigned cnt=0; cnt < len; cnt++) {

    printf("+++ JOE:(WRITE_BYTE) addr(0x%x), cnt(%d), data(0x%x)\n",
	   addr+cnt, cnt, data[cnt]);

    write_byte_data(addr + cnt, &data[cnt]);
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::write_byte_data(uint32_t addr, const uint8_t *data)
{
  // Step 1 : Set address
  //          High address byte shall be sent first
  if (m_nr_address_bytes == 1) {
    m_page_write_buffer[0] = LOW_BYTE(addr);
  } else {
    m_page_write_buffer[0] = HIGH_BYTE(addr);
    m_page_write_buffer[1] = LOW_BYTE(addr);
  }

  // Step 2 : Copy user data to page write buffer
  m_page_write_buffer[m_nr_address_bytes] = *data;

  // Step 3 : Write address and data
  if ( write(m_i2c_fd,
	     (void *) m_page_write_buffer,
	     (size_t) (m_nr_address_bytes + 1)) != (m_nr_address_bytes + 1) ) {

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Byte write failed to set addr(0x%x) and data(0x%x), device(%s)",
	      addr, *data, m_i2c_dev.c_str());
  }

  // Step 4 : Wait for completition with device specific timeout
  wait_eprom_ready(m_page_write_time, WRITE_POLL_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////

bool eprom24x_io::eprom_ready(void)
{
  // During a write cycle, the EEPROM is not accessible, and will therefore 
  // not ACK any requests. This feature can be used to determine when the
  // write is actually completed, and is denoted acknowledgement polling.

  struct i2c_rdwr_ioctl_data rdwr_msg;
  struct i2c_msg i2c_msg_list[1];

  rdwr_msg.msgs  = i2c_msg_list;
  rdwr_msg.nmsgs = 1;

  // Step 1 : Set address  
  i2c_msg_list[0].addr  = m_i2c_address;
  i2c_msg_list[0].flags = 0;        // Write operation
  i2c_msg_list[0].len   = 0;
  i2c_msg_list[0].buf   = NULL;

  // Start I2C transaction sequence, wait for result
  if ( ioctl(m_i2c_fd, I2C_RDWR, &rdwr_msg) < 0 ) {
    return false;
  }
  else {
    return true; // Transaction OK, EPROM is ready
  }
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::wait_eprom_ready(double timeout_in_sec,
				   double poll_interval_in_sec)
{
  eprom24x_timer ack_poll_timer;
  bool ack_poll_timeout = true;

  ack_poll_timer.reset();
  while ( ack_poll_timer.get_elapsed_time() < timeout_in_sec ) {
    if ( eprom_ready() ) {
      ack_poll_timeout = false;
      break;
    }
    delay(poll_interval_in_sec);
  }

  // Check if operation timeout
  if (ack_poll_timeout) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_EPROM_NOT_RESPONDING,
	      "Timeout occurred(%f sec), device(%s)",
	      timeout_in_sec, m_i2c_dev.c_str());
  }
}
