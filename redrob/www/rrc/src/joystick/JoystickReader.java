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

package joystick;

import static java.lang.Math.PI;

import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.concurrent.locks.ReadWriteLock;

public class JoystickReader extends JoystickEventNotifier implements Runnable {

    public enum JOYSTICK_ID {JOYSTICK_0, JOYSTICK_1};

    public class JoystickInfo {
	public String name;
	public int    axis;
	public int    buttons;
	public String lib_info;
	public JoystickInfo() {
	}
    }

    public class JoystickPosition {
	public float r     = (float)0.0; // polar
	public float theta = (float)0.0; // polar (degrees)
	public JoystickPosition() {
	}
    }

    public class Exp extends Exception {
	public Exp(String msg) {
	    super(msg);
	}
    }    

    private boolean m_setup_done;
    private boolean m_read_joystick;

    private JOYSTICK_ID m_joy_id;
    private int m_joy_dev;
    private String m_libjoy_search_path;    
    private libjoy m_joy;    

    private Thread m_thread;
    private long m_poll_interval_ms;
    private boolean m_thread_active;

    private JoystickInfo m_joy_info;

    private ReadWriteLock m_joy_pos_lock;
    private JoystickPosition m_joy_pos;

    ////////////////////////////////////////////////////////

    public JoystickReader(JoystickEvent event,
			  JOYSTICK_ID joy_id,
			  String libjoy_search_path,
			  long poll_interval_ms)
    {
	super(event); // Create the event notifier

	m_setup_done = false;
	m_read_joystick = false;

	m_joy_id = joy_id;
	switch (m_joy_id) {
	case JOYSTICK_0:
	    m_joy_dev = libjoy.JOY_DEV_0;
	    break;
	case JOYSTICK_1:
	    m_joy_dev = libjoy.JOY_DEV_1;
	}

	m_libjoy_search_path = libjoy_search_path;

	m_thread = new Thread(this);
	m_thread.setName("Thread-" + m_joy_id);
	m_poll_interval_ms = poll_interval_ms;
	m_thread_active = false;

	m_joy_info = new JoystickInfo();

	m_joy_pos_lock = new ReentrantReadWriteLock();
	m_joy_pos = new JoystickPosition();
    }

    ////////////////////////////////////////////////////////

    public void setup() throws JoystickReader.Exp
    {
	if (m_setup_done) {
	    throw new Exp(m_joy_id + " : Setup already done"); 
	}

	native_long rc;

	try {
	    // Load shared library
	    m_joy = libjoy.library_loader.load(m_libjoy_search_path);
	    
	    // Initialize shared library
	    rc = m_joy.joy_initialize(m_joy_dev, true);
	    if (!rc.equals(libjoy.JOY_SUCCESS)) {
		throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	    }

	    // Update joystick information
	    update_joystick_info();
	}
	catch (Error err) {
            throw new Exp(m_joy_id + " : " + err.getMessage());
        }

	m_setup_done = true;
    }

    ////////////////////////////////////////////////////////

    public void start() throws JoystickReader.Exp
    {
	// Setup must be done
	if (!m_setup_done) {
	    throw new Exp(m_joy_id + " : " + " : Setup not done"); 
	}
	
	// Thread can only be started once
	if (m_thread.getState() != Thread.State.NEW) {
	    throw new Exp(m_joy_id + " : Already started"); 
	}

	// Start thread
	m_read_joystick = true;
	m_thread.start();
    }

    ////////////////////////////////////////////////////////

