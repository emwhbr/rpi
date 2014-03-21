// ************************************************************************
// *                                                                      *
// * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#include <strings.h>
#include <string.h>
#include <sstream>
#include <iomanip>

#include "redrobd_rc_net_server_thread.h"
#include "socket_support.h"
#include "redrobd_log.h"
#include "redrobd_error_utility.h"
#include "daemon_utility.h"
#include "redrobd.h"
#include "excep.h"

/////////////////////////////////////////////////////////////////////////////
//               Definitions of macros
/////////////////////////////////////////////////////////////////////////////
#define DOTTED_IP_ADDR_LEN  20

// Client commands
#define CLI_CMD_STEER          1
#define CLI_CMD_GET_VOLTAGE    2
#define CLI_CMD_CAMERA         3
#define CLI_CMD_GET_SYS_STATS  4

/////////////////////////////////////////////////////////////////////////////
//               Definition of types and constants
/////////////////////////////////////////////////////////////////////////////

static const int client_com_error = 1; // Exception for send/recv client

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

redrobd_rc_net_server_thread::
redrobd_rc_net_server_thread(string thread_name,
			     string server_ip_address,
			     uint16_t server_port) : thread(thread_name)
{
  m_server_ip_address = server_ip_address;
  m_server_port = server_port;

  // Use default mutex attributes
  pthread_mutex_init(&m_steer_code_mutex, NULL);
  pthread_mutex_init(&m_voltage_mutex, NULL);
  pthread_mutex_init(&m_camera_code_mutex, NULL);
  pthread_mutex_init(&m_sys_stat_mutex, NULL);
  
  init_members();
}

////////////////////////////////////////////////////////////////

redrobd_rc_net_server_thread::~redrobd_rc_net_server_thread(void)
{
  pthread_mutex_destroy(&m_steer_code_mutex);
  pthread_mutex_destroy(&m_voltage_mutex);
  pthread_mutex_destroy(&m_camera_code_mutex);
  pthread_mutex_destroy(&m_sys_stat_mutex);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_rc_net_server_thread::get_steer_code(void)
{
  uint16_t the_code;

  // Lockdown get operation
  pthread_mutex_lock(&m_steer_code_mutex);

  the_code = m_steer_code;  
  m_steer_code = CLI_STEER_NONE;
  
  // Lockup get operation
  pthread_mutex_unlock(&m_steer_code_mutex);
  
  return the_code;
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::set_voltage(float value)
{
  // Lockdown set operation
  pthread_mutex_lock(&m_voltage_mutex);

  // Value is sent to client in milli-volts
  m_voltage = (uint16_t)(value * 1000.0);

  // Lockup set operation
  pthread_mutex_unlock(&m_voltage_mutex);
}

////////////////////////////////////////////////////////////////

uint16_t redrobd_rc_net_server_thread::get_camera_code(void)
{
  uint16_t the_code;

  // Lockdown get operation
  pthread_mutex_lock(&m_camera_code_mutex);

  the_code = m_camera_code;  
  m_camera_code = CLI_CAMERA_NONE;
  
  // Lockup get operation
  pthread_mutex_unlock(&m_camera_code_mutex);
  
  return the_code;
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::set_sys_stat(const RC_NET_SYS_STAT *sys_stat)
{
  // Lockdown set operation
  pthread_mutex_lock(&m_sys_stat_mutex);

  memcpy(&m_sys_stat, sys_stat, sizeof(m_sys_stat));

  // Lockup set operation
  pthread_mutex_unlock(&m_sys_stat_mutex);
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::shutdown_server(void)
{
  // Initiate a controlled server shutdown
  m_shutdown_requested = true;

  /////////////////////////////////////
  //         Server socket
  /////////////////////////////////////

  if (!m_server_closed) {

    // Shutdown server socket
    if (shutdown_socket(m_server_sd,
			true,
			true) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Shutdown (requested) server socket failed in thread %s",
		get_name().c_str());
    }
    
    // Close server socket
    if (close_socket(m_server_sd) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Close (requested) server socket failed in thread %s",
		get_name().c_str());
    }
    m_server_closed = true;
  }

  /////////////////////////////////////
  //         Client socket
  /////////////////////////////////////

  if (m_client_connected) {

    // Shutdown client socket
    if (shutdown_socket(m_client_sd,
			true,
			true) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Shutdown (requested) client socket failed in thread %s",
		get_name().c_str());
    }
    
    // Close client socket
    if (close_socket(m_client_sd) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Close (requested) client socket failed in thread %s",
		get_name().c_str());
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Protected member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

long redrobd_rc_net_server_thread::setup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : setup started");

    // Create server socket
    if (create_tcp_socket(&m_server_sd) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Create server socket failed in thread %s",
		get_name().c_str());
    }

    // Create socket address
    socket_address server_sa;
    if (to_net_address(m_server_ip_address.c_str(),
		       &server_sa.net_addr) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Create server socket address failed in thread %s",
		get_name().c_str());
    }
    server_sa.port = m_server_port;

    // Bind socket to local address and port
    if (bind_socket(m_server_sd,
		    server_sa) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Bind server socket failed in thread %s",
		get_name().c_str());
    }

    // Mark socket as accepting connections
    if (listen_socket(m_server_sd, 0) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Listen server socket failed in thread %s",
		get_name().c_str());
    }
            
    redrobd_log_writeln(get_name() + " : setup done");

    return THREAD_SUCCESS;    
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_rc_net_server_thread::setup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_rc_net_server_thread::cleanup(void)
{
  try {
    redrobd_log_writeln(get_name() + " : cleanup started");

    ///////////////////////////////////////////////
    // If no controlled shutdown was requested,
    // proceed with cleanup
    ///////////////////////////////////////////////
    if (!m_shutdown_requested) {

      // Shutdown server socket
      if (shutdown_socket(m_server_sd,
			  true,
			  true) != SOCKET_SUPPORT_SUCCESS) {
	THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		  "Shutdown server socket failed in thread %s",
		  get_name().c_str());
      }
      
      // Close server socket
      if (close_socket(m_server_sd) != SOCKET_SUPPORT_SUCCESS) {
	THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		  "Close server socket failed in thread %s",
		  get_name().c_str());
      }
      m_server_closed = true;
    }

    redrobd_log_writeln(get_name() + " : cleanup done");
    
    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_rc_net_server_thread::cleanup->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

