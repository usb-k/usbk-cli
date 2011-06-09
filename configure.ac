dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(usbk, 1.1, [murat@usbk.com])


AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE()

AC_PROG_CXX
AC_PROG_LIBTOOL

#AC_CHECK_LIB([usb], [usb_init], [], [echo "usb library is missing. usb is required for this program" exit -1])
AC_CHECK_LIB([udev], [udev_new], [], [echo "udev library is missing. udev is required for this program" exit -1])

# Checks for header files.
#AC_CHECK_HEADERS([src/General.h usbk.h lsubk.h ScsiStructure.h usbk_scsi.h usbk_sg_ctl.h])
#AC_CONFIG_HEADERS([src/General.h usbk.h lsubk.h ScsiStructure.h usbk_scsi.h usbk_sg_ctl.h])

AC_CONFIG_FILES(Makefile usbk_cli/Makefile libusbk/Makefile)
AC_OUTPUT

