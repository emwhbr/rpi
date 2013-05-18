/*************************************************************
*                                                            *
* Copyright (C) Bonden i Nol                                 *
*                                                            *
**************************************************************/

#ifndef __EPROM24x_IO_H__
#define __EPROM24x_IO_H__

#include <string>

#include "eprom24x.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////

class eprom24x_io {

public:
  eprom24x_io(EPROM24x_DEVICE eprom_device,
	      uint8_t i2c_address,
	      const char *i2c_dev);
  ~eprom24x_io(void);

  void initialize(void);
  void finalize(void);

  long read_u8(uint32_t addr, uint8_t *value);
  long write_u8(uint32_t addr, uint8_t value);

private:
  bool     m_eprom_supported;
  uint8_t  m_nr_address_bytes;
  uint32_t m_eprom_size_in_bytes;
  uint8_t  m_i2c_address;
  string   m_i2c_dev;
  int      m_i2c_fd;

  void init_members(void);
};

#endif // __EPROM24x_IO_H__
