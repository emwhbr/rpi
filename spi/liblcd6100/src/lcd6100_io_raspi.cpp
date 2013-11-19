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

#include "lcd6100_io_raspi.h"
#include "lcd6100_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

lcd6100_io_raspi::lcd6100_io_raspi(uint8_t hw_reset_pin,
				   LCD6100_CE ce,
				   uint32_t speed) : lcd6100_io(hw_reset_pin)
{
  switch (ce) {
  case LCD6100_CE_0:
    m_raspi_ce = RASPI_CE_0;
    break;
  case LCD6100_CE_1:
    m_raspi_ce = RASPI_CE_1;
    break;
  }

  m_raspi_speed = speed;

  init_members();
 }

/////////////////////////////////////////////////////////////////////////////

lcd6100_io_raspi::~lcd6100_io_raspi(void)
{
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_raspi::spi_initialize(void)
{
  long rc;

  // Initialize SPI layer
  rc = raspi_initialize(m_raspi_ce,
			RASPI_MODE_3,
			RASPI_BPW_9,
			m_raspi_speed,
			0);

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to initialize SPI, SPI layer info: %s",
	      error_string);
  }
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_raspi::spi_finalize(void)
{
  long rc;

  // Finalize SPI layer
  rc = raspi_finalize(m_raspi_ce);

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

    // Throw new error
    THROW_LXP(LCD6100_INTERNAL_ERROR, LCD6100_SPI_LAYER_ERROR,
	      "Failed to finalize SPI, SPI layer info: %s",
	      error_string);
  }

  init_members();
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_raspi::spi_write(const uint16_t *msg)
{
  long rc;

  // Send message to LCD
  rc = raspi_xfer(m_raspi_ce,
		  (const void *)msg,
		  NULL,
		  sizeof(uint16_t));

  if (rc != RASPI_SUCCESS) {

    RASPI_ERROR_STRING error_string;
    get_spi_layer_error(error_string);

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

void lcd6100_io_raspi::init_members(void)
{
}

/////////////////////////////////////////////////////////////////////////////

void lcd6100_io_raspi::get_spi_layer_error(RASPI_ERROR_STRING error_string)
{
  RASPI_STATUS status;

  // Get / clear RASPI error information
  raspi_get_last_error(&status);
  raspi_get_error_string(status.error_code, error_string);
}
