

# Project name
PROJECT (usbk_man)

# Search for gzip program
INCLUDE (FindGZIP)    

# Lists of source manuals
SET(usbk_man_src
    usbk.1)

# Lists of compressed manuals
STRING(REGEX REPLACE ".1" ".1.gz" usbk_man_gz "${usbk_man_src}")

# Compression of the manuals
FOREACH(man ${usbk_man_src})
  ADD_CUSTOM_COMMAND(OUTPUT ${man}.gz
                     COMMAND ${GZIP_TOOL} -c ${PROJECT_SOURCE_DIR}/${man} > ${PROJECT_BINARY_DIR}/${man}.gz
                     DEPENDS ${man}
                     COMMENT "Building ${man}.gz")
ENDFOREACH(man)

# Add the manual generation in the global rules
ADD_CUSTOM_TARGET(man ALL
                  DEPENDS ${PROJECT_BINARY_DIR}/${usbk_man_gz})

# Installation of the manuals
INSTALL(FILES ${PROJECT_BINARY_DIR}/${usbk_man_gz}
        DESTINATION "/usr/share/man/man1"
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
