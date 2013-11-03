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

#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lcd6100_io_bitbang.h"
#include "lcd6100_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define SPI_PCF8833_DEV  "/dev/spi-pcf8833"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_io_bitbang::lcd6100_io_bitbang(LCD6100_CE ce) : lcd6100_io()
{
  switch (ce) {
  case LCD6100_CE_0:
    m_spi_dev_file = SPI_PCF8833_DEV"-0";
    break;
  case LCD6100_CE_1:
    m_spi_dev_file = SPI_PCF8833_DEV"-1";  
    break;
  }

  init_members();
 }

/////////////////////////////////////////////////////////////////////////////

lcd6100_io_bitbang::~lcd6100_io_bitbang(void)
{
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_bitbang::spi_initialize(void)
{
  int fd;

  // Initialize SPI layer 
  fd = open(m_spi_dev_file.c_str(), O_RDWR);
  if (fd == -1) {

    int error_num=errno;
    char error_string[256];
    char *err;
    err = strerror_r(error_num, error_string, 256);
    strncpy(error_string, err, 256);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to initialize SPI, SPI layer info: %s",
	      error_string);
  }

  m_fd_spi_pcf8833 = fd;
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_bitbang::spi_finalize(void)
{
  int rc;

  // Finalize SPI layer
  rc = close(m_fd_spi_pcf8833);
  if (rc == -1) {

    int error_num=errno;
    char error_string[256];
    char *err;
    err = strerror_r(error_num, error_string, 256);
    strncpy(error_string, err, 256);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to finalize SPI, SPI layer info: %s",
	      error_string);
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_bitbang::spi_write(const uint16_t *msg)
{
  ssize_t rc;

  rc = write(m_fd_spi_pcf8833, (void *)msg, sizeof(uint16_t));
  if (rc == -1) {

    int error_num=errno;
    char error_string[256];
    char *err;
    err = strerror_r(error_num, error_string, 256);
    strncpy(error_string, err, 256);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to write message(0x%02x), SPI layer info: %s",
	      *msg, error_string);
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_bitbang::init_members(void)
{
  m_fd_spi_pcf8833 = -1;
}
