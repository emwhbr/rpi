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

import javax.swing.AbstractButton;
import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.ButtonModel;
import javax.swing.ImageIcon;
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButtonMenuItem;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.DecimalFormat;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;

import joystick.JoystickEvent;
import joystick.JoystickReader;

public class AppletRrc extends JApplet implements Runnable, JoystickEvent {    

    private static final String PROD_NAME = "Redrob Remote Control";
    private static final String PROD_REV  = "R1A08";

    private static final String STEER_BUTT_FORWARD = "FRAMÅT";
    private static final String STEER_BUTT_REVERSE = "BAKÅT";
    private static final String STEER_BUTT_RIGHT   = "HÖGER";
    private static final String STEER_BUTT_LEFT    = "VÄNSTER";

    private Map<String, Boolean> m_steer_map;    

    private Redrob m_redrob;

    private JSObject m_jso;

    private JMenu m_joystick_submenu_open;
    private JMenuItem m_joystick_menu_item_close;    
    private JoystickReader.JOYSTICK_ID m_joy_id;
    private JoystickReader m_joy_reader;
    private boolean m_joy_supported;
    private JoystickReader.JoystickInfo m_joy_info;
    private SteerPwm m_joy_pwm;

    private enum THREAD_CMD {NONE,
			     CONNECT,
			     DISCONNECT,
			     STEER,
                             VIDEO};

    private Thread m_main_thread;
    private boolean m_main_thread_running;
    private THREAD_CMD m_main_thread_cmd;

    private static final int MAIN_THREAD_PERIOD_TIME_MS = 15; // 66.7 Hz
    private static final int JOYSTICK_POLL_INTERVALL_MS = 15; // 66.7 Hz

    private JPanel m_content;    

    private JButton m_forward_button;
    private JButton m_reverse_button;
    private JButton m_right_button;
    private JButton m_left_button;

    private SteerIndicatorX m_steer_indicator;

    private JButton m_connect_button;
    private JButton m_disconnect_button;
    private static boolean m_is_connected;

    private JLabel m_connected_state_label;
    private ImageIcon m_connected_icon;
    private ImageIcon m_disconnected_icon;

    private JCheckBox m_joystick_checkbox;
    private JCheckBox m_video_checkbox;
    private JCheckBox m_stats_checkbox;

    private SystemStatDialog m_sys_stats_dialog;
    private long m_time_last_sys_stats;
    private static final long SYS_STATS_UPDATE_TIME_MS = 1000; // ms

    private VoltageBar m_voltage_bar;
    private JLabel m_voltage_label;
    private JLabel m_voltage_value_label;

    private DecimalFormat m_voltage_format;
    private long m_time_last_voltage;
    private boolean m_first_voltage_value;

    private static final String NO_VOLTAGE_VALUE = "N/A";
    private static final long VOLTAGE_UPDATE_TIME_MS = 250; // ms

    private static final int MIN_BAR_VOLTAGE = 6000; // mV
    private static final int MAX_BAR_VOLTAGE = 8500; // mV

    private static final int VOLTAGE_WARNING_LEVEL = 6900; // mV
    private static final int VOLTAGE_ALERT_LEVEL = 6500; // mV


    ////////////////////////////////////////////////////////

    @Override
    public void init()
    {
	//debug("init...");	

	// Initialize variables
        m_steer_map = new HashMap<>();
	m_steer_map.put(STEER_BUTT_FORWARD, false);
	m_steer_map.put(STEER_BUTT_REVERSE, false);
	m_steer_map.put(STEER_BUTT_RIGHT,   false);
	m_steer_map.put(STEER_BUTT_LEFT,    false);

	m_redrob = new Redrob(getCodeBase().getHost());

	m_jso = JSObject.getWindow(this); // Interface to javascript
                                          // in same HTML page as applet

	// Joystick layer not yet created
	m_joy_id        = null;
	m_joy_reader    = null;
	m_joy_supported = false; // Assume the worst	

	m_is_connected = false;

	m_time_last_sys_stats = System.nanoTime();

	m_voltage_format = new DecimalFormat("00.000");
	m_time_last_voltage = System.nanoTime();

	m_main_thread_running = true;
	m_main_thread_cmd = THREAD_CMD.NONE;

	// Platform specific initializations
	// Check OS and architecture
	String os_name = System.getProperty("os.name");
	String os_arch = System.getProperty("os.arch");
	if ( os_name.equals("Linux") &&
	     (os_arch.equals("amd64") || os_arch.equals("x86_64")) ) {
	    m_joy_supported = true;
	    m_joy_pwm = new SteerPwm((byte)4);
	}

	// Initialize the GUI 
	try { 
	    init_gui();
        } 
	catch(Exception exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}

	// Create the main thread
	m_main_thread = new Thread(this);
	m_main_thread.setName("Main");
	m_main_thread.start();
    }

