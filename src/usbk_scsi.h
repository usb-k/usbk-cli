/*
 * @file usbk_scsi.h
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

#ifndef USBK_SCSI_H_
#define USBK_SCSI_H_

// SCSI CMD DIRECTION
#define WRITE_SCSI           OUTDIR
#define READ_SCSI            INDIR

// SCSI CMD TYPES
#define GET_STATUS           0x01
#define GET_DEV_INFO         0x02
#define ACTIVATE_KEY         0x03
#define DEACTIVATE_KEY       0x04
#define CHANGE_PASS          0x05
#define SET_DEV_NAME         0x06
#define SET_AUTO_ACTIVE      0x07
#define SET_KEY              0x08
#define GENERATE_KEY         0x09
#define GET_PUBLIC_KEY       0x0A
#define EXIT                 0x30

#include "general.h"
#include "usbk_sg_ctl.h"

extern int send_scsi_command(t_usbk *usbk, unsigned char *buff, int cmd_index, int len, char rw);
extern st_cmd scsi_cmd[][10];

#endif

