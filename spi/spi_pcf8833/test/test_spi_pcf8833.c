/************************************************************************
 *                                                                      *
 * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

/*****************************************************************
 *                  Definition of macros
 *****************************************************************/
#define SPI_PCF8833_DEV  "/dev/spi-pcf8833-0"

/* Philips PCF8833 LCD controller commands.
 * Data sheet, 2003 Feb 14 (page 8-11) */
#define CMD_NOP       0x00
#define CMD_SLEEPIN   0x10
#define CMD_SLEEPOUT  0x11
#define CMD_INVOFF    0x20
#define CMD_INVON     0x21
#define CMD_SETCON    0x25
#define CMD_DISPOFF   0x28
#define CMD_DISPON    0x29
#define CMD_CASET     0x2A
#define CMD_PASET     0x2B
#define CMD_RAMWR     0x2C
#define CMD_COLMOD    0x3A
#define CMD_MADCTL    0x36

/* The LCD is a 132 x 132 pixel matrix */
#define LCD6100_ROW_MAX_ADDR  131
#define LCD6100_COL_MAX_ADDR  131

/*****************************************************************
 *                  Definition of types
 *****************************************************************/

/* 12-bit RGB : rrrrgggbbbb */
typedef union {
  struct {
    uint8_t blue     : 4;
    uint8_t green    : 4;
    uint8_t red      : 4;
    uint8_t not_used : 4;
  } __attribute__ ((packed)) bs;
  uint16_t wd;
} LCD6100_COLOUR; 


/*****************************************************************
 *                  Function prototypes
 *****************************************************************/
static void open_device(void);
static void close_device(void);
static void write_device(void);
static void init_sequence(void);
static void fill_screen(void);
static void read_device(void);
static void write_command(uint8_t cmd);
static void write_data(uint8_t data);
static void set_drawing_limits(uint8_t start_row,
			       uint8_t start_col,
			       uint8_t end_row,
			       uint8_t end_col);
static LCD6100_COLOUR get_user_colour(void);
static void print_menu(void);
static void do_test_spi_pcf8833(void);

/*****************************************************************
 *                  Global variables
 *****************************************************************/
static int g_fd_spi_pcf8833 = -1;

/*****************************************************************/

static void open_device(void)
{
  int fd;

  fd = open(SPI_PCF8833_DEV, O_RDWR);
  if (fd < 0) {
    perror("*** Error: open");
    return;
  }

  g_fd_spi_pcf8833 = fd;
}

/*****************************************************************/

static void close_device(void)
{
  int rc;

  rc = close(g_fd_spi_pcf8833);
  if (rc) {
    perror("*** Error: close");
    return;
  }

  g_fd_spi_pcf8833 = -1;
}

/*****************************************************************/

static void write_device(void)
{
  ssize_t rc;
  unsigned message_value;
  uint16_t message;

  /* User input */
  printf("Enter 9-bit message[hex]: 0x");
  scanf("%x", &message_value);

  message = 0x01ff & ( (uint16_t) message_value );

  rc = write(g_fd_spi_pcf8833, &message, sizeof(message));
  if (rc < 0) {
    perror("*** Error: write");
    return;
  }
}

/*****************************************************************/

static void init_sequence(void)
{
  /*
   * Philips PCF8833 LCD controller init sequence
   */

  /* Step 1. Reset
   * JOE: Not yet implemented
   */

  /* Step 2. Sleep out */
  write_command(CMD_SLEEPOUT);

  /* Step 3. Inversion off */
  write_command(CMD_INVOFF);

  /* Step 4. Colour interface pixel format */
  write_command(CMD_COLMOD);
  write_data(0x03);  /* 12-bit per pixel */

  /* Step 5. Memory data access control */
  write_command(CMD_MADCTL);
  write_data(0x80);  /* Mirror Y, RGB */

  /* Step 6. Contrast */
  write_command(CMD_SETCON);
  write_data(0x30);

  /* Step 7. Wait */
  /* JOE: delay(0.05); */

  /* Step 8. Turn ON display */
  write_command(CMD_DISPON);
}

/*****************************************************************/

