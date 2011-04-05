/*
 * @file usbk_sg_ctl.cpp
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

#include "usbk_sg_ctl.h"



// Error Messages
#define msgUSBK_SG_ERR_NOT_DEVICE          "Not device!\n"
#define msgUSBK_SG_ERR_IOCTL_SCSI           "Error in IOCTL_SCSI\n"
#define msgUSBK_SG_ERR_COMMAND_FAIL         "Command failed!\n"


static int sg_fd;

int usbk_sg_open(const char* DevicePath)
{
    int rtn = rtnUSBK_SG_PASS;

    sg_fd  = open(DevicePath , O_RDWR);

    if (sg_fd < 0) {
        fprintf(stderr, msgUSBK_SG_ERR_NOT_DEVICE);
        rtn = rtnUSBK_SG_ERR_NOT_DEVICE;
    }

    return rtn;
}

void usbk_sg_close(void)
{
    close(sg_fd);
}

int usbk_sg_tansfer(ST_PACKET_T *scsi_packet)
{
    sg_io_hdr_t  io_hdr;
    int i=0;
    int j=0;
    unsigned char  sense_buffer[32];

    memset(sense_buffer, 0, sizeof(sense_buffer));
    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));

    io_hdr.interface_id = 'S';

    if (scsi_packet->cmddir == 0x01) {
        io_hdr.dxfer_direction =  SG_DXFER_FROM_DEV ; //packet.cmddir;
    } else if (scsi_packet->cmddir == 0x00) {
        io_hdr.dxfer_direction =  SG_DXFER_TO_DEV ; //packet.cmddir;
    }

    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.sbp = sense_buffer;
    io_hdr.dxfer_len = scsi_packet->datalen;
    io_hdr.dxferp = scsi_packet->data;
    io_hdr.cmd_len = scsi_packet->cmdlen;
    io_hdr.cmdp = (unsigned char*) (scsi_packet->cmd);
    io_hdr.timeout = 1000;

    if (ioctl(sg_fd, SG_IO, &io_hdr) < 0) {
        fprintf(stderr, msgUSBK_SG_ERR_IOCTL_SCSI);
        return rtnUSBK_SG_ERR_IOCTL_SCSI;
    } else if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {

        if (io_hdr.sb_len_wr > 0) {
            i=0;
            printf("Sense data: \n");
            while(i < io_hdr.sb_len_wr) {
                printf("0x%02X ", sense_buffer[i]);
                i++;

                if(i%16 == 0) {
                    for(j=0; j<16; j++) {
                        printf("%c", sense_buffer[i-16+j]);
                    }

                    printf("\n");
                } else if(i%8 == 0)
                    printf("- ");
            }

            for (j=0; j<i%16; j++) {
                printf("%c", sense_buffer[i-i%16+j]);
            }

            printf("\n");
        }

        scsi_packet->datalen = 0;
        fprintf(stderr, msgUSBK_SG_ERR_COMMAND_FAIL);
        return rtnUSBK_SG_ERR_COMMAND_FAIL;
    }

    //printf("ByteReturn :: %x\n", io_hdr.dxfer_len);
    scsi_packet->datalen = io_hdr.dxfer_len;

    return rtnUSBK_SG_PASS;
}
/*
void usbk_sg_show_packet(ST_PACKET_T *scsi_packet)
{
    int i=0;
    int j=0;

    if(scsi_packet->datalen) {
        printf("Received/Sent data: \n");

        while(i < scsi_packet->datalen) {
            printf("0x%02X ", scsi_packet->data[i]);
            i++;

            if(i%16 == 0) {
                for(j=0; j<16; j++)
                    printf("%c", scsi_packet->data[i-16+j]);

                printf("\n");
            } else if(i%8 == 0)
                printf("- ");
        }

        for (j=0; j<i%16; j++)
            printf("%c", scsi_packet->data[i-i%16+j]);

        printf("\n\n");
    }
}
*/
