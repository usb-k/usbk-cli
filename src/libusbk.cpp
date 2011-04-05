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

#define msgLIBUSBK_UDEV_NOT_CREATE              "Can't create udev\n"
#define msgLIBUSBK_UDEV_NOT_NODE                "%s device node not found\n", usbk->dev
#define msgLIBUSBK_UDEV_WRONG_FILE_TYPE         "%s device node has wrong file type\n", usbk->dev
#define msgLIBUSBK_UDEV_USBKLIST_NOT_CREATE     "USBK List is not malloced\n."
#define msgLIBUSBK_SCSI_COMMAND_ERROR           "Scsi Command Sent Error\n"
#define msgLIBUSBK_GET_BACKDISK_ERROR           "BackDisk Get Error"



static int libusbk_get_device_info(USBK* usbk);
static int libusbk_get_backdisk(USBK *usbk);

USBK_List* LibUSBK__list_devices(void){
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    bool first = true;

    USBK_List *current_usbklink = NULL;
    USBK_List *first_usbklink = NULL;

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
                            current_usbklink = (USBK_List*)calloc(1,sizeof(USBK_List));
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
                            current_usbklink->usbk.dev_path = strdup(udev_device_get_devnode(dev));
                            current_usbklink->usbk.dev = strdup(udev_device_get_sysname(dev));
                            current_usbklink->usbk.vendor_id = strdup(udev_device_get_sysattr_value(dev_usb,"idVendor"));
                            current_usbklink->usbk.product_id = strdup(udev_device_get_sysattr_value(dev_usb, "idProduct"));
                            current_usbklink->usbk.manufacturer = strdup(udev_device_get_sysattr_value(dev_usb,"manufacturer"));
                            current_usbklink->usbk.product = strdup(udev_device_get_sysattr_value(dev_usb,"product"));
                            current_usbklink->usbk.serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));

                            // get device information from USBK
                            rtn = send_scsi_command(&current_usbklink->usbk, (unsigned char*) &current_usbklink->usbk.info, GET_DEV_INFO, sizeof(t_UIP_DEVINFO), READ_SCSI);
                            if (rtn < 0)
                            {
                                LibUSBK__list_devices_release(current_usbklink);
                                udev_device_unref(dev);
                                udev_enumerate_unref(enumerate);
                                udev_unref(udev);

                                fprintf(stderr, msgLIBUSBK_SCSI_COMMAND_ERROR);
                                return NULL;
                            }

                            // get BackDisk information from UDEV
                            rtn = libusbk_get_backdisk(&current_usbklink->usbk);
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

void LibUSBK__list_devices_release(USBK_List* p_usbklink)
{
    USBK_List *dummy_usbklink;

    for (p_usbklink;p_usbklink!=NULL;p_usbklink=dummy_usbklink)
    {
        dummy_usbklink = p_usbklink->next;
        free(p_usbklink->usbk.vendor_id);
        free(p_usbklink->usbk.product_id);
        free(p_usbklink->usbk.manufacturer);
        free(p_usbklink->usbk.product);
        free(p_usbklink->usbk.serial);
        free(p_usbklink->usbk.dev);
        free(p_usbklink->usbk.dev_path);
        free(p_usbklink->usbk.backdisk);
        free(p_usbklink->usbk.backdisk_path);
        free(p_usbklink);
    }
}

int LibUSBK__GetDeviceInfo(USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    if (libusbk_get_device_info(usbk)>=0)
    {
        if (send_scsi_command(usbk, (unsigned char*) &usbk->info, GET_DEV_INFO,sizeof(t_UIP_DEVINFO), READ_SCSI) >= 0)
        {
            rtn = rtnLIBUSBK_PASS;
        }
    }
    return rtn;
}

int LibUSBK__GetStatus (USBK* usbk)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, (unsigned char*) &usbk->status, GET_STATUS,sizeof(t_UIP_GETSTATUS), READ_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return ((int)usbk->status.lastop.me);
}

int LibUSBK__ActivateKey (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, ACTIVATE_KEY, len, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__DeActivateKey (USBK* usbk)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, (unsigned char*) NULL, DEACTIVATE_KEY, 0, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__ChangePassword (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, CHANGE_PASS, len, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__SetDeviceName (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, SET_DEV_NAME, len, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__SetKey (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, SET_KEY, len, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__SetAutoAct (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, SET_AUTO_ACTIVE, len, WRITE_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return LibUSBK__GetStatus (usbk);
}

int LibUSBK__GetRandomKey (USBK* usbk, unsigned char *buff, int len)
{
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
    rtn = send_scsi_command(usbk, buff, GENERATE_KEY, len, READ_SCSI);
    if (rtn < 0) return rtnLIBUSBK_GENERAL_ERROR;
    return OPRS_PASS;
}

static int libusbk_get_device_info(USBK* usbk) {
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
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
        return rtnLIBUSBK_UDEV_NOT_CREATE;
    }

    char devname[1024];
    sprintf(devname, "%s/%s", udev_get_dev_path(udev), usbk->dev);

    if (stat(devname, &statbuf) < 0) {
        fprintf(stderr, msgLIBUSBK_UDEV_NOT_NODE);
        return rtnLIBUSBK_UDEV_NOT_NODE;
    } else {
        char type;
        if (S_ISBLK(statbuf.st_mode)) {
            type = 'b';
        } else if (S_ISCHR(statbuf.st_mode)) {
            type = 'c';
        } else {
            udev_unref(udev);
            fprintf(stderr, msgLIBUSBK_UDEV_WRONG_FILE_TYPE);
            return rtnLIBUSBK_UDEV_WRONG_FILE_TYPE;
        }

        dev = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);

        if (dev == NULL) {
            fprintf(stderr, msgLIBUSBK_UDEV_NOT_NODE);
            return rtnLIBUSBK_UDEV_NOT_NODE;
        }
    }

    dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    if (dev_usb != NULL) {
        if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
            if (strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0) {
                dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                if (dev_scsi != NULL) {
                    if (strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0) {
                        usbk->dev_path = strdup(udev_device_get_devnode(dev));
                        usbk->dev = strdup(udev_device_get_sysname(dev));
                        usbk->serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));
                        rtn = libusbk_get_backdisk(usbk);
                        if (rtn < 0)
                        {
                            udev_device_unref(dev);
                            udev_unref(udev);

                            fprintf(stderr, msgLIBUSBK_GET_BACKDISK_ERROR);
                            return rtnLIBUSBK_GET_BACKDISK_ERROR;
                        }
                        rtn = rtnLIBUSBK_PASS;
                    }
                }
            }
        }
    }
    udev_device_unref(dev);
    udev_unref(udev);
    return rtn;
}

static int libusbk_get_backdisk(USBK *usbk){
    int rtn = rtnLIBUSBK_GENERAL_ERROR;
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
        return rtnLIBUSBK_UDEV_NOT_CREATE;
    }

    // Create a list of the devices in the 'usbk' subsystem.
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
                    if(strncmp(usbk->serial,
                       udev_device_get_sysattr_value(dev_usb, "serial"), strlen(usbk->serial)) == 0 ){
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if(strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0 ){
                                usbk->backdisk = strdup(udev_device_get_sysname(dev));
                                usbk->backdisk_path = strdup(udev_device_get_devnode(dev));
                                rtn = rtnLIBUSBK_PASS;
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
