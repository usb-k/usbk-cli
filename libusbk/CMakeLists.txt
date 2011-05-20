#
# Build file of USBK CryptoBridge
#
# Tamara Electronics (C) 2010
#
# GNU General Public License
#


# Project name
PROJECT (usbk_lib C CXX)

# Display Starting
message(STATUS "## ${PROJECT_NAME} <<<< ")

# CMAKE Settings
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_VERBOSE_MAKEFILE OFF)

# The libUSBK version number.
SET(${PROJECT_NAME}_VERSION_MAJOR 0)
SET(${PROJECT_NAME}_VERSION_MINOR 0)
SET(${PROJECT_NAME}_VERSION_PATCH 1)
SET(${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}")
SET(${PROJECT_NAME}_VERSION_FULL "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")
SET(${PROJECT_NAME}_SOVERSION "${${PROJECT_NAME}_VERSION}.${${PROJECT_NAME}_VERSION_PATCH}")


# Search for fuse package
INCLUDE (FindUDEV.cmake)

IF(NOT UDEV)
    message(FATAL_ERROR "Unable to find 'udev' library!")
ELSE(NOT UDEV)

    # Source Files
    AUX_SOURCE_DIRECTORY (src ${PROJECT_NAME}_SOURCES)
    
    # Include Headers are loacted in these directory 
    INCLUDE_DIRECTORIES (
        "/usr/local/include/"
        "/usr/include"
    )
            
    # Libraries are located in these directories
    LINK_DIRECTORIES (
        "/usr/lib/"
    )
    
    # Create Library File
    ADD_LIBRARY( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES})
    
    # These are the symbols for the linker
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} udev)    
    
    
    # create symbolic lib target for calling target lib_XXX
    #ADD_CUSTOM_TARGET( lib DEPENDS ${PROJECT_NAME} )
    
    # change lib_target properties
    SET_TARGET_PROPERTIES( ${PROJECT_NAME} PROPERTIES
        # create *nix style library versions + symbolic links
        VERSION ${${PROJECT_NAME}_VERSION}
        SOVERSION ${${PROJECT_NAME}_SOVERSION}
    
        # allow creating static and shared libs without conflicts
        CLEAN_DIRECT_OUTPUT 1
    
        # avoid conflicts between library and binary target names
        OUTPUT_NAME usbk 
    )
    
    # Install this library
    INSTALL(TARGETS ${PROJECT_NAME} 
            LIBRARY DESTINATION lib
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
            
    INSTALL(FILES src/libusbk.h 
            DESTINATION include
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
    
ENDIF(NOT UDEV)

# Display Finishing
message(STATUS "## >>>> ${PROJECT_NAME}")
