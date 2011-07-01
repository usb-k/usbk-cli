/*
 * @file libusbk.cpp
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
 * This is a utility program for listing SCSI devices and hosts (HBAs)
 * in the Linux operating system. It is applicable to kernel versions
 * 2.6.1 and greater.
 *
 */

//PRIVATE HEADERS
#include <sstream>

#if  defined(__linux__)
#include <libudev.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "general.h"
#include "uip.h"

#include "libusbk.h"
#include "usbk_scsi.h"

using namespace std;

#if  defined(__linux__)
    #define DBG_INFO(x, ...) {if(debug_enable) {fprintf(stderr, "\033[0;32;40m%s(%d):%s:\033[0;37;40m" x "\033[0;37;40m\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_ERROR(x, ...) {if(debug_enable) {fprintf(stderr, "\033[1;3;31m%s(%d):%s:\033[0;37;40m" x "\033[0;37;40m\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_RETURN_STRING(x) {if (x!=0) DBG_ERROR("%s", USBK_RTN_ERRORS_STRING[x]);}
    #define DBG_LASTOPR_STRING(x) {if (x!=0) {DBG_ERROR("last opration status: %s", lastopr_string[x].rtrn_string);} \
                                   else {DBG_INFO("last opration status: %s", lastopr_string[x].rtrn_string);}}

#elif defined(WIN32)
    #define DBG_INFO(x, ...) {if(debug_enable) {fprintf(stderr, "%s(%d):%s:" x "\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_ERROR(x, ...) {if(debug_enable) {fprintf(stderr, "%s(%d):%s:" x "\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_RETURN_STRING(x) {if (x!=0) DBG_ERROR("%s", USBK_RTN_ERRORS_STRING[x]);}
    #define DBG_LASTOPR_STRING(x) {if (x!=0) {DBG_ERROR("last opration status: %s", lastopr_string[x].rtrn_string);} \
                                   else {DBG_INFO("last opration status: %s", lastopr_string[x].rtrn_string);}}

#endif

typedef struct __RTRN_STRING {
    int rtrn;
    const char *rtrn_string;
} RTRN_STRING;

RTRN_STRING lastopr_string[] ={
    {USBK_LO_PASS,                  "Ok"                     },
    {USBK_LO_GEN_FAIL,              "fail"                   },
    {USBK_LO_FAILED_PASS,           "wrong password"         },
    {USBK_LO_FABRIC_RESET,          "Error, FABRIC_RESET"    },
    {USBK_LO_USBK_UNPLUGING,        "n/a"                    },
    {USBK_LO_INVALID_KEYNO,         "invalid key number"     },
    {USBK_LO_INVALID_KEYSIZE,       "invalid key size"       },
    {USBK_LO_INVALID_DEVICELABEL,   "invalid device label"   },
    {USBK_LO_INVALID_PASS,          "n/a"                    },
    {USBK_LO_INVALID_NEWPASS,       "invalid new password"   },
    {USBK_LO_STATE_ERROR,           "state error"            },
    {USBK_LO_SCSI_ERROR,            "scsi error"             },
    {USBK_LO_UNSUPPORTED_USBK,      "unsupported usbk"       },
    {USBK_LO_INVALID_KEY,           "invalid key"            },
#if  defined(__linux__)
    {USBK_LO_UDEV_ERROR,            "udev error"             },
#endif
    {USBK_LO_MEM_ERROR,             "memory allocation error"}
};

typedef struct __LIBUSBK_SUPPORTED_MODELS {
    const char* model;
    bool support;
} LIBUSBK_SUPPORTED_MODELS;

typedef struct __LIBUSBK_SUPPORTED_VERSIONS {
    int major_version;
    int minor_version;
    LIBUSBK_SUPPORTED_MODELS* models;
} LIBUSBK_SUPPORTED_VERSIONS;

typedef struct __LIBUSBK_SUPPORTED_PRODUCTS {
    const char* product;
    LIBUSBK_SUPPORTED_VERSIONS* versions;
} LIBUSBK_SUPPORTED_PRODUCTS;

LIBUSBK_SUPPORTED_MODELS models_1_1[] = {
    { "A101", true },
    { "A103", true },
    {NULL, false }
};

