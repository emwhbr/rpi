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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>

#include "raspi_core.h"
#include "raspi_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PRODUCT_NUMBER   "LIBRASPI"
#define RSTATE           "R1A01"

#define SPI_MASTER_SEM_NAME "LIBRASPI_MASTER_SEM"

#define MUTEX_LOCK(mutex) \
  ({ if (pthread_mutex_lock(&mutex)) { \
      return RASPI_MUTEX_FAILURE; \
    } })

#define MUTEX_UNLOCK(mutex) \
  ({ if (pthread_mutex_unlock(&mutex)) { \
      return RASPI_MUTEX_FAILURE; \
    } })

#ifdef DEBUG_PRINTS
//
// Notes!
// Macro 'debug_printf' can be used anywhere in LIBRASPI.
// The other macros can only be used in function 'update_error'.
//
#define debug_printf(fmt, args...)  printf("LIBRASPI - "); \
                                    printf(fmt, ##args); \
				    fflush(stdout)

#define debug_linux_error()         printf("LIBRASPI LINUX ERROR - "); \
                                    error(0, errno, NULL); \
				    fflush(stdout)

#define debug_internal_error()      printf("LIBRASPI INTERNAL ERROR\n"); \
				    fflush(stdout)
#else
#define debug_printf(fmt, args...) 
#define debug_linux_error()
#define debug_internal_error()
#endif // DEBUG_PRINTS

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

raspi_core::raspi_core(void)
{
  m_error_source    = RASPI_INTERNAL_ERROR;
  m_error_code      = 0;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL); // Use default mutex attributes

  // Initialize SPI device information
  m_spi_dev[RASPI_CE_0].device = "/dev/spidev0.0";
  m_spi_dev[RASPI_CE_0].master_sem = NULL;
  m_spi_dev[RASPI_CE_0].master_sem_name = SPI_MASTER_SEM_NAME"_0";
  m_spi_dev[RASPI_CE_0].io_auto.reset();

  m_spi_dev[RASPI_CE_1].device = "/dev/spidev0.1";
  m_spi_dev[RASPI_CE_1].master_sem = NULL;
  m_spi_dev[RASPI_CE_1].master_sem_name = SPI_MASTER_SEM_NAME"_1";
  m_spi_dev[RASPI_CE_1].io_auto.reset();
}

/////////////////////////////////////////////////////////////////////////////

