// ************************************************************************
// *                                                                      *
// * Copyright (C) 2015 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __DS18B20_IO_H__
#define __DS18B20_IO_H__

#include <string>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////
// Return codes
#define DS18B20_IO_SUCCESS                 0
#define DS18B20_IO_FILE_OPERATION_FAILED  -1
#define DS18B20_IO_SENSOR_NOT_FOUND       -2
#define DS18B20_IO_BAD_CRC                -3
#define DS18B20_IO_INVALID_TEMPERATURE    -4

/////////////////////////////////////////////////////////////////////////////
//               Class support types
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//               Definition of classes
/////////////////////////////////////////////////////////////////////////////
class ds18b20_io {
  
 public:
  ds18b20_io(string sensor_serial_number);
  ~ds18b20_io(void);

  long initialize(void);
  long finalize(void);

  long read_temperature(float &value);

 private:  
  string m_dev_serial_number;
};

#endif // __DS18B20_IO_H__