LIBUSBK_SUPPORTED_VERSIONS versions_1[] = {
     { 2, 5, models_1_1 },
     { 0, 0, NULL }
};

LIBUSBK_SUPPORTED_PRODUCTS products[] = {
     { "USBK CryptoBridge 2.0", versions_1 },
     { "USBK CryptoBridge", versions_1 },
     { NULL, NULL }
};

//-DEFINES FOR FINDING USBK AND BACKDISK
#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT_A103    "a103"
#define USBK_USB_IDPRODUCT_A101    "a101"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"


/*
#if  defined(__linux__)
typedef enum __attribute__((__packed__)) __KEYSIZE
#elif defined(WIN32)
typedef enum __KEYSIZE// : char
#else
#error must define environment
#endif
{
    KEYSIZE_NULL = 0,
    KEYSIZE_128BIT = 1,
    KEYSIZE_192BIT = 2,
    KEYSIZE_256BIT = 3,
}KEYSIZE;

typedef enum __USBK_DS
{
    USBK_DS_ACTIVATE                = 1,
    USBK_DS_ACTIVATE_WITH_BACKDISK  = 2,
    USBK_DS_DEACTIVATE              = 3,
    USBK_DS_FABRIC_DEFAULT          = 4,
    USBK_DS_MUST_REMOVE             = 5,
}USBK_DS;

typedef enum __USBK_LASTOPR
{
    USBK_LO_PASS = 0,
    USBK_LO_GEN_FAIL = 1,
    USBK_LO_FAILED_PASS =2,
    USBK_LO_FABRIC_RESET = 3,
    USBK_LO_USBK_UNPLUGING = 4,
    USBK_LO_INVALID_KEYNO = 5,
    USBK_LO_INVALID_KEYSIZE = 6,
    USBK_LO_INVALID_DEVICELABEL = 7,
    USBK_LO_INVALID_PASS = 8,
    USBK_LO_INVALID_NEWPASS = 9,

    USBK_LO_STATE_ERROR = 10,
    USBK_LO_SCSI_ERROR = 11,
    USBK_LO_UNSUPPORTED_USBK = 12,
    USBK_LO_INVALID_KEY = 13,

    USBK_LO_UDEV_ERROR = 14,
    USBK_LO_MEM_ERROR = 15,
}LIBUSBK_LASTOPR;
*/

typedef struct __USBK_INFO {
    char                *dev;
    char                *dev_path;
    char                *backdisk;
    char                *backdisk_path;

    bool                supported;

    char                *product;
    char                *model;
    char                *serial;
    char                *usb_serial;
    char                *firmware_ver;
    int                 multikey_cap;
    char                *dev_label;
    USBK_DS             dev_state;
    int                 current_key;
    int                 autoact_keyno;
    int                 retry_num;
    char                **key_names;
} USBK_INFO;

typedef struct __USBK
{
    USBK_INFO info;
    int lastopr;
}USBK;


static bool debug_enable = false;


static int getudevinfo(USBK* usbk, const char *device);
static int getudevbackdisk(USBK* usbk);
static int commandstatus(USBK* usbk);

static int convertkey_decimal2hex(uint8_t *key_hex, const char* key_decimal, KEYSIZE keysize);
static int convertkey_text2hex(uint8_t *key_hex, const char* key_text, KEYSIZE keysize);
static unsigned int keysize_byte(KEYSIZE keysize);


