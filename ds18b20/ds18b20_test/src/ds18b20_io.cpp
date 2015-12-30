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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

#include "ds18b20_io.h"

// Implementation notes:
// 1. Raspberry Pi, device tree overlay: w1-gpio
//
// 2. Raspberry Pi, kernel modules: w1-gpio and w1-therm
// 
// 3. DS18B20 datasheet, Maxim
//    https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define SENSOR_BASE_DIR "/sys/bus/w1/devices"

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

ds18b20_io::ds18b20_io(string sensor_serial_number)
{
  m_dev_serial_number = sensor_serial_number;
}

/////////////////////////////////////////////////////////////////////////////

ds18b20_io::~ds18b20_io(void)
{
}

/////////////////////////////////////////////////////////////////////////////

long ds18b20_io::initialize(void)
{
  int rc;
  DIR *dir;
  struct dirent *dirent;
  bool sensor_found = false;

  // Open sensor directory and find actual sensor
  dir = opendir(SENSOR_BASE_DIR);
  if (dir == NULL) {
    return DS18B20_IO_FILE_OPERATION_FAILED;
  }
  while ((dirent = readdir(dir))) {
    if ( (dirent->d_type == DT_LNK) &&
	 (string(dirent->d_name).find(m_dev_serial_number) != string::npos) ) {
      sensor_found = true;
      break;
    }
  }
  rc = closedir(dir);
  if (rc == -1) {
    return DS18B20_IO_FILE_OPERATION_FAILED;
  }
  if (!sensor_found) {
    return DS18B20_IO_SENSOR_NOT_FOUND;
  }

  return DS18B20_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long ds18b20_io::finalize(void)
{
  return DS18B20_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////

long ds18b20_io::read_temperature(float &value)
{
  int rc;
  int dev_fd;

  // Open sensor device file.
  // This will trigger a new conversion.
  string dev_file_path = string(SENSOR_BASE_DIR) + "/" + m_dev_serial_number + "/w1_slave";
  rc = open(dev_file_path.c_str(), O_RDONLY);
  if (rc == -1) {
    return DS18B20_IO_FILE_OPERATION_FAILED;
  }
  dev_fd = rc;

  // Read from sensor device file
  char data[256];
  unsigned nbytes = sizeof(data);

  unsigned total = 0;
  unsigned bytes_left = nbytes;

  int n = 0;

  while (total < nbytes) {
    n = read(dev_fd, data+total, bytes_left);
    if (n == -1) {
      return DS18B20_IO_FILE_OPERATION_FAILED;
    }
    else if (n == 0) {
      break;
    }
    else {
      total += n;
      bytes_left -= n;
    }
  }

  // Analyze output from sensor device file
  if (string(data).find("YES") == string::npos) {
    return DS18B20_IO_BAD_CRC;
  }
  if (string(data).find("t=") == string::npos) {
    return DS18B20_IO_INVALID_TEMPERATURE;
  }
  string temp_str = string(data).substr(string(data).find("t=")+2, 5);
  value = strtof(temp_str.c_str(), NULL) / 1000.0;

  // Close sensor device file
  rc = close(dev_fd);
  if (rc == -1) {
    return DS18B20_IO_FILE_OPERATION_FAILED;
  }

  return DS18B20_IO_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
