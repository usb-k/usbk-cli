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
#define VERSION_LEN        2
#define REVISION_LEN       2
#define PASSWD_LEN        16
#define SIZE_KEYNAME      12

//direction
#define INDIR             0X01
#define OUTDIR            0X00

// packed structures required or not
#ifdef PACKAGED_STRUCT
#  define ATTR_PACKED __attribute__((__packed__))
#else
#  define ATTR_PACKED
#endif

typedef char ui_status_t;
typedef char ui_nbretry_t;
typedef char ui_msg_t;
typedef char ui_msgcode_t;

typedef struct __UI_STATUSALL ATTR_PACKED {
    ui_status_t     lastopt;
    ui_nbretry_t    retry_num;
    ui_msg_t        msg;
    ui_msgcode_t    msg_code;
} UI_STATUSALL_T;

typedef char UI_AESNB_T;
typedef char t_UI_OPTION;

typedef struct __AESKEY {
  char key[16];
} AESKEY_T;

typedef union __UI_DEVNAME ATTR_PACKED {
    char c[SIZE_DEVNAME];
} UI_DEVNAME_T;

typedef union __UI_KEYNAME {
    char c[SIZE_KEYNAME];
} UI_KEYNAME_T;

typedef union __UI_PASSWD {
      char  u8 [PASSWD_LEN];
} UI_PASSWD_T;

typedef union __UI_FIRMVER {
    char u8[16];
    struct ATTR_PACKED {
        char version[VERSION_LEN];
        char revision[REVISION_LEN];
        char reserved[12];
    };
} UI_FIRMVER_T;

typedef struct __UI_DEVINFO ATTR_PACKED {
    UI_FIRMVER_T    firmver;
    char            serial[SIDLEN];
    char            login;
    UI_DEVNAME_T    devname;
    char            backdisk;
    ui_nbretry_t     retry_num;
    UI_AESNB_T        multikeycap;
    UI_AESNB_T        autologin_key_num;
    UI_AESNB_T        currentkeynum;
    UI_KEYNAME_T    keyname[NB_AESKEY];
} UI_DEVINFO_T;

//////////////////////////////////////////////////
// SCSI Data Buffer Structure
//////////////////////////////////////////////////
typedef struct __UI_ACTIVATE ATTR_PACKED {
    UI_PASSWD_T     parola;
    UI_AESNB_T        SelectedKeyNo;
} UI_ACTIVATE_T;

typedef struct __UI_SET_DEV_NAME ATTR_PACKED {
    UI_PASSWD_T     parola;
    UI_DEVNAME_T     devname;
} UI_SET_DEV_NAME_T;

typedef struct __UI_SET_AES_KEY ATTR_PACKED {
    UI_PASSWD_T         parola;
    UI_AESNB_T            aes_key_no;
    AESKEY_T                 aes_key;
    UI_KEYNAME_T        aes_name;
    char                nameonly;
} UI_SET_AES_KEY_T;

typedef struct __UI_SET_AUTO_ACTIVATE ATTR_PACKED {
    UI_PASSWD_T         parola;
    UI_AESNB_T            SelectedKeyNo;
    t_UI_OPTION            AutoActivate;
} UI_SET_AUTO_ACTIVATE_T;

typedef struct __UI_CHANGE_PASS ATTR_PACKED {
    UI_PASSWD_T  old_password;
    UI_PASSWD_T  new_password;
    UI_AESNB_T   aes_key_no; // not using for now
} UI_CHANGE_PASS_T;

#endif // SCSISTRUCTURE_H_

