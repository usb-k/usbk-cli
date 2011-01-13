/*
 * @file general.h
 *
 * Copyright (C) 2010
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * See http://www.gnu.org/licenses/ for more information
 *
 ****************************************************************************
 * General headers and definitions of usbk.
 *
 */

#ifndef _GENERAL_H
#define _GENERAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "scsi_structure.h"

//Good & Bad
#define GOOD    0x01
#define BAD     0x00

//Screen Message

//Msgs of SCSI_CMD_Sender
#define MSGINIT              "Init Processing... \n"
#define MSGINITOK            "Init OK! \n"

//Msgs of UserInterface
#define MSGPATHENTERY        "Enter sg Number (/dev/sg<number>)  ::::  "
#define DPATHLEN             256
#define DPATHLENEX           3
#define NOITEM               "Your Selection is not Available...\n"
#define MSGCLOSEDEV          "System Closing...."

//Msgs of DeviceControl
#define NOTDEVICE            "Not Device!!! \n"
#define MSGERRIOCTL          "Error in IOCTL_SCSI\n"
#define MSGERRCODE           "Error Code :: %x \n"
#define MSGRETBYTE           "ByteReturn :: %x\n"
#define CMDFAIL		         "Command failed!\n"
#define MSGSENSEDATA         "Sense Data: \n"
#define MSGSENSELEN          "Sense Len :: %x\n"
#define UNHANDLEDSTATUS      "Command sent but returned with an unhandled status code: %02X\n"
#define MISSING_PARAMETER    "Missing parameter"
#define WARNING              "Warning:"

typedef struct _st_cmd
{
    char opcode;
    char lun;
    char v_opcode;
    char dummy1;
    char dummy2;
    char dummy3;
    char reserved;
    char len1;
    char len0;
    char cntrl;
}st_cmd;

typedef struct _st_packet
{
    int		sg_fd;
    char    cmdtype;
    int     cmddir;
    st_cmd *cmd;
    unsigned int     cmdlen;
    unsigned char   *data;
    unsigned int     datalen;
}st_packet;

typedef struct _t_usbk
{
    char dev[SIZE_DEVNAME];
    char sg_dev[SIZE_DEVNAME];
    char vendor[32];
    char model[32];
    char rev[32];
    t_UI_DEVINFO info;
    char backdisk_dev[SIZE_DEVNAME];
    t_UI_STATUSALL status;
}t_usbk;

#endif