USBK* usbk_new(const char* dev) {
    int i;
    int rtn = 0;

    USBK* usbk = (USBK*)calloc(1, sizeof(USBK));
    if (usbk == NULL)
    {
        usbk->lastopr = USBK_LO_MEM_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return NULL;
    }

#if  defined(__linux__)
    // get information about usbk by udev
    rtn = getudevinfo(usbk, dev);
    if (rtn < 0) {
        free(usbk);
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return NULL;
    }
#elif  defined(WIN32)
	usbk->info.dev = strdup(dev);
	usbk->info.dev_path = strdup(dev);
#endif

    t_UIP_DEVINFO usbk_info;
    rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &usbk_info, GET_DEV_INFO, sizeof(usbk_info), READ_SCSI);
    if (rtn < 0) {
        free(usbk);
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return NULL;
    }

    usbk->info.product = strdup(usbk_info.product.s);
    usbk->info.model = strdup(usbk_info.model.s);
    usbk->info.firmware_ver = strdup(usbk_info.firmware_ver.s);

    //checksupported();
    usbk->info.supported = true;

    if (usbk->info.supported == true) {
        usbk->info.multikey_cap = usbk_info.multikeycap;
        usbk->info.current_key = usbk_info.current_keyno;
        usbk->info.autoact_keyno = usbk_info.autoactivate_keyno;
        usbk->info.retry_num = usbk_info.retry_num;
        usbk->info.dev_state = (USBK_DS) usbk_info.devstate.me;
        usbk->info.dev_label = strdup(usbk_info.devlabel.s);
        usbk->info.serial = (char*) calloc((sizeof(usbk_info.serial) * 2) + 2, sizeof(char));
        for (i = 0; i < 15; i++) {
            sprintf((usbk->info.serial + i * 2), "%2.2X", usbk_info.serial.u8[i]);
        }

        usbk->info.key_names = (char**) calloc(usbk->info.multikey_cap, sizeof(char*));
        for (i = 0; i < usbk->info.multikey_cap; i++) {
            usbk->info.key_names[i] = strdup(usbk_info.keyname[i].s);
        }
    }

    usbk->lastopr = USBK_LO_PASS;
    DBG_LASTOPR_STRING(usbk->lastopr);
    return usbk;
}

int usbk_release(USBK* usbk)
{
    int i;

    free(usbk->info.dev);
    free(usbk->info.dev_path);
    free(usbk->info.backdisk);
    free(usbk->info.backdisk_path);
    free(usbk->info.product);
    free(usbk->info.model);
    free(usbk->info.serial);
    free(usbk->info.usb_serial);
    free(usbk->info.firmware_ver);
    free(usbk->info.dev_label);

    for (i = 0; i < usbk->info.multikey_cap; i++) {
        free(usbk->info.key_names[i]);
    }
    free(usbk->info.key_names);
    free(usbk);

    return 0;
}

