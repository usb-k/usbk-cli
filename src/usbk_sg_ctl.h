/*
 * @file usbk_sg_ctl.h
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
 *
 */


#ifdef _USBK_SG_CTL_H

#include "general.h"
#include <scsi/sg.h>

int usbk_open(char* DevicePath);
void usbk_close(void);
int usbk_sg_tansfer(st_packet *scsi_packet);
void usbk_sg_show_packet(st_packet *scsi_packet);

#else

extern int usbk_open(char* DevicePath);
extern void usbk_close(void);
extern int usbk_sg_tansfer(st_packet *scsi_packet);
extern void usbk_sg_show_packet(st_packet *scsi_packet);

#endif
