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

import joystick.JoystickReader;

public class SteerPwm {

    // States for the internal state machine
    private enum STATE {
	STATE_INIT,
	STATE_FORWARD,
	STATE_REVERSE,
	STATE_LEFT,
	STATE_RIGHT;

	private int m_interval = 0;
	private int m_max_index = 0;
	private int m_index = 0;

	public void set_interval(int value) {
	    m_interval = value;
	}

	public int get_interval() {
	    return m_interval;
	}

	public void set_max_index(int value) {
	    m_max_index = value;
	}

	public void set_index(int value) {
	    m_index = value;
	}

	public void update_index() {
	    m_index++;
	    if (m_index > m_max_index) {
		m_index = 0;
	    }
	}

	public int get_index() {
	    return m_index;
	}
    }

    private STATE m_current_state;

    private int m_max_samples;

    private boolean[][] m_matrix; // Row : Interval, Col: Sample

    private int[] m_interval_index; // Current index for each interval

    ////////////////////////////////////////////////////////

    public SteerPwm(byte max_samples)
    {
	m_current_state = STATE.STATE_INIT;

	m_max_samples = 0xff & max_samples;	
	m_matrix = new boolean[m_max_samples][m_max_samples];
	m_interval_index = new int[m_max_samples];

	// Populate samples for all intervals (all samples OFF)
	for (int row=0; row < m_matrix.length; row++) {
	    for (int col=0; col < m_matrix[row].length; col++) {
		m_matrix[row][col] = true;
	    }
	}
	// Populate samples for all intervals (ON samples only)
	for (int row=0; row < m_matrix.length; row++) {
	    for (int col=row+1; col < m_matrix[row].length; col++) {
		m_matrix[row][col] = false;
	    }
	}

	// Reset index
	for (int i=0; i < m_interval_index.length; i++) {
	    m_interval_index[i] = 0;
	}
	
	// Debug: Visual inspection of PWM matrix
	/*
	for (int row=0; row < m_matrix.length; row++) {
	    for (int col=0; col < m_matrix[row].length; col++) {
		System.out.print(m_matrix[row][col] + " ");
	    }
	    System.out.println();
	}
	*/
    }

    ////////////////////////////////////////////////////////

    public Redrob.STEER_CODE get_steer_code(JoystickReader.JoystickPosition pos)
    {
	// Get direction
	Redrob.STEER_CODE code = get_base_code(pos);

	if (code == Redrob.STEER_CODE.NONE) {
	    return code;
	}

	// Get strength
	double amplitude = 0.0;
	switch (code) {
	case RIGHT:
	case LEFT:
	    amplitude = Math.abs( pos.r * Math.cos(Math.toRadians(pos.theta)) );
	    break;
	case FORWARD:
	case REVERSE:
	    amplitude = Math.abs( pos.r * Math.sin(Math.toRadians(pos.theta)) );;
	    break;	    
	}	

	// Get interval (and limit) : 1 -- MAX
	long interval = Math.round(amplitude * m_max_samples);
	if (interval > m_max_samples ) {
	    interval = m_max_samples;
	}
	else if (interval < 1) {
	    interval = 1;
	}
	//debug("interval="+interval+" / "+m_max_samples);	    

	// Get PWM steer code
	return get_pwm_code(code, (int)interval);
    }

    ////////////////////////////////////////////////////////

    private Redrob.STEER_CODE get_base_code(JoystickReader.JoystickPosition pos)
    {
	// Convert joystick position to actual steer code
	Redrob.STEER_CODE code;
	if (pos.r == 0.0) {
	    code = Redrob.STEER_CODE.NONE;
	}
	else if ( (pos.theta >= -45.0) && (pos.theta <= 45.0) ) {
	    code = Redrob.STEER_CODE.RIGHT;
	}
	else if ( (pos.theta > 45.0) && (pos.theta < 135.0) ) {
	    code = Redrob.STEER_CODE.FORWARD;
	}
	else if ( (pos.theta >= 135.0) ) {
	    code = Redrob.STEER_CODE.LEFT;
	}
	else if ( (pos.theta <= -135.0) ) {
	    code = Redrob.STEER_CODE.LEFT;
	}
	else {
	    code = Redrob.STEER_CODE.REVERSE;
	}
	
	return code;
    }

    ////////////////////////////////////////////////////////

    private Redrob.STEER_CODE get_pwm_code(Redrob.STEER_CODE code,
					   int interval)
    {
	STATE next_state = STATE.STATE_INIT;

	switch (m_current_state) {
	    /////////////////////////
	case STATE_INIT:
	case STATE_FORWARD:
	case STATE_REVERSE:
	case STATE_LEFT:
	case STATE_RIGHT:
	    /////////////////////////
	    switch (code) {
	    case NONE:
		next_state = STATE.STATE_INIT;
		break;
	    case FORWARD:
		next_state = STATE.STATE_FORWARD;
		break;
	    case REVERSE:
		next_state = STATE.STATE_REVERSE;
		break;
	    case RIGHT:
		next_state = STATE.STATE_RIGHT;
		break;
	    case LEFT:
		next_state = STATE.STATE_LEFT;
	    }
	    break;	    
	}	

	// Get PWM steer code for current state
	if (m_current_state == STATE.STATE_INIT) {
	    code = Redrob.STEER_CODE.NONE;
	}
	else {
	    if (!m_matrix[m_current_state.get_interval()][m_current_state.get_index()]) {
		code = Redrob.STEER_CODE.NONE;
	    }
	}
	/*
	debug("now="+m_current_state+
	      ", next="+next_state+
	      ", int="+interval+
	      ", cint="+m_current_state.get_interval()+
	      ", cidx="+m_current_state.get_index()+
	      ", val"+m_matrix[m_current_state.get_interval()][m_current_state.get_index()]+
	      ", code="+code);
	*/

	// Update state
	if (next_state != STATE.STATE_INIT) {
	    if (m_current_state != next_state) {
		next_state.set_interval(interval-1);
		next_state.set_max_index(m_max_samples-1);
		next_state.set_index(0);
		
		m_current_state = next_state;
	    }
	    else if (m_current_state.get_interval() != (interval-1)) {
		m_current_state.set_interval(interval-1);
		m_current_state.set_index(0);
	    }
	    else {
		m_current_state.update_index();
	    }
	}

	return code;
    }

    ////////////////////////////////////////////////////////

    private static void debug(String msg) {
        System.out.println(msg);
    }

}
