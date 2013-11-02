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

#ifndef __LCD6100_IO_BITBANG_H__
#define __LCD6100_IO_BITBANG_H__

#include "lcd6100_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class lcd6100_io_bitbang : public lcd6100_io {

public:
  lcd6100_io_bitbang(LCD6100_CE ce);
  ~lcd6100_io_bitbang(void);

protected:
  void spi_initialize(void);

  void spi_finalize(void);

  void spi_write(const uint16_t *msg);

private:
  string m_spi_dev_file;
  int    m_fd_spi_pcf8833;

  void init_members(void);
};

#endif // __LCD6100_IO_BITBANG_H__
