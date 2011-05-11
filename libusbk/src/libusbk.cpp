/*
 * @file libusbk.cpp
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
 * This is a utility program for listing SCSI devices and hosts (HBAs)
 * in the Linux operating system. It is applicable to kernel versions
 * 2.6.1 and greater.
 *
 */

//PRIVATE HEADERS
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "general.h"
#include "uip.h"

#include "libusbk.h"
#include "usbk_scsi.h"


//PRIVATE DEFINES
//-ERROR MESSAGES
typedef struct __LIBUSBK_RETURN_VALUE
{
    LIBUSBK_OPRSTATUS   return_value;
    const char          *return_string;
}LIBUSBK_RETURN_VALUE;

LIBUSBK_RETURN_VALUE return_values[] =
{
    { LIBUSBK_RTN_OPRS_PASS                   ,NULL},
    { LIBUSBK_RTN_OPRS_GEN_FAIL               ,NULL},
    { LIBUSBK_RTN_OPRS_INVALID_PASS           ,NULL},
    { LIBUSBK_RTN_OPRS_FABRIC_RESET           ,NULL},
    { LIBUSBK_RTN_OPRS_USBK_UNPLUGING         ,NULL},

    { LIBUSBK_RTN_GENERAL_ERROR               ,NULL},
    { LIBUSBK_RTN_UDEV_NOT_CREATE             ,"Can't create udev\n"},
    { LIBUSBK_RTN_UDEV_NOT_NODE               ,"device node not found\n"},
    { LIBUSBK_RTN_UDEV_WRONG_FILE_TYPE        ,"device node has wrong file type\n"},
    { LIBUSBK_RTN_UDEV_USBKLIST_NOT_CREATE    ,"USBK List is not malloced\n."},
    { LIBUSBK_RTN_SCSI_COMMAND_ERROR          ,"Scsi Command Sent Error\n"},
    { LIBUSBK_RTN_GET_BACKDISK_ERROR          ,"BackDisk Get Error\n"},
    { LIBUSBK_RTN_NOT_MALLOC                  ,"invalid space of memory\n"},
    { LIBUSBK_RTN_SHORT_GENERATEDKEY          ,NULL},
    { LIBUSBK_RTN_UNSUPPORTED_USBK            , "LIBUSBK_RTN_UNSUPPORTED_USBK"},

    { (LIBUSBK_OPRSTATUS)NULL                 ,NULL}
};

typedef struct __LIBUSBK_SUPPORTED_MODELS
{
    const char*         model;
    bool                support;
}LIBUSBK_SUPPORTED_MODELS;

typedef struct __LIBUSBK_SUPPORTED_VERSIONS
{
    int                             major_version;
    int                             minor_version;
    LIBUSBK_SUPPORTED_MODELS*       models;
}LIBUSBK_SUPPORTED_VERSIONS;

typedef struct __LIBUSBK_SUPPORTED_PRODUCTS
{
    const char*                         product;
    LIBUSBK_SUPPORTED_VERSIONS*         versions;
}LIBUSBK_SUPPORTED_PRODUCTS;



LIBUSBK_SUPPORTED_MODELS models_1_1[] =
{
     {"A101", true},
     {"A103", true},
     {NULL, false},
};

LIBUSBK_SUPPORTED_VERSIONS versions_1[] =
{
     {2, 5, models_1_1},
     {0, 0, NULL}
};

LIBUSBK_SUPPORTED_PRODUCTS products[] =
{
     {"USBK CryptoBridge 2.0", versions_1},
     {NULL, NULL}
};


//-DEFINES FOR FINDING USBK AND BACKDISK
#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT_A103    "a103"
#define USBK_USB_IDPRODUCT_A101    "a101"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"


//PRIVATE FUNCTION DECLERATIONS
static int libusbk_get_device_info(const char *usbk_dev, USBK_INFO* usbk_infos);
static int libusbk_get_backdisk(USBK_INFO* usbk_infos);
static LIBSUBK_DEVSTATE libusbk_getdevstate(e_UIP_DEVSTATE devstate_from_usbk);
static int CheckSupported(USBK_INFO* usbk_info);
static void debug_return_string (const LIBUSBK_RETURN_VALUE *return_values, LIBUSBK_OPRSTATUS return_status);

//ALL FUNCTIONS
//-PUBLIC FUNCTIONS


