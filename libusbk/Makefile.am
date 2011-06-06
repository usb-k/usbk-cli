lib_LTLIBRARIES = libusbk.la

VERSION_CURRENT = 6
REVISION = 0
VERSION_AGE = 2

libusbk_la_LDFLAGS = -version-info "${VERSION_CURRENT}:${REVISION}:${VERSION_AGE}"
libusbk_la_SOURCES = 	src/libusbk.cpp \
						src/usbk_scsi.cpp \
						src/usbk_sg_ctl.cpp

include_HEADERS = src/libusbk.h
METASOURCES = AUTO