raspi_core::~raspi_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::get_last_error(RASPI_STATUS *status)
{
  try {
    MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = RASPI_INTERNAL_ERROR;
    m_error_code      = RASPI_NO_ERROR;
    m_last_error_read = true;
    MUTEX_UNLOCK(m_error_mutex);
    return RASPI_SUCCESS;
  }
  catch (...) {
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::get_error_string(long error_code, 
				  RASPI_ERROR_STRING error_string)
{
  try {
    // Do the actual work
    return internal_get_error_string(error_code, error_string);
  }
  catch (...) {
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::initialize(RASPI_CE ce,
			    RASPI_MODE mode,
			    RASPI_BPW bpw,
			    uint32_t speed)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if already initialized
    if (m_initialized) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_ALREADY_INITIALIZED,
		"Already initialized");
    }

    // Do the actual initialization
    internal_initialize(ce, mode, bpw, speed);

    // Initialization completed
    m_initialized = true;
    MUTEX_UNLOCK(m_init_mutex);

    return RASPI_SUCCESS;
  }
  catch (raspi_exception &rxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(rxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::finalize(RASPI_CE ce)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual finalization
    internal_finalize(ce);

    // Finalization completed
    m_initialized = false;
    MUTEX_UNLOCK(m_init_mutex);

    return RASPI_SUCCESS;
  }
  catch (raspi_exception &rxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(rxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::xfer(RASPI_CE ce,
		      const void *tx_buf,
		      void *rx_buf,
		      uint32_t nbytes)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!tx_buf) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_BAD_ARGUMENT,
		"tx_buf is null pointer");
    }

    if (!rx_buf) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_BAD_ARGUMENT,
		"rx_buf is null pointer");
    }
    
    // Do the actual work
    return m_spi_dev[ce].io_auto->xfer(tx_buf,
				       rx_buf,
				       nbytes);
  }
  catch (raspi_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info)
{
  try {
    // Do the actual work
    return internal_test_get_lib_prod_info(prod_info);
  }
  catch (...) {
    return set_error(RXP(RASPI_INTERNAL_ERROR, RASPI_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long raspi_core::set_error(raspi_exception rxp)
{
#ifdef DEBUG_PRINTS
  // Get the stack trace
  STACK_FRAMES frames;
  rxp.get_stack_frames(frames);

  ostringstream oss_msg;
  char buffer[18];

  oss_msg << "stack frames:" << (int) frames.active_frames << "\n";

  for (unsigned i=0; i < frames.active_frames; i++) {
    sprintf(buffer, "0x%08x", frames.frames[i]);
    oss_msg << "\tframe:" << dec << setw(2) << setfill('0') << i
	    << "  addr:" << buffer << "\n";
  }

  // Get info from predefined macros
  oss_msg << "\tViolator: " << rxp.get_file() 
	  << ":" << rxp.get_line()
	  << ", " << rxp.get_function() << "\n";

  // Get the internal info
  oss_msg << "\tSource: " << rxp.get_source()
	  << ", Code: " << rxp.get_code() << "\n";

  oss_msg << "\tInfo: " << rxp.get_info() << "\n";

  // Print all info
  debug_printf(oss_msg.str().c_str());
#endif

  // Update internal error information
  return update_error(rxp);
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::update_error(raspi_exception rxp)
{
  MUTEX_LOCK(m_error_mutex);
  if (m_last_error_read) {
    m_error_source    = rxp.get_source();
    m_error_code      = rxp.get_code();
    m_last_error_read = false; // Latch last error until read
  }
  MUTEX_UNLOCK(m_error_mutex);

#ifdef DEBUG_PRINTS 
  switch(rxp.get_source()) {
  case RASPI_INTERNAL_ERROR:
    debug_internal_error();
    break;
  case RASPI_LINUX_ERROR:
    debug_linux_error();
    break;
  }
#endif

  return RASPI_FAILURE;
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::internal_get_error_string(long error_code,
					   RASPI_ERROR_STRING error_string)
{
  size_t str_len = sizeof(RASPI_ERROR_STRING);

  switch (error_code) {
  case RASPI_NO_ERROR:
    strncpy(error_string, "No error", str_len);
    break;
  case RASPI_NOT_INITIALIZED:
    strncpy(error_string, "Not initialized", str_len);
    break;
  case RASPI_ALREADY_INITIALIZED:
    strncpy(error_string, "Already initialized", str_len);
    break;
  case RASPI_BAD_ARGUMENT:
    strncpy(error_string, "Bad argument", str_len);
    break;
  case RASPI_MAX_LIMIT_EXCEEDED:
    strncpy(error_string, "Max limit exceeded", str_len);
    break;
  case RASPI_SEMAPHORE_OPERATION_FAILED:
    strncpy(error_string, "Semaphore operation failed", str_len);
    break;
  case RASPI_FILE_OPERATION_FAILED:
    strncpy(error_string, "File operation failed", str_len);
    break;
  case RASPI_SPI_OPERATION_FAILED:
    strncpy(error_string, "SPI operation failed", str_len);
    break;
  case RASPI_UNEXPECTED_EXCEPTION:
    strncpy(error_string, "Unexpected exception", str_len);
    break;
  default: 
    strncpy(error_string, "Undefined error", str_len);
  }

  return RASPI_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long raspi_core::internal_test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info)
{
  long rc = RASPI_SUCCESS;
 
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((RASPI_LIB_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((RASPI_LIB_PROD_INFO *)0)->rstate));

  return rc;
}

/////////////////////////////////////////////////////////////////////////////

void raspi_core::internal_initialize(RASPI_CE ce,
				     RASPI_MODE mode,
				     RASPI_BPW bpw,
				     uint32_t speed)
{
  // Only one master allowed in the system for each device
  check_one_master(ce);

  // Create the i/o object with garbage collector
  raspi_io *io_ptr = new raspi_io(m_spi_dev[ce].device.c_str());
  m_spi_dev[ce].io_auto = auto_ptr<raspi_io>(io_ptr);

  // Initialize i/o object
  m_spi_dev[ce].io_auto->initialize(mode, bpw, speed);
}

/////////////////////////////////////////////////////////////////////////////

void raspi_core::internal_finalize(RASPI_CE ce)
{
  finalize_one_master(ce);

  // Finalize the i/o object
  m_spi_dev[ce].io_auto->finalize();

  // Delete the i/o object
  m_spi_dev[ce].io_auto.reset();
}

/////////////////////////////////////////////////////////////////////////////

void raspi_core::check_one_master(RASPI_CE ce)
{
  // Try to create the master semaphore for device
  m_spi_dev[ce].master_sem = sem_open(m_spi_dev[ce].master_sem_name.c_str(),
				      O_CREAT | O_EXCL,
				      S_IRWXU | S_IRWXG | S_IRWXO,
				      1); // Semaphore is available

  // Like in movie Highlander, there can be only one...
  if (m_spi_dev[ce].master_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      THROW_RXP(RASPI_INTERNAL_ERROR, RASPI_MAX_LIMIT_EXCEEDED,
		"Only one spi master instance allowed, ce=%d",
		ce);
    }
    THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
	      "sem_open for %s",
	      m_spi_dev[ce].master_sem_name.c_str());
  }
}

/////////////////////////////////////////////////////////////////////////////

void raspi_core::finalize_one_master(RASPI_CE ce)
{
  int rc;

  // Destroy the master semaphore for device
  if (m_spi_dev[ce].master_sem) {
    rc = sem_close(m_spi_dev[ce].master_sem);
    if (rc) {
      THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
		"sem_close for %s",
		m_spi_dev[ce].master_sem_name.c_str());
    }
    rc = sem_unlink(m_spi_dev[ce].master_sem_name.c_str());
    if (rc) {
      THROW_RXP(RASPI_LINUX_ERROR, RASPI_SEMAPHORE_OPERATION_FAILED,
		"sem_unlink for %s",
		m_spi_dev[ce].master_sem_name.c_str());
    }
  }  
}