static void fill_screen(void)
{
  unsigned i;
  LCD6100_COLOUR colour;

  /* User input */
  colour = get_user_colour();

  /* Drawing area is entire screen */
  set_drawing_limits(0, 0,
		     LCD6100_ROW_MAX_ADDR, LCD6100_COL_MAX_ADDR);

  const unsigned tot_nr_pixels = 
    (LCD6100_ROW_MAX_ADDR + 1) * (LCD6100_COL_MAX_ADDR + 1);

  /* Fill screen using specified RGB colour value
   * Two pixels = 2 * 12bit = 24bit = 3 bytes */
  write_command(CMD_RAMWR);
  for (i=0; i < (tot_nr_pixels / 2); i++) {
    write_data( (colour.wd >> 4) & 0xFF );
    write_data( ((colour.wd & 0x0F) << 4) | ((colour.wd >> 8) & 0x0F) );
    write_data( colour.wd & 0xFF );
  }
}

/*****************************************************************/

static void read_device(void)
{
  ssize_t rc;
  uint16_t message;

  /*
   * Note!
   * Read operation not supported by driver.
   * This call should always fail.
   */

  rc = read(g_fd_spi_pcf8833, &message, sizeof(message));
  if (rc < 0) {
    perror("*** Error: read");
    return;
  }
}

/*****************************************************************/

static void write_command(uint8_t cmd)
{
  int rc;
  uint16_t lcd_cmd = cmd; /* Bit 8 is cleared ==> command */

  /* Send command to LCD */
  rc = write(g_fd_spi_pcf8833, &lcd_cmd, sizeof(lcd_cmd));
  if (rc < 0) {
    perror("*** Error: write_command");
    return;
  }
}

/*****************************************************************/

static void write_data(uint8_t data)
{
  int rc;
  uint16_t lcd_data = data | 0x0100; /* Bit 8 is set ==> data */

  /* Send data to LCD */
  rc = write(g_fd_spi_pcf8833, &lcd_data, sizeof(lcd_data));
  if (rc < 0) {
    perror("*** Error: write_data");
    return;
  }
}

/*****************************************************************/

static void set_drawing_limits(uint8_t start_row,
			       uint8_t start_col,
			       uint8_t end_row,
			       uint8_t end_col)
{
  /* Row address */
  write_command(CMD_PASET);
  write_data(start_row); /* Start row */
  write_data(end_row);   /* End row   */

  /* Column address */
  write_command(CMD_CASET);
  write_data(start_col); /* Start column */
  write_data(end_col);   /* End column   */
}

/*****************************************************************/

static LCD6100_COLOUR get_user_colour(void)
{
  unsigned val;
  LCD6100_COLOUR colour;

  /* User input */
  printf("Enter (R)GB[hex]: 0x");
  scanf("%x", &val);
  colour.bs.red = val;

  printf("Enter R(G)B[hex]: 0x");
  scanf("%x", &val);
  colour.bs.green = val;

  printf("Enter RG(B)[hex]: 0x");
  scanf("%x", &val);
  colour.bs.blue = val;

  return colour;
}

/*****************************************************************/

static void print_menu(void)
{
  printf("-------------------------------------\n");
  printf("------ SPI PCF8833 TEST MENU --------\n");
  printf("-------------------------------------\n");
  printf("\n");
  printf("  1. open device\n");
  printf("  2. close device\n");
  printf("  3. write device\n");
  printf("  4. init sequence\n");
  printf("  5. fill screen\n");
  printf("  6. (error) read device\n");
  printf("100. Exit\n\n");
}

/*****************************************************************/

static void do_test_spi_pcf8833(void)
{  
  int value;

  do {
    print_menu();
    
    printf("Enter choice : ");
    scanf("%d",&value);
    
    switch(value) {
    case 1:
      open_device();
      break;
    case 2:
      close_device();
      break;
    case 3:
      write_device();
      break;
    case 4:
      init_sequence();
      break;
    case 5:
      fill_screen();
      break;
    case 6:
      read_device();
      break;
    case 100: /* Exit */
      break;
    default:
      printf("Illegal choice!\n");
    }
  } while (value != 100);

  return;
}

/*****************************************************************/

int main(int argc, char *argv[])
{
  do_test_spi_pcf8833();
  
  printf("Goodbye!\n");
  return 0;
}
