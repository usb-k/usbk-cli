/*
 * @file usbk_scsi.cpp
 *
 * Copyright (C) 2009, 2010
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

#define _USBK_SCSI_H
#include "usbk_scsi.h"

int send_scsi_command(t_usbk *usbk, unsigned char *buff, int cmd_index, int len, char rw){
	  short int cmdlen = sizeof(st_cmd);
	  st_packet packet;
	  unsigned char buffer[512];

	  if(usbk_open(usbk->dev) == 1){
	    printf("Hata: device adi yanlis veya root yetkisine sahip degilsiniz.\n");
	    exit(1);
	  }

	  memset(buffer,0,sizeof(buffer));
	  if (rw == WRITE_SCSI){
		  if(len != 0)
			  memcpy(buffer, buff, len);
	  }

	  packet.cmd = scsi_cmd[cmd_index-1];
	  packet.cmdlen = cmdlen;
	  packet.cmddir = (rw == WRITE_SCSI) ? OUTDIR : INDIR;
	  packet.data = buffer;
	  packet.datalen = 512;

	  if(usbk_sg_tansfer(&packet) == 1){
	    printf("Hata: SCSI okuma hatasi.\n");
	    exit(1);
	  }

	  if (rw == READ_SCSI){
		  if(len != 0)
			  memcpy(buff, buffer, len);
	  }

	  usbk_close();
	  return 0;
}