//ALL FUNCTIONS
//-PUBLIC FUNCTIONS
int LibUSBK__list_devices(USBK_List** usbk_list){
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    int i;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;

    *usbk_list = NULL;

    USBK_List *current_usbklink = NULL;

    // Create the udev object
    udev = udev_new();
    if (udev == NULL) {
        rtn = LIBUSBK_RTN_UDEV_NOT_CREATE;
        goto _return;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        // usb device directory
        dev_usb = udev_device_get_parent_with_subsystem_devtype( dev, "usb", "usb_device");
        if (dev_usb != NULL) {
              if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
                if((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0) ||
                    (strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)){
                    // get scsi device
                    dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                    if (dev_scsi != NULL) {
                        if(strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0 ){
                            current_usbklink = (USBK_List*)calloc(1,sizeof(USBK_List));
                            if (current_usbklink == NULL)
                            {
                                LibUSBK__list_devices_release(usbk_list);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);
                                rtn = LIBUSBK_RTN_NOT_MALLOC;
                                goto _return;
                            }

                            current_usbklink->next = *usbk_list;

                            // get device information from UDEV
                            current_usbklink->usbk_info.dev_path = strdup(udev_device_get_devnode(dev));
                            current_usbklink->dev = strdup(udev_device_get_sysname(dev));
                            //current_usbklink->usbk_info.vendor_id = strdup(udev_device_get_sysattr_value(dev_usb,"idVendor"));
                            //current_usbklink->usbk_info.product_id = strdup(udev_device_get_sysattr_value(dev_usb, "idProduct"));
                            //current_usbklink->usbk_info.manufacturer = strdup(udev_device_get_sysattr_value(dev_usb,"manufacturer"));
                            current_usbklink->usbk_info.product = strdup(udev_device_get_sysattr_value(dev_usb,"product"));
                            current_usbklink->usbk_info.usb_serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));

                            // get device information from USBK
                            t_UIP_DEVINFO devinfo;
                            memset (&devinfo, 0, sizeof (devinfo));
                            rtn = send_scsi_command(current_usbklink->usbk_info.dev_path, (unsigned char*) &devinfo, GET_DEV_INFO, sizeof(devinfo), READ_SCSI);

                            if (rtn < 0)
                            {
                                LibUSBK__list_devices_release(&current_usbklink);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);
                                rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
                                goto _return;
                            }

                            current_usbklink->usbk_info.dev_label = strdup(devinfo.devlabel.s);
                            current_usbklink->usbk_info.product = strdup(devinfo.product.s);
                            current_usbklink->usbk_info.model = strdup(devinfo.model.s);
                            current_usbklink->usbk_info.firmware_ver = strdup(devinfo.firmware_ver.s);

                            current_usbklink->usbk_info.serial = (char*)calloc((sizeof(devinfo.serial.u8) * 2)+2, sizeof(char));
                            for (i = 0; i < sizeof(devinfo.serial.u8); i++)
                                sprintf((current_usbklink->usbk_info.serial+i*2), "%2.2X", devinfo.serial.u8[i]);

                            if (CheckSupported(&(current_usbklink->usbk_info)) == false)
                            {
                                current_usbklink->usbk_info.supported = false;
                            }
                            else
                            {
                                current_usbklink->usbk_info.supported = true;

                                current_usbklink->usbk_info.multikey_cap = devinfo.multikeycap;
                                current_usbklink->usbk_info.current_key = devinfo.current_keyno;
                                current_usbklink->usbk_info.autoact_keyno = devinfo.autoactivate_keyno;
                                current_usbklink->usbk_info.retry_num = devinfo.retry_num;
                                current_usbklink->usbk_info.dev_state = libusbk_getdevstate(devinfo.devstate.me);

                                current_usbklink->usbk_info.key_names = (char**) calloc(current_usbklink->usbk_info.multikey_cap, sizeof(char*));
                                for (i = 0; i < current_usbklink->usbk_info.multikey_cap; i++)
                                    current_usbklink->usbk_info.key_names[i] = strdup(devinfo.keyname[i].s);

                                // get BackDisk information from UDEV
                                rtn = libusbk_get_backdisk(&current_usbklink->usbk_info);

                                if (rtn < 0)
                                {
                                    LibUSBK__list_devices_release(&current_usbklink);
                                    udev_device_unref(dev);
                                    udev_enumerate_unref(enumerate);
                                    udev_unref(udev);
                                    rtn = LIBUSBK_RTN_GET_BACKDISK_ERROR;
                                    goto _return;
                                }
                            }

                            *usbk_list = current_usbklink;
                        }
                    }

                }
            }

        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    if (*usbk_list == NULL){
        rtn = LIBUSBK_RTN_NO_DEVICE_FOUND;
        goto _return;
    }
    else{
        rtn = LIBUSBK_RTN_OPRS_PASS;
        goto _return;
    }

