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

public interface JoystickEvent
{
    ////////////////////////////////////////////
    // ANY CLASS THAT WANTS TO RECEIVE EVENT
    // NOTIFICATION BY CALLBACK FUNCTIONS MUST
    // IMPLEMENT THIS INTERFACE.
    ////////////////////////////////////////////

    // Callbacks
    public void joystick_axis_event_callback();
}
