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

#ifndef __RASPI_IO_H__
#define __RASPI_IO_H__

#include <semaphore.h>
#include <string>

#include "raspi.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class raspi_io {

public:
  raspi_io(const char *spi_dev,
	   sem_t *spi_bus_protect_sem);
  ~raspi_io(void);

  void initialize(RASPI_MODE mode,
		  RASPI_BPW bpw,
		  uint32_t speed,
		  bool blocking);

  void finalize(void);

  long xfer(const void *tx_buf,
	    void *rx_buf,
	    uint32_t nbytes);

  long xfer_n(const RASPI_TRANSFER *transfer_list,
	      unsigned transfers);

private:
  string  m_spi_dev;
  int     m_spi_fd;
  sem_t   *m_spi_bus_protect_sem;
  bool    m_blocking;

  void init_members(void);

  void lock_spi(void);

  void unlock_spi(void);
};

#endif // __RASPI_IO_H__
