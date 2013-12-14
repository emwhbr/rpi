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

#include <strings.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#include "mcp3008_io.h"

// Implementation notes:
// 1. General
//    https://www.kernel.org/doc/Documentation/spi/spidev
//    https://www.kernel.org/doc/Documentation/spi/spi-summary
//
// 2. Raspberry PI kernel modules:
//    a) Protocol driver   (spidev.ko)
//    b) Controller driver (spi-bcm2708.ko)
//    See .../drivers/spi/spidev.c
//        .../drivers/spi/spi-bcm2708.c
//
// 3. Protocol driver has a character device interface.
//    /dev/spidevB.C	  (B=Bus, C=Chip select)
//    /dev/spidev0.0      (Chip select #0)
//    /dev/spidev0.1      (Chip select #1)
//    
// 4. User space API: /usr/include/linux/spi/spidev.h
// 
// 5. MCP3008 datasheet, Document: DS21295B (2002)
//    http://www.microchip.com/
//

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

// 10-bit resolution (0x000 - 0x3ff)
#define MCP3008_NR_BITS_RES  10
#define MCP3008_MAX_VAL      ( (1 << MCP3008_NR_BITS_RES) - 1 )

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

mcp3008_io::mcp3008_io(string spi_dev,
		       float vref)
{
  m_spi_dev = spi_dev;
  m_vref    = vref;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

mcp3008_io::~mcp3008_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

long mcp3008_io::initialize(uint32_t speed)
{
  int rc;
  const uint8_t spi_mode = SPI_MODE_3; // SPI mode (1,1)
  const uint8_t spi_bpw  = 8;


  // Open SPI device
  rc = open(m_spi_dev.c_str(), O_RDWR);
  if (rc == -1) {
    return MCP3008_IO_FILE_OPERATION_FAILED;
  }
  m_spi_fd = rc;

  // Set SPI mode
  if ( ioctl(m_spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0 ) {
    close(m_spi_fd);
    return MCP3008_IO_SPI_OPERATION_FAILED;
  }
  
  // Set SPI word length (bits per word)
   if ( ioctl(m_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0 ) {
     close(m_spi_fd);
     return MCP3008_IO_SPI_OPERATION_FAILED;
  }

  // Set max speed (Hz)
  if ( ioctl(m_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0 ) {
    close(m_spi_fd);
    return MCP3008_IO_SPI_OPERATION_FAILED;
  }

  return MCP3008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long mcp3008_io::finalize(void)
{
  // Close SPI device
  if ( close(m_spi_fd) == -1 ) {
    return MCP3008_IO_FILE_OPERATION_FAILED;
  }

  init_members();

  return MCP3008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long mcp3008_io::read_single(MCP3008_IO_CHANNEL channel,
			     uint16_t &value)
{
  long rc;
  uint8_t tx_buf[3];
  uint8_t rx_buf[3];

  const uint8_t chn = channel & 0x7;

  tx_buf[0] = 0x01;                  // Leading zeros + Start bit
  tx_buf[1] = ( 0x80 | (chn << 4) ); // Single ended + channel
  tx_buf[2] = 0x00;                  // Don't care

  // Transfer is done using 8-bit segments (x3)
  rc = spi_xfer((const void *)tx_buf,
		(void *)rx_buf,
		3);
  if (rc != MCP3008_IO_SUCCESS) {
    return rc;
  }

  // ADC conversion result is now in receive buffer
  value = (rx_buf[1] << 8) | rx_buf[2];

  // Check that null bit is zero
  if ( value & (1 << MCP3008_NR_BITS_RES) ) {
    return MCP3008_IO_UNEXPECTED_STATE;
  }

  // Mask valid bits
  value &= MCP3008_MAX_VAL;

  return MCP3008_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

float mcp3008_io::to_voltage(uint16_t value)
{
  if (value > MCP3008_MAX_VAL) {
    value = MCP3008_MAX_VAL;
  }

  return (value * m_vref) / (float)(1 << MCP3008_NR_BITS_RES);
} 

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void mcp3008_io::init_members(void)
{
  m_spi_fd = 0;
}

/////////////////////////////////////////////////////////////////////////////

long mcp3008_io::spi_xfer(const void *tx_buf,
			  void *rx_buf,
			  uint32_t nbytes)
{
  struct spi_ioc_transfer spi_transfer;

  // Clear SPI transfer
  bzero((void *) &spi_transfer, sizeof(spi_transfer));

  // Change SPI settings specific for this transfer
  spi_transfer.tx_buf = (uint64_t) tx_buf;
  spi_transfer.rx_buf = (uint64_t) rx_buf;
  spi_transfer.len = nbytes;

  // Do SPI transfer
  if ( ioctl(m_spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer) < 0 ) {
    return MCP3008_IO_SPI_OPERATION_FAILED;
  }

  return MCP3008_IO_SUCCESS;
}
