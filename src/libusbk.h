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

#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT         "a103"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"

typedef enum __LIBUSBK_OPRSTATUS
{
    LIBUSBK_OPRS_PASS           = 1,
    LIBUSBK_OPRS_GEN_FAIL       = 2,
    LIBUSBK_OPRS_INVALID_PASS   = 3,
    LIBUSBK_OPRS_FABRIC_RESET   = 4,
    LIBUSBK_OPRS_USBK_UNPLUGING = 5,

    LIBUSBK_RTN_PASS                     =  0,
    LIBUSBK_RTN_GENERAL_ERROR            = -1,
    LIBUSBK_RTN_UDEV_NOT_CREATE          = -2,
    LIBUSBK_RTN_UDEV_NOT_NODE            = -3,
    LIBUSBK_RTN_UDEV_WRONG_FILE_TYPE     = -4,
    LIBUSBK_RTN_UDEV_USBKLIST_NOT_CREATE = -5,
    LIBUSBK_RTN_SCSI_COMMAND_ERROR       = -6,
    LIBUSBK_RTN_GET_BACKDISK_ERROR       = -7,
    LIBUSBK_RTN_NOT_MALLOC               = -8,

    LIBUSBK_RTN_SHORT_GENERATEDKEY       = 10,
}LIBUSBK_OPRSTATUS;

typedef enum __LIBSUBK_DEVSTATE
{
    LIBSUBK_DEVSTATE_ACTIVATE                = 1,
    LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK  = 2,
    LIBSUBK_DEVSTATE_DEACTIVATE              = 3,
    LIBSUBK_DEVSTATE_FABRIC_DEFAULT          = 4,
    LIBSUBK_DEVSTATE_MUST_REMOVE             = 5,
}LIBSUBK_DEVSTATE;


typedef struct __USBK_INFO {
    char                *dev_path;
    char                *backdisk_path;
    char                *backdisk;

    char                *product;
    char                *model;
    char                *serial;
    char                *usb_serial;
    char                *firmware_ver;
    int                 multikey_cap;
    char                *dev_label;
    LIBSUBK_DEVSTATE    dev_state;
    int                 current_key;
    int                 autoact_keyno;
    int                 retry_num;
    char                **key_names;
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

int LibUSBK__GetRandomKey (const char *usbk_path, unsigned char **random_key, int get_key_size_byte);
int LibUSBK__GetRandomKey_Release(unsigned char **random_key);


int LibUSBK__ActivateKey (const char *usbk_path, const char *password, const int key_no);
int LibUSBK__DeActivateKey (const char *usbk_path);
int LibUSBK__ChangePassword (const char *usbk_path, const char *old_pass, const char *new_pass);
int LibUSBK__SetKey (const char *usbk_path, const char *pass, int key_no, int name_only, const char* key_name, const char* key_size, const unsigned char* key);
int LibUSBK__SetAutoAct (const char *usbk_path, const char *pass, int enable, int key_no);
int LibUSBK__SetDeviceName (const char *usbk_path, const char *pass, const char *device_label);



#endif

