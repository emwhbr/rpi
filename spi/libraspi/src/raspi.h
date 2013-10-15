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

#ifndef __RASPI_H__
#define __RASPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*
 * LIBRASPI Return codes
 */
#define RASPI_SUCCESS         0
#define RASPI_FAILURE        -1
#define RASPI_MUTEX_FAILURE  -2

/*
 * LIBRASPI Internal error codes
 */
#define RASPI_NO_ERROR                      0
#define RASPI_NOT_INITIALIZED               1
#define RASPI_ALREADY_INITIALIZED           2
#define RASPI_BAD_ARGUMENT                  3
#define RASPI_MAX_LIMIT_EXCEEDED            4
#define RASPI_SEMAPHORE_OPERATION_FAILED    5
#define RASPI_FILE_OPERATION_FAILED         6
#define RASPI_OPERATION_WOULD_BLOCK         7
#define RASPI_SPI_OPERATION_FAILED          8
#define RASPI_UNEXPECTED_EXCEPTION          9

/*
 * Error source values
 */
typedef enum {RASPI_INTERNAL_ERROR, 
	      RASPI_LINUX_ERROR} RASPI_ERROR_SOURCE;
/*
 * Flag values
 */
#define RASPI_F_NONBLOCK   0x001

/*
 * Basic API support types
 */

/*
 * API types
 */

typedef char RASPI_ERROR_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} RASPI_LIB_PROD_INFO;

typedef struct {
  RASPI_ERROR_SOURCE error_source;
  long               error_code;
} RASPI_STATUS;

typedef enum {RASPI_CE_0,
              RASPI_CE_1} RASPI_CE;     /* Chip selects */

typedef enum {RASPI_BPW_8,
              RASPI_BPW_9} RASPI_BPW;   /* Bits per word */

typedef enum {RASPI_MODE_0,
	      RASPI_MODE_1,
	      RASPI_MODE_2,
	      RASPI_MODE_3} RASPI_MODE; /* SPI mode */

typedef struct {
  void     *tx_buf;     /* Pointer to the send buffer.    	      */
  void     *rx_buf;     /* Pointer to the receive buffer (or null).   */
  uint32_t nbytes;      /* Buffer length in bytes.       	      */              
  uint16_t delay_usecs; /* If nonzero, delay after last bit transfer. */
  bool     ce_deactive; /* True to deselect device before starting
			   next transfer. False to keep CE active.    */
} RASPI_TRANSFER;

/****************************************************************************
*
* Name raspi_get_last_error
*
* Description Returns the error information held by LIBRASPI, when a call
*             returns unsuccessful completion. 
*             LIBRASPI clears its internal error information after it has
*             been read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns RASPI_SUCCESS if successful
*                otherwise RASPI_FAILURE or RASPI_MUTEX_FAILURE
*
****************************************************************************/
extern long raspi_get_last_error(RASPI_STATUS *status);

/****************************************************************************
*
* Name raspi_get_error_string
*
* Description Returns the error string corresponding to the provided
*             internal error code.
*
* Parameters error_code    IN      Actual error code
*            error_string  IN/OUT  Pointer to a buffer to hold the error string
*
* Error handling Returns always RASPI_SUCCESS
*
****************************************************************************/
extern long raspi_get_error_string(long error_code, 
				   RASPI_ERROR_STRING error_string);

/****************************************************************************
*
* Name raspi_initialize
*
* Description Allocates system resources and performs operations that are
*             necessary to be able to communicate with a device using the
*             SPI interface for specified chip select.
*             This function shall be called once for each chip select
*             to make LIBRASPI operational for that chip select.
*             This function can be called again after finalization.
*
* Parameters ce     IN  Identifies the chip select.
*            mode   IN  SPI mode.
*            bpw    IN  Bits per word used in SPI transfers.
*            speed  IN  Bitrate (Hz).
*            flags  IN  Controls operational behaviour.
*
* Error handling Returns RASPI_SUCCESS if successful
*                otherwise RASPI_FAILURE or RASPI_MUTEX_FAILURE
*
****************************************************************************/
extern long raspi_initialize(RASPI_CE ce,
			     RASPI_MODE mode,
			     RASPI_BPW bpw,
			     uint32_t speed,
			     int flags);

/****************************************************************************
*
* Name raspi_finalize
*
* Description Releases any resources that were claimed during initialization.
*
* Parameters ce  IN  Identifies the chip select.
*
* Error handling Returns RASPI_SUCCESS if successful
*                otherwise RASPI_FAILURE or RASPI_MUTEX_FAILURE
*
****************************************************************************/
extern long raspi_finalize(RASPI_CE ce);

/****************************************************************************
*
* Name raspi_xfer
*
* Description Performs a single SPI transfer to/from specifed buffers.
*
* Parameters ce       IN   Identifies the chip select.
*            *tx_buf  IN   Pointer to the send buffer.
*            *rx_buf  OUT  Pointer to the receive buffer (or null).
*            nbytes   IN   Buffer lenght in bytes.
*
* Error handling Returns RASPI_SUCCESS if successful
*                otherwise RASPI_FAILURE or RASPI_MUTEX_FAILURE
*
****************************************************************************/
extern long raspi_xfer(RASPI_CE ce,
		       const void *tx_buf,
		       void *rx_buf,
		       uint32_t nbytes);

/****************************************************************************
*
* Name raspi_xfer_n
*
* Description Performs multiple SPI transfers according to transfer list.
*
* Parameters ce              IN   Identifies the chip select.
*            *transfer_list  IN   Pointer to a list of transfer definitions.
*            transfers       IN   Number of transfers in list.
*
* Error handling Returns RASPI_SUCCESS if successful
*                otherwise RASPI_FAILURE or RASPI_MUTEX_FAILURE
*
****************************************************************************/
extern long raspi_xfer_n(RASPI_CE ce,
			 const RASPI_TRANSFER *transfer_list,
			 unsigned transfers);

/****************************************************************************
*
* Name raspi_test_get_lib_prod_info
*
* Description Returns the product number and the RState of LIBRASPI.
*
* Parameters prod_info  IN/OUT  Pointer to a buffer to hold the product
*                               number and the RState.
*
* Error handling Returns always RASPI_SUCCESS.
*
****************************************************************************/
extern long raspi_test_get_lib_prod_info(RASPI_LIB_PROD_INFO *prod_info);

#ifdef  __cplusplus
}
#endif

#endif /* __RASPI_H__ */
