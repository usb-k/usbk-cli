#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

PROJECT (usbk)


message(STATUS "## ${PROJECT_NAME} <<<< ")


CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory( LibUSBK )
add_subdirectory( USBK_CLI )

message(STATUS "## >>>> ${PROJECT_NAME}")
