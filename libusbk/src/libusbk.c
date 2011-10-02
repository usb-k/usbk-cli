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

#if defined(__linux__)
#  include <libudev.h>
#  include <alloca.h>
#  include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "uip.h"

#include "libusbk.h"
#include "usbk_scsi.h"

#if defined(__linux__)
    #define DBG_INFO(x, ...) {if(debug_enable) {fprintf(stderr, "\033[0;32;40m%s(%d):%s:\033[0;37;40m" x "\033[0;37;40m\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_ERROR(x, ...) {if(debug_enable) {fprintf(stderr, "\033[1;3;31m%s(%d):%s:\033[0;37;40m" x "\033[0;37;40m\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_RETURN_STRING(x) {if (x!=0) DBG_ERROR("%s", USBK_RTN_ERRORS_STRING[x]);}
    #define DBG_LASTOPR_STRING(x) {if (x!=0) {DBG_ERROR("last operation status: %s", lastopr_string[x].rtrn_string);} \
                                   else {DBG_INFO("last operation status: %s", lastopr_string[x].rtrn_string);}}
#elif defined(WIN32)
    #define DBG_INFO(x, ...) {if(debug_enable) {fprintf(stderr, "%s(%d):%s:" x "\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_ERROR(x, ...) {if(debug_enable) {fprintf(stderr, "%s(%d):%s:" x "\n", "libusbk",__LINE__, __FUNCTION__, ## __VA_ARGS__);}}
    #define DBG_RETURN_STRING(x) {if (x!=0) DBG_ERROR("%s", USBK_RTN_ERRORS_STRING[x]);}
    #define DBG_LASTOPR_STRING(x) {if (x!=0) {DBG_ERROR("last operation status: %s", lastopr_string[x].rtrn_string);} \
                                   else {DBG_INFO("last operation status: %s", lastopr_string[x].rtrn_string);}}
#endif

//-DEFINES FOR FINDING USBK AND BACKDISK
#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT_A103    "a103"
#define USBK_USB_IDPRODUCT_A101    "a101"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"

#define USBK_CHECK_SUPPORTED \
	if (!usbk->is_supported) { \
		usbk->lastopr = USBK_LO_UNSUPPORTED_USBK; \
		DBG_LASTOPR_STRING(usbk->lastopr); \
		return - usbk->lastopr; \
	}

typedef struct __RTRN_STRING {
    int rtrn;
    const char *rtrn_string;
} RTRN_STRING;