int usbk_activatekey(USBK* usbk, const char* password, uint8_t key_no) {
    int rtn;
    t_UIP_ACTIVATE activate;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    if (key_no > usbk->info.multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&activate, 0, sizeof(activate));
        strcpy(activate.password.s, password);
        activate.keyno = key_no;
        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char *) &activate, ACTIVATE_KEY, sizeof(activate), WRITE_SCSI);
        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        printf("state error");
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_deactivatekey(USBK* usbk) {
    int rtn;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) NULL, DEACTIVATE_KEY, 0, WRITE_SCSI);
        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }

        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_DEACTIVATE:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_changepassword(USBK* usbk, const char* old_pass, const char* new_pass) {
    int rtn;
    t_UIP_CHPASS chpass;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_DEACTIVATE:
    case USBK_DS_FABRIC_DEFAULT:
        memset(&chpass, 0, sizeof(chpass));
        if (new_pass != NULL) {
            strncpy(chpass.new_password.s, new_pass, sizeof(chpass.new_password.s));
        }
        if (old_pass != NULL) {
            strncpy(chpass.old_password.s, old_pass, sizeof(chpass.old_password.s));
        }

        DBG_INFO("old : %s",chpass.old_password.s);
        DBG_INFO("new : %s",chpass.new_password.s);


        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &chpass, CHANGE_PASS, sizeof(chpass), WRITE_SCSI);
        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_setdevicelabel(USBK* usbk, const char* pass, const char* device_label) {
    int rtn;
    t_UIP_SETDEVICELABEL devlabel;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&devlabel, 0, sizeof(devlabel));
        strncpy(devlabel.password.s, pass, sizeof(devlabel.password.s));
        strncpy(devlabel.devlabel.s, device_label, sizeof(devlabel.devlabel.s));
        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &devlabel, SET_DEV_NAME, sizeof(devlabel), WRITE_SCSI);

        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_setkey_keyname(USBK* usbk, const char *pass, int key_no, const char* key_name, KEYSIZE key_size, const uint8_t* key) {
    int rtn;
    t_UIP_SETKEY setkey;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    if (key_no > usbk->info.multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&setkey, 0, sizeof(setkey));
        strncpy(setkey.password.s, pass, sizeof(setkey.password.s));
        setkey.keyno = key_no;

        if (key_name != NULL) {
            strncpy(setkey.keyname.s, key_name, sizeof(setkey.keyname.s));
        } else {
            strncpy(setkey.keyname.s, usbk->info.key_names[key_no - 1], sizeof(setkey.keyname.s));
        }

        if ((key_size != KEYSIZE_NULL) && (key != NULL)) {
            setkey.options.me = SETKEY_NAME_AND_KEY;
            setkey.keysize.me = (e_UIP_KEYSIZE) key_size;
            memcpy(setkey.key.u8, key, keysize_byte(key_size));
        } else {
            setkey.options.me = SETKEY_NAMEONLY;
        }

        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &setkey, SET_KEY, sizeof(setkey), WRITE_SCSI);
        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_setkey_decimal(USBK* usbk, const char *pass, uint8_t key_no, KEYSIZE key_size, const char* key) {
    int rtn = 0;
    uint8_t key_hex[32];
    usbk->lastopr = convertkey_decimal2hex(key_hex, key, key_size);
    if (rtn == USBK_LO_PASS){
        usbk->lastopr = usbk_setkey_keyname(usbk, pass, key_no, NULL, key_size, key_hex);
    }
    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_setkey_hex(USBK* usbk, const char *pass, uint8_t key_no, KEYSIZE key_size, const uint8_t* key) {
    return usbk_setkey_keyname(usbk, pass, key_no, NULL, key_size, key);
}

int usbk_setkey_text(USBK* usbk, const char *pass, uint8_t key_no, KEYSIZE key_size, const char* key) {
    int rtn = 0;
    uint8_t key_hex[32];

    usbk->lastopr = convertkey_text2hex(key_hex, key, key_size);
    if (rtn == USBK_LO_PASS){
        usbk->lastopr = usbk_setkey_keyname(usbk, pass, key_no, NULL, key_size, key_hex);
    }
    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_setautact(USBK* usbk, const char *pass, int key_no) {
    int rtn;
    t_UIP_SETAUTOACTIVATE autoact;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    if (key_no > usbk->info.multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    switch (usbk->info.dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&autoact, 0, sizeof(autoact));
        strncpy(autoact.password.s, pass, sizeof(autoact.password.s));
        autoact.keyno = key_no;
        rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &autoact, SET_AUTO_ACTIVE, sizeof(autoact), WRITE_SCSI);
        if (rtn < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = commandstatus(usbk);
        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_getrandomkey(USBK* usbk, uint8_t* random_key, KEYSIZE keysize) {
    // request key len is checked. If request key len is larger than generated key, return  warning and random number is generated.
    // The app. will decide whether the key is used or not.
    // Maybe, give the len of generated key.
    // Also, the key is generated as 32 byte. Maybe, the key will be requested len.

    int rtn;
    t_UIP_GENERATEKEY genkey;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    unsigned int keysize_inbyte = keysize_byte(keysize);
    if ((keysize_inbyte == 0) || (keysize_inbyte > keysize_byte(KEYSIZE_256BIT))) {
        usbk->lastopr = USBK_LO_INVALID_KEYSIZE;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;

    }

    rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &genkey, GENERATE_KEY, sizeof(genkey), READ_SCSI);
    if (rtn < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    usbk->lastopr = commandstatus(usbk);
    if (usbk->lastopr == USBK_LO_PASS) {
        memcpy(random_key, genkey.key.u8, keysize_inbyte);
    }

    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

int usbk_refreshusbkinfo(USBK* usbk) {
    int i;
    int rtn = 0;

    if (usbk->info.supported == false) {
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    t_UIP_DEVINFO usbk_info;
    rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &usbk_info, GET_DEV_INFO, sizeof(usbk_info), READ_SCSI);
    if (rtn < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    usbk->info.current_key = usbk_info.current_keyno;
    usbk->info.autoact_keyno = usbk_info.autoactivate_keyno;
    usbk->info.retry_num = usbk_info.retry_num;
    usbk->info.dev_state = (USBK_DS) usbk_info.devstate.me;
    usbk->info.dev_label = strdup(usbk_info.devlabel.s);

    usbk->info.key_names = (char**) calloc(usbk->info.multikey_cap, sizeof(char*));
    for (i = 0; i < usbk->info.multikey_cap; i++) {
        usbk->info.key_names[i] = strdup(usbk_info.keyname[i].s);
    }
    usbk->lastopr = USBK_LO_PASS;
    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

static int commandstatus(USBK* usbk) {
    t_UIP_GETSTATUS status;
    int rtn = USBK_LO_PASS;
    rtn = send_scsi_command(usbk->info.dev_path, (unsigned char*) &status, GET_STATUS, sizeof(t_UIP_GETSTATUS), READ_SCSI);
    if (rtn < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    usbk->info.retry_num = status.retry_num;
    return (status.lastop.me - 1); //todo versiyon checking'ine göre düzleltilecek.
}

static int convertkey_decimal2hex(uint8_t *key_hex, const char* key_decimal, KEYSIZE keysize) {
    // 1) string icinde bosluk karakteri var mi?
    // 2) string icerisindeki '.' karakterinin sayisi 15 mi?
    // 3) '.' karakterlerini ' ' karakterine donustur
    // 4) string icerisinde bosluklar haric decimal olmayan karakter var mi?
    // 5) string to integer islemini yap
    // 6) 16 adet sayı cikartabildin mi?
    // 7) sayılar 0 ile 255 arasinda mi?

    //int ikey[this->keysize_byte(keysize)];
    int ikey[50];
    size_t found;
    string::iterator it;
    unsigned int i;
    string key_decimal_str = key_decimal;

    // string icinde bosluk karakteri var mi?
    if (key_decimal_str.find(" ") != string::npos) return USBK_LO_INVALID_KEY;

    // string icerisindeki '.' karakterinin sayisi 15 mi?
    // '.' karakterlerini ' ' karakterine donustur
    i = 0;
    found = key_decimal_str.find_first_of(".");
    while (found != string::npos) {
        i++;
        key_decimal_str[found] = ' ';
        found = key_decimal_str.find_first_of(".", found + 1);
    }
    if (i != (keysize_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;
    //cout << str << endl;

    // string icerisinde bosluklar haric decimal olmayan karakter var mi?
    i = 0;
    for (it = key_decimal_str.begin(); it < key_decimal_str.end(); it++) {
        if (*it != ' ') {
            if (isdigit(*it) == false) {
                i++;
            }
        }
    }
    if (i != 0) {
        return USBK_LO_INVALID_KEY;
    }

    // string to integer islemini yap
    // sayılar 0 ile 255 arasinda mi?
    istringstream iss(key_decimal_str, istringstream::in);
    i = 0;
    for (unsigned int n = 0; n < keysize_byte(keysize); n++) {
        iss >> ikey[n];

        if (iss.fail() || iss.bad()) return USBK_LO_INVALID_KEY;

        if (iss.eof() && n != (keysize_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;

        if (ikey[n] > 255 || ikey[n] < 0) {
            i++;
        }
        key_hex[n] = ikey[n];
        //cout << n+1 << "." << ikey[n] << endl;
    }
    if (i != 0) {
        return USBK_LO_INVALID_KEY;
    }

    return USBK_LO_PASS;
}

static int convertkey_text2hex(uint8_t *key_hex, const char* key_text, KEYSIZE keysize) {
    string key_string_text = key_text;

    if (key_string_text.size() > keysize_byte(keysize)) {
        return USBK_LO_INVALID_KEY;
    }
    strncpy((char*) key_hex, key_string_text.c_str(), keysize_byte(keysize));
    return USBK_LO_PASS;
}

static unsigned int keysize_byte(KEYSIZE keysize) {
    int size_byte = 0;

    switch (keysize) {
    case KEYSIZE_128:
        size_byte = 16;
        break;
    case KEYSIZE_192:
        size_byte = 24;
        break;
    case KEYSIZE_256:
        size_byte = 32;
        break;
    case KEYSIZE_NULL:
    default:
        size_byte = 0;
        break;
    }
    return size_byte;
}

const char* usbk_get_dev(USBK* usbk){
    return usbk->info.dev;
}
const char* usbk_get_dev_path(USBK* usbk){
    return usbk->info.dev_path;
}
const char* usbk_get_backdisk(USBK* usbk){
    return usbk->info.backdisk;
}
const char* usbk_get_backdisk_path(USBK* usbk){
    return usbk->info.backdisk_path;
}
const char* usbk_get_product(USBK* usbk){
    return usbk->info.product;
}
const char* usbk_get_model(USBK* usbk){
    return usbk->info.model;
}
const char* usbk_get_serial(USBK* usbk){
    return usbk->info.serial;
}
const char* usbk_get_usb_serial(USBK* usbk){
    return usbk->info.usb_serial;
}
const char* usbk_get_firmware_ver(USBK* usbk){
    return usbk->info.firmware_ver;
}
const char* usbk_get_dev_label(USBK* usbk){
    return usbk->info.dev_label;
}
const char* usbk_get_keyname(USBK* usbk, int key_no){
    return usbk->info.key_names[key_no];
}
int usbk_get_lastopr_status(USBK* usbk){
    return usbk->lastopr;
}

bool usbk_check_support(USBK* usbk){
    return usbk->info.supported;
}
int usbk_get_multikeycap(USBK* usbk)
{
    return usbk->info.multikey_cap;
}
int usbk_get_current_keyno(USBK* usbk){
    return usbk->info.current_key;
}
int usbk_get_autoactivation_keyno(USBK* usbk){
    return usbk->info.autoact_keyno;
}
int usbk_get_retry_number(USBK* usbk){
    return usbk->info.retry_num;
}
USBK_DS usbk_get_state(USBK* usbk){
    return usbk->info.dev_state;
}


void usbk_debug_enable(void){
    debug_enable = true;
}
void usbk_debug_disable(void){
    debug_enable = false;
}
void usbk_debug(bool enable){
    debug_enable = enable;
}
bool usbk_debug_check(void)
{
    return debug_enable;
}

const char* usbk_libversion(void){
    return VERSION;
}







#if  defined(__linux__)
static int getudevinfo(USBK* usbk, const char *device) {
    int rtn;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev = NULL;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;
    size_t len;
    const char *str;
    int i;
    struct udev_list_entry *list_entry;

    struct stat statbuf;

    udev = udev_new();
    if (udev == NULL) {
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    char devname[1024];
    sprintf(devname, "%s/%s", udev_get_dev_path(udev), device);

    if (stat(devname, &statbuf) < 0) {
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    } else {
        char type;
        if (S_ISBLK(statbuf.st_mode)) {
            type = 'b';
        } else if (S_ISCHR(statbuf.st_mode)) {
            type = 'c';
        } else {
            udev_unref(udev);
            usbk->lastopr = USBK_LO_UDEV_ERROR;
            DBG_LASTOPR_STRING(usbk->lastopr);
            return (-1) * usbk->lastopr;
        }

        dev = udev_device_new_from_devnum(udev, type, statbuf.st_rdev);

        if (dev == NULL) {
            usbk->lastopr = USBK_LO_UDEV_ERROR;
            DBG_LASTOPR_STRING(usbk->lastopr);
            return (-1) * usbk->lastopr;
        }
    }

    dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");

    usbk->lastopr = USBK_LO_UDEV_ERROR;
    if (dev_usb != NULL) {
        if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
            if ((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0)
                    || (strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)) {
                dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                if (dev_scsi != NULL) {
                    if (strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0) {
                        usbk->info.dev = strdup(udev_device_get_sysname(dev));
                        usbk->info.dev_path = strdup(udev_device_get_devnode(dev));
                        usbk->info.usb_serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));

                        rtn = getudevbackdisk(usbk);
                        if (rtn < 0) {
                            udev_device_unref(dev);
                            udev_unref(udev);
                            usbk->lastopr = USBK_LO_UDEV_ERROR;
                            DBG_LASTOPR_STRING(usbk->lastopr);
                            return (-1) * usbk->lastopr;
                        }
                        usbk->lastopr = USBK_LO_PASS;
                    }
                }
            }
        }
    }
    udev_device_unref(dev);
    udev_unref(udev);
    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}

static int getudevbackdisk(USBK* usbk) {
    int rtn;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi;

    // Create the udev object
    udev = udev_new();
    if (udev == NULL) {
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return (-1) * usbk->lastopr;
    }

    // Create a list of the devices in the 'abcde' subsystem.
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    usbk->lastopr = USBK_LO_UDEV_ERROR;

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        // usb device directory
        dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (dev_usb != NULL) {
            if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
                if ((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0)
                        || (strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb, "idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)) {
                    if (strcmp(usbk->info.usb_serial, udev_device_get_sysattr_value(dev_usb, "serial")) == 0) {
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if (strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0) {
                                usbk->info.backdisk = strdup(udev_device_get_sysname(dev));
                                usbk->info.backdisk_path = strdup(udev_device_get_devnode(dev));
                                usbk->lastopr = USBK_LO_PASS;
                            }
                        }
                    }
                }
            }
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    DBG_LASTOPR_STRING(usbk->lastopr);
    return (-1) * usbk->lastopr;
}
#endif

#if  defined(__linux__)
USBKS* usbk_list_new(void) {
    int i;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb = NULL;
    struct udev_device *dev_scsi = NULL;


    USBKS* usbks = (USBKS*)calloc(1, sizeof(USBKS));

    usbks->counter = 0;

    usbks->usbk = NULL;
    USBK_LIST *new_usbk = NULL;

    // Create the udev object
    udev = udev_new();
    if (udev == NULL) {
        usbks->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbks->lastopr);
        return NULL;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);
        // usb device directory
        dev_usb = udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device");
        if (dev_usb != NULL) {
            if (strncmp(USBK_USB_IDVENDOR, udev_device_get_sysattr_value(dev_usb, "idVendor"), strlen(USBK_USB_IDVENDOR)) == 0) {
//if((strncmp(USBK_USB_IDPRODUCT_A101, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A101)) == 0) ||
//(strncmp(USBK_USB_IDPRODUCT_A103, udev_device_get_sysattr_value(dev_usb,"idProduct"), strlen(USBK_USB_IDPRODUCT_A103)) == 0)){
                dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                if (dev_scsi != NULL) {
                    if (strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_VENDOR)) == 0) {
                        new_usbk = (USBK_LIST*) calloc(1, sizeof(USBK_LIST));
                        if (new_usbk == NULL) {
                            // todo: release all mem location
                            /*
                             LibUSBK__list_devices_release(usbk_list);
                             udev_device_unref(dev);
                             udev_enumerate_unref(enumerate);
                             udev_unref(udev);
                             */
                            usbks->lastopr = USBK_LO_MEM_ERROR;
                            DBG_LASTOPR_STRING(usbks->lastopr);
                            return NULL;
                        }



                        new_usbk->me = usbk_new(udev_device_get_sysname(dev));

                        new_usbk->next = NULL;
                        new_usbk->previous = usbks->usbk;
                        if (new_usbk->next != NULL){
                            new_usbk->previous->next = new_usbk;
                        }
                        usbks->usbk = new_usbk;
                        usbks->counter++;
                    }
                }
            }
        }
        udev_device_unref(dev);
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
    return usbks;
}

int usbk_list_release(USBKS* usbks) {
    USBK_LIST *d_usbks;
    while (usbks->usbk != NULL) {
        d_usbks = usbks->usbk->previous;
        usbk_release(usbks->usbk->me);
        usbks->usbk->me = NULL;
        usbks->usbk->previous = NULL;
        free(usbks->usbk);
        usbks->usbk = d_usbks;
        usbks->counter--;
    }
    free(usbks);
    return 0;
}

int usbk_list_refreshall(USBKS* usbks) {
    for (USBK_LIST* list_entry = usbks->usbk; list_entry != NULL; list_entry = list_entry->next) {
        usbk_refreshusbkinfo(list_entry->me);
    }
    usbks->lastopr = USBK_LO_PASS;
    DBG_LASTOPR_STRING(usbks->lastopr);
    return (-1) * usbks->lastopr;
}

#endif

#if 0//defined(WIN32)
BOOL APIENTRY DllMain (HINSTANCE hInst     /* Library instance handle. */ ,
                       DWORD reason        /* Reason this function is being called. */ ,
                       LPVOID reserved     /* Not used. */ )
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        break;

      case DLL_PROCESS_DETACH:
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    /* Returns TRUE on success, FALSE on failure */
    return TRUE;
}
#endif
