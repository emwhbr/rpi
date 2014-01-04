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

DAEMON_DIR="/proj/redrob/"
DAEMON_EXE="redrobd_dbg.arm"

SIG_RESTART_DAEMON="SIGHUP"
SIG_TERMINATE_DAEMON="SIGTERM"

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
    ${DAEMON_DIR}/${DAEMON_EXE}

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
