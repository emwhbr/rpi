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
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

// Implementation notes
// http://csis.pace.edu/~bergin/sol/java/gui/pt2/

public class SystemStatDialog extends JDialog {

    private JTextField m_cpu_load = new JTextField(6); // %
    private JTextField m_mem_used = new JTextField(6); // KBytes
    private JTextField m_irq = new JTextField(6);      // irq/s
    private JTextField m_uptime = new JTextField(6);   // hh:mm:ss

    private AbstractButton m_button;    

    ////////////////////////////////////////////////////////

    public void set_cpu_load(int value)
    {
	update_int_textfield(m_cpu_load, value);
    }

    ////////////////////////////////////////////////////////

    public void set_mem_used(int value)
    {
	update_int_textfield(m_mem_used, value);
    }

    ////////////////////////////////////////////////////////

    public void set_irq(int value)
    {
	update_int_textfield(m_irq, value);
    }

    ////////////////////////////////////////////////////////

    public void set_uptime(int value)
    {	
	m_uptime.setText(null);
	m_uptime.setText(to_hhmmss(value));
    }

    ////////////////////////////////////////////////////////

    public void reset_stats()
    {
	set_cpu_load(0);
	set_mem_used(0);
	set_irq(0);
	set_uptime(0);
    }

    ////////////////////////////////////////////////////////

    public SystemStatDialog(Frame frame,
			    AbstractButton button)
    {
	super(frame, "System Stats", false);
	Container contentPane = getContentPane();
	setSize(300, 150);
	setResizable(false);

	m_button = button;

	// Layout the fields and labels	
	JPanel dialogPanel = new JPanel();
	contentPane.add(dialogPanel, BorderLayout.CENTER);
	dialogPanel.setLayout(new GridLayout(4, 2, 0, 3));

	dialogPanel.add(new JLabel(" CPU load [%]"));
	dialogPanel.add(m_cpu_load);
	m_cpu_load.setEditable(false);	

	dialogPanel.add(new JLabel(" Memory usage [KB]"));
	dialogPanel.add(m_mem_used);
	m_mem_used.setEditable(false);       

	dialogPanel.add(new JLabel(" Interrupts [irq/s]"));
	dialogPanel.add(m_irq);	
	m_irq.setEditable(false);

	dialogPanel.add(new JLabel(" Uptime [hh:mm:ss]"));
	dialogPanel.add(m_uptime);
	m_uptime.setEditable(false);

	// Reset all fields
	reset_stats();
	
	// Layout the buttons
	JPanel button_panel = new JPanel(); 
	contentPane.add(button_panel, BorderLayout.SOUTH);

	JButton cancel_button = new JButton("Cancel");
	cancel_button.addActionListener(new cancel_listener());
	button_panel.add(cancel_button);		
    }

    ////////////////////////////////////////////////////////

    private class cancel_listener implements ActionListener {

	public void actionPerformed(ActionEvent e)
	{	
	    SystemStatDialog.this.setVisible(false);
	    m_button.setSelected(false);
	}
    }

    ////////////////////////////////////////////////////////

    private void update_int_textfield(JTextField txt,
				      int value)
    {
	txt.setText(null);
	if (value > 0) {
	    txt.setText(Integer.toString(value));
	}
	else {
	    txt.setText("N/A");
	}
    }

    ////////////////////////////////////////////////////////

    private static String to_hhmmss(int sec) 
    {
	if (sec > 0) {
	    int hours = sec / 3600;
	    int remainder = sec % 3600;
	    int minutes = remainder / 60;
	    int seconds = remainder % 60;
	    
	    return ( (hours < 10 ? "0" : "") + hours
		     + ":" + (minutes < 10 ? "0" : "") + minutes
		     + ":" + (seconds< 10 ? "0" : "") + seconds );
	}
	else {
	    return "N/A";
	}
    }

}
