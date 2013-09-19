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

#include "raspi_io.h"
#include "raspi_exception.h"

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

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

raspi_io::raspi_io(const char *spi_dev,
		   sem_t *spi_bus_protect_sem)
{
  m_spi_dev = spi_dev;
  m_spi_bus_protect_sem = spi_bus_protect_sem;

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

raspi_io::~raspi_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void raspi_io::initialize(RASPI_MODE mode,
			  RASPI_BPW bpw,
			  uint32_t speed,
			  bool blocking)
{
  int rc;
  uint8_t spi_mode = RASPI_MODE_0;
  uint8_t spi_bpw  = 8;

  // Define runtime behaviour
  m_blocking = blocking;

  // Open SPI device
  rc = open(m_spi_dev.c_str(), O_RDWR);
  if (rc == -1) {
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_FILE_OPERATION_FAILED,
	      "open failed, device (%s)", m_spi_dev.c_str());
  }
  m_spi_fd = rc;

  // Set SPI mode
  switch (mode) {
  case RASPI_MODE_0:
    spi_mode = SPI_MODE_0;
    break;
  case RASPI_MODE_1:
    spi_mode = SPI_MODE_1;
    break;
  case RASPI_MODE_2:
    spi_mode = SPI_MODE_2;
    break;
  case RASPI_MODE_3:
    spi_mode = SPI_MODE_3;
    break;
  }
  if ( ioctl(m_spi_fd, SPI_IOC_WR_MODE, &spi_mode) < 0 ) {
    close(m_spi_fd);
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_SPI_OPERATION_FAILED,
	      "Failed to set mode(%u), device(%s)",
	      spi_mode, m_spi_dev.c_str());
  }

  // Set SPI word length (bits per word)
  switch (bpw) {
  case RASPI_BPW_8:
    spi_bpw = 8;
    break;
  case RASPI_BPW_9:
    spi_bpw = 9;
    break;
  }
  if ( ioctl(m_spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bpw) < 0 ) {
    close(m_spi_fd);
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_SPI_OPERATION_FAILED,
	      "Failed to set bpw(%u), device(%s)",
	      spi_bpw, m_spi_dev.c_str());
  }

  // Set max speed (Hz)
  if ( ioctl(m_spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0 ) {
    close(m_spi_fd);
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_SPI_OPERATION_FAILED,
	      "Failed to set speed(%u), device(%s)",
	      speed, m_spi_dev.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////

void raspi_io::finalize(void)
{
  // Close SPI device
  if ( close(m_spi_fd) == -1 ) {
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_FILE_OPERATION_FAILED,
	      "close failed for device file %s", m_spi_dev.c_str());
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

long raspi_io::xfer(const void *tx_buf,
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
  
  // Lockdown the SPI transfer operation
  lock_spi();

  try {
    // Do SPI transfer
    if ( ioctl(m_spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer) < 0 ) {
      THROW_RXP(RASPI_LINUX_ERROR, RASPI_SPI_OPERATION_FAILED,
		"Failed to transfer bytes(%u), device(%s)",
		nbytes, m_spi_dev.c_str());
    }
  }
  catch (...) {
    sem_post(m_spi_bus_protect_sem);
    throw;
  }

  // Lockup the SPI transfer operation
  unlock_spi();

  return RASPI_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long raspi_io::xfer_n(const RASPI_TRANSFER *transfer_list,
		      unsigned transfers)
{
  struct spi_ioc_transfer spi_transfer[transfers];

  // Clear SPI transfers
  bzero((void *) spi_transfer, sizeof(spi_transfer));

  // Create the SPI transfers
  for (unsigned i=0; i < transfers; i++) {
    spi_transfer[i].tx_buf = (uint64_t) transfer_list[i].tx_buf;
    spi_transfer[i].rx_buf = (uint64_t) transfer_list[i].rx_buf;
    spi_transfer[i].len    = transfer_list[i].nbytes;
    spi_transfer[i].delay_usecs = transfer_list[i].delay_usecs;
    spi_transfer[i].cs_change   = (transfer_list[i].ce_deactive ? 1 : 0);
  }

  // Lockdown the SPI transfers operation
  lock_spi();

  try {
    // Do SPI transfers
    if ( ioctl(m_spi_fd, SPI_IOC_MESSAGE(transfers), spi_transfer) < 0 ) {
      THROW_RXP(RASPI_LINUX_ERROR, RASPI_SPI_OPERATION_FAILED,
		"Failed multiple transfers, device(%s)",
		m_spi_dev.c_str());
    }
  }
  catch (...) {
    sem_post(m_spi_bus_protect_sem);
    throw;
  }

  // Lockup the SPI transfers operation
  unlock_spi();

  return RASPI_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void raspi_io::init_members(void)
{
  m_spi_fd = 0;
  m_blocking = true;
}

/////////////////////////////////////////////////////////////////////////////

void raspi_io::lock_spi(void)
{
  if (m_blocking) {
    if ( sem_wait(m_spi_bus_protect_sem) == -1 ) {
      THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
		"sem_wait failed, device(%s)",
		m_spi_dev.c_str());
    }
  }
  else {
    if ( sem_trywait(m_spi_bus_protect_sem) == -1 ) {
      if (errno == EAGAIN) {
	THROW_RXP(RASPI_LINUX_ERROR, RASPI_OPERATION_WOULD_BLOCK,
		  "sem_trywait would block, device(%s)",
		  m_spi_dev.c_str());
      }
      else {
	THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
		  "sem_trywait failed, device(%s)",
		  m_spi_dev.c_str());
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////

void raspi_io::unlock_spi(void)
{
  if ( sem_post(m_spi_bus_protect_sem) == -1 ) {
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
	      "sem_post failed, device(%s)",
	      m_spi_dev.c_str());
  }
}
