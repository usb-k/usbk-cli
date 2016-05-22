#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#

IF(UNIX)
    message(STATUS "Cmake for Unix")
ENDIF(UNIX)

IF(WIN32)
    message(STATUS "Cmake for Windows")
ENDIF(WIN32)

IF(APPLE)
    message(FATAL_ERROR "APPLE NOT SUPPORTED")
ENDIF(APPLE)

# Project name
PROJECT (USBK_LIB_CPP_BINDING)

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# The libUSBK version number.
SET(${PROJECT_NAME}_VERSION_MAJOR 0)
SET(${PROJECT_NAME}_VERSION_MINOR 0)
SET(${PROJECT_NAME}_VERSION_PATCH 1)

SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")
SET(${PROJECT_NAME}_SOVERSION "${${PROJECT_NAME}_VERSION_FULL}")

CONFIGURE_FILE(src/config.hpp.cmake src/config.hpp)

IF(UNIX AND NOT APPLE)
    # Header files are located in these directories
    SET (${PROJECT_NAME}_INCLUDE_DIRS
        ${${PROJECT_NAME}_SOURCE_DIR}/../libusbk/src
        ${${PROJECT_NAME}_SOURCE_DIR}/src/
        ${CMAKE_SYSTEM_INCLUDE_PATH}
    )
    INCLUDE_DIRECTORIES (${${PROJECT_NAME}_INCLUDE_DIRS})

    # Libraries are located in these directories
    SET (${PROJECT_NAME}_LIBRARY_DIRS
        ${CMAKE_CURRENT_BINARY_DIR}/../libusbk/
        ${CMAKE_SYSTEM_LIBRARY_PATH}
    )
    LINK_DIRECTORIES ( ${${PROJECT_NAME}_LIBRARY_DIRS})

    # Depend Includes
    SET (${PROJECT_NAME}_INCLUDES
        libusbk.h
        libusbk.hpp
    )

    # Libraries are located in these directories
    SET (${PROJECT_NAME}_LIBRARIES_PATH
        ${CMAKE_SYSTEM_LIBRARY_PATH}
    )
    LINK_DIRECTORIES (${${PROJECT_NAME}_LIBRARIES_PATH})

    # Source Files
    #AUX_SOURCE_DIRECTORY (src ${PROJECT_NAME}_SOURCES)

    FILE(GLOB SRC_FILES "src/*" )

    # Create Library File
    ADD_LIBRARY( ${PROJECT_NAME} SHARED ${SRC_FILES})

    # These are the symbols for the linker
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} udev)    

    # change lib_target properties
    SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
        # create *nix style library versions + symbolic links
        VERSION ${${PROJECT_NAME}_VERSION_FULL}
        SOVERSION ${${PROJECT_NAME}_SOVERSION}

        # allow creating static and shared libs without conflicts
        CLEAN_DIRECT_OUTPUT 1

        # avoid conflicts between library and binary target names
        OUTPUT_NAME usbk++
    )

    # Install this library
    INSTALL(TARGETS ${PROJECT_NAME} 
            LIBRARY DESTINATION /usr/lib
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    INSTALL(FILES src/libusbk.hpp
            DESTINATION /usr/include
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

    IF (${CMAKE_PROJECT_NAME} MATCHES ${PROJECT_NAME})

        # Settings for debian package
        SET(CPACK_GENERATOR "DEB")
        SET(CPACK_DEBIAN_PACKAGE_NAME ${PROJECT_NAME})
        SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamara Elektronik <tamara@tamara.com.tr>")
        SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "USBK CryptoBridge Library")
        SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION " USBK CryptoBridge Library\n USBK CryptoBridge encrypts data independent by the operation systems and all filesystems.\n USBK works in SCSI level and encrypts/decrypts data on the fly. USBK has two model as A101\n and A103. A103 model uses 3 keys and A101 model uses 1 key. A password for this operation.\n Whenever user inserts USB-K into USB Hub of the PC, USB-K must be initialized (there is an\n exceptional situation!).")
        SET(CPACK_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
        SET(CPACK_DEBIAN_PACKAGE_VERSION "${${PROJECT_NAME}_VERSION}")
        #SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
        SET(CPACK_DEBIAN_PACKAGE_DEPENDS "udev (>= 151-12.3)")
        SET(CPACK_DEBIAN_PACKAGE_SECTION "devel")
        SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "recommends")	
        SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "suggests")
        INCLUDE(CPack)

    ENDIF (${CMAKE_PROJECT_NAME} MATCHES ${PROJECT_NAME})

ELSEIF(WIN32)

        # Header files are located in these directories
    SET (${PROJECT_NAME}_INCLUDE_DIRS
        ${${PROJECT_NAME}_SOURCE_DIR}/src/
        ${${PROJECT_NAME}_SOURCE_DIR}/src/windows/
        ${CMAKE_SYSTEM_INCLUDE_PATH}
    )
    INCLUDE_DIRECTORIES (${${PROJECT_NAME}_INCLUDE_DIRS})

    # Libraries are located in these directories
    SET (${PROJECT_NAME}_LIBRARIES_PATH
        ${CMAKE_SYSTEM_LIBRARY_PATH}
    )
    LINK_DIRECTORIES (${${PROJECT_NAME}_LIBRARIES_PATH})

    FILE(GLOB SRC_FILES "src/*" "src/windows/*")

    ADD_LIBRARY( ${PROJECT_NAME} SHARED ${SRC_FILES})

    SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
        VERSION ${${PROJECT_NAME}_VERSION}
        SOVERSION ${${PROJECT_NAME}_SOVERSION}

        # allow creating static and shared libs without conflicts
        CLEAN_DIRECT_OUTPUT 1

        # avoid conflicts between library and binary target names
        OUTPUT_NAME libusbk++
    )

    INSTALL(TARGETS
        ${PROJECT_NAME}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )

    INCLUDE(CPack)

ELSEIF(APPLE)

ELSE(UNIX AND NOT APPLE)
    message(FATAL_ERROR "Undefined Operating System") 
ENDIF(UNIX AND NOT APPLE)