    public void cleanup() throws JoystickReader.Exp
    {
	// Setup must be done
	if (!m_setup_done) {
	    throw new Exp(m_joy_id + " : Setup not done"); 
	}

	// Command thread to stop
	m_read_joystick = false;
	if (m_thread.isAlive()) {
	    try {
		//debug(m_joy_id + " : Waiting for thread to die");
		m_thread.join(1000);
		if (m_thread.isAlive()) {
		    throw new Exp(m_joy_id + " : Timout waiting for thread"); 
		}
	    }
	    catch (InterruptedException exp) {
		throw new Exp(m_joy_id + " : Error waiting for thread");
	    }
	}

	native_long rc;

	try {
	    // Finalize shared library
	    rc = m_joy.joy_finalize(m_joy_dev);
	    if (!rc.equals(libjoy.JOY_SUCCESS)) {
		throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	    }
	}
	catch (Error err) {
            throw new Exp(m_joy_id + " : " + err.getMessage());
        }
	
	m_setup_done = false;
    }

    ////////////////////////////////////////////////////////

    public void set_active(boolean b)
    {
	m_thread_active = b;
    }

    ////////////////////////////////////////////////////////

    public JoystickInfo get_info() throws JoystickReader.Exp
    {
	// Setup must be done
	if (!m_setup_done) {
	    throw new Exp(m_joy_id + " : " + " : Setup not done"); 
	}

	return m_joy_info;
    }

    ////////////////////////////////////////////////////////

    public boolean is_alive()
    {
	if (!m_thread.isAlive() || !m_thread_active) {
	    return false;
	}

	return true;
    }

    ////////////////////////////////////////////////////////

    public JoystickPosition get_current_position()
    {
	// Return "zero" if thread not active or dead
	if (!is_alive()) {
	    return new JoystickPosition();
	}

	// Lockdown the read operation
	try {
	    m_joy_pos_lock.readLock().lock();
	    return m_joy_pos;
	}
	finally {
	    m_joy_pos_lock.readLock().unlock();
	}
    }

    ////////////////////////////////////////////////////////

    private void update_joystick_info() throws JoystickReader.Exp
    {
	string_by_reference name = new string_by_reference(64);
	int_by_reference axis = new int_by_reference();
	int_by_reference buttons = new int_by_reference();

	libjoy.JOY_LIBJOY_PROD_INFO prod_info = new libjoy.JOY_LIBJOY_PROD_INFO();

	native_long rc;

	// Get joystick info
	rc = m_joy.joy_get_name(m_joy_dev, name, 64);
	if (!rc.equals(libjoy.JOY_SUCCESS)) {
	    throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	}
	m_joy_info.name = name.get_value();

	rc = m_joy.joy_get_axis(m_joy_dev, axis);
	if (!rc.equals(libjoy.JOY_SUCCESS)) {
	    throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	}
	m_joy_info.axis = axis.getValue();

	rc = m_joy.joy_get_buttons(m_joy_dev, buttons);
	if (!rc.equals(libjoy.JOY_SUCCESS)) {
	    throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	}		
	m_joy_info.buttons = buttons.getValue();

	// Get shared library product info
	rc = m_joy.joy_test_get_libjoy_prod_info(prod_info);
	if (!rc.equals(libjoy.JOY_SUCCESS)) {
	    throw new Exp(m_joy_id + " : " + get_last_libjoy_error());
	}
	m_joy_info.lib_info =
	    new String(prod_info.prod_num) + "-" + new String(prod_info.rstate);
    }

    ////////////////////////////////////////////////////////

