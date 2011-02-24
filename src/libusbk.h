/*
 * @file libusbk.h
 *
 * Copyright (C) 2010 USB-K Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * See http://www.gnu.org/licenses/ for more information
 *
 ****************************************************************************
 *
 */

#ifndef LIBUSBK_H_
#define LIBUSBK_H_

#include "general.h"

#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT         "a103"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"

typedef struct __USBK {
    char dev[1024];
    char backdisk_dev[1024];
    t_UIP_DEVINFO info;
    t_UIP_GETSTATUS status;
} USBK_T;

int usbk_list_devices(void);
int usbk_get_device_info(USBK_T *usbk);
int usbk_get_backdisk(char *usbk_backdisk, char *usbk_serial, char *backdisk_vendor);
int usbk_get_scsi_dev_info(USBK_T *usbk);


#endif

