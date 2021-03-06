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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <sstream>
#include <iomanip>

#include "eprom24x_core.h"
#include "eprom24x_exception.h"

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
#define PRODUCT_NUMBER   "LIBEPROM24x"
#define RSTATE           "R1A01"

#define MUTEX_LOCK(mutex) \
  ({ if (pthread_mutex_lock(&mutex)) { \
      return EPROM24x_MUTEX_FAILURE; \
    } })

#define MUTEX_UNLOCK(mutex) \
  ({ if (pthread_mutex_unlock(&mutex)) { \
      return EPROM24x_MUTEX_FAILURE; \
    } })

#ifdef DEBUG_PRINTS
//
// Notes!
// Macro 'debug_printf' can be used anywhere in LIBEPROM24x.
// The other macros can only be used in function 'update_error'.
//
#define debug_printf(fmt, args...)  printf("LIBEPROM24x - "); \
                                    printf(fmt, ##args); \
				    fflush(stdout)

#define debug_linux_error()         printf("LIBEPROM24x LINUX ERROR - "); \
                                    error(0, errno, NULL); \
				    fflush(stdout)

#define debug_internal_error()      printf("LIBEPROM24x INTERNAL ERROR\n"); \
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

eprom24x_core::eprom24x_core(void)
{
  m_error_source    = EPROM24x_INTERNAL_ERROR;
  m_error_code      = 0;
  m_last_error_read = true;
  pthread_mutex_init(&m_error_mutex, NULL); // Use default mutex attributes

  m_initialized = false;
  pthread_mutex_init(&m_init_mutex, NULL); // Use default mutex attributes
}

/////////////////////////////////////////////////////////////////////////////

eprom24x_core::~eprom24x_core(void)
{
  pthread_mutex_destroy(&m_error_mutex);
  pthread_mutex_destroy(&m_init_mutex);
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::get_last_error(EPROM24x_STATUS *status)
{
  try {
    MUTEX_LOCK(m_error_mutex);
    status->error_source = m_error_source;
    status->error_code   = m_error_code;
    
    // Clear internal error information
    m_error_source    = EPROM24x_INTERNAL_ERROR;
    m_error_code      = EPROM24x_NO_ERROR;
    m_last_error_read = true;
    MUTEX_UNLOCK(m_error_mutex);
    return EPROM24x_SUCCESS;
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::get_error_string(long error_code, 
				     EPROM24x_ERROR_STRING error_string)
{
  try {
    // Do the actual work
    return internal_get_error_string(error_code, error_string);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::initialize(EPROM24x_DEVICE eprom_device,
			       uint8_t i2c_address,
			       const char *i2c_dev)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if already initialized
    if (m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_ALREADY_INITIALIZED,
		"Already initialized");
    }

    // Check input values
    if (!i2c_dev) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"i2c_dev is null pointer");
    }

    // Do the actual initialization
    internal_initialize(eprom_device, i2c_address, i2c_dev);

    // Initialization completed
    m_initialized = true;
    MUTEX_UNLOCK(m_init_mutex);

    return EPROM24x_SUCCESS;
  }
  catch (eprom24x_exception &rxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(rxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::finalize(void)
{
  try {
    MUTEX_LOCK(m_init_mutex);

    // Check if initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual finalization
    internal_finalize();

    // Finalization completed
    m_initialized = false;
    MUTEX_UNLOCK(m_init_mutex);

    return EPROM24x_SUCCESS;
  }
  catch (eprom24x_exception &rxp) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(rxp);
  }
  catch (...) {
    MUTEX_UNLOCK(m_init_mutex);
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::read_u8(uint32_t addr, uint8_t *value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!value) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"value is null pointer");
    }
    
    // Do the actual work
    return m_eprom24x_io_auto->read(addr, (void *)value, sizeof(uint8_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::read_u16(uint32_t addr, uint16_t *value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!value) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"value is null pointer");
    }
    
    // Do the actual work
    return m_eprom24x_io_auto->read(addr, (void *)value, sizeof(uint16_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::read_u32(uint32_t addr, uint32_t *value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!value) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"value is null pointer");
    }
    
    // Do the actual work
    return m_eprom24x_io_auto->read(addr, (void *)value, sizeof(uint32_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::read(uint32_t addr, void *data, uint16_t len)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!data) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"data is null pointer");
    }
    
    // Do the actual work
    return m_eprom24x_io_auto->read(addr, data, len);
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::write_u8(uint32_t addr, uint8_t value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    return m_eprom24x_io_auto->write(addr, (const void*)&value, sizeof(uint8_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::write_u16(uint32_t addr, uint16_t value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    return m_eprom24x_io_auto->write(addr, (const void *)&value, sizeof(uint16_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::write_u32(uint32_t addr, uint32_t value)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    return m_eprom24x_io_auto->write(addr, (const void*)&value, sizeof(uint32_t));
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::write(uint32_t addr, const void *data, uint16_t len)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Check input values
    if (!data) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_BAD_ARGUMENT,
		"data is null pointer");
    }

    // Do the actual work
    return m_eprom24x_io_auto->write(addr, data, len);
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::erase(void)
{
  try {
    // Check if not initialized
    if (!m_initialized) {
      THROW_RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_NOT_INITIALIZED,
		"Not initialized");
    }

    // Do the actual work
    return m_eprom24x_io_auto->erase();
  }
  catch (eprom24x_exception &rxp) {
    return set_error(rxp);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info)
{
  try {
    // Do the actual work
    return internal_test_get_lib_prod_info(prod_info);
  }
  catch (...) {
    return set_error(RXP(EPROM24x_INTERNAL_ERROR, EPROM24x_UNEXPECTED_EXCEPTION, NULL));
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::set_error(eprom24x_exception rxp)
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

long eprom24x_core::update_error(eprom24x_exception rxp)
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
  case EPROM24x_INTERNAL_ERROR:
    debug_internal_error();
    break;
  case EPROM24x_LINUX_ERROR:
    debug_linux_error();
    break;
  }
#endif

  return EPROM24x_FAILURE;
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::internal_get_error_string(long error_code,
					      EPROM24x_ERROR_STRING error_string)
{
  size_t str_len = sizeof(EPROM24x_ERROR_STRING);

  switch (error_code) {
  case EPROM24x_NO_ERROR:
    strncpy(error_string, "No error", str_len);
    break;
  case EPROM24x_NOT_INITIALIZED:
    strncpy(error_string, "Not initialized", str_len);
    break;
  case EPROM24x_ALREADY_INITIALIZED:
    strncpy(error_string, "Already initialized", str_len);
    break;
  case EPROM24x_BAD_ARGUMENT:
    strncpy(error_string, "Bad argument", str_len);
    break;
  case EPROM24x_OPERATION_NOT_ALLOWED:
    strncpy(error_string, "Operation not allowed", str_len);
    break; 
  case EPROM24x_FILE_OPERATION_FAILED:
    strncpy(error_string, "File operation failed", str_len);
    break;
  case EPROM24x_I2C_OPERATION_FAILED:
    strncpy(error_string, "I2C operation failed", str_len);
    break;
  case EPROM24x_EPROM_NOT_SUPPORTED:
    strncpy(error_string, "EPROM not supported", str_len);
    break;
  case EPROM24x_EPROM_NOT_RESPONDING:
    strncpy(error_string, "EPROM not responding", str_len);
    break;
  case EPROM24x_EPROM_INVALID_ADDRESS:
    strncpy(error_string, "EPROM invalid address", str_len);
    break;  
  case EPROM24x_CLOCK_OPERATION_FAILED:
    strncpy(error_string, "Clock operation failed", str_len);
    break;
  case EPROM24x_UNEXPECTED_EXCEPTION:
    strncpy(error_string, "Unexpected exception", str_len);
    break;
  default: 
    strncpy(error_string, "Undefined error", str_len);
  }

  return EPROM24x_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long eprom24x_core::internal_test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info)
{
  long rc = EPROM24x_SUCCESS;
 
  strncpy(prod_info->prod_num, 
	  PRODUCT_NUMBER, 
	  sizeof(((EPROM24x_LIB_PROD_INFO *)0)->prod_num));

  strncpy(prod_info->rstate, 
	  RSTATE, 
	  sizeof(((EPROM24x_LIB_PROD_INFO *)0)->rstate));

  return rc;
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_core::internal_initialize(EPROM24x_DEVICE eprom_device,
					uint8_t i2c_address,
					const char *i2c_dev)
{
  // Create the i/o object with garbage collector
  eprom24x_io *io_ptr = new eprom24x_io(eprom_device, i2c_address, i2c_dev);
  m_eprom24x_io_auto = auto_ptr<eprom24x_io>(io_ptr);

  // Initialize i/o object
  m_eprom24x_io_auto->initialize();
}

/////////////////////////////////////////////////////////////////////////////

void eprom24x_core::internal_finalize(void)
{
  // Finalize the i/o object
  m_eprom24x_io_auto->finalize();

  // Delete the i/o object
  m_eprom24x_io_auto.reset();
}
