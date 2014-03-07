#!/bin/bash
# /************************************************************************
#  *                                                                      *
#  * Copyright (C) 2014 Bonden i Nol (hakanbrolin@hotmail.com)            *
#  *                                                                      *
#  * This program is free software; you can redistribute it and/or modify *
#  * it under the terms of the GNU General Public License as published by *
#  * the Free Software Foundation; either version 2 of the License, or    *
#  * (at your option) any later version.                                  *
#  *                                                                      *
#  ************************************************************************/

#raspivid -fps 25 -w 500 -h 240 -t 0 -n -o - | cvlc -vvv stream:///dev/stdin --sout '#rtp{dst=0.0.0.0,port=52000,sdp=rtsp://:8554/}' :demux=h264


# RASPIVID
RASPIVID_DIR="/usr/bin"
RASPIVID_EXE="raspivid"

RASPIVID_ARG="-fps 30 -w 500 -h 240 -t 0 -n -o -"

# VLC
CVLC_DIR="/usr/bin"
CVLC_EXE="cvlc"
VLC_EXE="vlc"

CVLC_ARG="-q \
          stream:///dev/stdin \
          --h264-fps 30.0 \
          --sout-rtp-port 52000 \
          --sout-rtp-proto udp \
          --sout-rtp-caching 0 \
          --sout '#rtp{dst=0.0.0.0,sdp=rtsp://:8554/}' \
          :demux=h264"

################################################################
function pid_of_exe()
################################################################
{
    PID=`ps -e | grep ${1} | awk '{print $1}'`
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
    #
    # Capture video with the camera module (raspivid) and
    # stream it over the network (vlc).
    #
    # NOTE! VLC is not supposed to be run as root, run as
    #       user pi instead.
    #
    cmd="${RASPIVID_DIR}/${RASPIVID_EXE} ${RASPIVID_ARG} | \
         ${CVLC_DIR}/${CVLC_EXE} ${CVLC_ARG} &"
    #echo $cmd
    eval 'su pi -c "$cmd"'

    exit 0
}

################################################################
function do_shutdown()
################################################################
{
    # Get PID of RASPIVID
    PID_RASPIVID=`pid_of_exe ${RASPIVID_EXE}`
    if [ $? -ne 0 ]; then
	exit 1
    fi
    #echo "PID_RASPIVID: ${PID_RASPIVID}"

    # Get PID of VLC
    PID_VLC=`pid_of_exe ${VLC_EXE}`
    if [ $? -ne 0 ]; then
	exit 1
    fi
    #echo "PID_VLC: ${PID_VLC}"

    # Shutdown RASPIVID and VLC
    kill -9 ${PID_RASPIVID}
    kill -9 ${PID_VLC}    

    exit 0
}

################################################################
#                 MAIN SCRIPT STARTS HERE ...
################################################################

case "$1" in
    start)
	do_start
        ;;

    shutdown)
	do_shutdown
        ;;

    *)
        echo "Usage $0 {start|shutdown}"
        exit 1
        ;;
esac
