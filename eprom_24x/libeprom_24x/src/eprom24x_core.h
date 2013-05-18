/*************************************************************
*                                                            *
* Copyright (C) Bonden i Nol                                 *
*                                                            *
**************************************************************/

#ifndef __EPROM24x_CORE_H__
#define __EPROM24x_CORE_H__

#include <pthread.h>
#include <memory>

#include "eprom24x.h"
#include "eprom24x_exception.h"
#include "eprom24x_io.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class eprom24x_core {

public:
  eprom24x_core(void);
  ~eprom24x_core(void);

  long get_last_error(EPROM24x_STATUS *status);

  long get_error_string(long error_code,
			EPROM24x_ERROR_STRING error_string);

  long initialize(EPROM24x_DEVICE eprom_device,
		  uint8_t i2c_address,
		  const char *i2c_dev);

  long finalize(void);

  long read_u8(uint32_t addr, uint8_t *value);
  long write_u8(uint32_t addr, uint8_t value);

  long test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info);

private:
  // Error handling information
  EPROM24x_ERROR_SOURCE m_error_source;
  long                  m_error_code;
  bool                  m_last_error_read;
  pthread_mutex_t       m_error_mutex;

  // Keep track of initialization
  bool             m_initialized;
  pthread_mutex_t  m_init_mutex;

  // The i/o object
  auto_ptr<eprom24x_io> m_eprom24x_io_auto;

  // Private member functions
  long set_error(eprom24x_exception rxp);
  long update_error(eprom24x_exception rxp);

  long internal_get_error_string(long error_code,
				 EPROM24x_ERROR_STRING error_string);

  long internal_test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info);

  void internal_initialize(EPROM24x_DEVICE eprom_device,
			   uint8_t i2c_address,
			   const char *i2c_dev);

  void internal_finalize(void);
};

#endif // __EPROM24x_CORE_H__