RTRN_STRING lastopr_string[] = {
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

LIBUSBK_SUPPORTED_MODELS models_1_1[] = {
    { "A101", true },
    { "A103", true },
    {NULL, false }
};

typedef struct __LIBUSBK_SUPPORTED_VERSIONS {
    int major_version;
    int minor_version;
    LIBUSBK_SUPPORTED_MODELS* models;
} LIBUSBK_SUPPORTED_VERSIONS;

LIBUSBK_SUPPORTED_VERSIONS versions_1[] = {
     { 2, 5, models_1_1 },
     { 0, 0, NULL }
};

typedef struct __LIBUSBK_SUPPORTED_PRODUCTS {
    const char* product;
    LIBUSBK_SUPPORTED_VERSIONS* versions;
} LIBUSBK_SUPPORTED_PRODUCTS;

LIBUSBK_SUPPORTED_PRODUCTS products[] = {
     { "USBK CryptoBridge 2.0", versions_1 },
     { "USBK CryptoBridge", versions_1 },
     { NULL, NULL }
};

static bool debug_enable = false;

static int _get_udev_info(USBK* usbk, const char *device);
static int _get_udev_backdisk(USBK* usbk);
static int _command_status(USBK* usbk);

#if 0
static int _convert_key_decimal2hex(uint8_t *key_hex, const char* key_decimal, usbk_keysize_t keysize);
#endif

static int _convert_key_text2hex(uint8_t *key_hex, const char* key_text, usbk_keysize_t keysize);
static unsigned int _keysize_as_byte(usbk_keysize_t keysize);

USBK* usbk_new(const char* dev)
{
    int ret;
    int i;

    USBK* usbk = (USBK*)calloc(1, sizeof(USBK));
    if (usbk == NULL) {
        usbk->lastopr = USBK_LO_MEM_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return NULL;
    }

#if defined(__linux__)
    // get information about usbk by udev
    ret = _get_udev_info(usbk, dev);
    if (ret < 0) {
        free(usbk);
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return NULL;
    }

#elif defined(WIN32)
	usbk->dev_node = strdup(dev);
	usbk->dev_node_path = strdup(dev);
#endif

    t_UIP_DEVINFO usbk_info;
    ret = send_scsi_command(
    		usbk->dev_node_path,
    		(unsigned char*) &usbk_info,
    		USBK_GET_DEV_INFO,
    		sizeof(usbk_info),
    		USBK_READ_SCSI);
    if (ret < 0) {
        free(usbk);
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return NULL;
    }

    usbk->product = strdup(usbk_info.product.s);
    usbk->model = strdup(usbk_info.model.s);
    usbk->firmware_ver = strdup(usbk_info.firmware_ver.s);
    usbk->is_supported = true;

	usbk->multikey_cap = usbk_info.multikeycap;
	usbk->current_key = usbk_info.current_keyno;
	usbk->autoact_keyno = usbk_info.autoactivate_keyno;
	usbk->retry_num = usbk_info.retry_num;
	usbk->dev_state = (usbk_ds_t) usbk_info.devstate.me;
	usbk->dev_label = strdup(usbk_info.devlabel.s);

	/* FIXME why we need this much space */
	usbk->serial = (char*) calloc((sizeof(usbk_info.serial) * 2) + 2, sizeof(char));
	for (i = 0; i < UIP_SERIAL_SIZE; i++) {
		sprintf((usbk->serial + i * 2), "%2.2X", usbk_info.serial.u8[i]);
	}

	usbk->key_names = (char**) calloc(usbk->multikey_cap, sizeof(char*));
	for (i = 0; i < usbk->multikey_cap; i++) {
		usbk->key_names[i] = strdup(usbk_info.keyname[i].s);
	}

    usbk->lastopr = USBK_LO_PASS;
    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk;
}

int usbk_release(USBK* usbk)
{
    int i;

    free(usbk->dev_node);
    free(usbk->dev_node_path);
    free(usbk->backdisk_node);
    free(usbk->backdisk_node_path);
    free(usbk->product);
    free(usbk->model);
    free(usbk->serial);
    free(usbk->usb_serial);
    free(usbk->firmware_ver);
    free(usbk->dev_label);

    for (i = 0; i < usbk->multikey_cap; i++) {
        free(usbk->key_names[i]);
    }

    free(usbk->key_names);
    free(usbk);

    return 0;
}

int usbk_key_activate(USBK* usbk, const char* password, uint8_t key_no)
{
    int ret;
    t_UIP_ACTIVATE activate;

    USBK_CHECK_SUPPORTED

    if (key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&activate, 0, sizeof(activate));
        strcpy(activate.password.s, password);
        activate.keyno = key_no;
        ret = send_scsi_command(usbk->dev_node_path, (unsigned char *) &activate, USBK_ACTIVATE_KEY, sizeof(activate), USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = _command_status(usbk);

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

    return usbk->lastopr;
}

int usbk_key_deactivate(USBK* usbk)
{
    int ret;

    USBK_CHECK_SUPPORTED

    switch (usbk->dev_state) {
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) NULL, USBK_DEACTIVATE_KEY, 0, USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = _command_status(usbk);

        break;
    case USBK_DS_DEACTIVATE:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;

        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

int usbk_change_password(USBK* usbk, const char* old_pass, const char* new_pass)
{
    int ret;
    t_UIP_CHPASS chpass;

    USBK_CHECK_SUPPORTED

    switch (usbk->dev_state) {
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

        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &chpass, USBK_CHANGE_PASS, sizeof(chpass), USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;

            break;
        }
        usbk->lastopr = _command_status(usbk);

        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;

        break;
    }

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

int usbk_set_devicelabel(USBK* usbk, const char* pass, const char* device_label)
{
    int ret;
    t_UIP_SETDEVICELABEL devlabel;

    USBK_CHECK_SUPPORTED

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&devlabel, 0, sizeof(devlabel));
        strncpy(devlabel.password.s, pass, sizeof(devlabel.password.s));
        strncpy(devlabel.devlabel.s, device_label, sizeof(devlabel.devlabel.s));
        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &devlabel, USBK_SET_DEV_NAME, sizeof(devlabel), USBK_WRITE_SCSI);

        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            break;
        }
        usbk->lastopr = _command_status(usbk);

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

    return usbk->lastopr;
}

