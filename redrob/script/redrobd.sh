#!/bin/bash
# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2013 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

DAEMON_DIR="/proj/redrob"
DAEMON_EXE="redrobd_dbg.arm"

SIG_RESTART_DAEMON="SIGHUP"
SIG_TERMINATE_DAEMON="SIGTERM"

GPIO_PIN_START_DISABLED="4"

################################################################
function is_start_disabled()
################################################################
{
    # Set pin as input
    echo "${GPIO_PIN_START_DISABLED}" > /sys/class/gpio/export
    echo "in"  > /sys/class/gpio/gpio${GPIO_PIN_START_DISABLED}/direction
    
    # Read pin
    pin_value=`cat /sys/class/gpio/gpio${GPIO_PIN_START_DISABLED}/value`

    # Restore pin
    echo "${GPIO_PIN_START_DISABLED}" > /sys/class/gpio/unexport

    echo ${pin_value}
    return 0
}

################################################################
function pid_of_daemon()
################################################################
{
    PID=`ps -e | grep ${DAEMON_EXE} | awk '{print $1}'`
    if [ -z "$PID" ]; then
	echo ""
	return 1
    fi

    echo $PID
    return 0
}

################################################################
function do_start()
################################################################
{
    # Check if start is disabled by GPIO pin
    start_disabled=`is_start_disabled`

    # Start daemon
    if [ $start_disabled -eq 1 ]; then
	echo "Start Redrob daemon - disabled by GPIO pin ${GPIO_PIN_START_DISABLED}"
    else
	echo "Start Redrob daemon"
	${DAEMON_DIR}/${DAEMON_EXE}
    fi;

    exit 0
}

################################################################
function do_restart()
################################################################
{
    PID=`pid_of_daemon`
    if [ $? -ne 0 ]; then
	exit 1
    fi

    echo "Restart Redrob daemon"
    kill -${SIG_RESTART_DAEMON} ${PID}

    exit 0
}

################################################################
function do_shutdown()
################################################################
{
    PID=`pid_of_daemon`
    if [ $? -ne 0 ]; then
	exit 1
    fi

    echo "Shutdown Redrob daemon"
    kill -${SIG_TERMINATE_DAEMON} ${PID}

    exit 0
}

################################################################
#                 MAIN SCRIPT STARTS HERE ...
################################################################

case "$1" in
    start)
	do_start
        ;;

    restart)
	do_restart
        ;;

    shutdown)
	do_shutdown
        ;;

    *)
        echo "Usage $0 {start|restart|shutdown}"
        exit 1
        ;;
esac
