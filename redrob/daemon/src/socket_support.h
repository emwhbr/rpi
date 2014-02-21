// ************************************************************************
// *                                                                      *
// * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
// *                                                                      *
// * This program is free software; you can redistribute it and/or modify *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation; either version 2 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// ************************************************************************

#ifndef __SOCKET_SUPPORT_H__
#define __SOCKET_SUPPORT_H__

#include <stdint.h>
#include <stdbool.h>

/////////////////////////////////////////////////////////////////////////////
//               Definition of macros
/////////////////////////////////////////////////////////////////////////////

// Return codes
#define SOCKET_SUPPORT_SUCCESS          0
#define SOCKET_SUPPORT_WOULD_BLOCK     -1
#define SOCKET_SUPPORT_INTERRUPTED     -2
#define SOCKET_SUPPORT_NOT_CONNECTED   -3
#define SOCKET_SUPPORT_IS_CONNECTED    -4
#define SOCKET_SUPPORT_CONNECT_REFUSED -5
#define SOCKET_SUPPORT_ADDR_IN_USE     -6
#define SOCKET_SUPPORT_BROKEN_PIPE     -7
#define SOCKET_SUPPORT_FAILURE         -8

// Special IP-addresses
#define ANY_IP_ADDRESS       "ANY_IP_ADDRESS"
#define LOOPBACK_IP_ADDRESS  "LOOPBACK_IP_ADDRESS"
#define BROADCAST_IP_ADDRESS "BROADCAST_IP_ADDRESS"

/////////////////////////////////////////////////////////////////////////////
//               Definition of support types
/////////////////////////////////////////////////////////////////////////////

typedef struct {
  uint32_t net_addr; // IPv4 network address, host byte order
  uint16_t port;     // Port number, host byte order
} socket_address;

typedef struct {
  uint32_t net_addr; // IPv4 network address, host byte order
  int      protocol; // Standard IP protocol identifier
  int      socktype; // SOCK_DGRAM, SOCK_STREAM, ...
} resolve_element;

/////////////////////////////////////////////////////////////////////////////
//               Definition of exported functions
/////////////////////////////////////////////////////////////////////////////

extern uint64_t hton64(uint64_t value);
extern uint64_t ntoh64(uint64_t value);

extern void hton64(void *value);
extern void ntoh64(void *value);

extern void hton32(void *value);
extern void ntoh32(void *value);

extern void hton16(void *value);
extern void ntoh16(void *value);

extern long to_net_address(const char *ip_address, uint32_t *net_addr);
extern long to_ip_address(uint32_t net_addr, char *ip_address, int ip_len);

extern long resolve_hostname(const char *hostname,
			     uint16_t port,
			     resolve_element *resolve_list,
			     unsigned nr_elements,
			     unsigned *actual_nr_elements);

extern long create_udp_socket(int *sockd);
extern long create_tcp_socket(int *sockd);

extern long bind_socket(int sockd, socket_address socka);

extern long connect_socket(int sockd, socket_address socka);

extern long listen_socket(int sockd, int backlog);
extern long accept_socket(int listen_sockd,
			  int *client_sockd,
			  socket_address *client_socka);

extern long get_socket_local_address(int sockd, socket_address *socka);
extern long get_socket_peer_address(int sockd, socket_address *socka);

extern long send_socket_unconnected(int sockd,
				    const void *data, unsigned nbytes,
				    socket_address desta,
				    unsigned *actual_bytes);
extern long send_socket(int sockd,
			const void *data, unsigned nbytes,
			bool send_all, // Ignored for UDP
			unsigned *actual_bytes);

extern long recv_socket_unconnected(int sockd,
				    void *data, unsigned nbytes,
				    socket_address *srca,
				    bool peek,
				    unsigned *actual_bytes);
extern long recv_socket(int sockd,
			void *data, unsigned nbytes,
			bool recv_all, // Ignored for UDP
			bool peek,     // Don't use this with 'recv_all'
			unsigned *actual_bytes);

extern long shutdown_socket(int sockd, bool recv, bool send);
extern long close_socket(int sockd);

extern long set_opt_recv_buffer_size(int sockd, unsigned nbytes);
extern long get_opt_recv_buffer_size(int sockd, unsigned *nbytes);

extern long set_opt_send_buffer_size(int sockd, unsigned nbytes);
extern long get_opt_send_buffer_size(int sockd, unsigned *nbytes);

extern long set_opt_reuse_addr(int sockd, bool on);
extern long get_opt_reuse_addr(int sockd, bool *on);

extern long set_opt_broadcast(int sockd, bool on);
extern long get_opt_broadcast(int sockd, bool *on);

extern long set_opt_tcp_nodelay(int sockd, bool on);
extern long get_opt_tcp_nodelay(int sockd, bool *on);

extern long set_opt_recv_timeout(int sockd, double timeout_sec);
extern long get_opt_recv_timeout(int sockd, double *timeout_sec);

extern long set_attr_blocked(int sockd, bool on);
extern long get_attr_blocked(int sockd, bool *on);

#endif // __SOCKET_SUPPORT_H__
