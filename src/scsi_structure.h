/*
 * @file scsi_structure.h
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
 * This header contains definitions of usbk crytographic bridge SCSI structure.
 *
 */

#ifndef SCSISTRUCTURE_H_
#define SCSISTRUCTURE_H_

#define SIDLEN            6
#define NB_AESKEY         3

#define SIZE_DEVNAME      11
#define VERSIONLEN        2
#define REVISIONLEN       2
#define PAROLA_LEN        16
#define SIZE_KEYNAME      12

//direction
#define INDIR             0X01
#define OUTDIR            0X00

typedef char t_UI_STATUS;
typedef char t_UI_NBRETRY;
typedef char t_UI_MSG;
typedef char t_UI_MSGCODE;

typedef struct { //__attribute__((__packed__))
    t_UI_STATUS        lastopt;
    t_UI_NBRETRY    retry_num;
    t_UI_MSG        msg;
    t_UI_MSGCODE    msg_code;
} t_UI_STATUSALL;

typedef char t_UI_AESNB;
typedef char t_UI_OPTION;

typedef struct {
  char key[16];
} tAESKEY;

typedef union //__attribute__((__packed__))
{
    char c[SIZE_DEVNAME];
} t_UI_DEVNAME;

typedef union
{
    char c[SIZE_KEYNAME];
} t_UI_KEYNAME;

typedef union
{
      char  u8 [PAROLA_LEN];
} t_UI_PAROLA;

typedef union
{
    char u8[16];
    struct //__attribute__((__packed__))
    {
        char version[VERSIONLEN];
        char revision[REVISIONLEN];
        char reserved[12];
    };
} t_UI_FIRMVER;

typedef struct //__attribute__((__packed__))
{
    t_UI_FIRMVER    firmver;
    char            serial[SIDLEN];
    char            login;
    t_UI_DEVNAME    devname;
    char            backdisk;
    t_UI_NBRETRY     retry_num;
    t_UI_AESNB        multikeycap;
    t_UI_AESNB        autologin_key_num;
    t_UI_AESNB        currentkeynum;
    t_UI_KEYNAME    keyname[NB_AESKEY];
}t_UI_DEVINFO;

//////////////////////////////////////////////////
// SCSI Data Buffer Structure
//////////////////////////////////////////////////
typedef struct //__attribute__((__packed__))
{
    t_UI_PAROLA     parola;
    t_UI_AESNB        SelectedKeyNo;
}t_UI_ACTIVATE;

typedef struct //__attribute__((__packed__))
{
    t_UI_PAROLA     parola;
    t_UI_DEVNAME     devname;
}t_UI_SET_DEV_NAME;

typedef struct //__attribute__((__packed__))
{
    t_UI_PAROLA         parola;
    t_UI_AESNB            aes_key_no;
    tAESKEY                 aes_key;
    t_UI_KEYNAME        aes_name;
    char                nameonly;
}t_UI_SET_AES_KEY;

typedef struct //__attribute__((__packed__))
{
    t_UI_PAROLA         parola;
    t_UI_AESNB            SelectedKeyNo;
    t_UI_OPTION            AutoActivate;
}t_UI_SET_AUTO_ACTIVATE;

typedef struct //__attribute__((__packed__))
{
    t_UI_PAROLA         old_password;
    t_UI_PAROLA         new_password;
    t_UI_AESNB            aes_key_no; // simdilik kullanilmiyor
}t_UI_CHANGE_PASS;

#endif // SCSISTRUCTURE_H_

