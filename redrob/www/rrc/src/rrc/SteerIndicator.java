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

import javax.swing.JButton;
import java.awt.Color;
import java.util.EnumMap;

public class SteerIndicator {
    
    // States for the internal state machine
    private enum STATE {STATE_INIT,
			STATE_FORWARD,
			STATE_REVERSE,
			STATE_LEFT,
			STATE_RIGHT};

    private STATE m_previous_state;
    private STATE m_current_state;

    // EnumMap with key as enum type STATE
    EnumMap<STATE, JButton> m_button_map;

    ////////////////////////////////////////////////////////

    public SteerIndicator(JButton forward_button,
			  JButton reverse_button,
			  JButton right_button,
			  JButton left_button)
    {
	m_button_map = new EnumMap<STATE, JButton>(STATE.class);

	m_button_map.put(STATE.STATE_INIT,    null);
	m_button_map.put(STATE.STATE_FORWARD, forward_button);
	m_button_map.put(STATE.STATE_REVERSE, reverse_button);
	m_button_map.put(STATE.STATE_LEFT,    left_button);
	m_button_map.put(STATE.STATE_RIGHT,   right_button);

	reset();
    }

    ////////////////////////////////////////////////////////

    public void reset()
    {
	m_previous_state = STATE.STATE_INIT;
	m_current_state  = STATE.STATE_INIT;

	// Turn of all indicators
	unmark(m_button_map.get(STATE.STATE_FORWARD));
	unmark(m_button_map.get(STATE.STATE_REVERSE));
	unmark(m_button_map.get(STATE.STATE_LEFT));
	unmark(m_button_map.get(STATE.STATE_RIGHT));
    }

    ////////////////////////////////////////////////////////

    private void mark(JButton button)
    {
	// Turn on indicator
	if (button != null) {
	    button.setForeground(Color.RED);
	}
    }

    ////////////////////////////////////////////////////////

    private void unmark(JButton button)
    {
	// Turn off indicator
	if (button != null) {
	    button.setForeground(Color.BLACK);
	}
    }

    ////////////////////////////////////////////////////////

    public void process(Redrob.STEER_CODE code)
    {
	STATE next_state = STATE.STATE_INIT;

	switch (m_current_state) {
	    /////////////////////////
	case STATE_INIT:
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
	    if (next_state != m_current_state) {
		mark(m_button_map.get(next_state));
	    }
	    break;	    
	    /////////////////////////
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
	    if (next_state != m_current_state) {
		unmark(m_button_map.get(m_current_state));
		mark(m_button_map.get(next_state));
	    }
	    break;	    
	}

	m_current_state = next_state;
    }

}
