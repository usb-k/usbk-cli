#
# Build file of USBK CryptoBridge
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

# To see the command line CMake produces
SET( CMAKE_VERBOSE_MAKEFILE on )

# Uncomment this for production releases.
SET (VERSION_SUFFIX "prealpha")
SET (PACKAGE "${VERSION_SUFFIX}")
SET (VERSION "1.2.1")

ADD_DEFINITIONS ( -DPACKAGE="${PACKAGE}"
                  -DVERSION="${VERSION}" )

#SET (FLAGS "-O2 -Werror -Wcast-align -Wno-uninitialized -Wall -fstrict-aliasing -ggdb3 -funwind-tables -fasynchronous-unwind-tables")
#SET (CMAKE_C_FLAGS "${FLAGS}")

# Define version, config dir, data dir and log file.
ADD_DEFINITIONS ( -D'PID_FILE="/var/run/${APP_NAME}.pid"'
                  -D'DATA_DIR="/var/lib/${APP_NAME}"'
                  -D'LOG_DIR="/var/log/${APP_NAME}"' )

# Set standard libraries
SET (UDEV_LIBS udev)

# Set standard sources
SET (SOURCES src/main.cpp
             src/libusbk.cpp
             src/usbk_scsi.cpp
             src/usbk_sg_ctl.cpp )

# Include standard headers
INCLUDE_DIRECTORIES (src/)

# Compile usbk from specified sources
ADD_EXECUTABLE (usbk ${SOURCES})

# Link usbk to specified libraries
TARGET_LINK_LIBRARIES(usbk ${UDEV_LIBS})

# Install usbk to /usr/sbin
INSTALL (PROGRAMS build/usbk DESTINATION /usr/sbin)




SET (LibUSBK_SOURCES 	src/libusbk.cpp
             			src/usbk_scsi.cpp
             			src/usbk_sg_ctl.cpp )

# ADD_LIBRARY -> create static library
# ADD_LIBRARY with SHARED -> create shared library             		 
# ADD_LIBRARY( libUSBK SHARED ${LibUSBK_SOURCES} )
ADD_LIBRARY( USBK SHARED ${LibUSBK_SOURCES} )             		 


message(STATUS "NOTES:::::::::::::::::::::::::::::::::::")

message(STATUS "my sources: ${SOURCES}")

foreach(next_ITEM ${MY_SOURCES})
   message(STATUS "next item: ${next_ITEM}")
endforeach(next_ITEM ${MY_SOURCES})

if (UNIX)
   message(STATUS "This is UNIX (including OS X and CygWin)")
endif (UNIX)
 
if (MSVC)
   set(MY_SRCS ${MY_SRCS} winextra.cpp)
endif (MSVC)



SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME "usbk")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamara Elektronik - Murat Kılıvan <murat@usb-k.com>, Timuçin Anuşlu <timucin@usb-k.com>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge\n USBK CryptoBridge encrypts data independent by the operation systems and all filesystems.\n USBK works in SCSI level and encrypts/decrypts data on the fly. USBK has two model as A101\n and A103. A103 model uses 3 keys and A101 model uses 1 key. A password for this operation.\n Whenever user inserts USB-K into USB Hub of the PC, USB-K must be initialized (there is an\n exceptional situation!).")
SET(CPACK_PACKAGE_VERSION "${VERSION}")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${VERSION}")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
# CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA	 This variable allow advanced user to add custom script to the control.tar.gz (inside the .deb archive)	 ${CMAKE_CURRENT_SOURCE_DIR}/postinst
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.3.1-6), libgcc1 (>= 1:3.4.2-12)")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
# İncluding CPack
INCLUDE(CPack)