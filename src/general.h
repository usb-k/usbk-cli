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
 * General headers and definitions of abcde.
 *
 */

#ifndef GENERAL_H_
#define GENERAL_H_

#define U8 unsigned char
#define U16 unsigned int
#define U32 unsigned long

//direction
#define INDIR             0X01
#define OUTDIR            0X00
#define NB_AESKEY         3


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "uip.h"

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
} ST_CMD_T;

typedef struct __ST_PACKET {
    int  sg_fd;
    char cmdtype;
    int  cmddir;
    ST_CMD_T      *cmd;
    unsigned int   cmdlen;
    unsigned char *data;
    unsigned int   datalen;
} ST_PACKET_T;


#endif // GENERAL_H_

