/***********************************************************************
*                                                                      *
* Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 2 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* User space application for testing the BACH kernel driver.           *
*                                                                      *
************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BACH_DEV "/dev/bach0"

#define WRITE_BUFFER_SIZE_IN_BYTES 100
#define READ_BUFFER_SIZE_IN_BYTES   50

static uint8_t g_write_buffer[WRITE_BUFFER_SIZE_IN_BYTES];
static uint8_t g_read_buffer[READ_BUFFER_SIZE_IN_BYTES];

int main(void)
{
  int fd;
  ssize_t rc;

  /* Open device */
  fd = open(BACH_DEV, O_RDWR);
  if (fd < 0) {
    printf("*** Error: open\n");
    return 1;
  }

  /* Read device */
  rc = read(fd, g_read_buffer, sizeof(g_read_buffer));
  if (rc < 0) {
    perror("*** Error: read");
    close(fd);
    return 1;
  }
  printf("Read: expected=%d, actual=%d\n",
	 sizeof(g_read_buffer), rc);

  /* Write device */
  rc = write(fd, g_write_buffer, sizeof(g_write_buffer));
  if (rc < 0) {
    perror("*** Error: write");
    close(fd);
    return 1;
  }
  printf("Write: expected=%d, actual=%d\n",
	 sizeof(g_write_buffer), rc);

  /* Close device */
  close(fd);

  return 0;
}
