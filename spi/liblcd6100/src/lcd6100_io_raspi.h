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

#ifndef __LCD6100_IO_RASPI_H__
#define __LCD6100_IO_RASPI_H__

#include "lcd6100_io.h"
#include "raspi.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_io_raspi : public lcd6100_io {

public:
  lcd6100_io_raspi(LCD6100_CE ce,
		   uint32_t speed);
  ~lcd6100_io_raspi(void);

protected:
  void spi_initialize(void);

  void spi_finalize(void);

  void spi_write(const uint16_t *msg);

private:
  RASPI_CE m_raspi_ce;    // Chip select
  uint32_t m_raspi_speed; // Bitrate (Hz)

  void init_members(void);

  void get_spi_layer_error(RASPI_ERROR_STRING error_string);
};

#endif // __LCD6100_IO_RASPI_H__
