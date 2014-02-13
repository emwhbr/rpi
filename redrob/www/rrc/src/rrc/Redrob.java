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

package rrc;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;

public class Redrob {

    public enum STEER_CODE {
	NONE((byte)0x00),
	FORWARD((byte)0x01),
	REVERSE((byte)0x02),
	RIGHT((byte)0x04),
	LEFT((byte)0x08);

	private final byte value;  
  
	private STEER_CODE(byte value) {  
	    this.value = value;
	}    
	public byte get_value() {  
	    return value;
	}  
    }

    private static final int CONNECT_TIMEOUT_MS = 3000;
    private static final int RECV_TIMEOUT_MS = 1000;
    private static final int SERVER_PORT = 52022;

    private static final int COMMAND_STEER = 1;
    private static final int COMMAND_GET_VOLTAGE = 2;

    private final String m_peer_ip_address;
    private Socket m_sock;

    private DataInputStream m_in;
    private DataOutputStream m_out;

    ////////////////////////////////////////////////////////

    public Redrob()
    {
	m_peer_ip_address = "TBD";
	m_sock = null;
	m_in = null;
	m_out = null;
    }

    ////////////////////////////////////////////////////////

    public Redrob(String ip_address)
    {
	m_peer_ip_address = ip_address;
	m_sock = null;
	m_in = null;
	m_out = null;
    }

    ////////////////////////////////////////////////////////

    public void connect() throws IOException,
				 SocketTimeoutException,
				 SocketException
    {
	SocketAddress local_addr =
	    new InetSocketAddress(InetAddress.getByName("0.0.0.0"),
				  0);

	SocketAddress peer_addr =
	    new InetSocketAddress(InetAddress.getByName(m_peer_ip_address),
				  SERVER_PORT);

	try {
	    // Create socket
	    if (m_sock == null) {
		m_sock = new Socket();
	    }

	    // Bind to local address and port	    
	    m_sock.bind(local_addr);

	    //debug("LOCAL => " +
	    //	  m_sock.getLocalAddress().getHostAddress()
	    //	  + " : " + m_sock.getLocalPort());
	    
	    // Connect to peer
	    m_sock.connect(peer_addr, CONNECT_TIMEOUT_MS);

	    //debug("PEER => " +
	    //	  m_sock.getInetAddress().getHostAddress()
	    //	  + " : " + m_sock.getPort());

	    // Set receive timeout
	    m_sock.setSoTimeout(RECV_TIMEOUT_MS);

	    // Disable disable Nagle's algorithm
	    m_sock.setTcpNoDelay(true);

	    // Setup TCP input/output stream
	    m_in = new DataInputStream(m_sock.getInputStream());
	    m_out = new DataOutputStream(m_sock.getOutputStream());
	}
	catch(IOException exp) {
	    // Handle exception when connect
	    if (m_sock.isBound()) {
		m_sock.close();
		m_sock = null;
	    }
	    throw exp;
	}
    }

    ////////////////////////////////////////////////////////

    public void disconnect() throws IOException
    {
	// Shutdown socket
	m_sock.shutdownInput();
	m_sock.shutdownOutput();

	// Close socket
	m_sock.close();
	m_sock = null;

	// Free input/output stream
	m_in = null;
	m_out = null;
    }

    ////////////////////////////////////////////////////////

    public void send_steer_code(STEER_CODE code) throws IOException
    {
	//debug("send_steer_code: " + code);

	// Send command to Redrob using TCP
	m_out.writeShort(COMMAND_STEER);   // Steer command
	m_out.writeByte(code.get_value()); // Actual steer code
	m_out.flush();
    }

    ////////////////////////////////////////////////////////

    public int get_voltage_mv() throws IOException
    {
	// Send command to Redrob using TCP
	m_out.writeShort(COMMAND_GET_VOLTAGE); // Get voltage command
	m_out.flush();

	// Receive voltage (sent by Redrob [mV])
	int voltage_mv = m_in.readUnsignedShort();

	//debug("get_voltage_mv : " + voltage_mv);

	return voltage_mv;
    }

    ////////////////////////////////////////////////////////

    private void debug(String msg)
    {
        System.out.println(msg);
    }

}
