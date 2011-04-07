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


#include "libusbk.h"
#include "usbk_scsi.h"
#include "uip.h"

#define msgLIBUSBK_UDEV_NOT_CREATE              "Can't create udev\n"
#define msgLIBUSBK_UDEV_NOT_NODE                "device node not found\n"
#define msgLIBUSBK_UDEV_WRONG_FILE_TYPE         "device node has wrong file type\n"
#define msgLIBUSBK_UDEV_USBKLIST_NOT_CREATE     "USBK List is not malloced\n."
#define msgLIBUSBK_SCSI_COMMAND_ERROR           "Scsi Command Sent Error\n"
#define msgLIBUSBK_GET_BACKDISK_ERROR           "BackDisk Get Error"

static int libusbk_get_device_info(const char *usbk_dev, USBK_INFO* usbk_infos);
static int libusbk_get_backdisk(USBK_INFO* usbk_infos);



LIBSUBK_DEVSTATE libusbk_getdevstate(e_UIP_DEVSTATE devstate_from_usbk)
{
    return (LIBSUBK_DEVSTATE)devstate_from_usbk;
}


int LibUSBK__GetDeviceInfo(const char *usbk_dev, USBK_INFO** usbk_infos)
{

    t_UIP_DEVINFO real_usbk_info;
    int i;
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;

    *usbk_infos = (USBK_INFO*) calloc(1, sizeof(USBK_INFO));
    if (*usbk_infos == NULL) return -1;

    if (libusbk_get_device_info(usbk_dev, *usbk_infos)>=0)
    {
        if (send_scsi_command((*usbk_infos)->dev_path, (unsigned char*) &real_usbk_info, GET_DEV_INFO,sizeof(real_usbk_info), READ_SCSI) >= 0)
        {
            (*usbk_infos)->multikey_cap = real_usbk_info.multikeycap;
            (*usbk_infos)->current_key = real_usbk_info.current_keyno;
            (*usbk_infos)->autoact_keyno = real_usbk_info.autoactivate_keyno;
            (*usbk_infos)->retry_num = real_usbk_info.retry_num;
            (*usbk_infos)->dev_state = libusbk_getdevstate(real_usbk_info.devstate.me);
            (*usbk_infos)->product = strdup(real_usbk_info.product.s);
            (*usbk_infos)->model = strdup(real_usbk_info.model.s);
            (*usbk_infos)->firmware_ver = strdup(real_usbk_info.firmware_ver.s);
            (*usbk_infos)->dev_label = strdup(real_usbk_info.devlabel.s);

            (*usbk_infos)->serial = (char*)calloc((sizeof(real_usbk_info.serial) * 2)+2, sizeof(char));
            for (i = 0; i < 15; i++)
                sprintf(((*usbk_infos)->serial+i*2), "%2.2X", real_usbk_info.serial.u8[i]);

            (*usbk_infos)->key_names = (char**) calloc((*usbk_infos)->multikey_cap, sizeof(char*));
            for (i = 0; i < (*usbk_infos)->multikey_cap; i++)
                (*usbk_infos)->key_names[i] = strdup(real_usbk_info.keyname[i].s);

            rtn = LIBUSBK_RTN_PASS;
        }
    }
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

    return LIBUSBK_RTN_PASS;
}

int LibUSBK__GetStatus (const char *usbk_path)
{
    t_UIP_GETSTATUS status;
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    rtn = send_scsi_command(usbk_path, (unsigned char*) &status, GET_STATUS,sizeof(t_UIP_GETSTATUS), READ_SCSI);
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
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
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__DeActivateKey (const char *usbk_path)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    rtn = send_scsi_command(usbk_path, (unsigned char*) NULL, DEACTIVATE_KEY, 0, WRITE_SCSI);
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__ChangePassword (const char *usbk_path, const char *old_pass, const char *new_pass)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_CHPASS chpass;

    strncpy(chpass.new_password.s, new_pass, sizeof (chpass.new_password.s));
    strncpy(chpass.old_password.s, old_pass, sizeof (chpass.old_password.s));

    rtn = send_scsi_command(usbk_path, (unsigned char*)&chpass, CHANGE_PASS, sizeof(chpass), WRITE_SCSI);
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__SetDeviceName (const char *usbk_path, const char *pass, const char *device_label)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_SETDEVICELABEL devlabel;

    strncpy(devlabel.password.s, pass, sizeof(devlabel.password.s));
    strncpy(devlabel.devlabel.s, device_label, sizeof(devlabel.devlabel.s));

    rtn = send_scsi_command(usbk_path, (unsigned char*)&devlabel, SET_DEV_NAME, sizeof(devlabel), WRITE_SCSI);
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
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
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__SetAutoAct (const char *usbk_path, const char *pass, int enable, int key_no)
{
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    t_UIP_SETAUTOACTIVATE autoact;

    strncpy(autoact.password.s, pass, sizeof(autoact.password.s));
    autoact.keyno = (enable == true)?0:key_no;

    rtn = send_scsi_command(usbk_path, (unsigned char*)&autoact, SET_AUTO_ACTIVE, sizeof(autoact), WRITE_SCSI);
    if (rtn < 0) return LIBUSBK_RTN_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk_path);
}