////////////////////////////////////////////////////////////////

long redrobd_rc_net_server_thread::execute(void *arg)
{
  // Make GCC happy (-Wextra)
  if (arg) {
    return THREAD_INTERNAL_ERROR;
  }

  try {
    redrobd_log_writeln(get_name() + " : execute started");

    handle_clients();
        
    redrobd_log_writeln(get_name() + " : execute done");

    return THREAD_SUCCESS;
  }
  catch (excep &exp) {
    syslog_error(redrobd_error_syslog_string(exp).c_str());
    return THREAD_INTERNAL_ERROR;
  }
  catch (...) {
    syslog_error("redrobd_rc_net_server_thread::execute->Unexpected exception");
    return THREAD_INTERNAL_ERROR;
  }
}

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::init_members(void)
{
  m_shutdown_requested = false;
  m_client_connected = false;
  m_server_closed = false;

  m_steer_code = CLI_STEER_NONE;
  m_voltage = 0;
  m_camera_code = CLI_CAMERA_NONE;

  bzero(&m_sys_stat, sizeof(m_sys_stat));

  m_server_sd = 0;
  m_client_sd = 0;
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::handle_clients(void)
{
  long rc;
  socket_address client_sa;
  char client_ip[DOTTED_IP_ADDR_LEN];
  ostringstream oss_msg;

  while (1) {
    
    oss_msg << "Wait for client on port:" << dec << m_server_port;
    redrobd_log_writeln(get_name() + " : " + oss_msg.str());
    oss_msg.str("");

    // Wait for client to connect
    rc = accept_socket(m_server_sd,
		       &m_client_sd,
		       &client_sa);
   
    // Check if controlled server shutdown
    if ( (rc != SOCKET_SUPPORT_SUCCESS) &&
	 (m_shutdown_requested) ) {

      // This was a controlled shutdown.
      // Quit server thread with no error.
      break;
    }
    else if (rc != SOCKET_SUPPORT_SUCCESS) {
      // This was not a controlled shutdown.
      // Quit server thread with error.
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Accept server socket failed in thread %s",
		get_name().c_str());
    }
    else {
      m_client_connected = true;
    }

    // Get address info for connected client
    if (to_ip_address(client_sa.net_addr,
		      client_ip,
		      DOTTED_IP_ADDR_LEN) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Client address for server socket failed in thread %s",
		get_name().c_str());
    }
    oss_msg << "Client connected => " << client_ip
	    << ", port:" << dec << client_sa.port;
    redrobd_log_writeln(get_name() + " : " + oss_msg.str());
    oss_msg.str("");

    // Handle client commands
    bool handle_command = true;
    while (handle_command) {      
      
      try {
	uint16_t client_command;	

	// Wait for command
	recv_client((void *)&client_command,
		    sizeof(client_command));

	ntoh16(&client_command);

	// Handle command
	if (client_command == CLI_CMD_STEER) {
	  uint8_t steer_code;

	  // Get steer code
	  recv_client((void *)&steer_code,
		      sizeof(steer_code));

	  // Update latest steer code
	  pthread_mutex_lock(&m_steer_code_mutex);
	  m_steer_code = steer_code;
	  pthread_mutex_unlock(&m_steer_code_mutex);
	  	  
	}
	else if (client_command == CLI_CMD_GET_VOLTAGE) {
	  uint16_t voltage;

	  // Reply with latest voltage
	  pthread_mutex_lock(&m_voltage_mutex);
	  voltage = m_voltage;
	  pthread_mutex_unlock(&m_voltage_mutex);

	  hton16(&voltage);

	  send_client((void *)&voltage,
		      sizeof(voltage));
	}
	else if (client_command == CLI_CMD_CAMERA) {
	  uint8_t camera_code;

	  // Get camera code
	  recv_client((void *)&camera_code,
		      sizeof(camera_code));

	  // Update latest camera code
	  pthread_mutex_lock(&m_camera_code_mutex);
	  m_camera_code = camera_code;
	  pthread_mutex_unlock(&m_camera_code_mutex);
	}
	else if (client_command == CLI_CMD_GET_SYS_STATS) {
	  RC_NET_SYS_STAT sys_stat;

	  // Reply with latest system statistics
	  pthread_mutex_lock(&m_sys_stat_mutex);
	  memcpy(&sys_stat, &m_sys_stat, sizeof(m_sys_stat));	  
	  pthread_mutex_unlock(&m_sys_stat_mutex);

	  hton32(&sys_stat.mem_used);
	  hton16(&sys_stat.irq);
	  hton32(&sys_stat.uptime);

	  send_client((void *)&sys_stat,
		      sizeof(sys_stat));
	}
	else {
	  oss_msg << "Unknown client command : 0x"
		  << hex << (unsigned)client_command;
	  redrobd_log_writeln(get_name() + " : " + oss_msg.str());
	  oss_msg.str("");

	  handle_command = false;
	}
      }
      catch (...) {
	handle_command = false;
      }
    }
    
    if (m_shutdown_requested) {
      // This was a controlled shutdown.
      // Quit server thread with no error.
      break;
    }

    // Shutdown client socket
    if (shutdown_socket(m_client_sd,
			true,
			true) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Shutdown client socket failed in thread %s",
		get_name().c_str());
    }
    
    // Close client socket
    if (close_socket(m_client_sd) != SOCKET_SUPPORT_SUCCESS) {
      THROW_EXP(REDROBD_INTERNAL_ERROR, REDROBD_SOCKET_OPERATION_FAILED,
		"Close client socket failed in thread %s",
		get_name().c_str());
    }

    m_client_connected = false;
  }
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::recv_client(void *data,
					       unsigned nbytes)
{
  long rc;
  unsigned actual_bytes;

  // Receive data from client
  rc = recv_socket(m_client_sd,
		   data,
		   nbytes,
		   true,  // Receive all
		   false, // No peek
		   &actual_bytes);
  
  if (rc != SOCKET_SUPPORT_SUCCESS) {
    throw client_com_error;
  }

  // Check if client closed connection
  if (!actual_bytes) {
    throw client_com_error;
  }
}

////////////////////////////////////////////////////////////////

void redrobd_rc_net_server_thread::send_client(void *data,
					       unsigned nbytes)
{
  long rc;
  unsigned actual_bytes;

  // Send data to client
  rc = send_socket(m_client_sd,
		   data,
		   nbytes,
		   true,  // Send all
		   &actual_bytes);
  
  if (rc != SOCKET_SUPPORT_SUCCESS) {
    throw client_com_error;
  }
}
