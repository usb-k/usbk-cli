#
# Build file of USB-K CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

# Project name
PROJECT (usbk)

# CMake 2.6 required
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Application name
SET (APP_NAME "usbk")

# Uncomment this for production releases.
SET (VERSION_SUFFIX "prealpha")
SET (PACKAGE "${VERSION_SUFFIX}")
SET (VERSION "0.0.1")

ADD_DEFINITIONS ( -D'PACKAGE="${PACKAGE}"
                  -D'VERSION="${VERSION}" )

SET (FLAGS "-O2 -Werror -Wcast-align -Wno-uninitialized -Wall -fstrict-aliasing -ggdb3 -funwind-tables -fasynchronous-unwind-tables")
SET (CMAKE_C_FLAGS "${FLAGS}")

# Define version, config dir, data dir and log file.
ADD_DEFINITIONS ( -D'PID_FILE="/var/run/${APP_NAME}.pid"'
                  -D'DATA_DIR="/var/lib/${APP_NAME}"'
                  -D'LOG_DIR="/var/log/${APP_NAME}"' )

# Set standard sources
SET (SOURCES src/usbk.cpp
             src/lsusbk.cpp
             src/usbk_scsi.cpp
             src/usbk_sg_ctl.cpp )

# Set standard libraries
SET (LIBS usb)

# Include standard headers
INCLUDE_DIRECTORIES (src/)

# Compile usbk from specified sources
ADD_EXECUTABLE (usbk ${SOURCES})

# Link usbk to specified libraries
TARGET_LINK_LIBRARIES(usbk ${LIBS})

# Install usbk to /usr/sbin
INSTALL (PROGRAMS usbk
         DESTINATION /usr/sbin)