int usbk_set_key_and_keyname(USBK* usbk, const char *pass, int key_no, const char* key_name, usbk_keysize_t key_size, const uint8_t* key)
{
    int ret;
    t_UIP_SETKEY setkey;

    USBK_CHECK_SUPPORTED

    if (key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&setkey, 0, sizeof(setkey));
        strncpy(setkey.password.s, pass, sizeof(setkey.password.s));
        setkey.keyno = key_no;

        if (key_name != NULL) {
            strncpy(setkey.keyname.s, key_name, sizeof(setkey.keyname.s));
        } else {
        	/* preserve old keyname if keyname is not changed */
            strncpy(setkey.keyname.s, usbk->key_names[key_no - 1], sizeof(setkey.keyname.s));
        }

        if ((key_size != USBK_KEYSIZE_NULL) && (key != NULL)) {
            setkey.options.me = SETKEY_NAME_AND_KEY;
            setkey.keysize.me = (e_UIP_KEYSIZE)  key_size;
            memcpy(setkey.key.u8, key, _keysize_as_byte(key_size));
        } else {
            setkey.options.me = SETKEY_NAMEONLY;
        }

        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &setkey, USBK_SET_KEY, sizeof(setkey), USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;

            break;
        }
        usbk->lastopr = _command_status(usbk);

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

    return usbk->lastopr;
}

#if 0
int usbk_set_key_decimal(USBK* usbk, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const char* key)
{
    int ret = 0;
    uint8_t key_hex[UIP_KEYSIZE_TOTALSIZE];

    USBK_CHECK_SUPPORTED

    usbk->lastopr = _convert_key_decimal2hex(key_hex, key, key_size);
    if (ret == USBK_LO_PASS){
        usbk->lastopr = usbk_set_key_and_keyname(usbk, pass, key_no, NULL, key_size, key_hex);
    }

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}
#endif

int usbk_set_key_hex(USBK* usbk, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const uint8_t* key)
{
	USBK_CHECK_SUPPORTED

    return usbk_set_key_and_keyname(usbk, pass, key_no, NULL, key_size, key);
}

int usbk_set_key_text(USBK* usbk, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const char* key)
{
    int ret = 0;
    uint8_t key_hex[32];

    USBK_CHECK_SUPPORTED

    usbk->lastopr = _convert_key_text2hex(key_hex, key, key_size);
    if (ret == USBK_LO_PASS){
        usbk->lastopr = usbk_set_key_and_keyname(usbk, pass, key_no, NULL, key_size, key_hex);
    }

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

int usbk_set_autact(USBK* usbk, const char *pass, int key_no)
{
    int ret;
    t_UIP_SETAUTOACTIVATE autoact;

    USBK_CHECK_SUPPORTED

    if (key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&autoact, 0, sizeof(autoact));
        strncpy(autoact.password.s, pass, sizeof(autoact.password.s));
        autoact.keyno = key_no;
        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &autoact, USBK_SET_AUTO_ACTIVE, sizeof(autoact), USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;

            break;
        }
        usbk->lastopr = _command_status(usbk);

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

    return usbk->lastopr;
}

