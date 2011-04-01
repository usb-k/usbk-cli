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

typedef struct __USBK_List {
    char *dev_path;
    char *dev;
    char *vendor_id;
    char *product_id;
    char *manufacturer;
    char *product;
    char *serial;
    char *backdisk_path;
    char *backdisk;

    t_UIP_DEVINFO info;
    t_UIP_GETSTATUS status;

    struct __USBK_List  *next;
} USBK_List;




USBK_List* LibUSBK__list_devices(void);
void LibUSBK__list_devices_release(USBK_List* p_usbk);
int LibUSBK__GetDeviceInfo(USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__ActivateKey (USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__DeActivateKey (USBK_List* usbk);
int LibUSBK__ChangePassword (USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__SetKey (USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__SetAutoAct (USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__SetDeviceName (USBK_List* usbk, unsigned char *buff, int len);
int LibUSBK__GetRandomKey (USBK_List* usbk, unsigned char *buff, int len);

#endif

