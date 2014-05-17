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

import com.sun.jna.NativeLong;

public class native_long extends NativeLong {

    ////////////////////////////////////////////////////////
    // Helper class for encapsulating JNA type NativeLong.
    ////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////

    public native_long() {
        this(0);
    }

    ////////////////////////////////////////////////////////

    public native_long(int value) {
        super(value);
    }
}
