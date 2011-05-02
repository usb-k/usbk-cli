#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

# Project name
PROJECT (usbk)

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

ADD_SUBDIRECTORY( libusbk )
ADD_SUBDIRECTORY( usbk_cli )

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
