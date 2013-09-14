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

#ifndef __RASPI_CORE_H__
#define __RASPI_CORE_H__

#include <pthread.h>
#include <semaphore.h>
#include <memory>

#include "raspi.h"
#include "raspi_exception.h"
#include "raspi_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////

#define NR_SPI_DEVICES 2   // There are two SPI devices in the system:
                           //  /dev/spidev0.0  (RASPI_CE_0)
                           //  /dev/spidev0.1  (RASPI_CE_1)

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

typedef struct {
  bool             initialized;
  pthread_mutex_t  init_mutex;
} INITIALIZE_INFO;

typedef struct {  
  string             device;          // SPI device file
  sem_t              *master_sem;     // Only one master allowed for a device
  string             master_sem_name; // Semaphore name
  auto_ptr<raspi_io> io_auto;         // SPI I/O object
} SPI_DEV;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class raspi_core {

public:
  raspi_core(void);
  ~raspi_core(void);

  long get_last_error(RASPI_STATUS *status);

  long get_error_string(long error_code,
			RASPI_ERROR_STRING error_string);

  long initialize(RASPI_CE ce,
		  RASPI_MODE mode,
		  RASPI_BPW bpw,
		  uint32_t speed);

  long finalize(RASPI_CE ce);

  long xfer(RASPI_CE ce,
	    const void *tx_buf,
	    void *rx_buf,
	    uint32_t nbytes);

  long test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info);

private:
  // Error handling information
  RASPI_ERROR_SOURCE m_error_source;
  long               m_error_code;
  bool               m_last_error_read;
  pthread_mutex_t    m_error_mutex;

  // Keep track of initialization
  INITIALIZE_INFO m_init_info[NR_SPI_DEVICES];

  // SPI devices
  SPI_DEV m_spi_dev[NR_SPI_DEVICES];

  // Private member functions
  long set_error(raspi_exception rxp);
  long update_error(raspi_exception rxp);

  long internal_get_error_string(long error_code,
				 RASPI_ERROR_STRING error_string);

  long internal_test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info);

  void internal_initialize(RASPI_CE ce,
			   RASPI_MODE mode,
			   RASPI_BPW bpw,
			   uint32_t speed);

  void internal_finalize(RASPI_CE ce);

  void check_one_master(RASPI_CE ce);

  void finalize_one_master(RASPI_CE ce);
};

#endif // __RASPI_CORE_H__
