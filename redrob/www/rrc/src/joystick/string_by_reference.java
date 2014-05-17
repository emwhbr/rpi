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

import com.sun.jna.ptr.ByReference;

public class string_by_reference extends ByReference {

    ////////////////////////////////////////////////////////
    // Helper class for passing a String by reference to 
    // a native shared library so that the string can be
    // modified by the library.
    ////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////

    public string_by_reference() {
        this(0);
    }

    ////////////////////////////////////////////////////////

    public string_by_reference(int size) {
        super(size < 4 ? 4 : size);
        getPointer().clear(size < 4 ? 4 : size);
    }

    ////////////////////////////////////////////////////////

    public string_by_reference(String str) {
        super(str.length() < 4 ? 4 : str.length() + 1);
        set_value(str);
    }    

    ////////////////////////////////////////////////////////

    public String get_value() {
        return getPointer().getString(0);
    }

    ////////////////////////////////////////////////////////

    private void set_value(String str) {
        getPointer().setString(0, str);
    }
}
