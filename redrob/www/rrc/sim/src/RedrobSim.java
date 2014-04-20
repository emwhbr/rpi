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

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.PrintWriter;
import java.io.StringWriter;

import java.net.ServerSocket;
import java.net.Socket;

class RedrobSim {

    private static final int SERVER_PORT = 52022;

    private static final int COMMAND_STEER         = 1;
    private static final int COMMAND_GET_VOLTAGE   = 2;
    private static final int COMMAND_CAMERA        = 3;
    private static final int COMMAND_GET_SYS_STATS = 4;

    private static ServerSocket m_server_sock;
    private static Socket m_client_sock;

    private static DataInputStream m_in;
    private static DataOutputStream m_out;

    ////////////////////////////////////////////////////////

    public static void main(String[] args)
    {
	boolean handle_command;

	float voltage = (float)7.2;
	float voltage_delta = (float)-0.1;

	int uptime = 0;
	
	try {
	    // Create and bind server socket
	    m_server_sock = new ServerSocket(SERVER_PORT, 1);

	    while (true) {		
		// Wait for client to connect
		debug("Waiting for connection on port:" + SERVER_PORT);
		m_client_sock = m_server_sock.accept();
		
		debug("PEER => " +
		      m_client_sock.getInetAddress().getHostAddress()
		      + " : " + m_client_sock.getPort());
		
		// Setup TCP input/output stream
		m_in = new DataInputStream(m_client_sock.getInputStream());
		m_out = new DataOutputStream(m_client_sock.getOutputStream());
		
		// Handle client command requests
		handle_command = true;
		while (handle_command) {
		    
		    try {
			// Wait for command
			int command = m_in.readUnsignedShort();
		    
			// Handle command
			if (command == COMMAND_STEER) {
			    debug("Got STEER command");
			    
			    byte steer_code = m_in.readByte();
			    debug("    steer code " + steer_code);
			}
			else if (command == COMMAND_GET_VOLTAGE) {
			    debug("Got VOLTAGE command, voltage : " + voltage);
			    
			    // Reply is sent in milli-volts
			    int scaled_voltage = (int)(voltage * (float)1000.0);
			    m_out.writeShort(scaled_voltage);
			    m_out.flush();
			    
			    // Update voltage ramp data
			    voltage = voltage + voltage_delta;
			    if (voltage < 6.1) {
				voltage_delta = (float)0.1;
			    }
			    if (voltage > 8.4) {
				voltage_delta = (float)-0.1;
			    }			    
			}
			else if (command == COMMAND_CAMERA) {
			    debug("Got CAMERA command");

			    byte camera_code = m_in.readByte();
			    debug("    camera code " + camera_code);
			}
			else if (command == COMMAND_GET_SYS_STATS) {
			    debug("Got SYS_STATS command");

			    // Reply with system statistics
			    m_out.writeByte(50);
			    m_out.writeInt(4096);
			    m_out.writeShort(1972);
			    m_out.writeInt(uptime);
			    m_out.writeInt(72123);
			    m_out.writeShort(1250);
			    m_out.writeShort(700);
			    m_out.flush();

			    // Update uptime ramp data
			    uptime = uptime + 1;
			}
			else {
			    debug("*** Unknown command : " + command);
			    handle_command = false;
			}
		    }
		    catch (EOFException exp) {
			debug("PEER closed connection");
			handle_command = false;
		    }
		    catch (Exception exp) {
			StringWriter sw = new StringWriter();
			PrintWriter pw = new PrintWriter(sw);
			exp.printStackTrace(pw);
			debug(exp.getMessage() + "\n" + sw);
			handle_command = false;
		    }
		}
		
		// Shutdown client socket
		m_client_sock.shutdownInput();
		m_client_sock.shutdownOutput();
		
		// Close client socket
		m_client_sock.close();
		m_client_sock = null;

		// Free input/output stream
		m_in = null;
		m_out = null;
	    }
	}
	catch (Exception exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    debug(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private static void debug(String msg)
    {
        System.out.println(msg);
    }
}
