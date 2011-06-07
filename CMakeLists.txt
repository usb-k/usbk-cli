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

SET(${PROJECT_NAME}_VERSION_MAJOR 1)
SET(${PROJECT_NAME}_VERSION_MINOR 2)
SET(${PROJECT_NAME}_VERSION_PATCH 6)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")


ADD_SUBDIRECTORY( libusbk )
ADD_SUBDIRECTORY( usbk_cli )

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Settings for debian package
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamara Elektronik <tamara@tamara.com.tr>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge Linux CLI Program")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge Linux CLI Program\n USBK Linux CLI program allows user to manage USBK(s) plugged to the host system. The USBK is a device which protects user's critical data on USB Flash Disk and USB External Hard Drive by AES (Advanced Encryption Standard). The two models of USBK are available as A101 and A103. A101 has one encryption key and A103 has also 3 encryption keys.")
SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION_FULL}")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION_FULL}")
#SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "udev (>= 151-12.3)")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
INCLUDE(CPack)
