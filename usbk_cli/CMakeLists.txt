#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

# Project name
PROJECT (usbk_cli C CXX)

# Display Starting
message(STATUS "## ${PROJECT_NAME} <<<< ")

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# The USBK_CLI version number.
SET(${PROJECT_NAME}_VERSION_MAJOR 1)
SET(${PROJECT_NAME}_VERSION_MINOR 2)
SET(${PROJECT_NAME}_VERSION_PATCH 1)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")
ADD_DEFINITIONS ( -DVERSION="${${PROJECT_NAME}_VERSION}" )
ADD_DEFINITIONS ( -DPACKAGE="${PROJECT_NAME}")


# Source Files
AUX_SOURCE_DIRECTORY (${${PROJECT_NAME}_SOURCE_DIR}/src ${PROJECT_NAME}_SOURCES)

# Header files are located in these directories
SET (${PROJECT_NAME}_INCLUDE_DIRS
    "/usr/local/include/"
    "/usr/include"
    "${${PROJECT_NAME}_SOURCE_DIR}/../libusbk/src"
)
INCLUDE_DIRECTORIES (${${PROJECT_NAME}_INCLUDE_DIRS})


# Libraries are located in these directories
SET (${PROJECT_NAME}_LIBRARY_DIRS
    "/usr/local/lib/"
    "/usr/lib/"
    "/usr/X11R6/lib/"
    "${${PROJECT_NAME}_SOURCE_DIR}/../build/libusbk/"
    "${${PROJECT_NAME}_SOURCE_DIR}/build"
    "${${PROJECT_NAME}_SOURCE_DIR}/../build"
)
LINK_DIRECTORIES ( ${${PROJECT_NAME}_LIBRARY_DIRS})

# Depend Includes
SET (${PROJECT_NAME}_INCLUDES
    libusbk.h
)

# Depend Libraries
SET (${PROJECT_NAME}_LIBRARIES
    usbk
)

FOREACH (INCLUDE ${${PROJECT_NAME}_INCLUDES})
    FIND_PATH(FOUND_INCLUDE ${INCLUDE} PATHS ${${PROJECT_NAME}_INCLUDE_DIRS})
    IF(NOT FOUND_INCLUDE)
        message(FATAL_ERROR "${INCLUDE} is not Found!")
    ELSE(NOT FOUND_INCLUDE)    
#        MESSAGE("Include: ${INCLUDE}")
#        MESSAGE("Found Path: ${FOUND_INCLUDE}")
    ENDIF(NOT FOUND_INCLUDE)
ENDFOREACH(INCLUDE)

# Create project
ADD_EXECUTABLE (${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

# These are the symbols for the linker
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${${PROJECT_NAME}_LIBRARIES})

SET_TARGET_PROPERTIES(${PROJECT_NAME}
                      PROPERTIES OUTPUT_NAME usbk)

# Install project
INSTALL(TARGETS ${PROJECT_NAME} 
        RUNTIME 
        DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

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