    @Override
    public void run()
    {
	native_long rc;

	short x_value = 0;
	short y_value = 0;
	libjoy.JOY_EVENT event = new libjoy.JOY_EVENT();
	libjoy.JOY_POS_UNIT_CIRCLE pos = new libjoy.JOY_POS_UNIT_CIRCLE();

	int event_cnt = 0;

	//debug(m_thread.getName() + " : start, active=" + m_thread_active);

	/////////////////////////////////////////////
	//     JOYSTICK READER THREAD MAIN LOOP
	/////////////////////////////////////////////
	try {
	    while (m_read_joystick) {
		// Check if thread is active
		if (!m_thread_active) {
		    //debug(m_thread.getName() + " : Not active");
		    try {
			Thread.sleep(1000); // Take it easy
		    }
		    catch (InterruptedException e) {
		    }		    
		    continue;
		}

		// Get joystick event
		rc = m_joy.joy_get_event(m_joy_dev, event);
		if (!rc.equals(libjoy.JOY_SUCCESS)) {
		    throw new Exp("joy_get_event : " + get_last_libjoy_error());
		}

		// Take it easy if no event available
		if ( event.type == libjoy.JOY_EVENT_NONE ) {
		    try {
			Thread.sleep(m_poll_interval_ms);
		    }
		    catch (InterruptedException e) {
		    }

		    // Notify this event using callback
		    m_joystick_event.joystick_axis_event_callback();

		    continue;
		}				

		/*
		debug(m_thread.getName() +
		      " : event=" + event_cnt++ +
		      ", type=" + Integer.toHexString((int)(event.type & 0xFF)) +
		      ", value=" + event.value +
		      ", number=" + (int)(event.number & 0xFF));
		*/

		// Handle horizontal value
                if ( ( (event.type & libjoy.JOY_EVENT_AXIS) != 0) &&
                     (event.number == libjoy.JOY_X_AXIS) ) {
                    x_value = event.value;
                }
                // Handle vertical value
                if ( ( (event.type & libjoy.JOY_EVENT_AXIS) != 0) &&
                     (event.number == libjoy.JOY_Y_AXIS) ) {
                    y_value = event.value;
                }

		// Only handle horizontal and vertical events
                if ( ( (event.type & libjoy.JOY_EVENT_AXIS) != 0) &&
                     ( (event.number == libjoy.JOY_X_AXIS) ||
                       (event.number == libjoy.JOY_Y_AXIS) ) ) {
                    
                    // Convert to unit circle
                    m_joy.joy_get_pos_unit_circle(x_value, y_value, pos);

		    // Update current position (lockdown the write operation)
		    try {
			m_joy_pos_lock.writeLock().lock();
			m_joy_pos.r     = pos.r;
			m_joy_pos.theta = pos.theta * (float)180.0 / (float)PI;
		    }
		    finally {
			m_joy_pos_lock.writeLock().unlock();
		    }

		    // Notify this event using callback
		    m_joystick_event.joystick_axis_event_callback();
                }		
	    }	    	    
	}
	catch(Exception exp) {
	    debug(m_thread.getName() + " => *** Exception : " + exp.getMessage());
	}
	catch (Error err) {
            debug(m_thread.getName() + " => *** Error : " + err.getMessage());
        }

	//debug(m_thread.getName() + " : exit" );
    }

    ////////////////////////////////////////////////////////

    private String get_last_libjoy_error()
    {
	native_long rc;
        libjoy.JOY_LIB_STATUS status = new libjoy.JOY_LIB_STATUS();
        string_by_reference error_string =
            new string_by_reference(libjoy.JOY_ERROR_STRING_LEN);

	try {
	    rc = m_joy.joy_get_last_error(status);
	    if (!rc.equals(libjoy.JOY_SUCCESS)) {
		return "LIBJOY: Failed to get last error";
	    }
	    
	    rc = m_joy.joy_get_error_string(status.error_code, error_string);
	    if (!rc.equals(libjoy.JOY_SUCCESS)) {
		return "LIBJOY: Failed to get error string";
	    }
	    
	    String err_str = "";
	    
	    switch (status.error_source) {
	    case libjoy.JOY_INTERNAL_ERROR:
		err_str = "LIBJOY error source : JOY_INTERNAL_ERROR";
		break;
	    case libjoy.JOY_LINUX_ERROR:
		err_str = "LIBJOY error source : JOY_LINUX_ERROR";
		break;
	    default:
		err_str = "LIBJOY error source : *** UNKNOWN";
		break;
	    }

	    err_str = err_str + "\nLIBJOY error code   : " + status.error_code;
	    err_str = err_str + "\nLIBJOY error string : " + error_string.get_value();

	    return err_str;
	}
	catch (Error err) {
            return err.getMessage();
        }
    }    

}
