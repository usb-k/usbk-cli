#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

# Project name
PROJECT (libUSBK)

# Display Starting
message(STATUS "## ${PROJECT_NAME} <<<< ")

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# The libUSBK version number.
SET(${PROJECT_NAME}_VERSION_MAJOR 2)
SET(${PROJECT_NAME}_VERSION_MINOR 4)
SET(${PROJECT_NAME}_VERSION_PATCH 3)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")

# Source Files
AUX_SOURCE_DIRECTORY (src ${PROJECT_NAME}_SOURCES)

# Include Headers are loacted in these directory 
INCLUDE_DIRECTORIES (
    "/usr/local/include/"
    "/usr/include"
)
        
# Libraries are located in these directories
LINK_DIRECTORIES (
    "/usr/local/lib/"
)

# Create Library File
ADD_LIBRARY( USBK SHARED ${${PROJECT_NAME}_SOURCES} )        

# These are the symbols for the linker
TARGET_LINK_LIBRARIES(USBK udev)    

# Install this library
INSTALL(TARGETS USBK LIBRARY DESTINATION lib)
INSTALL(FILES src/libusbk.h DESTINATION include)

# Settings for debian package
SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamara Elektronik <tamara@tamara.com.tr>")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge Library")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge Library\n USBK CryptoBridge encrypts data independent by the operation systems and all filesystems.\n USBK works in SCSI level and encrypts/decrypts data on the fly. USBK has two model as A101\n and A103. A103 model uses 3 keys and A101 model uses 1 key. A password for this operation.\n Whenever user inserts USB-K into USB Hub of the PC, USB-K must be initialized (there is an\n exceptional situation!).")
SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "udev (>= 151-12.3)")
SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
INCLUDE(CPack)

# Display Finishing
message(STATUS "## >>>> ${PROJECT_NAME}")