/*************************************************************
*                                                            *
* Copyright (C) Bonden i Nol                                 *
*                                                            *
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>

#include "eprom24x_io.h"
#include "eprom24x_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define LOW_BYTE(x)     ((uint8_t)((x) & 0xFF)) 
#define HIGH_BYTE(x)    ((uint8_t)(((x)>>8) & 0xFF)) 
#define LOW_WORD(x)     ((uint16_t)((x) & 0xFFFF)) 
#define HIGH_WORD(x)    ((uint16_t)(((x)>>16) & 0xFFFF)) 

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
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_1Kbit:
    m_eprom_size_in_bytes = 128;    // Data:128 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_2Kbit:
    m_eprom_size_in_bytes = 256;    // Data:256 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_4Kbit:
    m_eprom_size_in_bytes = 512;    // Data:512 B, Address bytes:1
    m_nr_address_bytes    = 1;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_8Kbit:
    m_eprom_size_in_bytes = 1024;   // Data:1 KB, Address bytes:1
    m_nr_address_bytes    = 1;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_16Kbit:
    m_eprom_size_in_bytes = 2048;   // Data:2 KB, Address bytes:1
    m_nr_address_bytes    = 1;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  case EPROM24x_32Kbit:
    m_eprom_size_in_bytes = 4096;   // Data: 4 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_64Kbit:
    m_eprom_size_in_bytes = 8192;   // Data:8 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_128Kbit:
    m_eprom_size_in_bytes = 16384;  // Data:16 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_256Kbit:
    m_eprom_size_in_bytes = 32768;  // Data:32 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_512Kbit:
    m_eprom_size_in_bytes = 65536;  // Data:64 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = true;   // Single block addressing
    break;
  case EPROM24x_1Mbit:
    m_eprom_size_in_bytes = 131072; // Data:128 KB, Address bytes:2
    m_nr_address_bytes    = 2;
    m_eprom_supported     = false;  // Block addressing not supported yet
    break;
  }

  m_i2c_address = i2c_address;
  m_i2c_dev     = i2c_dev;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

eprom24x_io::~eprom24x_io(void)
{
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
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::finalize(void)
{
  int rc;

  // Close I2C device
  rc = close(m_i2c_fd);
  if (rc == -1) {
    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_FILE_OPERATION_FAILED,
	      "close failed for device file %s", m_i2c_dev.c_str());
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_io::read_u8(uint32_t addr, uint8_t *value)
{
  uint8_t i2c_buffer[2];

  // Check that address is valid
  if ( addr > (m_eprom_size_in_bytes-1) ) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
	      "Specified address(0x%x) too high, max address(0x%x)",
	      addr, m_eprom_size_in_bytes-1);
  }

  // High address byte shall be sent first
  if (m_nr_address_bytes == 1) {
    i2c_buffer[0] = LOW_BYTE(addr);
  } else {
    i2c_buffer[0] = HIGH_BYTE(addr);
    i2c_buffer[1] = LOW_BYTE(addr);
  }

  // Step 1 : Set address
  if ( write(m_i2c_fd,
	     (void *) i2c_buffer,
	     (size_t) m_nr_address_bytes) != m_nr_address_bytes ) {

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Random read failed to set addr(0x%x), device(%s)",
	      addr, m_i2c_dev.c_str());
  }

  // Step 2 : Read data byte
  if ( read(m_i2c_fd,
	     (void *) value,
	     (size_t) 1) != 1 ) {

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Random read failed to get data from addr(0x%x), device(%s)",
	      addr, m_i2c_dev.c_str());
  }

  return EPROM24x_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_io::write_u8(uint32_t addr, uint8_t value)
{
  uint8_t i2c_buffer[3];

  // Check that address is valid
  if ( addr > (m_eprom_size_in_bytes-1) ) {
    THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
	      "Specified address(0x%x) too high, max address(0x%x)",
	      addr, m_eprom_size_in_bytes-1);
  }

  // High address byte shall be sent first
  if (m_nr_address_bytes == 1) {
    i2c_buffer[0] = LOW_BYTE(addr);
    i2c_buffer[1] = value;
  } else {
    i2c_buffer[0] = HIGH_BYTE(addr);
    i2c_buffer[1] = LOW_BYTE(addr);
    i2c_buffer[2] = value;
  }

  // Step 1 : Set address and data
  if ( write(m_i2c_fd,
	     (void *) i2c_buffer,
	     (size_t) (m_nr_address_bytes + 1)) != (m_nr_address_bytes + 1) ) {

    THROW_RXP(EPROM24x_LINUX_ERROR, EPROM24x_I2C_OPERATION_FAILED,
	      "Random write failed to set addr(0x%x) and data(0x%x), device(%s)",
	      addr, value, m_i2c_dev.c_str());
  }

  // Step 2 : Wait for completition
  //          Acknowledge polling, see datasheet, JOE!
  

  // JOE: Hint for performing complex transfers
  //      ioctl(file, I2C_RDWR, struct i2c_rdwr_ioctl_data *msgset)
  //      linux/i2c-dev.h && linux/i2c.h
  //      See example: http://bunniestudios.com/blog/images/infocast_i2c.c

  return EPROM24x_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void eprom24x_io::init_members(void)
{
  m_i2c_fd = 0;
}