_return:
    debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
    return rtn;
}

void LibUSBK__list_devices_release(USBK_List** p_usbklink)
{
    USBK_List *dummy_usbklink;

    for (*p_usbklink;*p_usbklink!=NULL;*p_usbklink=dummy_usbklink)
    {
        dummy_usbklink = (*p_usbklink)->next;
        free ((*p_usbklink)->dev);
        LibUSBK__GetDeviceInfo_Release(&(*p_usbklink)->usbk_info);
        free(*p_usbklink);
        *p_usbklink = NULL;
    }
}

int LibUSBK__GetDeviceInfo(const char *usbk_dev, USBK_INFO** usbk_infos)
{
    t_UIP_DEVINFO real_usbk_info;
    int i;
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;

    *usbk_infos = (USBK_INFO*) calloc(1, sizeof(USBK_INFO));
    if (*usbk_infos == NULL){
        rtn = LIBUSBK_RTN_NOT_MALLOC;
        goto _return;
    }

    // get information about usbk by udev
    rtn = libusbk_get_device_info(usbk_dev, *usbk_infos);
    if (rtn<0){
        goto _return;
    }

    // get information about usbk by scsi command "GetDevInfo"
    rtn = send_scsi_command((*usbk_infos)->dev_path, (unsigned char*) &real_usbk_info, GET_DEV_INFO,sizeof(real_usbk_info), READ_SCSI);
    if (rtn<0){
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        goto _return;
    }

    (*usbk_infos)->product = strdup(real_usbk_info.product.s);
    (*usbk_infos)->model = strdup(real_usbk_info.model.s);
    (*usbk_infos)->firmware_ver = strdup(real_usbk_info.firmware_ver.s);

    if (CheckSupported(*usbk_infos) == false)
    {
        (*usbk_infos)->supported = false;
        rtn = LIBUSBK_RTN_UNSUPPORTED_USBK;
    }
    else
    {
        (*usbk_infos)->supported = true;
        // USBK_INFO creating
        (*usbk_infos)->multikey_cap = real_usbk_info.multikeycap;
        (*usbk_infos)->current_key = real_usbk_info.current_keyno;
        (*usbk_infos)->autoact_keyno = real_usbk_info.autoactivate_keyno;
        (*usbk_infos)->retry_num = real_usbk_info.retry_num;
        (*usbk_infos)->dev_state = libusbk_getdevstate(real_usbk_info.devstate.me);
        (*usbk_infos)->dev_label = strdup(real_usbk_info.devlabel.s);

        (*usbk_infos)->serial = (char*)calloc((sizeof(real_usbk_info.serial) * 2)+2, sizeof(char));
        for (i = 0; i < 15; i++){
            sprintf(((*usbk_infos)->serial+i*2), "%2.2X", real_usbk_info.serial.u8[i]);
        }

        (*usbk_infos)->key_names = (char**) calloc((*usbk_infos)->multikey_cap, sizeof(char*));
        for (i = 0; i < (*usbk_infos)->multikey_cap; i++){
            (*usbk_infos)->key_names[i] = strdup(real_usbk_info.keyname[i].s);
        }

        rtn = LIBUSBK_RTN_OPRS_PASS;
    }


_return:
    debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
    return rtn;
}

int LibUSBK__GetDeviceInfo_Release(USBK_INFO* usbk_infos)
{

    int i;
    for (i = 0; i < usbk_infos->multikey_cap; i++) free(usbk_infos->key_names[i]);
    free(usbk_infos->key_names);
    free(usbk_infos->dev_path);
    free(usbk_infos->backdisk_path);
    free(usbk_infos->backdisk);
    free(usbk_infos->product);
    free(usbk_infos->model);
    free(usbk_infos->firmware_ver);
    free(usbk_infos->dev_label);
    free(usbk_infos->serial);

    debug_return_string(return_values, LIBUSBK_RTN_OPRS_PASS);
    return LIBUSBK_RTN_OPRS_PASS;
}

