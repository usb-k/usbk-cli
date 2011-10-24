/*
 * @file libusbk.c
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

#include "config.h"

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

/* terminal color definitions */
#if defined(__linux__)
#  define _COLOR_NO_COLOR     "\e[0m"
#  define _COLOR_WHITE        "\e[1;37m"
#  define _COLOR_YELLOW       "\e[1;33m"
#  define _COLOR_CYAN         "\e[0;36m"
#  define _COLOR_BLACK        "\e[0;30m"
#  define _COLOR_BLUE         "\e[0;34m"
#  define _COLOR_GREEN        "\e[0;32m"
#  define _COLOR_RED          "\e[0;31m"
#  define _COLOR_PURPLE       "\e[0;35m"
#  define _COLOR_BROWN        "\e[0;33m"
#  define _COLOR_LIGHTGRAY    "\e[0;37m"
#  define _COLOR_LIGHTPURPLE  "\e[1;35m"
#  define _COLOR_LIGHTRED     "\e[1;31m"
#  define _COLOR_LIGHTCYAN    "\e[1;36m"
#  define _COLOR_LIGHTGREEN   "\e[1;32m"
#  define _COLOR_LIGHTBLUE    "\e[1;34m"
#  define _COLOR_DARKGRAY     "\e[1;30m"
#else
#  define _COLOR_NO_COLOR
#  define _COLOR_WHITE
#  define _COLOR_YELLOW
#  define _COLOR_CYAN
#  define _COLOR_BLACK
#  define _COLOR_BLUE
#  define _COLOR_GREEN
#  define _COLOR_RED
#  define _COLOR_PURPLE
#  define _COLOR_BROWN
#  define _COLOR_LIGHTGRAY
#  define _COLOR_LIGHTPURPLE
#  define _COLOR_LIGHTRED
#  define _COLOR_LIGHTCYAN
#  define _COLOR_LIGHTGREEN
#  define _COLOR_LIGHTBLUE
#  define _COLOR_DARKGRAY
#endif /* defined(__linux__) */

