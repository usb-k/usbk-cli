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

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>

#include "general.h"
#include "usbk_scsi.h"
#include "usbk_sg_ctl.h"

int usbk_get_backdisk(struct __USBK *usbk){
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    /* Create a list of the devices in the 'usbk' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* usb device directory */
        dev_usb = udev_device_get_parent_with_subsystem_devtype(
                dev,
                "usb",
                "usb_device");
        if (dev_usb != NULL) {
            if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
                if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                    if(strncmp(usbk->usb_serial_no,
                       udev_device_get_sysattr_value(dev_usb, "serial"), strlen(usbk->usb_serial_no)) == 0 ){
                        /* get scsi device */
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if(strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0 ){
                                strcpy(usbk->backdisk_dev, udev_device_get_devnode(dev));
                            }
                        }

                    }
                }
            }
        }
//        udev_device_unref(dev_scsi);
//        udev_device_unref(dev_usb);
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}


int usbk_get_device_info(struct __USBK *usbk){
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev = NULL;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    char devname[1024];
    size_t len;
    const char *str;
    int i;
    struct udev_list_entry *list_entry;

    struct stat statbuf;

    udev = udev_new();

    sprintf(devname, "%s/%s", udev_get_dev_path(udev), usbk->dev);
    if (stat(devname, &statbuf) < 0) {
        printf("%s device node not found\n", usbk->dev);
        exit(1);
    } else {
        char type;
        if (S_ISBLK(statbuf.st_mode)) {
              type = 'b';
        } else if (S_ISCHR(statbuf.st_mode)) {
              type = 'c';
        } else {
              printf("%s device node has wrong file type\n", usbk->dev);
              exit(1);
        }
        dev = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);
        if (dev == NULL) {
              printf("%s device node not found\n", usbk->dev);
              exit(1);
        }
    }

    /* usb device directory */
    dev_usb = udev_device_get_parent_with_subsystem_devtype(
            dev,
            "usb",
            "usb_device");
    if (dev_usb != NULL) {
        if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
            if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                /* get scsi device */
                dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                if (dev_scsi != NULL) {
                    if(strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0 ){
                        /* usb_device_get_devnode() returns the path to the device node
                           itself in /dev. */
                        /*
                        printf("P: %s\n", udev_device_get_devpath(dev));
                        printf("  scsi device\n");
                        printf("    vendor: %s\n", udev_device_get_sysattr_value(dev_scsi, "vendor"));
                        printf("    model: %s\n", udev_device_get_sysattr_value(dev_scsi, "model"));
                        printf("    rev: %s\n", udev_device_get_sysattr_value(dev_scsi, "rev"));
                        udev_list_entry_foreach(list_entry, udev_device_get_devlinks_list_entry(dev)) {
                                 len = strlen(udev_get_dev_path(udev_device_get_udev(dev)));
                                 printf("    S: %s\n", &udev_list_entry_get_name(list_entry)[len+1]);
                        }

                        printf("\n  usb device\n");
                        printf("    %s\n", udev_device_get_sysattr_value(dev_usb,"product"));
                        printf("    %s\n", udev_device_get_sysattr_value(dev_usb,"manufacturer"));
                        printf("    serial: %s\n",
                                  udev_device_get_sysattr_value(dev_usb, "serial"));
                        */

                        strncpy(usbk->usb_serial_no, udev_device_get_sysattr_value(dev_usb, "serial"), sizeof(usbk->usb_serial_no));
                        usbk_get_backdisk(usbk);
                        strcpy(usbk->dev, devname);
                    }
                }
            }
        }
        //udev_device_unref(dev_scsi);
        //udev_device_unref(dev_usb);
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_unref(udev);
    return 0;
}





int usbk_get_backdisk(USBK_List *usbk){
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    /* Create a list of the devices in the 'usbk' subsystem. */
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* usb device directory */
        dev_usb = udev_device_get_parent_with_subsystem_devtype(
                dev,
                "usb",
                "usb_device");
        if (dev_usb != NULL) {
            if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
                if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                    if(strncmp(usbk->serial,
                       udev_device_get_sysattr_value(dev_usb, "serial"), strlen(usbk->serial)) == 0 ){
                        /* get scsi device */
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if(strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0 ){
                                strcpy(usbk->backdisk_name, udev_device_get_devnode(dev));
                            }
                        }

                    }
                }
            }
        }
//        udev_device_unref(dev_scsi);
//        udev_device_unref(dev_usb);
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}


 int usbk_list_devices(USBK_List** pusbk){
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    bool first = true;

    USBK_List *dummy_pusbk = NULL;

    // Create the udev object
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");	//todo stderr yollanabilir "fprintf(stderr, "message");"
        exit(1);
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
        dev_usb = udev_device_get_parent_with_subsystem_devtype(
                dev,
                "usb",
                "usb_device");
        if (dev_usb != NULL) {
              if(strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb,"idVendor"), strlen(USBK_USB_IDVENDOR)) == 0 ){
                if(strncmp(USBK_USB_IDPRODUCT, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT)) == 0){
                    // get scsi device
                    dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                    if (dev_scsi != NULL) {
                        if(strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0 ){

                            if (first == true)
                            {
                            	*pusbk = (USBK_List*)malloc(sizeof(**pusbk));
                            	dummy_pusbk = *pusbk;
                            	first = false;
                            }
                            else
                            {
                            	dummy_pusbk->next = (USBK_List*)malloc(sizeof(*dummy_pusbk));
                            	dummy_pusbk = dummy_pusbk->next;
                            }

                            if (dummy_pusbk == NULL)
                            {
                            	fprintf(stderr, "USBK List is not malloced.");
                            	return 0;
                            }

                            memset (dummy_pusbk, 0, sizeof(*dummy_pusbk));

                            strcpy(dummy_pusbk->dev_name, udev_device_get_devnode(dev));
                            strcpy(dummy_pusbk->vendor_id, udev_device_get_sysattr_value(dev_usb,"idVendor"));
                            strcpy(dummy_pusbk->product_id, udev_device_get_sysattr_value(dev_usb, "idProduct"));
                            strcpy(dummy_pusbk->manufacturer, udev_device_get_sysattr_value(dev_usb,"manufacturer"));
                            strcpy(dummy_pusbk->product, udev_device_get_sysattr_value(dev_usb,"product"));
                            strcpy(dummy_pusbk->serial, udev_device_get_sysattr_value(dev_usb, "serial"));
                            usbk_get_backdisk(dummy_pusbk);
                        }
                    }
                }
            }

        }
        //udev_device_unref(dev_scsi);
        //udev_device_unref(dev_usb);
        udev_device_unref(dev);
    }
    // Free the enumerator object
    udev_enumerate_unref(enumerate);

    udev_unref(udev);
    return 0;
}

void usbk_list_devices_release(USBK_List** pusbk)
{
    USBK_List *dummy_usbk;
    USBK_List *next_dummy_usbk;

    for (dummy_usbk = *pusbk;dummy_usbk!=NULL;dummy_usbk=next_dummy_usbk)
    {
        next_dummy_usbk = dummy_usbk->next;
        free(next_dummy_usbk);
    }
    *pusbk = NULL;
}


int usbk_get_scsi_dev_info(USBK_T *usbk) {
    send_scsi_command(usbk, (unsigned char*) &usbk->info, GET_DEV_INFO, sizeof(t_UIP_DEVINFO), READ_SCSI);
    //memcpy((char*) &usbk->info, buffer, sizeof(UI_DEVINFO_T));
    return 0;
}

