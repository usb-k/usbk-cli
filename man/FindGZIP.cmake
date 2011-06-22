# - Find gzip
# This module finds if gzip is installed and determines where the
# executable is. This code sets the following variable:
#
#  GZIP_TOOL:       path to the gzip

FIND_PROGRAM(GZIP_TOOL
  NAMES gzip
  PATHS /bin
        /usr/bin
        /usr/local/bin
)


IF(NOT GZIP_TOOL)
  MESSAGE(FATAL_ERROR "Unable to find 'gzip' program")
ENDIF(NOT GZIP_TOOL)
