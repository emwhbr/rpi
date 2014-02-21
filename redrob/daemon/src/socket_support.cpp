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

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "socket_support.h"

/////////////////////////////////////////////////////////////////////////////
//               Implementation notes
/////////////////////////////////////////////////////////////////////////////

// The implemented error handling transforms system errors (errno values)
// to codes returned by the exported API's. Not all system errors are
// handled, only the ones selected according to table below.

// -----------------------------------------------------------------------
// Exported API              System API    Selected errno(s)
//------------------------------------------------------------------------
// to_net_address    	     inet_pton     -
// to_ip_address     	     inet_ntop     -
// resolve_hostname  	     getaddrinfo   -
// create_udp_socket 	     socket        -
// create_tcp_socket 	     socket        -
// bind_socket       	     bind          EADDRINUSE
// connect_socket    	     connect       EADDRINUSE, ECONNREFUSED, EINTR
//                                         EISCONN
// listen_socket     	     listen        EADDRINUSE
// accept_socket     	     accept        EWOULDBLOCK, EINTR
// get_socket_local_address  getsockname   -
// get_socket_peer_address   getpeername   ENOTCONN
// send_socket_unconnected   sendto        EWOULDBLOCK, EINTR, EISCONN
// send_socket               send          EWOULDBLOCK, EINTR, ENOTCONN, EPIPE
// recv_socket_unconnected   recvfrom      EWOULDBLOCK, EINTR
// recv_socket               recv          EWOULDBLOCK, EINTR, ENOTCONN,
// shutdown_socket           shutdown      ENOTCONN
// close_socket              close         EINTR
// set_opt_xxx               setsockopt    -
// get_opt_xxx               getsockopt    -
// set_attr_xxx              fcntl         -
// get_attr_xxx              fcntl         -

/////////////////////////////////////////////////////////////////////////////
//               Private member functions
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////

static inline long get_error_code(int local_errno)
{
  switch (local_errno) {
  case EADDRINUSE:
    return SOCKET_SUPPORT_ADDR_IN_USE;
    break;
  case ECONNREFUSED:
    return SOCKET_SUPPORT_CONNECT_REFUSED;
    break;
  case EINTR:
    return SOCKET_SUPPORT_INTERRUPTED;
    break;
  case EISCONN:
    return SOCKET_SUPPORT_IS_CONNECTED;
    break;
  case ENOTCONN:
    return SOCKET_SUPPORT_NOT_CONNECTED;
    break;
  case EWOULDBLOCK:
    return SOCKET_SUPPORT_WOULD_BLOCK;
    break;
  case EPIPE:
    return SOCKET_SUPPORT_BROKEN_PIPE;
    break;
  default:
    return SOCKET_SUPPORT_FAILURE;
  }
}

////////////////////////////////////////////////////////////////