int usbk_get_randomkey(USBK* usbk, uint8_t* random_key, usbk_keysize_t keysize)
{
    int ret;
    t_UIP_GENERATEKEY genkey;

    USBK_CHECK_SUPPORTED

    unsigned int keysize_inbyte = _keysize_as_byte(keysize);
    if ((keysize_inbyte == 0) || (keysize_inbyte > _keysize_as_byte(USBK_KEYSIZE_256BIT))) {
        usbk->lastopr = USBK_LO_INVALID_KEYSIZE;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &genkey, USBK_GENERATE_KEY, sizeof(genkey), USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    usbk->lastopr = _command_status(usbk);
    if (usbk->lastopr == USBK_LO_PASS) {
        memcpy(random_key, genkey.key.u8, keysize_inbyte);
    }

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

int usbk_refresh_usbkinfo(USBK* usbk)
{
    int i;
    int ret;

    USBK_CHECK_SUPPORTED

    t_UIP_DEVINFO usbk_info;
    ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &usbk_info, USBK_GET_DEV_INFO, sizeof(usbk_info), USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    usbk->current_key = usbk_info.current_keyno;
    usbk->autoact_keyno = usbk_info.autoactivate_keyno;
    usbk->retry_num = usbk_info.retry_num;
    usbk->dev_state = (usbk_ds_t) usbk_info.devstate.me;
    usbk->dev_label = strdup(usbk_info.devlabel.s);

    usbk->key_names = (char**) calloc(usbk->multikey_cap, sizeof(char*));
    for (i = 0; i < usbk->multikey_cap; i++) {
        usbk->key_names[i] = strdup(usbk_info.keyname[i].s);
    }
    usbk->lastopr = USBK_LO_PASS;

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

static int _command_status(USBK* usbk)
{
    t_UIP_GETSTATUS status;
    int ret;

    ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &status, USBK_GET_STATUS, sizeof(t_UIP_GETSTATUS), USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);
        return -  usbk->lastopr;
    }

    usbk->retry_num = status.retry_num;

    return (status.lastop.me - 1); //todo versiyon checking'ine göre düzleltilecek.
}

#if 0
static int _convert_key_decimal2hex(uint8_t *key_hex, const char* key_decimal, usbk_keysize_t keysize)
{
    // 1) string icinde bosluk karakteri var mi?
    // 2) string icerisindeki '.' karakterinin sayisi 15 mi?
    // 3) '.' karakterlerini ' ' karakterine donustur
    // 4) string icerisinde bosluklar haric decimal olmayan karakter var mi?
    // 5) string to integer islemini yap
    // 6) 16 adet sayı cikartabildin mi?
    // 7) sayılar 0 ile 255 arasinda mi?

	unsigned int key_len = _keysize_as_byte(keysize);
	int i;

	/* check key_text if it is null or not */
	if (key_decimal == NULL || key_hex == NULL)
		return USBK_LO_INVALID_KEY;

	/* check key length if it is proper or not */
	if(strlen(key_hex) > key_len) {
		/* FIXME USBK_LO_INVALID_KEYSIZE? */
		return USBK_LO_INVALID_KEY;
	}

	for(i=0; i<key_len; i++) {
		/* check key_text[i] is a valid hexadecimal or not */
		char k = key_decimal[i];
		if (!isxdigit(k))
			return USBK_LO_INVALID_KEY;

		key_hex[i] = k;
	}


    //int ikey[this->_keysize_as_byte(keysize)];
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
    if (i != (_keysize_as_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;
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
    for (unsigned int n = 0; n < _keysize_as_byte(keysize); n++) {
        iss >> ikey[n];

        if (iss.fail() || iss.bad()) return USBK_LO_INVALID_KEY;

        if (iss.eof() && n != (_keysize_as_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;

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
#endif

static inline int16_t _xtod(const char c)
{
	if (c>='0' && c<='9') return c-'0';
	if (c>='A' && c<='F') return c-'A'+10;
	if (c>='a' && c<='f') return c-'a'+10;

	return -1;
}

static int _convert_key_text2hex(uint8_t *key_hex, const char* key_text, usbk_keysize_t keysize)
{
	unsigned int key_len = _keysize_as_byte(keysize);
	int i;

	/* check key_text if it is null or not */
	if (key_text == NULL || key_hex == NULL)
		return USBK_LO_INVALID_KEY;

	/* check key length if it is proper or not */
	if(strlen(key_hex) > key_len) {
		/* FIXME USBK_LO_INVALID_KEYSIZE? */
		return USBK_LO_INVALID_KEY;
	}

	for(i=0; i<key_len; i++) {
		/* check key_text[i] is a valid hexadecimal or not */
		char k = key_text[i];
		int16_t ret = _xtod(k);

		if (ret == -1)
			return USBK_LO_INVALID_KEY;

		key_hex[i] = ret;
	}

    return USBK_LO_PASS;
}

static unsigned int _keysize_as_byte(usbk_keysize_t keysize)
{
    unsigned int size = 0;

    switch (keysize) {
    case USBK_KEYSIZE_128BIT:
        size = 16;
        break;
    case USBK_KEYSIZE_192BIT:
        size = 24;
        break;
    case USBK_KEYSIZE_256BIT:
        size = 32;
        break;
    case USBK_KEYSIZE_NULL:
    default:
        size = 0;
        break;
    }

    return size;
}

#if defined(__linux__)

static int _get_udev_info(USBK* usbk, const char *device)
{
    int ret;

    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb;
    struct udev_device *dev_scsi;
    struct udev_list_entry *list_entry;

    const char *str;
    size_t len;
    int i;
    const char *udev_dev_path;

    struct stat statbuf;

    udev = udev_new();
    if (udev == NULL) {
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    udev_dev_path = udev_get_dev_path(udev);

    char *devname = alloca(strlen(udev_dev_path) + strlen(device) + 1);
    assert(devname != NULL);
    sprintf(devname, "%s/%s", udev_get_dev_path(udev), device);

    if (stat(devname, &statbuf) < 0) {
        usbk->lastopr = USBK_LO_UDEV_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
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
                        usbk->dev_node = strdup(udev_device_get_sysname(dev));
                        usbk->dev_node_path = strdup(udev_device_get_devnode(dev));
                        usbk->usb_serial = strdup(udev_device_get_sysattr_value(dev_usb, "serial"));

                        ret = _get_udev_backdisk(usbk);
                        if (ret < 0) {
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
#endif

#if  defined(__linux__)

static int _get_udev_backdisk(USBK* usbk) {
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
                    if (strcmp(usbk->usb_serial, udev_device_get_sysattr_value(dev_usb, "serial")) == 0) {
                        dev_scsi = udev_device_get_parent_with_subsystem_devtype(dev, "scsi", "scsi_device");
                        if (dev_scsi != NULL) {
                            if (strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(dev_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR)) == 0) {
                                usbk->backdisk_node= strdup(udev_device_get_sysname(dev));
                                usbk->backdisk_node_path = strdup(udev_device_get_devnode(dev));
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

struct USBKS
{
    USBK* usbk;
    int usbk_disk_count;
    int lastopr;
};

USBKS* usbk_list_new(void) {
    int i;
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;
    struct udev_device *dev_usb;
    struct udev_device *dev_scsi;

    USBKS* usbks = (USBKS*)calloc(1, sizeof(USBKS));

    usbks->usbk_disk_count = 0;

    usbks->usbk = NULL;
    USBK *new_usbk = NULL;

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
                        new_usbk = usbk_new(udev_device_get_sysname(dev));
                        if (new_usbk ==NULL)
                        {
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

                        new_usbk->previous = NULL;
                        new_usbk->next = usbks->usbk;
                        if (new_usbk->previous != NULL){
                            new_usbk->next->previous = new_usbk;
                        }
                        usbks->usbk = new_usbk;
                        usbks->usbk_disk_count++;
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
    USBK *d_usbks;
    while (usbks->usbk != NULL) {
        d_usbks = usbks->usbk->next;
        usbk_release(usbks->usbk);
        usbks->usbk->next = NULL;
        usbks->usbk = d_usbks;
        usbks->usbk_disk_count--;
    }
    free(usbks);
    return 0;
}

USBK* usbk_list_get_entry(USBKS* usbks){
    return usbks->usbk;
}

USBK* usbk_list_get_next(USBK* usbk){
    return usbk->next;
}

USBK* usbk_list_get_previous(USBK* usbk){
    return usbk->previous;
}

int usbk_list_refreshall(USBKS* usbks){

    USBK* list_entry;
    usbk_list_entry_foreach(list_entry, usbks->usbk){
        usbk_refresh_usbkinfo(list_entry);
    }

    usbks->lastopr = USBK_LO_PASS;
    DBG_LASTOPR_STRING(usbks->lastopr);
    return (-1) * usbks->lastopr;
}

int usbk_list_get_counter(USBKS* usbks){
    return usbks->usbk_disk_count;
}

int usbk_list_get_lastoprstatus(USBKS* usbks){
    return usbks->lastopr;
}

#endif

void usbk_debug_enable(void)
{
    debug_enable = true;
}

void usbk_debug_disable(void)
{
    debug_enable = false;
}

void usbk_debug(bool enable)
{
    debug_enable = enable;
}

bool usbk_debug_check(void)
{
    return debug_enable;
}

#if 0
const char* usbk_libversion(void)
{
    return VERSION;
}
#endif
