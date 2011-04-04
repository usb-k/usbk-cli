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
#include "usbk_sg_ctl.h"

// Vender Specific SCSI Command

ST_CMD_T scsi_cmd[][10] = {
  {0xFE, 0x00, GET_STATUS,      0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, GET_DEV_INFO,    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, ACTIVATE_KEY,    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, DEACTIVATE_KEY,  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, CHANGE_PASS,     0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, SET_DEV_NAME,    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, SET_AUTO_ACTIVE, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, SET_KEY,         0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00},
  {0xFE, 0x00, GENERATE_KEY,    0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00}
};


// Error Messages
#define msgUSBK_SCSI_OPEN_FAIL      "Hata: device adi yanlis veya root yetkisine sahip degilsiniz.\n"
#define msgUSBK_SCSI_TRANSFER_FAIL  "Hata: SCSI okuma hatasi.\n"


int send_scsi_command(USBK* usbk, unsigned char *buff, int cmd_index, int len, char rw) {
    short int cmdlen = sizeof(ST_CMD_T);
    ST_PACKET_T packet;
    unsigned char buffer[512];

    if (usbk_sg_open(usbk->dev_path) < 0) {
        fprintf(stderr, msgUSBK_SCSI_OPEN_FAIL);
        return rtnUSBK_SCSI_OPEN_FAIL;
    }

    memset(buffer, 0, sizeof(buffer));

    if (rw == WRITE_SCSI) {
        if (len != 0) {
            memcpy(buffer, buff, len);
        }
    }

    packet.cmd = scsi_cmd[cmd_index - 1];
    packet.cmdlen = cmdlen;
    packet.cmddir = (rw == WRITE_SCSI) ? OUTDIR : INDIR;
    packet.data = buffer;
    packet.datalen = 512;

    if (usbk_sg_tansfer(&packet) < 0) {
        fprintf(stderr, msgUSBK_SCSI_TRANSFER_FAIL);
        return rtnUSBK_SCSI_TRANSFER_FAIL;
    }

    if (rw == READ_SCSI) {
        if (len != 0) {
            memcpy(buff, buffer, len);
        }
    }
    usbk_sg_close();
    return rtnUSBK_SCSI_PASS;
}
