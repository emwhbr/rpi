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

import javax.swing.JProgressBar;
import javax.swing.SwingConstants;

import java.awt.Container;
import java.awt.Color;

public class VoltageBar {

    private JProgressBar m_bar = new JProgressBar();

    private int m_warning_level; // mV
    private int m_alert_level;   // mV


    ////////////////////////////////////////////////////////

    public VoltageBar(Container container,
		      int min_value,
		      int max_value,
		      int x,
		      int y,
		      int width,
		      int height)
    {
	initialize(container,
		   min_value,
		   max_value,
		   x,
		   y,
		   width,
		   height);
    }

    ////////////////////////////////////////////////////////

    public void set_levels(int warning_level, int alert_level)
    {
	m_warning_level = warning_level;
	m_alert_level = alert_level;
    }

    ////////////////////////////////////////////////////////

    public void activate()
    {
	m_bar.setIndeterminate(false);
    }

    ////////////////////////////////////////////////////////

    public void deactivate()
    {
	m_bar.setIndeterminate(true);
	m_bar.setForeground(Color.RED); // Progressbar
    }

    ////////////////////////////////////////////////////////

    public void update_voltage(int voltage_mv)
    {
	m_bar.setValue(voltage_mv);

	// Change colour of progressbar
	if (voltage_mv > m_warning_level) {
	    m_bar.setForeground(Color.GREEN);
	}
	else if (voltage_mv < m_alert_level) {
	    m_bar.setForeground(Color.RED);
	}
	else {
	    m_bar.setForeground(Color.ORANGE);
	}
    }

    ////////////////////////////////////////////////////////

    private void initialize(Container container,
			    int min_value,
			    int max_value,
			    int x,
			    int y,
			    int width,
			    int height)
    {
	m_bar = new JProgressBar(SwingConstants.HORIZONTAL,
				 min_value,
				 max_value);

	m_bar.setIndeterminate(true); 
	m_bar.setStringPainted(false);
	m_bar.setBounds(x, y, width, height);

	this.deactivate();
	this.set_levels(min_value, min_value);

	container.add(m_bar);
    }
}