static long do_send_socket_all(int sockd,
			       const uint8_t *data,
			       unsigned nbytes,			   
			       unsigned *actual_bytes)
{
  unsigned total = 0;           // How many bytes we've sent
  unsigned bytes_left = nbytes; // How many we have left to send
  int n = 0;
  int local_errno;

  while (total < nbytes) {
    n = send(sockd, data+total, bytes_left, MSG_NOSIGNAL);
    if (n == -1) { local_errno = errno; break; }
    total += n;
    bytes_left -= n;
  }

  *actual_bytes = total; // Return number bytes actually sent

  if (n==-1) {
    return get_error_code(local_errno);
  }
  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

static long do_recv_socket_all(int sockd,
			       uint8_t *data,
			       unsigned nbytes,			   
			       unsigned *actual_bytes)
{
  unsigned total = 0;           // How many bytes we've received
  unsigned bytes_left = nbytes; // How many we have left to receive
  int n = 0;
  int local_errno = 0;

  while (total < nbytes) {
    n = recv(sockd, data+total, bytes_left, 0);
    if ( (n == -1) || (n == 0) ) { local_errno = errno; break; }
    total += n;
    bytes_left -= n;
  }

  *actual_bytes = total; // Return number bytes actually received

  if (n==-1) {
    return get_error_code(local_errno);
  }
  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

static long do_send_socket(int sockd,
			   const void *data, unsigned nbytes,
			   const socket_address *desta,
			   bool send_all,
			   unsigned *actual_bytes)
{
  long status = SOCKET_SUPPORT_SUCCESS; // Assume OK
  int rc;

  *actual_bytes = 0; // No bytes sent yet

  if (desta) {
    ////////////////////////////
    // Send unconnected
    ////////////////////////////
    struct sockaddr_in dest_addr;

    // Create destination address structure (IPv4)
    bzero((void *)&dest_addr, sizeof(dest_addr));
    dest_addr.sin_family      = AF_INET;
    dest_addr.sin_port        = htons(desta->port);
    dest_addr.sin_addr.s_addr = htonl(desta->net_addr);

    // Note! No sense to check argument 'send_all'.
    //       Send-all using UDP sockets doesn't work the same way
    //       as when using TCP. The send will either succed or not.
    //       There are no partial send when using UDP.
    
    rc = sendto(sockd,
		data,
		nbytes,
		0, // Flags
		(struct sockaddr *)&dest_addr,
		sizeof(dest_addr));
    
    if (rc == -1) {
      int local_errno = errno;
      status = get_error_code(local_errno);
    }
    else {
      *actual_bytes = rc;
    }
  }
  else {
    ////////////////////////////
    // Send connected
    ////////////////////////////
    if (send_all) {
      status = do_send_socket_all(sockd,
				  (const uint8_t *)data,
				  nbytes,
				  actual_bytes);
    }
    else {      
      rc = send(sockd,
		data,
		nbytes,
		MSG_NOSIGNAL); // Flags
      
      if (rc == -1) {
	int local_errno = errno;
	status = get_error_code(local_errno);
      }
      else {
	*actual_bytes = rc;
      }
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////

static long do_recv_socket(int sockd,
			   void *data, unsigned nbytes,
			   socket_address *srca,
			   bool recv_all,
			   bool peek,
			   unsigned *actual_bytes)
{
  long status = SOCKET_SUPPORT_SUCCESS; // Assume OK
  int rc;
  int flags = 0;

  if (peek) {
    flags |= MSG_PEEK;
  }
  
  *actual_bytes = 0; // No bytes received yet

  if (srca) {
    ////////////////////////////
    // Receive unconnected
    ////////////////////////////
    struct sockaddr_in src_addr;
    socklen_t len = sizeof(src_addr);

    // Create source address structure (IPv4)
    bzero((void *)&src_addr, sizeof(src_addr));

    // Note! No sense to check argument 'recv_all'.
    //       Receive-all using UDP sockets doesn't work the same way
    //       as when using TCP. The recv will either succed or not.
    //       There are no partial recv when using UDP.
    
    rc = recvfrom(sockd,
		  data,
		  nbytes,
		  flags,
		  (struct sockaddr *)&src_addr,
		  &len);
    
    if (rc == -1) {
      int local_errno = errno;
      status = get_error_code(local_errno);
    }
    else {
      *actual_bytes  = rc;
      srca->net_addr = ntohl(src_addr.sin_addr.s_addr);
      srca->port     = ntohs(src_addr.sin_port);
    }
  }
  else {
    ////////////////////////////
    // Receive connected
    ////////////////////////////
    if (recv_all) {
      status = do_recv_socket_all(sockd,
				  (uint8_t *)data,
				  nbytes,
				  actual_bytes);
    }
    else {      
      rc = recv(sockd,
		data,
		nbytes,
		flags);
      
      if (rc == -1) {
	int local_errno = errno;
	status = get_error_code(local_errno);
      }
      else {
	*actual_bytes = rc;
      }
    }
  }

  return status;
}

/////////////////////////////////////////////////////////////////////////////
//               Public member functions
/////////////////////////////////////////////////////////////////////////////

typedef enum {ENDIAN_NONE, ENDIAN_BIG, ENDIAN_LITTLE} ENDIAN_T;

////////////////////////////////////////////////////////////////

uint64_t hton64(uint64_t value)
{
  union {
    uint64_t w;
    uint8_t  b[8];
  }u;

  static ENDIAN_T e_type = ENDIAN_NONE;

  // Test of endianess
  if (e_type == ENDIAN_NONE) {
    u.w = 0x01;
    e_type = (u.b[7] == 0x01 ? ENDIAN_BIG : ENDIAN_LITTLE);
  }

  // Big endian : Return value unchanged
  if (e_type == ENDIAN_BIG) {
    return value;
  }

  // Little endian: Convert to big endian (network order)
  return 
    ((value & uint64_t(0x00000000000000FF)) << 56) |
    ((value & uint64_t(0x000000000000FF00)) << 40) |
    ((value & uint64_t(0x0000000000FF0000)) << 24) |
    ((value & uint64_t(0x00000000FF000000)) << 8)  |
    ((value & uint64_t(0x000000FF00000000)) >> 8)  | 
    ((value & uint64_t(0x0000FF0000000000)) >> 24) |
    ((value & uint64_t(0x00FF000000000000)) >> 40) |
    ((value & uint64_t(0xFF00000000000000)) >> 56);
}

////////////////////////////////////////////////////////////////

uint64_t ntoh64(uint64_t value)
{
  return hton64(value);
}

////////////////////////////////////////////////////////////////

void hton64(void *value)
{
  *(uint64_t *)value = hton64( *((uint64_t *) value) );
}

////////////////////////////////////////////////////////////////

void ntoh64(void *value)
{
  *(uint64_t *)value = ntoh64( *((uint64_t *) value) );
}

////////////////////////////////////////////////////////////////

void hton32(void *value)
{
  *(uint32_t *)value = htonl( *(uint32_t *)value );
}

////////////////////////////////////////////////////////////////

void ntoh32(void *value)
{
  *(uint32_t *)value = ntohl( *(uint32_t *)value );
}

////////////////////////////////////////////////////////////////

void hton16(void *value)
{
  *(uint16_t *)value = htons( *(uint16_t *)value );
}

////////////////////////////////////////////////////////////////

void ntoh16(void *value)
{
  *(uint16_t *)value = ntohs( *(uint16_t *)value );
}

////////////////////////////////////////////////////////////////

long to_net_address(const char *ip_address, uint32_t *net_addr)
{
  int rc;
  struct in_addr the_net_addr;

  // Check for special IP's
  if ( strcmp(ip_address, ANY_IP_ADDRESS) == 0 ) {
    *net_addr = INADDR_ANY;
    return SOCKET_SUPPORT_SUCCESS;
  }
  if ( strcmp(ip_address, LOOPBACK_IP_ADDRESS) == 0 ) {
    *net_addr = INADDR_LOOPBACK;
    return SOCKET_SUPPORT_SUCCESS;
  }
  if ( strcmp(ip_address, BROADCAST_IP_ADDRESS) == 0 ) {
    *net_addr = INADDR_BROADCAST;
    return SOCKET_SUPPORT_SUCCESS;
  }

  // Convert dotted-decimal into network address
  rc = inet_pton(AF_INET, ip_address, (void *)&the_net_addr);
  if (rc != 1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  // Convert to host byte order
  *net_addr = ntohl(the_net_addr.s_addr);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long to_ip_address(uint32_t net_addr, char *ip_address, int ip_len)
{
  struct in_addr the_net_addr;

  // Convert to network byte order
  the_net_addr.s_addr = htonl(net_addr);

  // Convert network address into dotted-decimal
  if (inet_ntop(AF_INET, &the_net_addr, ip_address, ip_len) == NULL) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long resolve_hostname(const char *hostname,
		      uint16_t port,
		      resolve_element *resolve_list,
		      unsigned nr_elements,
		      unsigned *actual_nr_elements)
{
  int rc;
  unsigned i;
  struct addrinfo hints;
  struct addrinfo *host_info; // Point to the result linked-list
  struct addrinfo *p;         // For looping linked-list
  char service[10];

  // Create "service" string
  snprintf(service, sizeof(service), "%d", port);

  bzero((void *)&hints, sizeof(hints));
  hints.ai_flags  = AI_NUMERICSERV; // Interpret service as port
  hints.ai_family = AF_INET;        // IPv4 only
  
  rc = getaddrinfo(hostname,
		   service,
		   &hints,
		   &host_info);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  // Loop through the linked-list
  for(i=0, p=host_info; (p != NULL) && (i < nr_elements); p=p->ai_next) {
    // Only check IPv4
    if (p->ai_family == AF_INET) {
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      resolve_list[i].net_addr = ntohl(ipv4->sin_addr.s_addr);
      resolve_list[i].protocol = p->ai_protocol;
      resolve_list[i].socktype = p->ai_socktype;
      i++;
    }    
  }
  freeaddrinfo(host_info); // Free the linked-list

  *actual_nr_elements = i;

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long create_udp_socket(int *sockd)
{
  int rc;
  struct protoent *prot;

  // Get UDP protocol number
  prot = getprotobyname("udp");
  if (prot == NULL) {
    return SOCKET_SUPPORT_FAILURE;
  }
  endprotoent();

  // Create UDP socket
  rc = socket(PF_INET, SOCK_DGRAM, prot->p_proto);
  if (rc == -1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *sockd = rc; // Return socket descriptor

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long create_tcp_socket(int *sockd)
{
  int rc;
  struct protoent *prot;

  // Get TCP protocol number
  prot = getprotobyname("tcp");
  if (prot == NULL) {
    return SOCKET_SUPPORT_FAILURE;
  }
  endprotoent();

  // Create UDP socket
  rc = socket(PF_INET, SOCK_STREAM, prot->p_proto);
  if (rc == -1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *sockd = rc; // Return socket descriptor

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long bind_socket(int sockd, socket_address socka)
{
  int rc;
  struct sockaddr_in local_addr;

  // Create local address structure (IPv4)
  bzero((void *)&local_addr, sizeof(local_addr));
  local_addr.sin_family      = AF_INET;
  local_addr.sin_port        = htons(socka.port);
  local_addr.sin_addr.s_addr = htonl(socka.net_addr);

  // Bind socket to local address and port
  rc = bind(sockd, (struct sockaddr *)&local_addr, sizeof(local_addr));
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long connect_socket(int sockd, socket_address socka)
{
  int rc;
  struct sockaddr_in peer_addr;

  // Create peer address structure (IPv4)
  bzero((void *)&peer_addr, sizeof(peer_addr));
  peer_addr.sin_family      = AF_INET;
  peer_addr.sin_port        = htons(socka.port);
  peer_addr.sin_addr.s_addr = htonl(socka.net_addr);
  
  // Connect socket
  rc = connect(sockd, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long listen_socket(int sockd, int backlog)
{
  int rc;

  rc = listen(sockd, backlog);
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long accept_socket(int listen_sockd,
		   int *client_sockd,
		   socket_address *client_socka)
{
  int rc;
  struct sockaddr_in client_addr;
  socklen_t len = sizeof(client_addr);

  rc = accept(listen_sockd, (struct sockaddr *)&client_addr, &len);
  if (rc == -1) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }
  
  *client_sockd = rc;

  client_socka->net_addr = ntohl(client_addr.sin_addr.s_addr);
  client_socka->port     = ntohs(client_addr.sin_port);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_socket_local_address(int sockd, socket_address *socka)
{
  int rc;
  struct sockaddr_in local_addr;
  socklen_t len = sizeof(local_addr);

  rc = getsockname(sockd, (struct sockaddr *)&local_addr, &len);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  socka->net_addr = ntohl(local_addr.sin_addr.s_addr);
  socka->port     = ntohs(local_addr.sin_port);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_socket_peer_address(int sockd, socket_address *socka)
{
  int rc;
  struct sockaddr_in peer_addr;
  socklen_t len = sizeof(peer_addr);

  rc = getpeername(sockd, (struct sockaddr *)&peer_addr, &len);
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  socka->net_addr = ntohl(peer_addr.sin_addr.s_addr);
  socka->port     = ntohs(peer_addr.sin_port);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long send_socket_unconnected(int sockd,
			     const void *data, unsigned nbytes,
			     socket_address desta,
			     unsigned *actual_bytes)
{
  return do_send_socket(sockd,
			data, nbytes,
			&desta,
			false, // No use for UDP
			actual_bytes);
}

////////////////////////////////////////////////////////////////

long send_socket(int sockd,
		 const void *data, unsigned nbytes,
		 bool send_all,
		 unsigned *actual_bytes)
{
  return do_send_socket(sockd,
			data, nbytes,
			NULL,
			send_all, // Only useful for TCP
			actual_bytes);
}

////////////////////////////////////////////////////////////////

long recv_socket_unconnected(int sockd,
			     void *data, unsigned nbytes,
			     socket_address *srca,
			     bool peek,
			     unsigned *actual_bytes)
{
  return do_recv_socket(sockd,
			data, nbytes,
			srca,
			false, // No use for UDP
			peek,
			actual_bytes);
}

////////////////////////////////////////////////////////////////

long recv_socket(int sockd,
		 void *data, unsigned nbytes,
		 bool recv_all,
		 bool peek,
		 unsigned *actual_bytes)
{
  if (recv_all && peek) {
    // Error specifying parameters, not a valid combo
    return SOCKET_SUPPORT_FAILURE;
  }

  return do_recv_socket(sockd,
			data, nbytes,
			NULL,
			recv_all, // Only useful for TCP
			peek,
			actual_bytes);
}

////////////////////////////////////////////////////////////////

long shutdown_socket(int sockd, bool recv, bool send)
{
  int rc;
  int how;
  
  // Check type of shutdown
  if ( send && recv ) {

    how = SHUT_RDWR; // Done both send and receive

  } else if ( send && (!recv) ) {

    how = SHUT_WR; // Done send only

  } else if ( (!send) && recv ) {

    how = SHUT_RD; // Done receive only

  } else {
    // Error in specifying socket shutdown
    return SOCKET_SUPPORT_FAILURE;
  }

  // Do shutdown
  rc = shutdown(sockd, how);
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long close_socket(int sockd)
{
  int rc;

  rc = close(sockd);
  if (rc) {
    int local_errno = errno;
    return get_error_code(local_errno);
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_recv_buffer_size(int sockd, unsigned nbytes)
{
  int rc;
  int optval = (int)nbytes;

  rc = setsockopt(sockd, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_recv_buffer_size(int sockd, unsigned *nbytes)
{
  int rc;
  int optval;
  socklen_t optlen = sizeof(optval);

  rc = getsockopt(sockd, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *nbytes = (unsigned)optval;

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_send_buffer_size(int sockd, unsigned nbytes)
{
  int rc;
  int optval = (int)nbytes;

  rc = setsockopt(sockd, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_send_buffer_size(int sockd, unsigned *nbytes)
{
  int rc;
  int optval;
  socklen_t optlen = sizeof(optval);

  rc = getsockopt(sockd, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *nbytes = (unsigned)optval;

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_reuse_addr(int sockd, bool on)
{
  int rc;
  int optval = (on ? 1 : 0);

  rc = setsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_reuse_addr(int sockd, bool *on)
{
  int rc;
  int optval;
  socklen_t optlen = sizeof(optval);

  rc = getsockopt(sockd, SOL_SOCKET, SO_REUSEADDR, &optval, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *on = (optval ? true : false);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_broadcast(int sockd, bool on)
{
  int rc;
  int optval = (on ? 1 : 0);

  rc = setsockopt(sockd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_broadcast(int sockd, bool *on)
{
  int rc;
  int optval;
  socklen_t optlen = sizeof(optval);

  rc = getsockopt(sockd, SOL_SOCKET, SO_BROADCAST, &optval, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *on = (optval ? true : false);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_tcp_nodelay(int sockd, bool on)
{
  int rc;
  int optval = (on ? 1 : 0);

  rc = setsockopt(sockd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_tcp_nodelay(int sockd, bool *on)
{
  int rc;
  int optval;
  socklen_t optlen = sizeof(optval);

  rc = getsockopt(sockd, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *on = (optval ? true : false);

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_opt_recv_timeout(int sockd, double timeout_sec)
{
  int rc;
  struct timeval tv;

  if (timeout_sec <= 0.0)  {
    tv.tv_sec  = 0; // Note! tv={0,0} means NO timeout
    tv.tv_usec = 0;
  }
  else {
    unsigned long timeout_us = timeout_sec * 1000000; // Convert to us

    tv.tv_sec  = (time_t)(timeout_us / 1000000);
    tv.tv_usec = (suseconds_t)(timeout_us % 1000000); // Remainder
  }

  rc = setsockopt(sockd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_opt_recv_timeout(int sockd, double *timeout_sec)
{
  int rc;
  struct timeval tv;
  socklen_t optlen = sizeof(tv);

  rc = getsockopt(sockd, SOL_SOCKET, SO_RCVTIMEO, &tv, &optlen);
  if (rc) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *timeout_sec = (double)tv.tv_sec + ( (double)(tv.tv_usec) / 1000000.0 );

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long set_attr_blocked(int sockd, bool on)
{
  int rc;
  int flags;

  // Get flags
  rc = fcntl(sockd, F_GETFL);
  if (rc == -1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  // Update flags
  flags = (on ? (rc & ~O_NONBLOCK) : (rc | O_NONBLOCK));

  // Set new flags
  rc = fcntl(sockd, F_SETFL, flags);
  if (rc == -1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  return SOCKET_SUPPORT_SUCCESS;
}

////////////////////////////////////////////////////////////////

long get_attr_blocked(int sockd, bool *on)
{
  int rc;

  // Get flags
  rc = fcntl(sockd, F_GETFL);
  if (rc == -1) {
    return SOCKET_SUPPORT_FAILURE;
  }

  *on = ((rc & O_NONBLOCK) ? false : true);

  return SOCKET_SUPPORT_SUCCESS;
}
