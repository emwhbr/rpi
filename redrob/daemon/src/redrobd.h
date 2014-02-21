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

#ifndef __REDROBD_H__
#define __REDROBD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * REDROBD daemon name
 */
#define REDROBD_NAME "redrobd"

/*
 * REDROBD Return codes
 */
#define REDROBD_SUCCESS         0
#define REDROBD_FAILURE        -1
#define REDROBD_MUTEX_FAILURE  -2

/*
 * REDROBD Internal error codes
 */
#define REDROBD_NO_ERROR                   0
#define REDROBD_NOT_INITIALIZED            1
#define REDROBD_ALREADY_INITIALIZED        2
#define REDROBD_BAD_ARGUMENT               3
#define REDROBD_CFG_FILE_BAD_FORMAT        4
#define REDROBD_CFG_FILE_UNEXCPECTED_ERROR 5
#define REDROBD_TIMEOUT_OCCURRED           6
#define REDROBD_TIME_ERROR                 7
#define REDROBD_FILE_OPERATION_FAILED      8
#define REDROBD_SPI_OPERATION_FAILED       9
#define REDROBD_SIGNAL_OPERATION_FAILED    10
#define REDROBD_SOCKET_OPERATION_FAILED    11
#define REDROBD_THREAD_OPERATION_FAILED    12
#define REDROBD_THREAD_STATE_NOT_OK        13
#define REDROBD_THREAD_STATUS_NOT_OK       14
#define REDROBD_UNEXPECTED_EXCEPTION       15

/*
 * Error source values
 */
typedef enum {REDROBD_INTERNAL_ERROR, 
	      REDROBD_LINUX_ERROR} REDROBD_ERROR_SOURCE;

/*
 * API types
 */
typedef char REDROBD_STRING[256];

typedef struct {
  char prod_num[20];
  char rstate[10];
} REDROBD_PROD_INFO;

typedef struct {
  REDROBD_ERROR_SOURCE error_source;
  long                 error_code;
} REDROBD_STATUS;

typedef struct {
  bool           daemonize;
  REDROBD_STRING user;
  REDROBD_STRING work_dir;
  REDROBD_STRING lock_file;
  REDROBD_STRING log_file;
  bool           log_stdout;
  double         supervision_freq;
  double         ctrl_thread_freq;
  bool           verbose;
} REDROBD_CONFIG;

/****************************************************************************
*
* Name redrobd_prod_info
*
* Description Returns the product number and the RState of REDROBD.
*
* Parameters prod_info  IN/OUT  Pointer to a buffer to hold the product
*                               number and the RState.
*
* Error handling Returns always REDROBD_SUCCESS.
*
****************************************************************************/
extern long redrobd_get_prod_info(REDROBD_PROD_INFO *prod_info);

/****************************************************************************
*
* Name redrobd_get_config
*
* Description Returns the configuration of REDROBD.
*
* Parameters config  IN/OUT  Pointer to a buffer to hold the configuration
*
* Error handling Returns REDROBD_SUCCESS if successful
*                otherwise REDROBD_FAILURE or REDROBD_MUTEX_FAILURE
*
****************************************************************************/
extern long redrobd_get_config(REDROBD_CONFIG *config);

/****************************************************************************
*
* Name redrobd_get_last_error
*
* Description Returns the error information held by REDROBD, when a call
*             returns unsuccessful completion. 
*             REDROBD clears its internal error information after it has
*             been read by the calling application.
*
* Parameters status  IN/OUT  pointer to a buffer to hold the error information
*
* Error handling Returns REDROBD_SUCCESS if successful
*                otherwise REDROBD_FAILURE or REDROBD_MUTEX_FAILURE
*
****************************************************************************/
extern long redrobd_get_last_error(REDROBD_STATUS *status);

/****************************************************************************
*
* Name redrobd_check_run_status
*
* Description Check the running status of REDROBD, including all threads.
*
* Parameters None
*
* Error handling Returns REDROBD_SUCCESS if successful
*                otherwise REDROBD_FAILURE or REDROBD_MUTEX_FAILURE
*
****************************************************************************/
extern long redrobd_check_run_status(void);

/****************************************************************************
*
* Name redrobd_initialize
*
* Description Allocates system resources and performs operations that are
*             necessary to start REDROBD.
*
* Parameters logfile                IN  Path to daemon internal log file
*            log_stdout             IN  Controls if internal log to STDOUT
*            ctrl_thread_frequency  IN  Frequency (Hz) of control thread
*            verbose                IN  Full verbose logging enable
*
* Error handling Returns REDROBD_SUCCESS if successful
*                otherwise REDROBD_FAILURE or REDROBD_MUTEX_FAILURE
*
****************************************************************************/
extern long redrobd_initialize(const char *logfile,
			       bool log_stdout,
			       double ctrl_thread_frequency,
			       bool verbose);

/****************************************************************************
*
* Name redrobd_finalize
*
* Description Releases any resources that were claimed during initialization.
*
* Parameters None 
*
* Error handling Returns REDROBD_SUCCESS if successful
*                otherwise REDROBD_FAILURE or REDROBD_MUTEX_FAILURE
*
****************************************************************************/
extern long redrobd_finalize(void);

#ifdef  __cplusplus
}
#endif

#endif /* __REDROBD_H__ */