#define DBG_INFO(message, ...) \
{ \
	if (_libusbk_debug_enabled) \
		fprintf(stderr, _COLOR_LIGHTGRAY "%s::%s:%04d: " \
						_COLOR_GREEN message \
						_COLOR_NO_COLOR "\n", \
		                "libusbk", __FUNCTION__, __LINE__, \
		                ## __VA_ARGS__);\
}

#define DBG_ERROR(message, ...) \
{ \
	if (_libusbk_debug_enabled) \
		fprintf(stderr, _COLOR_LIGHTGRAY "%s::%s:%04d: " \
						_COLOR_RED message \
						_COLOR_NO_COLOR "\n", \
		                "libusbk", __FUNCTION__, __LINE__, \
		                ## __VA_ARGS__); \
}

#define DBG_RETURN_STRING(x) \
{ \
	if (x!=0) { DBG_ERROR("%s", USBK_RTN_ERRORS_STRING[x]); } \
}

#define DBG_LASTOPR_STRING(x) \
{ \
	if (x!=0) { \
		DBG_ERROR("last operation status: %s", lastopr_string[x].retval_string); \
	} else { \
		DBG_INFO("last operation status: %s", lastopr_string[x].retval_string); \
	} \
}

//-DEFINES FOR FINDING USBK AND BACKDISK
#define USBK_USB_IDVENDOR          "2384"
#define USBK_USB_IDPRODUCT_A103    "a103"
#define USBK_USB_IDPRODUCT_A101    "a101"
#define USBK_SCSI_VENDOR           "USBK"
#define USBK_SCSI_BACKDISK_VENDOR  "BackDisk"

#define USBK_CHECK_SUPPORTED(usbk) \
    if (!usbk->is_supported) { \
        usbk->lastopr = USBK_LO_UNSUPPORTED_USBK; \
        DBG_LASTOPR_STRING(usbk->lastopr); \
        return - usbk->lastopr; \
    }

struct {
    int retval;
    const char *retval_string;
} lastopr_string[] = {
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
#if defined(__linux__)
    {USBK_LO_UDEV_ERROR,            "udev error"             },
#endif
    {USBK_LO_MEM_ERROR,             "memory allocation error"}
};

struct LIBUSBK_SUPPORTED_MODELS{
    const char* model;
    bool support;
} models_1_1[] = {
    { "A101", true },
    { "A103", true },
    { NULL,   false }
};

struct LIBUSBK_SUPPORTED_VERSIONS{
    int major_version;
    int minor_version;
    struct LIBUSBK_SUPPORTED_MODELS* models;
} versions_1[] = {
     { 2, 5, models_1_1 },
     { 0, 0, NULL }
};

struct LIBUSBK_SUPPORTED_PRODUCTS {
    const char* product;
    struct LIBUSBK_SUPPORTED_VERSIONS* versions;
} products[] = {
     { "USBK CryptoBridge 2.0", versions_1 },
     { "USBK CryptoBridge", versions_1 },
     { NULL, NULL }
};

#if 0
static int _convert_key_decimal2hex(uint8_t *key_hex, const char* key_decimal, usbk_keysize_t keysize);
#endif

static int _command_status(USBK* usbk);
static int _convert_key_text2hex(uint8_t *key_hex, const char* key_text, usbk_keysize_t keysize);

#if defined(__linux__)
static int _get_udev_info(USBK* usbk, const char *device);
static int _get_udev_backdisk(USBK* usbk);
#endif

static bool _libusbk_debug_enabled = false;
static const char* _libusbk_version = USBK_LIB_VERSION_FULL;

/* Debugging informations */
void libusbk_enable_debug() { _libusbk_debug_enabled = true; }
void libusbk_disable_debug() { _libusbk_debug_enabled = false; }
const char* libusbk_version() { return _libusbk_version; }

USBK* usbk_new(const char* dev)
{
    int ret;
    int i;

    USBK* usbk = (USBK*) calloc(1, sizeof(USBK));
    assert(usbk != NULL);

#if defined(__linux__)
    // get information about usbk by udev
    ret = _get_udev_info(usbk, dev);
    if (ret < 0) {
        free(usbk);
        DBG_LASTOPR_STRING(USBK_LO_UDEV_ERROR);

        return NULL;
    }
#elif defined(WIN32)
    usbk_head->dev_node = strdup(dev);
    usbk_head->dev_node_path = strdup(dev);
#endif

    t_UIP_DEVINFO usbk_info;
    ret = send_scsi_command (
            usbk->dev_node_path,
            (unsigned char*) &usbk_info,
            USBK_GET_DEV_INFO,
            sizeof(usbk_info),
            USBK_READ_SCSI);
    if (ret < 0) {
        free(usbk);
        DBG_LASTOPR_STRING(USBK_LO_SCSI_ERROR);

        return NULL;
    }

    /** fill usbk struct elements */
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
    /* --- */

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

    USBK_CHECK_SUPPORTED(usbk)

    if (key_no < 0 || key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        ret = - usbk->lastopr;

        goto done;
    }

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&activate, 0, sizeof(activate));

        /* FIXME: check password size */
        strcpy(activate.password.s, password);
        activate.keyno = key_no;
        ret = send_scsi_command (
        		usbk->dev_node_path,
        		(unsigned char *) &activate,
        		USBK_ACTIVATE_KEY,
        		sizeof(activate),
        		USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;

    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_key_deactivate(USBK* usbk)
{
    int ret;

    USBK_CHECK_SUPPORTED(usbk)

    switch (usbk->dev_state) {
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
        ret = send_scsi_command (
        		usbk->dev_node_path,
        		(unsigned char*) NULL,
        		USBK_DEACTIVATE_KEY,
        		0,
        		USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;

    case USBK_DS_DEACTIVATE:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_change_password(USBK* usbk, const char* old_pass, const char* new_pass)
{
    int ret;
    t_UIP_CHPASS chpass;

    USBK_CHECK_SUPPORTED(usbk)

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
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;

    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_set_devicelabel(USBK* usbk, const char* pass, const char* device_label)
{
    int ret;
    t_UIP_SETDEVICELABEL devlabel;

    USBK_CHECK_SUPPORTED(usbk)

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&devlabel, 0, sizeof(devlabel));
        strncpy(devlabel.password.s, pass, sizeof(devlabel.password.s));
        strncpy(devlabel.devlabel.s, device_label, sizeof(devlabel.devlabel.s));
        ret = send_scsi_command(usbk->dev_node_path, (unsigned char*) &devlabel, USBK_SET_DEV_NAME, sizeof(devlabel), USBK_WRITE_SCSI);

        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_set_key_and_keyname(USBK* usbk, const char *pass, int key_no, const char* key_name, usbk_keysize_t key_size, const uint8_t* key)
{
    int ret;
    t_UIP_SETKEY setkey;

    USBK_CHECK_SUPPORTED(usbk)

    if (key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        ret = - usbk->lastopr;

        goto done;
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
            memcpy(setkey.key.u8, key, usbk_keysize_as_byte(key_size));
        } else {
            setkey.options.me = SETKEY_NAMEONLY;
        }

        ret = send_scsi_command (
        		usbk->dev_node_path,
        		(unsigned char*) &setkey,
        		USBK_SET_KEY,
        		sizeof(setkey),
        		USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

#if 0
int usbk_set_key_decimal(USBK* usbk_head, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const char* key)
{
    int ret = 0;
    uint8_t key_hex[UIP_KEYSIZE_TOTALSIZE];

    USBK_CHECK_SUPPORTED(usbk_head)

    usbk_head->lastopr = _convert_key_decimal2hex(key_hex, key, key_size);
    if (ret == USBK_LO_PASS){
        usbk_head->lastopr = usbk_set_key_and_keyname(usbk_head, pass, key_no, NULL, key_size, key_hex);
    }

    DBG_LASTOPR_STRING(usbk_head->lastopr);

    return usbk_head->lastopr;
}
#endif

int usbk_set_key_hex(USBK* usbk, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const uint8_t* key)
{
    USBK_CHECK_SUPPORTED(usbk)

    return usbk_set_key_and_keyname(usbk, pass, key_no, NULL, key_size, key);
}

int usbk_set_key_text(USBK* usbk, const char *pass, uint8_t key_no, usbk_keysize_t key_size, const char* key)
{
    int ret;
    uint8_t key_hex[32]; /* use macro definition instead of 32 */

    USBK_CHECK_SUPPORTED(usbk)

    ret = _convert_key_text2hex(key_hex, key, key_size);
    if (ret == USBK_LO_PASS){
        ret = usbk_set_key_and_keyname(usbk, pass, key_no, NULL, key_size, key_hex);
    }

    usbk->lastopr = ret;

    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_set_autact(USBK* usbk, const char *pass, int key_no)
{
    int ret;
    t_UIP_SETAUTOACTIVATE autoact;

    USBK_CHECK_SUPPORTED(usbk)

    if (key_no > usbk->multikey_cap) {
        usbk->lastopr = USBK_LO_INVALID_KEYNO;
        ret = - usbk->lastopr;

        goto done;
    }

    switch (usbk->dev_state) {
    case USBK_DS_DEACTIVATE:
        memset(&autoact, 0, sizeof(autoact));
        strncpy(autoact.password.s, pass, sizeof(autoact.password.s));
        autoact.keyno = key_no;
        ret = send_scsi_command (
        		usbk->dev_node_path,
        		(unsigned char*) &autoact,
        		USBK_SET_AUTO_ACTIVE,
        		sizeof(autoact),
        		USBK_WRITE_SCSI);
        if (ret < 0) {
            usbk->lastopr = USBK_LO_SCSI_ERROR;
            ret = - usbk->lastopr;

            goto done;
        }

        /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
        usbk->lastopr = _command_status(usbk);

        break;
    case USBK_DS_ACTIVATE:
    case USBK_DS_ACTIVATE_WITH_BACKDISK:
    case USBK_DS_FABRIC_DEFAULT:
    case USBK_DS_MUST_REMOVE:
    default:
        usbk->lastopr = USBK_LO_STATE_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_get_randomkey(USBK* usbk, uint8_t* random_key, usbk_keysize_t keysize)
{
    int ret;
    t_UIP_GENERATEKEY genkey;

    USBK_CHECK_SUPPORTED(usbk)

    int keysize_inbyte = usbk_keysize_as_byte(keysize);
    if ((keysize_inbyte == 0) || (keysize_inbyte > usbk_keysize_as_byte(USBK_KEYSIZE_256BIT))) {
        usbk->lastopr = USBK_LO_INVALID_KEYSIZE;
        ret = - usbk->lastopr;

        goto done;
    }

    ret = send_scsi_command (
    		usbk->dev_node_path,
    		(unsigned char*) &genkey,
    		USBK_GENERATE_KEY,
    		sizeof(genkey),
    		USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        ret = - usbk->lastopr;

        goto done;
    }

    /* FIXME: POTENTIONAL BUG: _command_status function does not return USBK_LO_* values */
    usbk->lastopr = _command_status(usbk);

    if (usbk->lastopr == USBK_LO_PASS) {
        memcpy(random_key, genkey.key.u8, keysize_inbyte);
    }

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

int usbk_refresh_usbkinfo(USBK* usbk)
{
    int i;
    int ret;

    USBK_CHECK_SUPPORTED(usbk)

    t_UIP_DEVINFO usbk_info;
    ret = send_scsi_command (
    		usbk->dev_node_path,
    		(unsigned char*) &usbk_info,
    		USBK_GET_DEV_INFO,
    		sizeof(usbk_info),
    		USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        ret = - usbk->lastopr;

        goto done;
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

done:
    DBG_LASTOPR_STRING(usbk->lastopr);

    return ret;
}

static int _command_status(USBK* usbk)
{
    int ret;
    t_UIP_GETSTATUS status;

    ret = send_scsi_command (
    		usbk->dev_node_path,
    		(unsigned char*) &status,
    		USBK_GET_STATUS,
    		sizeof(t_UIP_GETSTATUS),
    		USBK_READ_SCSI);
    if (ret < 0) {
        usbk->lastopr = USBK_LO_SCSI_ERROR;
        DBG_LASTOPR_STRING(usbk->lastopr);

        return - usbk->lastopr;
    }

    usbk->retry_num = status.retry_num;

    /* FIXME a new review required!! */
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

    unsigned int key_len = usbk_keysize_as_byte(keysize);
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
    if (i != (usbk_keysize_as_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;
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
    for (unsigned int n = 0; n < usbk_keysize_as_byte(keysize); n++) {
        iss >> ikey[n];

        if (iss.fail() || iss.bad()) return USBK_LO_INVALID_KEY;

        if (iss.eof() && n != (usbk_keysize_as_byte(keysize) - 1)) return USBK_LO_INVALID_KEY;

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
    int key_len = usbk_keysize_as_byte(keysize);
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

unsigned int usbk_keysize_as_byte(usbk_keysize_t keysize)
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

    struct udev *_udev = NULL;
    struct udev_enumerate *_udev_enumerate = NULL;

    struct udev_list_entry *_udev_enumerated_devices;
    struct udev_list_entry *_udev_enumerated_device = NULL;
    struct udev_list_entry *_udev_list_entry;

    struct udev_device *_udev_device = NULL;
    struct udev_device *_udev_device_scsi = NULL;
    struct udev_device *_udev_device_usb = NULL;

    const char *_udev_enumerated_device_path;
    const char *_udev_device_idvendor;
    const char *_udev_device_idproduct;

    const char *_udev_dev_path;
    char *_udev_devname;
    char dev_type;

    struct stat statbuf;

    usbk->lastopr = USBK_LO_UDEV_ERROR;

    _udev = udev_new();
    if (!_udev) goto done;

    _udev_dev_path = udev_get_dev_path(_udev);

    /* allocate ram space from stack with alloca */
    _udev_devname = alloca(strlen(_udev_dev_path) + strlen(device) + 1);
    assert(_udev_devname != NULL);
    sprintf(_udev_devname, "%s/%s", udev_get_dev_path(_udev), device);

    if (stat(_udev_devname, &statbuf) < 0) goto done;

	if (S_ISBLK(statbuf.st_mode)) {
		dev_type = 'b';
	}
	else if (S_ISCHR(statbuf.st_mode)) {
		/* FIXME char devtype is legal? */
		dev_type = 'c';
	} else {
		goto done;
	}

	_udev_device = udev_device_new_from_devnum(_udev, dev_type, statbuf.st_rdev);

	if (!_udev_device) goto done;

    _udev_device_usb = udev_device_get_parent_with_subsystem_devtype(_udev_device, "usb", "usb_device");
    if (!_udev_device_usb) goto done;

    _udev_device_idvendor = udev_device_get_sysattr_value(_udev_device_usb, "idVendor");

    ret = strncmp(USBK_USB_IDVENDOR, _udev_device_idvendor, strlen(USBK_USB_IDVENDOR));
    if (ret) goto done;

    _udev_device_idproduct = udev_device_get_sysattr_value(_udev_device_usb, "idProduct");

    if (!strncmp(USBK_USB_IDPRODUCT_A101, _udev_device_idproduct, strlen(_udev_device_idproduct)) &&
    	!strncmp(USBK_USB_IDPRODUCT_A103, _udev_device_idproduct, strlen(_udev_device_idproduct)))
    	goto done;

    _udev_device_scsi = udev_device_get_parent_with_subsystem_devtype(_udev_device, "scsi", "scsi_device");
    if (!_udev_device_scsi) goto done;

    ret = strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(_udev_device_scsi, "vendor"), strlen(USBK_SCSI_VENDOR));
    if (ret) goto done;

	usbk->dev_node = strdup(udev_device_get_sysname(_udev_device));
	usbk->dev_node_path = strdup(udev_device_get_devnode(_udev_device));
	usbk->usb_serial = strdup(udev_device_get_sysattr_value(_udev_device_usb, "serial"));

	/* another usbk_lo_** required for backdisk error */
	ret = _get_udev_backdisk(usbk);
	if (ret < 0) goto done;

	usbk->lastopr = USBK_LO_PASS;

done:
	udev_device_unref(_udev_device);
	udev_enumerate_unref(_udev_enumerate);
	udev_unref(_udev);

    DBG_LASTOPR_STRING(usbk->lastopr);

    return usbk->lastopr;
}

static int _get_udev_backdisk(USBK* usbk)
{
    int ret;

    struct udev *_udev = NULL;
    struct udev_enumerate *_udev_enumerate = NULL;
    struct udev_list_entry *_udev_enumerated_devices = NULL;
    struct udev_list_entry *_udev_enumerated_device = NULL;

    struct udev_device *_udev_device = NULL;
    struct udev_device *_udev_device_scsi = NULL;
    struct udev_device *_udev_device_usb = NULL;

    const char *_udev_enumerated_device_path;
    const char *_udev_device_idvendor;
    const char *_udev_device_idproduct;
    const char *_udev_device_usb_serial;

    usbk->lastopr = USBK_LO_UDEV_ERROR;

    // Create the _udev object
    _udev = udev_new();

    if (!_udev) goto done;

    /* get udev enumerated devices */
    _udev_enumerate = udev_enumerate_new(_udev);

    /* add block devices */
    ret = udev_enumerate_add_match_subsystem(_udev_enumerate, "block");
    if (ret) goto done;

    /* add scan for devices */
    ret = udev_enumerate_scan_devices(_udev_enumerate);
    if (ret) goto done;

    /* get device list */
    _udev_enumerated_devices = udev_enumerate_get_list_entry(_udev_enumerate);

    udev_list_entry_foreach (_udev_enumerated_device, _udev_enumerated_devices) {
        _udev_enumerated_device_path = udev_list_entry_get_name(_udev_enumerated_device);
        _udev_device = udev_device_new_from_syspath(_udev, _udev_enumerated_device_path);

        /* get usb devices */
        _udev_device_usb = udev_device_get_parent_with_subsystem_devtype(_udev_device, "usb", "usb_device");
        if (!_udev_device_usb) continue;

        _udev_device_idvendor = udev_device_get_sysattr_value(_udev_device_usb, "idVendor");

        ret = strncmp(USBK_USB_IDVENDOR, _udev_device_idvendor, strlen(USBK_USB_IDVENDOR));
        if (ret) continue;

        _udev_device_idproduct = udev_device_get_sysattr_value(_udev_device_usb, "idProduct");

        /* FIXME: code enhancement required */
        if (!strncmp(USBK_USB_IDPRODUCT_A101, _udev_device_idproduct, strlen(_udev_device_idproduct)) &&
        	!strncmp(USBK_USB_IDPRODUCT_A103, _udev_device_idproduct, strlen(_udev_device_idproduct)))
        	continue;

        _udev_device_usb_serial = udev_device_get_sysattr_value(_udev_device_usb, "serial");

        ret = strcmp(usbk->usb_serial, _udev_device_usb_serial);
        if (ret) continue;

		_udev_device_scsi = udev_device_get_parent_with_subsystem_devtype(_udev_device, "scsi", "scsi_device");
		if (!_udev_device_scsi) continue;

		ret = strncmp(USBK_SCSI_BACKDISK_VENDOR, udev_device_get_sysattr_value(_udev_device_scsi, "vendor"), strlen(USBK_SCSI_BACKDISK_VENDOR));
        if (ret) continue;

		usbk->backdisk_node = strdup(udev_device_get_sysname(_udev_device));
		usbk->backdisk_node_path = strdup(udev_device_get_devnode(_udev_device));

		usbk->lastopr = USBK_LO_PASS;

		/* no need to proceed loop */
		break;
    }

done:
	udev_device_unref(_udev_device);
	udev_enumerate_unref(_udev_enumerate);
	udev_unref(_udev);

    DBG_LASTOPR_STRING(usbk->lastopr);

	return usbk->lastopr;
}

USBK_LIST* usbk_list_new(void)
{
    struct udev *_udev = NULL;
    struct udev_enumerate *_udev_enumerate = NULL;
    struct udev_list_entry *_udev_enumerated_devices = NULL;
    struct udev_list_entry *_udev_enumerated_device = NULL;

    struct udev_device *_udev_device = NULL;
    struct udev_device *_udev_device_scsi = NULL;
    struct udev_device *_udev_device_usb = NULL;

    const char *_udev_enumerated_device_path;
    const char *_udev_device_idvendor;

    int ret;
    USBK_LIST *usbk_list;

    USBK *usbk_last_entry = NULL;
    USBK *usbk_new_entry = NULL;

    /* initialize udev */
    _udev = udev_new();
    if (_udev == NULL) {
        fprintf(stderr, "Error! Udev cannot be initialized!\n");

        return NULL;
    }

    /* get udev enumerated devices */
    _udev_enumerate = udev_enumerate_new(_udev);

    /* add block devices */
    ret = udev_enumerate_add_match_subsystem(_udev_enumerate, "block");
    if (ret) goto done;

    /* add scan for devices */
    ret = udev_enumerate_scan_devices(_udev_enumerate);
    if (ret) goto done;

    /* get device list */
    _udev_enumerated_devices = udev_enumerate_get_list_entry(_udev_enumerate);

    usbk_list = (USBK_LIST *) calloc(1, sizeof(USBK_LIST));

    if (!usbk_list) {
        DBG_LASTOPR_STRING(USBK_LO_MEM_ERROR);

        goto done;
    }

    udev_list_entry_foreach (_udev_enumerated_device, _udev_enumerated_devices) {
        _udev_enumerated_device_path = udev_list_entry_get_name(_udev_enumerated_device);
        _udev_device = udev_device_new_from_syspath(_udev, _udev_enumerated_device_path);
        
        /* get usb devices */
        _udev_device_usb = udev_device_get_parent_with_subsystem_devtype(_udev_device, "usb", "usb_device");
        if (!_udev_device_usb) continue;

        _udev_device_idvendor = udev_device_get_sysattr_value(_udev_device_usb, "idVendor");

        ret = strncmp(USBK_USB_IDVENDOR, _udev_device_idvendor, strlen(USBK_USB_IDVENDOR));
        if (ret) continue;

        _udev_device_scsi = udev_device_get_parent_with_subsystem_devtype(_udev_device, "scsi", "scsi_device");
        if (!_udev_device_scsi) continue;

        ret = strncmp(USBK_SCSI_VENDOR, udev_device_get_sysattr_value(_udev_device_scsi, "vendor"), strlen(USBK_SCSI_VENDOR));
        if (ret) continue;

        usbk_new_entry = usbk_new(udev_device_get_sysname(_udev_device));
        if (!usbk_new_entry) {
            DBG_LASTOPR_STRING(USBK_LO_MEM_ERROR);

            goto done;

        }

        if (!usbk_list->usbk_head) {
        	usbk_list->usbk_head = usbk_last_entry = usbk_new_entry;
        } else {
            usbk_last_entry->next = usbk_new_entry;
            usbk_last_entry = usbk_new_entry;
        }

    	usbk_list->count++;
    }

done:
    udev_device_unref(_udev_device);
    udev_enumerate_unref(_udev_enumerate);
    udev_unref(_udev);

    return usbk_list;
}

int usbk_list_release(USBK_LIST *usbk_list)
{
    USBK *_usbk=usbk_list->usbk_head;

    while (_usbk->next) {
        usbk_release(_usbk);
        _usbk = _usbk->next;
        usbk_list->count--;
    }

    free(usbk_list);

    return 0;
}

USBK* usbk_list_get_next(USBK* usbk)
{
    return usbk->next;
}

int usbk_list_refreshall(USBK_LIST *usbk_list)
{
    USBK* list_entry;

    usbk_list_entry_foreach (list_entry, usbk_list) {
        usbk_refresh_usbkinfo(list_entry);
    }

    DBG_LASTOPR_STRING(USBK_LO_PASS);

    return USBK_LO_PASS;
}

int usbk_list_get_count(USBK_LIST *usbk_list)
{
    return usbk_list->count;
}

#endif


