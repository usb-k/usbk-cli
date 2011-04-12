#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

# Project name
PROJECT (USBK_CLI)

# Display Starting
message(STATUS "## ${PROJECT_NAME} <<<< ")

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# The USBK_CLI version number.
SET(${PROJECT_NAME}_VERSION_MAJOR 2)
SET(${PROJECT_NAME}_VERSION_MINOR 4)
SET(${PROJECT_NAME}_VERSION_PATCH 3)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")
ADD_DEFINITIONS ( -DVERSION="${${PROJECT_NAME}_VERSION}" )
ADD_DEFINITIONS ( -DPACKAGE="${PROJECT_NAME}")

# Source Files
AUX_SOURCE_DIRECTORY (src ${PROJECT_NAME}_SOURCES)

# Header files are located in these directories
INCLUDE_DIRECTORIES (
    "/usr/local/include/"
    "/usr/include"
    "${usbk_SOURCE_DIR}/LibUSBK/src"
)



# Libraries are located in these directories
LINK_DIRECTORIES (
    "/usr/local/lib/"
    "/usr/X11R6/lib/"
    "${usbk_SOURCE_DIR}/build/LibUSBK/"
)


# Look for the header file
FIND_PATH( LIBUSBK_INCLUDE_DIR NAMES libusbk.h)
MARK_AS_ADVANCED( LIBUSBK_INCLUDE_DIR )

IF(NOT LIBUSBK_INCLUDE_DIR)
    message(FATAL_ERROR "libusbk.h is not found!!!")
ENDIF(NOT LIBUSBK_INCLUDE_DIR)

# Look for the library
FIND_LIBRARY( LIBUSBK_LIBRARY NAMES USBK )
MARK_AS_ADVANCED( LIBUSBK_LIBRARY )

IF(NOT LIBUSBK_LIBRARY)
    message(FATAL_ERROR "libusbk.co is not found!!!")
ENDIF(NOT LIBUSBK_LIBRARY)


# Create project
ADD_EXECUTABLE (${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

# These are the symbols for the linker
TARGET_LINK_LIBRARIES(${PROJECT_NAME} USBK)

# Install project
INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Settings for Debian Package
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamara Elektronik")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge CLI")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge CLI\n USBK CryptoBridge encrypts data independent by the operation systems and all filesystems.\n USBK works in SCSI level and encrypts/decrypts data on the fly. USBK has two model as A101\n and A103. A103 model uses 3 keys and A101 model uses 1 key. A password for this operation.\n Whenever user inserts USB-K into USB Hub of the PC, USB-K must be initialized (there is an\n exceptional situation!).")
SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libusbk (>= 1.2.1)")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
INCLUDE(CPack)

# Display finishing
message(STATUS "## >>>> ${PROJECT_NAME}")