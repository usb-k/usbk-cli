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
SET(${PROJECT_NAME}_VERSION_PATCH 3)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")


ADD_SUBDIRECTORY( usbk_cli )
ADD_SUBDIRECTORY( libusbk )

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
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge Library")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge Library\n USBK CryptoBridge encrypts data independent by the operation systems and all filesystems.\n USBK works in SCSI level and encrypts/decrypts data on the fly. USBK has two model as A101\n and A103. A103 model uses 3 keys and A101 model uses 1 key. A password for this operation.\n Whenever user inserts USB-K into USB Hub of the PC, USB-K must be initialized (there is an\n exceptional situation!).")
SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION_FULL}")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION_FULL}")
#SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "udev (>= 151-12.3)")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
INCLUDE(CPack)
