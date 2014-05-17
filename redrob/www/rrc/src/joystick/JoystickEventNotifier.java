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

public class JoystickEventNotifier
{
    //////////////////////////////////////////////////////////////////
    // This is the base class for event notification.
    // The class that will signal the event needs to expect objects 
    // that implement the JoystickEvent-interface and then invoke 
    // the callback functions as appropriate.
    //////////////////////////////////////////////////////////////////

    protected JoystickEvent m_joystick_event;

    ////////////////////////////////////////////////////////

    public JoystickEventNotifier(JoystickEvent event)
    {
	// Save the event object for later (to execute callbacks)
	m_joystick_event = event;
    }

    ////////////////////////////////////////////////////////

    protected void debug(String msg)
    {
        System.out.println(msg);
    }
}
