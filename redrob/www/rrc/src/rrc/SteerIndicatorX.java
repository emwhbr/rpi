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

public class SteerIndicatorX {
    
    private States m_states;

    // EnumMap with key as enum type States
    private static EnumMap<States, JButton> m_button_map;
    
    ////////////////////////////////////////////////////////

    public SteerIndicatorX(JButton forward_button,
			   JButton reverse_button,
			   JButton right_button,
			   JButton left_button)
    {        
	m_button_map = new EnumMap<States, JButton>(States.class);

	m_button_map.put(States.INIT,    null);
	m_button_map.put(States.FORWARD, forward_button);
	m_button_map.put(States.REVERSE, reverse_button);
	m_button_map.put(States.LEFT,    left_button);
	m_button_map.put(States.RIGHT,   right_button);

        reset();
    }
    
    ////////////////////////////////////////////////////////

    public void process(Redrob.STEER_CODE code)
    {
        m_states = m_states.process(code);
    }

    ////////////////////////////////////////////////////////

    public void reset()
    {
	m_states = States.INIT;

	// Turn of all indicators
	unmark(m_button_map.get(States.FORWARD));
	unmark(m_button_map.get(States.REVERSE));
	unmark(m_button_map.get(States.LEFT));
	unmark(m_button_map.get(States.RIGHT));
    }
    
    ////////////////////////////////////////////////////////
    
    private static void mark(JButton button)
    {
	// Turn on indicator
	if (button != null) {
	    button.setForeground(Color.RED);
	}
    }
    
    ////////////////////////////////////////////////////////
    
    private static void unmark(JButton button)
    {
	// Turn off indicator
	if (button != null) {
	    button.setForeground(Color.BLACK);
	}
    }
    
    ////////////////////////////////////////////////////////

    private interface State {
        States process(Redrob.STEER_CODE code);
    }
    
    ////////////////////////////////////////////////////////

    private enum States implements State {	
	
        INIT {
            @Override
            public States process(Redrob.STEER_CODE code) {
                //debug(this.toString()+ " : " + code);
                
                States next_state = get_next_state(code);
                
                if (!next_state.equals(this)) {
                    //debug("Change state -> " + next_state.toString());
                    mark(m_button_map.get(next_state));
                }
                
                return next_state;
            }
        },
        FORWARD,
        REVERSE,
        RIGHT,
        LEFT;
            
        @Override
        public States process(Redrob.STEER_CODE code) {
            //debug(this.toString()+ " : " + code);
            
            States next_state = get_next_state(code);
            
            if (!next_state.equals(this)) {
                //debug("Change state -> " + next_state.toString());
		unmark(m_button_map.get(this));
		mark(m_button_map.get(next_state));
	    }
            
            return next_state;
        }

	////////////////////////////////////////////////////////
        
        private static States get_next_state(Redrob.STEER_CODE code)
        {
            States next_state = INIT;
            
            switch (code) {
                case NONE:
                    next_state = INIT;
                    break;
                case FORWARD:
                    next_state = FORWARD;
                    break;
                case REVERSE:
                    next_state = REVERSE;
                    break;
                case RIGHT:
                    next_state = RIGHT;
                    break;
                case LEFT:
                    next_state = LEFT;
                }
            
            return next_state;
        }                
    }

    ////////////////////////////////////////////////////////

    private static void debug(String msg) {
        System.out.println(msg);
    }
}
