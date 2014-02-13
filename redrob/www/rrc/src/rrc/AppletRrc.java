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

import javax.swing.BorderFactory;
import javax.swing.ButtonModel;
import javax.swing.ImageIcon;
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;

public class AppletRrc extends JApplet implements Runnable {

    private static final String PROD_NAME = "Redrob Remote Control";
    private static final String PROD_REV  = "R1A02";

    private static final String STEER_BUTT_FORWARD = "FRAMÅT";
    private static final String STEER_BUTT_REVERSE = "BAKÅT";
    private static final String STEER_BUTT_RIGHT   = "HÖGER";
    private static final String STEER_BUTT_LEFT    = "VÄNSTER";

    Map<String, Boolean> m_steer_map;

    private Redrob m_redrob;

    private enum THREAD_CMD {NONE,
			     CONNECT,
			     DISCONNECT,
			     STEER};

    private Thread m_main_thread;
    private boolean m_main_thread_running;
    private THREAD_CMD m_main_thread_cmd;
    private static final int MAIN_THREAD_PERIOD_TIME_MS = 200;

    private JPanel m_content;    

    private JButton m_forward_button;
    private JButton m_reverse_button;
    private JButton m_right_button;
    private JButton m_left_button;

    private JButton m_connect_button;
    private JButton m_disconnect_button;
    private static boolean m_is_connected;

    private JLabel m_connected_state_label;
    private ImageIcon m_connected_icon;
    private ImageIcon m_disconnected_icon;

    private VoltageBar m_voltage_bar;
    private JLabel m_voltage_label;
    private JLabel m_voltage_value_label;

    private DecimalFormat m_voltage_format;
    private long m_time_last_voltage;
    private boolean m_first_voltage_value;

    private static final String NO_VOLTAGE_VALUE = "N/A";
    private static final long VOLTAGE_UPDATE_TIME = 1000; // ms

    private static final int MIN_BAR_VOLTAGE = 6000; // mV
    private static final int MAX_BAR_VOLTAGE = 7500; // mV

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

	m_is_connected = false;

	m_voltage_format = new DecimalFormat("00.000");
	m_time_last_voltage = System.nanoTime();

	m_main_thread_running = true;
	m_main_thread_cmd = THREAD_CMD.NONE;

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

	// Create the thread
	m_main_thread = new Thread(this);
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
        //debug("stop... ");
	if (m_is_connected) {
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
	m_main_thread = null;
    }

    ////////////////////////////////////////////////////////

    @Override
    public void run()
    {
	//debug("run...thread started");

	int cnt = 0;

	while (m_main_thread_running) {	    
	    //debug("main thread=" + cnt++ + ", cmd=" + m_main_thread_cmd);

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
	    }	    

	    // Take it easy
	    try {
		Thread.sleep(MAIN_THREAD_PERIOD_TIME_MS);
	    }
	    catch (InterruptedException e) {
	    }
	}

	//debug("run...thread stopped");
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
	    m_redrob.connect();
	    m_is_connected = true; // Mark as connected

	    // Enable steer buttons
	    m_forward_button.setEnabled(true);
	    m_reverse_button.setEnabled(true);
	    m_right_button.setEnabled(true);
	    m_left_button.setEnabled(true);

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
	    m_redrob.disconnect();
	    m_is_connected = false; // Mark as NOT connected

	    // Disable steer buttons
	    m_forward_button.setEnabled(false);
	    m_reverse_button.setEnabled(false);
	    m_right_button.setEnabled(false);
	    m_left_button.setEnabled(false);

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
	String steer_button = "";

	// Find which steer button that is active
	for (String key : m_steer_map.keySet()) {
	    if (m_steer_map.get(key)) {
		//debug("do_command_steer=" + key);
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
	try {
	    // Send actual steer code
	    m_redrob.send_steer_code(code);

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

    private void update_voltage() throws IOException
    {
	// Check if time to update voltage
	if ( (System.nanoTime() - m_time_last_voltage) >
	     (VOLTAGE_UPDATE_TIME * 1000000) ) {
	    
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
	    m_steer_map.put(button_id, false); // Steer button not active
	    m_main_thread_cmd = THREAD_CMD.NONE; // Clear command
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

    private void init_gui() throws Exception
    {
	// Top level
	m_content = new JPanel();
	m_content.setLayout(null); // Use no layout
	m_content.setBorder(BorderFactory.createLineBorder(Color.black));	

	// Create the menu bar
        JMenuBar menu_bar = new JMenuBar();
        menu_bar.setOpaque(true);
        menu_bar.setBackground(new Color(154, 165, 127));
        menu_bar.setPreferredSize(new Dimension(500, 20));

	// Create the help menu.
	JMenu help_menu = new JMenu("Help");
	menu_bar.add(help_menu);
	JMenuItem help_menu_item_about = new JMenuItem("About");
	help_menu.add(help_menu_item_about);

	// Adding action listener to menu items
	help_menu_item_about.addActionListener(new ActionListener()
	    {
		@Override
		public void actionPerformed(ActionEvent e)
		{
		    info_msg_box("Name: "+ PROD_NAME + "\n" +
				 "Rev: " + PROD_REV + "\n" +
				 "Creator: Bonden i Nol\n" +
				 "e-mail: hakanbrolin@hotmail.com");
		}
	    });
	
	this.setJMenuBar(menu_bar);
	this.setContentPane(m_content);

	// Create steer buttons
	m_forward_button = new JButton();
	m_reverse_button = new JButton();
	m_right_button = new JButton();
	m_left_button = new JButton();
	create_steer_button(m_forward_button, STEER_BUTT_FORWARD, 180,  10);
	create_steer_button(m_reverse_button, STEER_BUTT_REVERSE, 180, 100);
	create_steer_button(m_right_button,   STEER_BUTT_RIGHT,   260,  55);
	create_steer_button(m_left_button,    STEER_BUTT_LEFT,    100,  55);

	// Create connect button
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

	// Create disconnect button
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

	// Create the connected state label (and icons)
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
