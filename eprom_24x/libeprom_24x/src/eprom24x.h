/*************************************************************
*                                                            *
* Copyright (C) Bonden i Nol                                 *
*                                                            *
**************************************************************/

#ifndef __EPROM24x_H__
#define __EPROM24x_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * LIBEPROM24x Return codes
 */
#define EPROM24x_SUCCESS         0
#define EPROM24x_FAILURE        -1
#define EPROM24x_MUTEX_FAILURE  -2

/*
 * LIBEPROM24x Internal error codes
 */
#define EPROM24x_NO_ERROR                      0
#define EPROM24x_NOT_INITIALIZED               1
#define EPROM24x_ALREADY_INITIALIZED           2
#define EPROM24x_BAD_ARGUMENT                  3
#define EPROM24x_FILE_OPERATION_FAILED         4
#define EPROM24x_I2C_OPERATION_FAILED          5
#define EPROM24x_EPROM_NOT_SUPPORTED           6
#define EPROM24x_EPROM_NOT_RESPONDING          7
#define EPROM24x_UNEXPECTED_EXCEPTION          8

/*
 * Error source values
 */
typedef enum {EPROM24x_INTERNAL_ERROR, 
	      EPROM24x_LINUX_ERROR} EPROM24x_ERROR_SOURCE;

/*
 * Basic API support types
 */


/*
 * API types
 */

typedef char EPROM24x_ERROR_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} EPROM24x_LIB_PROD_INFO;

typedef struct {
  EPROM24x_ERROR_SOURCE error_source;
  long                  error_code;
} EPROM24x_STATUS;

typedef enum {EPROM24x_128bit,
              EPROM24x_1Kbit,
              EPROM24x_2Kbit,
	      EPROM24x_4Kbit,
              EPROM24x_8Kbit,
	      EPROM24x_16Kbit,
              EPROM24x_32Kbit,
	      EPROM24x_64Kbit,
              EPROM24x_128Kbit,
	      EPROM24x_256Kbit,
              EPROM24x_512Kbit,
	      EPROM24x_1Mbit} EPROM24x_DEVICE;

/****************************************************************************
*
* Name eprom24x_get_last_error
*
* Description Returns the error information held by LIBEPROM24x, when a call
*             returns unsuccessful completion. 
*             LIBEPROM24x clears its internal error information after it has
*             been read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_get_last_error(EPROM24x_STATUS *status);

/****************************************************************************
*
* Name eprom24x_get_error_string
*
* Description Returns the error string corresponding to the provided
*             internal error code.
*
* Parameters error_code    IN      Actual error code
*            error_string  IN/OUT  Pointer to a buffer to hold the error string
*
* Error handling Returns always EPROM24x_SUCCESS
*
****************************************************************************/
extern long eprom24x_get_error_string(long error_code, 
				      EPROM24x_ERROR_STRING error_string);

/****************************************************************************
*
* Name eprom24x_initialize
*
* Description TBD (Add more info later)
*
* Parameters eprom_device IN  Identifies the eprom.
*            i2c_address  IN  I2C address of eprom.
*            *i2c_dev     IN  I2C device file.
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_initialize(EPROM24x_DEVICE eprom_device,
				uint8_t i2c_address,
				const char *i2c_dev);

/****************************************************************************
*
* Name eprom24x_finalize
*
* Description TBD
*
* Parameters None 
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_finalize(void);

/****************************************************************************
*
* Name eprom24x_read_u8
*
* Description Reads 8 bits from EPROM.
*
* Parameters addr    IN   Address to read from.
*            *value  OUT  Pointer to a buffer to hold data read from EPROM.
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_read_u8(uint32_t addr, uint8_t *value);

/****************************************************************************
*
* Name eprom24x_read_u16
*
* Description Reads 16 bits from EPROM.
*
* Parameters addr    IN   Address to read from.
*            *value  OUT  Pointer to a buffer to hold data read from EPROM.
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_read_u16(uint32_t addr, uint16_t *value);

/****************************************************************************
*
* Name eprom24x_read_u32
*
* Description Reads 32 bits from EPROM.
*
* Parameters addr    IN   Address to read from.
*            *value  OUT  Pointer to a buffer to hold data read from EPROM.
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_read_u32(uint32_t addr, uint32_t *value);

/****************************************************************************
*
* Name eprom24x_write_u8
*
* Description Writes 8 bits to EPROM.
*
* Parameters addr   IN  Address to write to.
*            value  IN  Value to be written to EPROM.
*
* Error handling Returns EPROM24x_SUCCESS if successful
*                otherwise EPROM24x_FAILURE or EPROM24x_MUTEX_FAILURE
*
****************************************************************************/
extern long eprom24x_write_u8(uint32_t addr, uint8_t value);

/****************************************************************************
*
* Name eprom24x_test_get_lib_prod_info
*
* Description Returns the product number and the RState of LIBEPROM24x.
*
* Parameters prod_info  IN/OUT  Pointer to a buffer to hold the product number
*                               and the RState.
*
* Error handling Returns always EPROM24x_SUCCESS.
*
****************************************************************************/
extern long eprom24x_test_get_lib_prod_info(EPROM24x_LIB_PROD_INFO *prod_info);

#ifdef  __cplusplus
}
#endif

#endif /* __EPROM24x_H__ */
