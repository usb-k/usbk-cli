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
    char usb_serial_no[64];
    t_UIP_DEVINFO info;
    t_UIP_GETSTATUS status;
    struct __USBK *next;
} USBK_T;

typedef struct __USBK_List {
    char vendor_id [1024];
    char product_id [1024];
    char manufacturer [1024];
    char product [1024];
    char serial [1024];
    char dev_name [1024];
    char backdisk_name [1024];

    t_UIP_DEVINFO info;
    t_UIP_GETSTATUS status;

    struct __USBK_List *next;
} USBK_List;


int usbk_list_devices(USBK_List** pusbk);
void usbk_list_devices_release(USBK_List** pusbk);
//TODO: int usbk_create_list_devices(USBK_T *usbk);
//TODO: int usbk_realese_list_devices(USBK_T *usbk);
int usbk_get_device_info(USBK_T *usbk);
int usbk_get_backdisk(USBK_T *usbk);
int usbk_get_scsi_dev_info(USBK_T *usbk);


#endif

