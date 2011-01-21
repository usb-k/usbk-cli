/*
 * @file usbk_scsi.cpp
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
 * Main function of USB-K crypro device
 *
 */

#include "usbk_scsi.h"

st_cmd scsi_cmd[][10] = {
        {0xFE, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 0 GET_STATUS
        {0xFE, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 1 GET_DEV_INFO
        {0xFE, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 2
        {0xFE, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 3
        {0xFE, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 4
        {0xFE, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 5
        {0xFE, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 6
        {0xFE, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 7
        {0xFE, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 8
        {0xFE, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}, // 9
};

int send_scsi_command(t_usbk *usbk, unsigned char *buff, int cmd_index, int len, char rw)
{
    short int cmdlen = sizeof(st_cmd);
    st_packet packet;
    unsigned char buffer[512];

    if(usbk_open(usbk->dev) == 1) {
        printf("Hata: device adi yanlis veya root yetkisine sahip degilsiniz.\n");
        exit(1);
    }

    memset(buffer,0,sizeof(buffer));

    if (rw == WRITE_SCSI) {
        if(len != 0)
            memcpy(buffer, buff, len);
    }

    packet.cmd = scsi_cmd[cmd_index-1];
    packet.cmdlen = cmdlen;
    packet.cmddir = (rw == WRITE_SCSI) ? OUTDIR : INDIR;
    packet.data = buffer;
    packet.datalen = 512;

    if(usbk_sg_tansfer(&packet) == 1) {
        printf("Hata: SCSI okuma hatasi.\n");
        exit(1);
    }

    if (rw == READ_SCSI) {
        if(len != 0)
            memcpy(buff, buffer, len);
    }

    usbk_close();

    return 0;
}