    ////////////////////////////////////////////////////////

    @Override
    public void start()
    {
	//debug("start...");
	m_first_voltage_value = true;
    }

    ////////////////////////////////////////////////////////

    @Override
    public void stop()
    {
        //debug("stop...");

	// Cleanup joystick layer
	if (m_joy_reader != null) {
	    try {		
		//debug("stop: Cleanup joystick layer");
		m_joy_reader.cleanup();
	    }
	    catch(JoystickReader.Exp exp) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		exp.printStackTrace(pw);
		err_msg_box(exp.getMessage() + "\n" + sw);
	    }
	}

	// Disconnect from target
	if (m_is_connected) {

	    m_main_thread_running = false;

	    try {
		m_redrob.disconnect();
	    }
	    catch(IOException exp) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		exp.printStackTrace(pw);
		err_msg_box(exp.getMessage() + "\n" + sw);
	    }
	    m_is_connected = false; // Mark as NOT connected
	}	
    }

    ////////////////////////////////////////////////////////

    @Override
    public void destroy()
    {
        //debug("destroy...");

	// Finalize thread
	m_main_thread_running = false;
	try {
	    if (m_main_thread.isAlive()) {
		//debug("destroy : Waiting for thread " +  m_main_thread.getName() + " to die");
		try {		   
		    m_main_thread.join(1000);
		    if (m_main_thread.isAlive()) {
			throw new Exp("Timout waiting for thread " + m_main_thread.getName()); 
		    }
		}
		catch (InterruptedException exp) {
		    throw new Exp("Error waiting for thread " + m_main_thread.getName());
		}
	    }
	}
	catch(Exp exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}

	m_main_thread = null;
    }

    ////////////////////////////////////////////////////////

    @Override
    public void run()
    {
	//debug(m_main_thread.getName() + " : started");	

	int cnt = 0;

	while (m_main_thread_running) {
	    /*
	    debug(m_main_thread.getName() + 
		  " : " + cnt++ + ", cmd=" + m_main_thread_cmd);
	    */

	    // Execute command
	    switch (m_main_thread_cmd) {
	    case NONE:
		do_command_none();
		break;
	    case CONNECT:
		do_command_connect();
		break;
	    case DISCONNECT:
		do_command_disconnect();
		break;
	    case STEER:
		do_command_steer();
		break;
	    case VIDEO:
		do_command_video();
		break;
	    }	    

	    // Take it easy
	    try {
		Thread.sleep(MAIN_THREAD_PERIOD_TIME_MS);
	    }
	    catch (InterruptedException e) {
	    }
	}

	//debug(m_main_thread.getName() + " : exit");
    }

    ////////////////////////////////////////////////////////

    @Override
    public void joystick_axis_event_callback()
    {
	// This is the callback for joystick AXIS events	
	//debug("CALLBACK - joystick_axis_event_callback()");

	m_main_thread_cmd = THREAD_CMD.STEER;
    }

    ////////////////////////////////////////////////////////

    public class Exp extends Exception {
	public Exp(String msg) {
	    super(msg);
	}
    }    

    ////////////////////////////////////////////////////////

    private void do_command_none()
    {
	// Check if connected
	if (!m_is_connected) {
	    return;
	}

	try {
	    // Keep Redrob happy, send steer code
	    m_redrob.send_steer_code(Redrob.STEER_CODE.NONE);

	    // Update system statistics
	    update_system_stats();

	    // Update voltage value
	    update_voltage();
	}
	catch(IOException exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private void do_command_connect()
    {
	m_main_thread_cmd = THREAD_CMD.NONE; // Clear command

	try {
	    // Do connect
	    m_redrob.connect();
	    m_is_connected = true; // Mark as connected

	    // Enable steer buttons
	    m_forward_button.setEnabled(true);
	    m_reverse_button.setEnabled(true);
	    m_right_button.setEnabled(true);
	    m_left_button.setEnabled(true);

	    // Enable joystick checkbox
	    if (m_joy_reader != null) {
		m_joystick_checkbox.setEnabled(true);
	    }

	    // Enable video stream checkbox
	    m_video_checkbox.setEnabled(true);

	    // Enable system stats checkbox
	    m_stats_checkbox.setEnabled(true);

	    // Enable disconnect button, disable connect button
	    m_connect_button.setEnabled(false);
	    m_disconnect_button.setEnabled(true);
	    m_connected_state_label.setIcon(m_connected_icon);
	    
	    // Mark voltage as unavailable
	    m_voltage_value_label.setText(NO_VOLTAGE_VALUE);
	    m_voltage_bar.deactivate();
	}
	catch(IOException exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private void do_command_disconnect()
    {
	m_main_thread_cmd = THREAD_CMD.NONE; // Clear command

	try {
	    // Stop video stream if started
	    if (m_video_checkbox.isSelected()) {
		m_redrob.send_camera_code(Redrob.CAMERA_CODE.STOP_STREAM);
		m_video_checkbox.setSelected(false);

		// Call javascript's method to disable vlc toolbar
		try {
		    m_jso.call("set_vlc_toolbar_state",
			       new String[]{"disable"});
		}
		catch (JSException exp) {
		    StringWriter sw = new StringWriter();
		    PrintWriter pw = new PrintWriter(sw);
		    exp.printStackTrace(pw);
		    err_msg_box(exp.getMessage() + "\n" + sw);
		}
	    }

	    // Do disconnect
	    m_redrob.disconnect();
	    m_is_connected = false; // Mark as NOT connected

	    // Disable steer buttons
	    m_forward_button.setEnabled(false);
	    m_reverse_button.setEnabled(false);
	    m_right_button.setEnabled(false);
	    m_left_button.setEnabled(false);

	    // Disable joystick checkbox
	    if (m_joy_reader != null) {
		m_joystick_checkbox.setSelected(false);
		m_joystick_checkbox.setEnabled(false);
	    }

	    // Disable video stream checkbox
	    m_video_checkbox.setEnabled(false);

	    // Disable system stats checkbox
	    if (m_stats_checkbox.isSelected()) {
		m_stats_checkbox.setSelected(false);
		m_sys_stats_dialog.setVisible(false);
	    }
	    m_stats_checkbox.setEnabled(false);

	    // Enable connect button, disable disconnect button
	    m_connect_button.setEnabled(true);
	    m_disconnect_button.setEnabled(false);
	    m_connected_state_label.setIcon(m_disconnected_icon);
	    
	    // Mark voltage as unavailable
	    m_voltage_value_label.setText(NO_VOLTAGE_VALUE);
	    m_voltage_bar.deactivate();
	    m_first_voltage_value = true;
	}
	catch(IOException exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private void do_command_steer()
    {
	// Check if connected
	if (!m_is_connected) {
	    m_main_thread_cmd = THREAD_CMD.NONE; // Clear command
	    return;
	}

	Redrob.STEER_CODE code;

	// Joystick layer has higher priority
	if (m_joystick_checkbox.isSelected()) {
	    code = get_joystick_steer_code();
	}
	else {
	    code = get_button_steer_code();
	}

	// Update steer indicator
	m_steer_indicator.process(code);

	try {
	    // Send actual steer code
	    m_redrob.send_steer_code(code);

	    // Update system statistics
	    update_system_stats();

	    // Update voltage value
	    update_voltage();
	}
	catch(IOException exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private void do_command_video()
    {
	m_main_thread_cmd = THREAD_CMD.NONE; // Clear command
	
	try {
	    // Start/stop video stream
	    if (m_video_checkbox.isSelected()) {
		m_redrob.send_camera_code(Redrob.CAMERA_CODE.START_STREAM);
	    }
	    else {
		m_redrob.send_camera_code(Redrob.CAMERA_CODE.STOP_STREAM);
	    }
	}
	catch(IOException exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}
    }

    ////////////////////////////////////////////////////////

    private Redrob.STEER_CODE get_button_steer_code()
    {
	String steer_button = "";

	// Find which steer button that is active
	for (String key : m_steer_map.keySet()) {
	    if (m_steer_map.get(key)) {
		//debug("get_button_steer_code=" + key);
		steer_button = key;
		break;
	    }
	}
	
	// Get actual steer code
	Redrob.STEER_CODE code;
        switch (steer_button) {
            case STEER_BUTT_FORWARD:		
                code = Redrob.STEER_CODE.FORWARD;
                break;
            case STEER_BUTT_REVERSE:
                code = Redrob.STEER_CODE.REVERSE;
                break;
            case STEER_BUTT_RIGHT:
                code = Redrob.STEER_CODE.RIGHT;
                break;
            case STEER_BUTT_LEFT:
                code = Redrob.STEER_CODE.LEFT;
                break;
            default:
                // This should not happen when when this function
                // is called. There should be a button pressed.
                code = Redrob.STEER_CODE.NONE;
                break;
        }

	return code;
    }

    ////////////////////////////////////////////////////////

    private Redrob.STEER_CODE get_joystick_steer_code()
    {
	if (m_joy_reader == null) {
	    return Redrob.STEER_CODE.NONE;
	}

	// Check that joystick layer is producing as expected
	if (!m_joy_reader.is_alive()) {
	    info_msg_box("Joystick not responding");
	    close_joystick();
	    return Redrob.STEER_CODE.NONE;
	}	

	// Get position from joystick layer
	JoystickReader.JoystickPosition pos = m_joy_reader.get_current_position();
	/*
	debug("get_joystick_steer_code: pos=> r=" + pos.r + 
	      ", theta=" + pos.theta);
	*/

	// Convert joystick position to actual steer code
	return m_joy_pwm.get_steer_code(pos);
    }

    ////////////////////////////////////////////////////////

    private void update_voltage() throws IOException
    {
	// Check if time to update voltage
	if ( (System.nanoTime() - m_time_last_voltage) >
	     (VOLTAGE_UPDATE_TIME_MS * 1000000) ) {
	    
	    // Get actual voltage
	    int voltage_mv = m_redrob.get_voltage_mv();
	    float voltage = ((float) voltage_mv) / (float)1000.0;

	    // Update
	    m_voltage_value_label.setText(m_voltage_format.format(voltage));
	    m_voltage_bar.update_voltage(voltage_mv);
	    if (m_first_voltage_value) {
		m_voltage_bar.activate();
		m_first_voltage_value = false;
	    }	    
	    
	    // Save last update time
	    m_time_last_voltage = System.nanoTime();	
	}
    }

    ////////////////////////////////////////////////////////

    private void update_system_stats() throws IOException
    {
	// Only update if dialog is visible
	if (m_sys_stats_dialog.isVisible()) {

	    // Check if time to update stats
	    if ( (System.nanoTime() - m_time_last_sys_stats) >
		 (SYS_STATS_UPDATE_TIME_MS * 1000000) ) {
		
		// Get actual stats
		Redrob.SysStats s = new Redrob.SysStats();
		m_redrob.get_sys_stats(s);
		
		// Update GUI
		m_sys_stats_dialog.set_cpu_load(s.cpu_load);
		m_sys_stats_dialog.set_mem_used(s.mem_used);
		m_sys_stats_dialog.set_irq(s.irq);
		m_sys_stats_dialog.set_uptime(s.uptime);
		m_sys_stats_dialog.set_cpu_temp(s.cpu_temp);
		m_sys_stats_dialog.set_cpu_voltage(s.cpu_voltage);
		m_sys_stats_dialog.set_cpu_freq(s.cpu_freq);
		
		// Save last update time
		m_time_last_sys_stats = System.nanoTime();	
	    }
	}
    }

    ////////////////////////////////////////////////////////

    private void debug(String msg)
    {
        System.out.println(msg);
    }

    ////////////////////////////////////////////////////////

    private void err_msg_box(String msg) {
	JOptionPane.showMessageDialog(null,
				      "***EXCEPTION: " + msg);
    }

    ////////////////////////////////////////////////////////

    private void info_msg_box(String msg) {
	JOptionPane.showMessageDialog(null, msg);
    }

    ////////////////////////////////////////////////////////

    private void steer_button_state_changed(ChangeEvent e,
					    JButton button)
    {
	String button_id = button.getText();
	ButtonModel model = (ButtonModel)e.getSource();

	if (model.isPressed()) {
	    //debug(button_id + " - PRESSED");
	    m_steer_map.put(button_id, true); // Steer button active
	    m_main_thread_cmd = THREAD_CMD.STEER;
	}
	else {
	    //debug(button_id + " - NOT PRESSED");
	    m_steer_map.put(button_id, false);   // Steer button not active
	    m_main_thread_cmd = THREAD_CMD.NONE; // Clear command
	    m_steer_indicator.reset();           // Reset indicators
	}
    }

    ////////////////////////////////////////////////////////

    private void create_steer_button(final JButton button,
				     String text,
				     int loc_x,
				     int loc_y)
    {
	button.setEnabled(false);
	button.setText(text);
	button.getModel().addChangeListener(new ChangeListener()
	    {
		@Override
		public void stateChanged(ChangeEvent e)
		{
		    steer_button_state_changed(e, button);
		}
	    });
	button.setBounds(loc_x, loc_y, 130, 30);
	m_content.add(button);
    }

    ////////////////////////////////////////////////////////

    private void help_menu_item_about_action_performed(ActionEvent e)
    {
	info_msg_box("Name: "+ PROD_NAME + "\n" +
		     "Rev: " + PROD_REV + "\n" +
		     "Creator: Bonden i Nol\n" +
		     "E-mail: hakanbrolin@hotmail.com");
    }

    ////////////////////////////////////////////////////////

    private void joystick_menu_item_info_action_performed(ActionEvent e)
    {
	if (!m_joy_supported) {
	    String os_name = System.getProperty("os.name");
	    String os_arch = System.getProperty("os.arch");	    
	    info_msg_box("Joystick only supported for 64-bit Linux\n" +
			 "Current OS : " + os_name + "-" + os_arch);
	    return;
	}

	if (m_joy_reader == null) {
	    info_msg_box("No joystick open");
	}
	else {
	    String msg =
		String.format("Name: %s\n" +
			      "Axis: %s\n" +
			      "Buttons: %s\n" +
			      "Lib: %s",
			      m_joy_info.name,
			      m_joy_info.axis,
			      m_joy_info.buttons,
			      m_joy_info.lib_info);
	    info_msg_box(msg);
	}
    }

    ////////////////////////////////////////////////////////
    
    private class RbJoyDevListener implements ActionListener {

	private JoystickEvent m_joystick_event;

	public RbJoyDevListener(JoystickEvent joystick_event)
	{
	    m_joystick_event = joystick_event;
	}

	@Override
        public void actionPerformed(ActionEvent e) {

	    switch (e.getActionCommand()) {
	    case "JOY_0":
		m_joy_id = JoystickReader.JOYSTICK_ID.JOYSTICK_0;
		break;
	    case "JOY_1":
		m_joy_id = JoystickReader.JOYSTICK_ID.JOYSTICK_1;
		break;
	    }

	    //debug("Creating joystick layer");
	    m_joy_reader = new JoystickReader(m_joystick_event,
					      m_joy_id,
					      "/linux-lib64",
					      JOYSTICK_POLL_INTERVALL_MS);
	    try {
		//debug("Setup joystick layer");
		m_joy_reader.setup();
		
		// Save this for later
		m_joy_info = m_joy_reader.get_info();
		
		//debug("Start joystick layer");
		m_joy_reader.start();
		
		// Activate joystick layer
		m_joy_reader.set_active(true);

		if (m_is_connected) {
		    m_joystick_checkbox.setEnabled(true);
		}
		m_joystick_menu_item_close.setEnabled(true);
		m_joystick_submenu_open.setEnabled(false);
	    }
	    catch(JoystickReader.Exp exp) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		exp.printStackTrace(pw);
		err_msg_box(exp.getMessage() + "\n" + sw);
				
		// Try to cleanup
		close_joystick();
		m_joy_reader = null;
	    }	    	    	    
        }
    }

    ////////////////////////////////////////////////////////

    private void close_joystick()
    {
	// Cleanup joystick layer
	try {		
	    //debug("Cleanup joystick layer");
	    m_joy_reader.cleanup();
	    m_joy_reader = null;

	    m_joystick_checkbox.setSelected(false);
	    m_joystick_checkbox.setEnabled(false);
	    m_joystick_menu_item_close.setEnabled(false);
	    m_joystick_submenu_open.setEnabled(true);
	}
	catch(JoystickReader.Exp exp) {
	    StringWriter sw = new StringWriter();
	    PrintWriter pw = new PrintWriter(sw);
	    exp.printStackTrace(pw);
	    err_msg_box(exp.getMessage() + "\n" + sw);
	}	
    }

    ////////////////////////////////////////////////////////

    private void joystick_menu_item_close_action_performed(ActionEvent e)
    {
	close_joystick();
    }

    ////////////////////////////////////////////////////////

    private void connect_button_action_performed(ActionEvent e)
    {
	String action = e.getActionCommand();

	if (action.equals(m_connect_button.getText())) {
	    //debug("CONNECT BUTTON - ACTIVATED");
	    m_main_thread_cmd = THREAD_CMD.CONNECT;
	}
	if (action.equals(m_disconnect_button.getText())) {
	    //debug("DISCONNECT BUTTON - ACTIVATED");
	    m_main_thread_cmd = THREAD_CMD.DISCONNECT;
	}
    }

    ////////////////////////////////////////////////////////

    private void joystick_checkbox_action_performed(ActionEvent e)
    {
	AbstractButton abstractButton = (AbstractButton) e.getSource();
        if (abstractButton.getModel().isSelected()) {
	    //debug("JOYSTICK CHECKBOX - SELECTED");	    
	    // Activate joystick layer
	    m_joy_reader.set_active(true);
	}
	else {
	    //debug("JOYSTICK CHECKBOX - NOT SELECTED");
	    // Deactivate joystick layer
	    m_joy_reader.set_active(false);
	}
    }

    ////////////////////////////////////////////////////////

    private void video_checkbox_action_performed(ActionEvent e)
    {
	AbstractButton abstractButton = (AbstractButton) e.getSource();
        if (abstractButton.getModel().isSelected()) {
	    //debug("VIDEO CHECKBOX - SELECTED");
	    
	    // Call javascript's method to enable vlc toolbar
	    try {
		m_jso.call("set_vlc_toolbar_state",
			   new String[]{"enable"});
	    }
	    catch (JSException exp) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		exp.printStackTrace(pw);
		err_msg_box(exp.getMessage() + "\n" + sw);
	    }
	}
	else {
	    //debug("VIDEO CHECKBOX - NOT SELECTED");

	    // Call javascript's method to disable vlc toolbar
	    try {
		m_jso.call("set_vlc_toolbar_state",
			   new String[]{"disable"});
	    }
	    catch (JSException exp) {
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		exp.printStackTrace(pw);
		err_msg_box(exp.getMessage() + "\n" + sw);
	    }
	}

	m_main_thread_cmd = THREAD_CMD.VIDEO;
    }

    ////////////////////////////////////////////////////////

    private void sys_stats_checkbox_action_performed(ActionEvent e)
    {
	AbstractButton abstractButton = (AbstractButton) e.getSource();
        if (abstractButton.getModel().isSelected()) {
	    //debug("SYS STATS CHECKBOX - SELECTED");
	    m_sys_stats_dialog.reset_stats();
	    m_sys_stats_dialog.setVisible(true);
	}
	else {
	    //debug("SYS STATS CHECKBOX - NOT SELECTED");
	    m_sys_stats_dialog.setVisible(false);
	}
    }    

    ////////////////////////////////////////////////////////

    private void set_enable_all_buttons(ButtonGroup group,
					boolean b)
    {
	Enumeration<AbstractButton> buttons = group.getElements();
	while (buttons.hasMoreElements()) {
	    AbstractButton button = buttons.nextElement();
	    button.setEnabled(b);
	}
    }

    ////////////////////////////////////////////////////////

    private void init_gui() throws Exception
    {
	//////////////
	// Top level
	//////////////
	m_content = new JPanel();
	m_content.setLayout(null); // Use no layout
	m_content.setBorder(BorderFactory.createLineBorder(Color.black));
	this.setContentPane(m_content);

	////////////////////////
	// Create the menu bar
	////////////////////////
        JMenuBar menu_bar = new JMenuBar();
        menu_bar.setOpaque(true);
        menu_bar.setBackground(new Color(154, 165, 127));
        menu_bar.setPreferredSize(new Dimension(500, 20));
	this.setJMenuBar(menu_bar);

	/////////////////////////
	// Create the help menu
	/////////////////////////
	JMenu help_menu = new JMenu("Help");
	menu_bar.add(help_menu);
	JMenuItem help_menu_item_about = new JMenuItem("About");	
	help_menu_item_about.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    help_menu_item_about_action_performed(e);		    
		}
	    });
	help_menu.add(help_menu_item_about);

	/////////////////////////////
	// Create the joystick menu
	/////////////////////////////
	JMenu joystick_menu = new JMenu("Joystick");	
	menu_bar.add(joystick_menu);
	JMenuItem joystick_menu_item_info = new JMenuItem("Info");	
	joystick_menu_item_info.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    joystick_menu_item_info_action_performed(e);
		}
	    });
	joystick_menu.add(joystick_menu_item_info);

	/////////////////////////////////
	// Create the joystick sub-menu
	/////////////////////////////////
	m_joystick_submenu_open = new JMenu("Open");
	joystick_menu.add(m_joystick_submenu_open);	
	if (m_joy_supported) {
	    ButtonGroup rb_grp_joy_dev = new ButtonGroup();
	    JRadioButtonMenuItem rb_joy_dev_0 = new JRadioButtonMenuItem("JOYSTICK 0");
	    JRadioButtonMenuItem rb_joy_dev_1 = new JRadioButtonMenuItem("JOYSTICK 1");
	    rb_joy_dev_0.setSelected(true);
	    rb_joy_dev_1.setSelected(false);
	    rb_grp_joy_dev.add(rb_joy_dev_0);
	    rb_grp_joy_dev.add(rb_joy_dev_1);
	    set_enable_all_buttons(rb_grp_joy_dev, true);
	    rb_joy_dev_0.setActionCommand("JOY_0");
	    rb_joy_dev_1.setActionCommand("JOY_1");
	    RbJoyDevListener rb_dev_listener = new RbJoyDevListener(this);
	    rb_joy_dev_0.addActionListener(rb_dev_listener);
	    rb_joy_dev_1.addActionListener(rb_dev_listener);

	    m_joystick_submenu_open.add(rb_joy_dev_0);	       	
	    m_joystick_submenu_open.add(rb_joy_dev_1);
	    m_joystick_submenu_open.setEnabled(true);
	}
	else {
	    m_joystick_submenu_open.setEnabled(false);
	}			

	///////////////////////////////
	// Continue the joystick menu
	///////////////////////////////
	m_joystick_menu_item_close = new JMenuItem("Close");
	joystick_menu.add(m_joystick_menu_item_close); 
	m_joystick_menu_item_close.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    joystick_menu_item_close_action_performed(e);
		}
	    });
	m_joystick_menu_item_close.setEnabled(false);	      	

	/////////////////////////
	// Create steer buttons
	/////////////////////////
	m_forward_button = new JButton();
	m_reverse_button = new JButton();
	m_right_button   = new JButton();
	m_left_button    = new JButton();
	create_steer_button(m_forward_button, STEER_BUTT_FORWARD, 180,  10);
	create_steer_button(m_reverse_button, STEER_BUTT_REVERSE, 180, 100);
	create_steer_button(m_right_button,   STEER_BUTT_RIGHT,   260,  55);
	create_steer_button(m_left_button,    STEER_BUTT_LEFT,    100,  55);

	// Create steer indicator object (based on steer buttons)
	m_steer_indicator = new SteerIndicatorX(m_forward_button,
						m_reverse_button,
						m_right_button,
						m_left_button);

	///////////////////////////
	// Create connect button
	///////////////////////////
	m_connect_button = new JButton("CONNECT");
	m_connect_button.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    connect_button_action_performed(e);
		}
	    });
	m_connect_button.setBounds(10, 165, 100, 25);
	m_connect_button.setEnabled(true);
	m_content.add(m_connect_button);

	/////////////////////////////
	// Create disconnect button
	/////////////////////////////
	m_disconnect_button = new JButton("DISCONNECT");
	m_disconnect_button.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    connect_button_action_performed(e);
		}
	    });
	m_disconnect_button.setBounds(360, 165, 130, 25);
	m_disconnect_button.setEnabled(false);
	m_content.add(m_disconnect_button);

	/////////////////////////////////////////////////
	// Create the connected state label (and icons)
	/////////////////////////////////////////////////
	class EmptyClass{}
	Class load_class = new EmptyClass().getClass();

	m_connected_icon =
	    new ImageIcon(load_class.getResource("/res/led_green.png"));

	m_disconnected_icon =
	    new ImageIcon(load_class.getResource("/res/led_red.png"));

	m_connected_state_label = new JLabel();
	m_connected_state_label.setIcon(m_disconnected_icon);
	m_connected_state_label.setBounds(10, 10, 30, 30);	
	m_content.add(m_connected_state_label);

	/////////////////////////////////
	// Create the joystick checkbox
	/////////////////////////////////
	m_joystick_checkbox = new JCheckBox("Joystick");
	m_joystick_checkbox.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    joystick_checkbox_action_performed(e);
		}
	    });
	m_joystick_checkbox.setBounds(10, 60, 85, 20);
	m_joystick_checkbox.setSelected(false);
	m_joystick_checkbox.setEnabled(false);
	m_content.add(m_joystick_checkbox);

	/////////////////////////////////////
	// Create the video stream checkbox
	/////////////////////////////////////
	m_video_checkbox = new JCheckBox("Redcam");
	m_video_checkbox.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    video_checkbox_action_performed(e);
		}
	    });
	m_video_checkbox.setBounds(10, 105, 85, 20);
	m_video_checkbox.setSelected(false);
	m_video_checkbox.setEnabled(false);
	m_content.add(m_video_checkbox);
	
	/////////////////////////////////////
	// Create the system stats checkbox
	/////////////////////////////////////
	m_stats_checkbox = new JCheckBox("Redstats");
	m_stats_checkbox.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    sys_stats_checkbox_action_performed(e);
		}
	    });
	m_stats_checkbox.setBounds(10, 125, 90, 20);
	m_stats_checkbox.setSelected(false);
	m_stats_checkbox.setEnabled(false);
	m_content.add(m_stats_checkbox);

	///////////////////////////////////
	// Create the system stats dialog
	///////////////////////////////////
	Component ac = this;
	while (!(ac instanceof Frame)) {
	    ac = ac.getParent(); // Walk upwards until we hit a frame
	}
	Frame my_frame = (Frame)ac;
	m_sys_stats_dialog = new SystemStatDialog(my_frame, m_stats_checkbox);
	m_sys_stats_dialog.setLocation(150, 50);
	m_sys_stats_dialog.setVisible(false);	

	// Create the battery voltage labels
	m_voltage_label = new JLabel("Battery voltage :");
	m_voltage_label.setBounds(150, 173, 120, 20);
	m_content.add(m_voltage_label);

	m_voltage_value_label = new JLabel(NO_VOLTAGE_VALUE);
	m_voltage_value_label.setBounds(275, 173, 100, 20);
	m_content.add(m_voltage_value_label);

	// Create the battery voltage bar
	m_voltage_bar = new VoltageBar(this.getContentPane(),
				       MIN_BAR_VOLTAGE,
				       MAX_BAR_VOLTAGE,
				       130, 155, 210, 20);

	m_voltage_bar.set_levels(VOLTAGE_WARNING_LEVEL,
				 VOLTAGE_ALERT_LEVEL);	
    }

}
