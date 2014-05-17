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

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Structure;

public interface libjoy extends Library {

    ////////////////////////////////////////////////
    //     THIS IS AN INTERFACE MAPPED CLASS
    //     FOR THE NATIVE SHARED LIBRARY (LIBJOY).
    ///////////////////////////////////////////////

    // Native shared library name
    public static final String LINUX_LIB_NAME = "libjoy.so";

    // Helper class to load a library interface from the shared library
    public static class library_loader
    {
	public static libjoy load()
	{
	    return load("");
	}

	public static libjoy load(String search_path)
	{
	    String lib_path;

	    if (search_path.isEmpty()) {
		lib_path = LINUX_LIB_NAME;
	    }
	    else {
		lib_path = search_path + "/" + LINUX_LIB_NAME;
	    }
	    return (libjoy) Native.loadLibrary(lib_path,
					       libjoy.class);
	}
    }
    
    // Native shared library : Return codes
    public static final native_long JOY_SUCCESS             = new native_long( 0);
    public static final native_long JOY_FAILURE             = new native_long(-1);
    public static final native_long JOY_ERROR_MUTEX_FAILURE = new native_long(-2);

    // Native shared library : Internal error codes
    public static final native_long JOY_NO_ERROR                = new native_long(0);
    public static final native_long JOY_NOT_INITIALIZED         = new native_long(1);
    public static final native_long JOY_ALREADY_INITIALIZED     = new native_long(2);
    public static final native_long JOY_BAD_ARGUMENT            = new native_long(3);
    public static final native_long JOY_MUTEX_LOCK_FAILED       = new native_long(4);
    public static final native_long JOY_MUTEX_UNLOCK_FAILED     = new native_long(5);
    public static final native_long JOY_FILE_OPERATION_FAILED   = new native_long(6);
    public static final native_long JOY_DEVICE_OPERATION_FAILED = new native_long(7);
    public static final native_long JOY_WRONG_EVENT_SIZE        = new native_long(8);
    public static final native_long JOY_UNEXPECTED_EXCEPTION    = new native_long(9);

    // Native shared library : Error source values (4 bytes)
    public static final int JOY_INTERNAL_ERROR = 0;
    public static final int JOY_LINUX_ERROR    = 1;
    
    // Native shared library : Exported types
    public static final byte JOY_EVENT_NONE   = (byte)0x00;
    public static final byte JOY_EVENT_BUTTON = (byte)0x01;
    public static final byte JOY_EVENT_AXIS   = (byte)0x02;
    public static final byte JOY_EVENT_INIT   = (byte)0x80;

    public static final int JOY_DEV_0 = 0;
    public static final int JOY_DEV_1 = 1;

    public static final int JOY_ERROR_STRING_LEN = 256;

    public class JOY_LIBJOY_PROD_INFO extends Structure
    {
	public byte[] prod_num = new byte[20];
	public byte[] rstate = new byte[10];
	
	protected List getFieldOrder() {
	    return Arrays.asList(new String[] 
		{"prod_num", "rstate"});
	}
    }

    public class JOY_LIB_STATUS extends Structure
    {
	public int         error_source; // 4 bytes
	public native_long error_code;   // 8 bytes
	
	protected List getFieldOrder() {
	    return Arrays.asList(new String[] 
		{"error_source", "error_code"});
	}
    }

    public class JOY_EVENT extends Structure
    {
	public byte  type;   // event type
	public short value;  // axis: -32,767..+32,767, button: 0..1
	public byte  number; // axis/button number
	
	protected List getFieldOrder() {
	    return Arrays.asList(new String[] 
		{"type", "value", "number"});
	}
    }

    public class JOY_POS_UNIT_CIRCLE extends Structure
    {
	public float x;     // cartesian
	public float y;     // cartesian
	public float r;     // polar
	public float theta; // polar
	
	protected List getFieldOrder() {
	    return Arrays.asList(new String[] 
		{"x", "y", "r", "theta"});
	}
    }

    public static final byte JOY_X_AXIS = 0; // horizontal axis number
    public static final byte JOY_Y_AXIS = 1; // vertical axis number
    public static final byte JOY_Z_AXIS = 2; // "twist" axis number
    
    // Native shared library : Exported functions
    public native_long joy_get_last_error(JOY_LIB_STATUS status);

    public native_long joy_get_error_string(native_long error_code, 
					    string_by_reference error_string);

    public native_long joy_initialize(int dev,
				      boolean non_block);

    public native_long joy_finalize(int dev);

    public native_long joy_get_name(int dev,
				    string_by_reference name,
				    int len);

    public native_long joy_get_axis(int dev,
				    int_by_reference axis);

    public native_long joy_get_buttons(int dev,
				       int_by_reference buttons);

    public native_long joy_get_event(int dev,
				     JOY_EVENT event);

    public native_long joy_get_pos_unit_circle(short x_value,
					       short y_value,
					       JOY_POS_UNIT_CIRCLE pos);

    public native_long joy_test_get_libjoy_prod_info(JOY_LIBJOY_PROD_INFO prod_info);
}
