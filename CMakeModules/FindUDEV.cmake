#
# Set global variable UDEV to TRUE if both udev lib and include files are found
#


# Look for FUSE library
FIND_LIBRARY(UDEV_LIB
             NAMES udev)


IF(NOT UDEV_LIB)
  MESSAGE(STATUS "Unable to find 'udev' library")
  SET(UDEV FALSE)
ELSE(NOT UDEV_LIB)
  SET(UDEV TRUE)
ENDIF(NOT UDEV_LIB)


# Look for FUSE include files
FIND_PATH(UDEV_INC
          NAMES libudev.h)

IF (NOT UDEV_INC)
  MESSAGE(STATUS "Unable to find 'udev' include files")
  SET(UDEV FALSE)
ELSE(NOT UDEV_INC)
  SET(UDEV TRUE)
ENDIF(NOT UDEV_INC)

