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

export JAVA_HOME=/opt/java/jdk1.7.0_51
export PATH=${JAVA_HOME}/bin:${PATH}

JNA_HOME=/opt/java/jna
JNA_JAR="jna-4.1.0.jar"

CLASSPATH="${JAVA_HOME}/jre/lib/plugin.jar:${JNA_HOME}/${JNA_JAR}"

CLASSES_DIR="./classes"
DIST_DIR="./dist"
HTML_DIR="./html"
SRC_DIR="./src"
RES_DIR="./res"
SIM_DIR="./sim"
KEY_DIR="./keystore"
LIBJOY_DIR="./libjoy"

case "$1" in
    applet)
        echo "==[APPLET]==="
	javac -cp ${CLASSES_DIR} \
	      -cp ${CLASSPATH} \
              -d ${CLASSES_DIR} \
	      -sourcepath ${SRC_DIR}/rrc \
	      -sourcepath ${SRC_DIR}/joystick \
              ${SRC_DIR}/rrc/*.java \
	      ${SRC_DIR}/joystick/*.java
        ;;

    sim)
	echo "==[SIMULATOR]==="
	javac -cp ${SIM_DIR}/classes \
              -d ${SIM_DIR}/classes \
	      -sourcepath ${SIM_DIR}/src \
              ${SIM_DIR}/src/*.java
	;;

    dist)
	echo "==[DIST-SIGNED JAR]==="	
	cp -r ${CLASSES_DIR}/rrc      ${DIST_DIR}
	cp -r ${CLASSES_DIR}/joystick ${DIST_DIR}
	cp -r ${RES_DIR}              ${DIST_DIR}
	jar cvf0e /tmp/rrc.jar AppletRrc -C ${DIST_DIR} .

	rm -rf ${DIST_DIR}/rrc
	rm -rf ${DIST_DIR}/joystick
	rm -rf ${DIST_DIR}/res

	mv /tmp/rrc.jar           ${DIST_DIR}
	cp ${JNA_HOME}/${JNA_JAR} ${DIST_DIR}

	jarsigner -keystore ${KEY_DIR}/rrc_keystore \
                  -storepass rrcpasswd \
                  ${DIST_DIR}/rrc.jar rrc_alias

	jarsigner -keystore ${KEY_DIR}/rrc_keystore \
                  -storepass rrcpasswd \
                  ${DIST_DIR}/${JNA_JAR} rrc_alias

	cp ${HTML_DIR}/index_jar.html ${DIST_DIR}/index.html

	cp -r ${LIBJOY_DIR}/linux-lib64 ${DIST_DIR}

	tar czvf /tmp/server-root.tar.gz -C ${DIST_DIR} . --exclude=./README
	mv /tmp/server-root.tar.gz ${DIST_DIR}
	;;

    clean)
        echo "==[CLEAN]==="
	rm -rf ./*~      	
	rm -rf ${CLASSES_DIR}/rrc
	rm -rf ${CLASSES_DIR}/joystick
	rm -rf ${DIST_DIR}/*.jar
	rm -rf ${DIST_DIR}/*.html
	rm -rf ${DIST_DIR}/linux-lib64
	rm -rf ${DIST_DIR}/server-root.tar.gz
	rm -rf ${HTML_DIR}/*~
	rm -rf ${SRC_DIR}/*~
	rm -rf ${SRC_DIR}/rrc/*~
	rm -rf ${SRC_DIR}/joystick/*~
	rm -rf ${RES_DIR}/*~
	rm -rf ${SIM_DIR}/*~
	rm -rf ${SIM_DIR}/src/*~
	rm -rf ${SIM_DIR}/classes/*.class
        ;;

    *)
        echo "Usage $0 {applet|sim|dist|clean}"
        exit 1
        ;;
esac