int LibUSBK__GetStatus (const char *usbk_path)
{
    t_UIP_GETSTATUS status;
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    rtn = send_scsi_command(usbk_path, (unsigned char*) &status, GET_STATUS,sizeof(t_UIP_GETSTATUS), READ_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return ((int)status.lastop.me);
}

int LibUSBK__ActivateKey (const char *usbk_path, const char *password, const int key_no)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_ACTIVATE activate;
    memset (&activate, 0, sizeof (activate));

    strcpy(activate.password.s, password);
    activate.keyno = key_no;

    rtn = send_scsi_command(usbk_path, (unsigned char *)&activate, ACTIVATE_KEY, sizeof(activate), WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__DeActivateKey (const char *usbk_path)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    rtn = send_scsi_command(usbk_path, (unsigned char*) NULL, DEACTIVATE_KEY, 0, WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__ChangePassword (const char *usbk_path, const char *old_pass, const char *new_pass)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_CHPASS chpass;

    strncpy(chpass.new_password.s, new_pass, sizeof (chpass.new_password.s));
    strncpy(chpass.old_password.s, old_pass, sizeof (chpass.old_password.s));

    rtn = send_scsi_command(usbk_path, (unsigned char*)&chpass, CHANGE_PASS, sizeof(chpass), WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__SetDeviceName (const char *usbk_path, const char *pass, const char *device_label)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_SETDEVICELABEL devlabel;

    strncpy(devlabel.password.s, pass, sizeof(devlabel.password.s));
    strncpy(devlabel.devlabel.s, device_label, sizeof(devlabel.devlabel.s));

    rtn = send_scsi_command(usbk_path, (unsigned char*)&devlabel, SET_DEV_NAME, sizeof(devlabel), WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__SetKey (const char *usbk_path, const char *pass, int key_no, int name_only, const char* key_name, const char* key_size, const unsigned char* key)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_SETKEY setkey;
    memset (&setkey, 0, sizeof (setkey));

    strncpy(setkey.password.s, pass, sizeof(setkey.password.s));
    setkey.keyno = key_no;
    strncpy(setkey.keyname.s, key_name, sizeof(setkey.keyname.s));

    if (name_only == true){
        setkey.options.me = SETKEY_NAMEONLY;
    }
    else{
        setkey.options.me = SETKEY_NAME_AND_KEY;
        if (!strcmp(key_size, "128")) {
            setkey.keysize.me = KEYSIZE_128;
            memcpy(setkey.key.u8,key,16);
        } else if (!strcmp(key_size, "192")) {
            setkey.keysize.me = KEYSIZE_192;
            memcpy(setkey.key.u8,key,24);
        } else if (!strcmp(key_size, "256")) {
            setkey.keysize.me = KEYSIZE_256;
            memcpy(setkey.key.u8,key,32);
        } else {
            setkey.keysize.me = KEYSIZE_256;    // if not defined, default is 256-Bit Key
            memcpy(setkey.key.u8,key,32);
        }
    }

    rtn = send_scsi_command(usbk_path, (unsigned char*)&setkey, SET_KEY, sizeof(setkey), WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__SetAutoAct (const char *usbk_path, const char *pass, int enable, int key_no)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_SETAUTOACTIVATE autoact;

    strncpy(autoact.password.s, pass, sizeof(autoact.password.s));
    autoact.keyno = (enable == true)?0:key_no;

    rtn = send_scsi_command(usbk_path, (unsigned char*)&autoact, SET_AUTO_ACTIVE, sizeof(autoact), WRITE_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__GetRandomKey (const char *usbk_path, unsigned char **random_key, int get_key_size_byte)
{
    // request key len is checked. If request key len is larger than generated key, return  warning and random number is generated.
    // The app. will decide whether the key is used or not.
    // Maybe, give the len of generated key.
    // Also, the key is generated as 32 byte. Maybe, the key will be requested len.

    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_GENERATEKEY genkey;


    rtn = send_scsi_command(usbk_path, (unsigned char*)&genkey, GENERATE_KEY, sizeof(genkey), READ_SCSI);
    if (rtn < 0) {
        rtn = LIBUSBK_RTN_SCSI_COMMAND_ERROR;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }

    *random_key = (unsigned char*)calloc(1, sizeof(genkey.key.u8));
    if (*random_key == NULL)
    {
        rtn = LIBUSBK_RTN_NOT_MALLOC;
        debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
        return rtn;
    }

    memcpy(*random_key, genkey.key.u8, sizeof(genkey.key.u8));

    if (get_key_size_byte > sizeof(genkey.key.u8))      rtn = LIBUSBK_RTN_SHORT_GENERATEDKEY;
    else                                                rtn = LIBUSBK_RTN_OPRS_PASS;

    debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
    return rtn;
}

int LibUSBK__GetRandomKey_Release(unsigned char **random_key)
{
    memset (*random_key, 0, sizeof (random_key));    // This memory space is filled with NULL before releasing because of security.
    free(*random_key);
    *random_key = NULL;
}

//-PRIVATE FUNCTIONS
static int libusbk_get_device_info(const char *usbk_dev, USBK_INFO* usbk_infos) {
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev = NULL;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    size_t len;
    const char *str;
    int i;
    struct udev_list_entry *list_entry;

    struct stat statbuf;

    udev = udev_new();
    udev = udev_new();
    if (udev == NULL) {
        rtn = LIBUSBK_RTN_UDEV_NOT_CREATE;
        goto _return;
    }

    char devname[1024];
    sprintf(devname, "%s/%s", udev_get_dev_path(udev), usbk_dev);

    if (stat(devname, &statbuf) < 0) {
        rtn = LIBUSBK_RTN_UDEV_NOT_NODE;
        goto _return;
    } else {
        char type;
        if (S_ISBLK(statbuf.st_mode)) {
            type = 'b';
        } else if (S_ISCHR(statbuf.st_mode)) {
            type = 'c';
        } else {
            udev_unref(udev);
            rtn = LIBUSBK_RTN_UDEV_WRONG_FILE_TYPE;
            goto _return;
        }

        dev = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);

        if (dev == NULL) {
            rtn = LIBUSBK_RTN_UDEV_NOT_NODE;
            goto _return;
        }
    }

    dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    if (dev_usb != NULL) {
        if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
            if ((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0) ||
                (strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)) {
                dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                if (dev_scsi != NULL) {
                    if (strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0) {
                        usbk_infos->dev_path = strdup(udev_device_get_devnode(dev));
                        usbk_infos->usb_serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));
                        rtn = libusbk_get_backdisk(usbk_infos);
                        if (rtn < 0)
                        {
                            udev_device_unref(dev);
                            udev_unref(udev);

                            rtn = LIBUSBK_RTN_GET_BACKDISK_ERROR;
                            goto _return;
                        }
                        rtn = LIBUSBK_RTN_OPRS_PASS;
                    }
                }
            }
        }
    }
    udev_device_unref(dev);
    udev_unref(udev);


_return:
    debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
    return rtn;
}

static int libusbk_get_backdisk(USBK_INFO* usbk_infos) {
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi;

    // Create the udev object
    udev = udev_new();
    if (udev == NULL) {
        rtn = LIBUSBK_RTN_UDEV_NOT_CREATE;
        goto _return;
    }

    // Create a list of the devices in the 'abcde' subsystem.
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        // usb device directory
        dev_usb = udev_device_get_parent_with_subsystem_devtype( dev, "usb", "usb_device");
        if (dev_usb != NULL) {
            if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
                if((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0) ||
                    (strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)){
                    if(strcmp(usbk_infos->usb_serial, udev_device_get_sysattr_value(dev_usb, "serial")) == 0 ){
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if(strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0 ){
                                usbk_infos->backdisk = strdup(udev_device_get_sysname(dev));
                                usbk_infos->backdisk_path = strdup(udev_device_get_devnode(dev));
                                rtn = LIBUSBK_RTN_OPRS_PASS;
                            }
                        }
                    }
                }
            }
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);

_return:
    debug_return_string(return_values, (LIBUSBK_OPRSTATUS)rtn);
    return rtn;
}

static LIBSUBK_DEVSTATE libusbk_getdevstate(e_UIP_DEVSTATE devstate_from_usbk)
{
    return (LIBSUBK_DEVSTATE)devstate_from_usbk;
}

static int CheckSupported(USBK_INFO* usbk_info)
{
    int major_version, minor_version, revision;

    major_version = atoi(strtok(usbk_info->firmware_ver, "."));
    minor_version = atoi(strtok(NULL, "."));
    revision = atoi(strtok( NULL,".\n " ));

    LIBUSBK_SUPPORTED_PRODUCTS * d_products = products;

    for (; d_products->product; d_products++)
    {
        if (strcmp(usbk_info->product, d_products->product) == 0)
        {
            LIBUSBK_SUPPORTED_VERSIONS* d_version = d_products->versions;
            for (; d_version->models; d_version++)
            {
                if (major_version == d_version->major_version)
                {
                    if (minor_version == d_version->minor_version)
                    {
                        LIBUSBK_SUPPORTED_MODELS* d_model = d_products->versions->models;
                        for (; d_model->model; d_model++)
                        {
                            if (strcmp(usbk_info->model, d_model->model) == 0)
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// DEBUG TOOLS
static void debug_return_string (const LIBUSBK_RETURN_VALUE *return_values, LIBUSBK_OPRSTATUS return_status)
{
    for (; return_values->return_value; return_values++)
    {
        if (return_values->return_value == return_status){
            if (return_values->return_string != NULL)
            {
                fprintf(stderr, "libusbk--> %s", return_values->return_string);
            }
            break;
        }
    }
}
