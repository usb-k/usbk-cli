/*
 * @file general.h
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
 * General headers and definitions of usbk.
 *
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "scsi_structure.h"

#define MAX_STR_LEN          32

typedef struct __ST_CMD {
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
} st_cmd;

typedef struct __ST_PACKET {
    int     sg_fd;
    char    cmdtype;
    int     cmddir;
    st_cmd *cmd;
    unsigned int   cmdlen;
    unsigned char *data;
    unsigned int   datalen;
} st_packet;

typedef struct __USBK {
    char dev[SIZE_DEVNAME];
    char sg_dev[SIZE_DEVNAME];
    char vendor[MAX_STR_LEN];
    char model[MAX_STR_LEN];
    char rev[MAX_STR_LEN];
    t_UI_DEVINFO info;
    char backdisk_dev[SIZE_DEVNAME];
    t_UI_STATUSALL status;
} t_usbk;

#endif // GENERAL_H_

