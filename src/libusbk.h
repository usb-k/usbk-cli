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

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

#include "general.h"

#define rtnLIBUSBK_PASS                      0
#define rtnLIBUSBK_GENERAL_ERROR            -1
#define rtnLIBUSBK_UDEV_NOT_CREATE          -2
#define rtnLIBUSBK_UDEV_NOT_NODE            -3
#define rtnLIBUSBK_UDEV_WRONG_FILE_TYPE     -4
#define rtnLIBUSBK_UDEV_USBKLIST_NOT_CREATE -5
#define rtnLIBUSBK_SCSI_COMMAND_ERROR       -6
#define rtnLIBUSBK_GET_BACKDISK_ERROR       -7


#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT         "a103"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"

typedef struct __USBK {
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
} USBK;

typedef struct __USBK_List {
    struct __USBK_List  *next;
    USBK                usbk;
} USBK_List;


typedef struct __USBK_INFO {
    char *dev_path;
    char *backdisk_path;
    char *backdisk;

    char    *product;
    char    *model;
    char    *serial;
    char    *usb_serial;
    char    *firmware_ver;
    int     multikey_cap;
    char    *dev_label;
    e_UIP_DEVSTATE     dev_state;
    int     current_key;
    int     autoact_keyno;
    int     retry_num;
    char    **key_names;
} USBK_INFO;


typedef struct __USBK_List2 {
    struct __USBK_List2  *next;
    char                *dev;
    USBK_INFO           usbk_info;
} USBK_List2;



USBK_List2* LibUSBK__list_devices(void);
void LibUSBK__list_devices_release(USBK_List2* p_usbklink);

int LibUSBK__GetDeviceInfo(const char *usbk_path, USBK_INFO** usbk_infos);
int LibUSBK__GetDeviceInfo_Release(USBK_INFO* usbk_infos);

int LibUSBK__ActivateKey (const char *usbk_path, const char *password, const int key_no);
int LibUSBK__DeActivateKey (const char *usbk_path);
int LibUSBK__ChangePassword (const char *usbk_path, const char *old_pass, const char *new_pass);
int LibUSBK__SetKey (const char *usbk_path, const char *pass, int key_no, int name_only, const char* key_name, const char* key_size, const unsigned char* key);
int LibUSBK__SetAutoAct (const char *usbk_path, const char *pass, int enable, int key_no);
int LibUSBK__SetDeviceName (const char *usbk_path, const char *pass, const char *device_label);
int LibUSBK__GetRandomKey (const char *usbk_path, unsigned char *random_key);



#endif