int LibUSBK__GetRandomKey (const char *usbk_path, unsigned char *random_key)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    t_UIP_GENERATEKEY genkey;

    rtn = send_scsi_command_new(usbk_path, (unsigned char*)&genkey, GENERATE_KEY, sizeof(genkey), READ_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;

    memcpy(random_key, genkey, sizeof(genkey));

    return OPRS_PASS;
}

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
    if (!udev) {
        fprintf(stderr, msgLIBUSBK_UDEV_NOT_CREATE);
        return LIBUSBK_RTN_UDEV_NOT_CREATE;
    }

    char devname[1024];
    sprintf(devname, "%s/%s", udev_get_dev_path(udev), usbk_dev);

    if (stat(devname, &statbuf) < 0) {
        fprintf(stderr, "%s "msgLIBUSBK_UDEV_NOT_NODE, usbk_dev);
        return LIBUSBK_RTN_UDEV_NOT_NODE;
    } else {
        char type;
        if (S_ISBLK(statbuf.st_mode)) {
            type = 'b';
        } else if (S_ISCHR(statbuf.st_mode)) {
            type = 'c';
        } else {
            udev_unref(udev);
            fprintf(stderr, msgLIBUSBK_UDEV_WRONG_FILE_TYPE);
            return LIBUSBK_RTN_UDEV_WRONG_FILE_TYPE;
        }

        dev = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);

        if (dev == NULL) {
            fprintf(stderr, "%s "msgLIBUSBK_UDEV_NOT_NODE, usbk_dev);
            return LIBUSBK_RTN_UDEV_NOT_NODE;
        }
    }

    dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    if (dev_usb != NULL) {
        if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
            if (strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0) {
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

                            fprintf(stderr, msgLIBUSBK_GET_BACKDISK_ERROR);
                            return LIBUSBK_RTN_GET_BACKDISK_ERROR;
                        }
                        rtn = LIBUSBK_RTN_PASS;
                    }
                }
            }
        }
    }
    udev_device_unref(dev);
    udev_unref(udev);
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
    if (!udev) {
        fprintf(stderr, msgLIBUSBK_UDEV_NOT_CREATE);
        return LIBUSBK_RTN_UDEV_NOT_CREATE;
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
                if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                    if(strcmp(usbk_infos->usb_serial, udev_device_get_sysattr_value(dev_usb, "serial")) == 0 ){
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if(strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0 ){
                                usbk_infos->backdisk = strdup(udev_device_get_sysname(dev));
                                usbk_infos->backdisk_path = strdup(udev_device_get_devnode(dev));
                                rtn = LIBUSBK_RTN_PASS;
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
    return rtn;
}

USBK_List2* LibUSBK__list_devices(void){
    int rtn = LIBUSBK_RTN_GENERAL_ERROR;
    int i;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    bool first = true;

    USBK_List2 *current_usbklink = NULL;
    USBK_List2 *first_usbklink = NULL;

    // Create the udev object
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, msgLIBUSBK_UDEV_NOT_CREATE);
        return NULL;
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
                if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                    // get scsi device
                    dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                    if (dev_scsi != NULL) {
                        if(strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0 ){
                            current_usbklink = (USBK_List2*)calloc(1,sizeof(USBK_List2));
                            if (current_usbklink == NULL)
                            {
                                LibUSBK__list_devices_release(first_usbklink);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);

                                fprintf(stderr, msgLIBUSBK_UDEV_USBKLIST_NOT_CREATE);
                                return NULL;
                            }

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
                                LibUSBK__list_devices_release(current_usbklink);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);

                                fprintf(stderr, msgLIBUSBK_SCSI_COMMAND_ERROR);
                                return NULL;
                            }

                            current_usbklink->usbk_info.multikey_cap = devinfo.multikeycap;
                            current_usbklink->usbk_info.current_key = devinfo.current_keyno;
                            current_usbklink->usbk_info.autoact_keyno = devinfo.autoactivate_keyno;
                            current_usbklink->usbk_info.retry_num = devinfo.retry_num;
                            current_usbklink->usbk_info.dev_state = libusbk_getdevstate(devinfo.devstate.me);

                            current_usbklink->usbk_info.product = strdup(devinfo.product.s);
                            current_usbklink->usbk_info.model = strdup(devinfo.model.s);
                            current_usbklink->usbk_info.firmware_ver = strdup(devinfo.firmware_ver.s);
                            current_usbklink->usbk_info.dev_label = strdup(devinfo.devlabel.s);

                            current_usbklink->usbk_info.serial = (char*)calloc((sizeof(devinfo.serial.u8) * 2)+2, sizeof(char));
                            for (i = 0; i < sizeof(devinfo.serial.u8); i++)
                                sprintf((current_usbklink->usbk_info.serial+i*2), "%2.2X", devinfo.serial.u8[i]);

                            current_usbklink->usbk_info.key_names = (char**) calloc(current_usbklink->usbk_info.multikey_cap, sizeof(char*));
                            for (i = 0; i < current_usbklink->usbk_info.multikey_cap; i++)
                                current_usbklink->usbk_info.key_names[i] = strdup(devinfo.keyname[i].s);


                            // get BackDisk information from UDEV
                            rtn = libusbk_get_backdisk(&current_usbklink->usbk_info);

                            if (rtn < 0)
                            {
                                LibUSBK__list_devices_release(current_usbklink);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);

                                fprintf(stderr, msgLIBUSBK_GET_BACKDISK_ERROR);
                                return NULL;
                            }
                            current_usbklink->next = first_usbklink;
                            first_usbklink = current_usbklink;
                        }
                    }

                }
            }

        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return first_usbklink;
}

void LibUSBK__list_devices_release(USBK_List2* p_usbklink)
{
    USBK_List2 *dummy_usbklink;

    for (p_usbklink;p_usbklink!=NULL;p_usbklink=dummy_usbklink)
    {
        dummy_usbklink = p_usbklink->next;
        free (p_usbklink->dev);
        LibUSBK__GetDeviceInfo_Release(&p_usbklink->usbk_info);
        free(p_usbklink);
    }
}
