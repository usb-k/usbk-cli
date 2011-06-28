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

//PRIVATE HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winioctl.h>
#include "../general.h"
#include "../uip.h"
#include "usbk_sg_ctl.h"




//PRIVATE DEFINES
//-ERROR MESSAGES
#define msgUSBK_SG_ERR_NOT_DEVICE          "Not device!\n"
#define msgUSBK_SG_ERR_IOCTL_SCSI           "Error in IOCTL_SCSI\n"
#define msgUSBK_SG_ERR_COMMAND_FAIL         "Command failed!\n"

//PRIVATE VARIABLES;
static HANDLE sg_fd;

//PUBLIC FUNCTIONS

int usbk_sg_open(const char* DevicePath)
{
    int rtn = rtnUSBK_SG_PASS;

    char actDevPath[] = "\\\\.\\X:";

    if(!DevicePath)       return rtnUSBK_SG_ERR_NOT_DEVICE;
    actDevPath[4] = DevicePath[0];

    sg_fd = CreateFileA( actDevPath,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if(sg_fd == INVALID_HANDLE_VALUE){
        fprintf(stderr, msgUSBK_SG_ERR_NOT_DEVICE);
        return rtnUSBK_SG_ERR_NOT_DEVICE;
    }
    return rtn;
}

void usbk_sg_close(void)
{
    CloseHandle(sg_fd);
}




int usbk_sg_tansfer(ST_PACKET_T *scsi_packet)
{
    int rtn;
    DWORD dwBytesReturned = 0;
    T_SPDT_SBUF wpacket;
    memset(&wpacket, 0, sizeof(wpacket));


    wpacket.spt.Length = sizeof(wpacket.spt);
    wpacket.spt.PathId = 0; //SCSI_PathId;
    wpacket.spt.TargetId = 1; //SCSI_TargetId;
    wpacket.spt.Lun = 0; //SCSI_Lun;
    memcpy(wpacket.spt.Cdb, scsi_packet->cmd, sizeof(ST_CMD_T));
    wpacket.spt.CdbLength = sizeof(ST_CMD_T);

    wpacket.spt.SenseInfoLength = MAX_SENSE_LEN;
    wpacket.spt.DataIn = (scsi_packet->cmddir == 0x01)?0X01:0X00;

    wpacket.spt.DataBuffer = scsi_packet->data;
    wpacket.spt.DataTransferLength = 512;
    wpacket.spt.TimeOutValue = 5000;
    wpacket.spt.SenseInfoOffset = sizeof(SCSI_PASS_THROUGH_DIRECT);

    rtn = DeviceIoControl( sg_fd, // handle to device
                    IOCTL_SCSI_PASS_THROUGH_DIRECT, // operation
                    &wpacket, // input data buffer
                    sizeof(T_SPDT_SBUF), // size of input data buffer
                    &wpacket, // output data buffer
                    sizeof(T_SPDT_SBUF), // size of output data buffer
                    &dwBytesReturned, // byte count
                    NULL); // overlapped


    if (!rtn)
    {
        fprintf(stderr, msgUSBK_SG_ERR_IOCTL_SCSI);
        fprintf(stderr, "error code: %d\n", GetLastError());
        return rtnUSBK_SG_ERR_IOCTL_SCSI;
    }
    else {
        if(wpacket.spt.ScsiStatus == STATUS_GOOD)           return rtnUSBK_SG_PASS;
        else if(wpacket.spt.ScsiStatus == STATUS_CHKCOND)   return rtnUSBK_SG_ERR_COMMAND_FAIL;
        else                                                return rtnUSBK_SG_ERR_COMMAND_FAIL;
    }
}

/*
// FOR DEBUB
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

