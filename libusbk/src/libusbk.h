/*
 * @file libusbk.h
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

#ifndef LIBUSBK_H_
#define LIBUSBK_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__AVR32__) || defined(__linux__)
#  include <inttypes.h>
#elif defined(WIN32)
#  include <stdint.h>
#else
#  error unsupported environment
#endif

/*
 * usbk keysize type values
 *
 * all keysize value is uint8_t (1 byte) long
 * */
enum E_USBK_KEYSIZE
{
    USBK_KEYSIZE_NULL   = 0x00,
    USBK_KEYSIZE_128BIT = 0x01,
    USBK_KEYSIZE_192BIT = 0x02,
    USBK_KEYSIZE_256BIT = 0x03,
};
typedef enum E_USBK_KEYSIZE E_USBK_KEYSIZE;
typedef int8_t usbk_keysize_t;

/*
 * usbk device status type values
 *
 * all ds value is uint8_t (1 byte) long
 * */
enum E_USBK_DEVICE_STATUS
{
    USBK_DS_ACTIVATE                = 0x01,
    USBK_DS_ACTIVATE_WITH_BACKDISK  = 0x02,
    USBK_DS_DEACTIVATE              = 0x03,
    USBK_DS_FABRIC_DEFAULT          = 0x04,
    USBK_DS_MUST_REMOVE             = 0x05,
};
typedef enum E_USBK_DEVICE_STATUS E_USBK_DEVICE_STATUS;
typedef int8_t usbk_ds_t;

/*
 * usbk last operation type values
 *
 * all lo value is uint8_t (1 byte) long
 * */
enum E_USBK_LASTOPP
{
    USBK_LO_PASS                = 0x00,
    USBK_LO_GEN_FAIL            = 0x01,
    USBK_LO_FAILED_PASS         = 0x02,
    USBK_LO_FABRIC_RESET        = 0x03,
    USBK_LO_USBK_UNPLUGING      = 0x04,
    USBK_LO_INVALID_KEYNO       = 0x05,
    USBK_LO_INVALID_KEYSIZE     = 0x06,
    USBK_LO_INVALID_DEVICELABEL = 0x07,
    USBK_LO_INVALID_PASS        = 0x08,
    USBK_LO_INVALID_NEWPASS     = 0x09,

    USBK_LO_STATE_ERROR         = 0x0a,
    USBK_LO_SCSI_ERROR          = 0x0b,
    USBK_LO_UNSUPPORTED_USBK    = 0x0c,
    USBK_LO_INVALID_KEY         = 0x0d,

    USBK_LO_UDEV_ERROR          = 0x0e,
    USBK_LO_MEM_ERROR           = 0x0f,
};
typedef enum E_USBK_LASTOPP E_USBK_LASTOPP;
typedef int8_t usbk_lo_t;

#ifndef __cplusplus
/*
 * bool definition for c
 * */
enum { false = 0, true = !false };

typedef int8_t bool;
#endif

/*
 * usbk structure, contains all the details about usbk device.
 * */
struct USBK
{
    /* refers the last operation of usbk device */
    usbk_lo_t lastopr;

    /* refers node of usbk device in /dev.
     * e.g. usbk device node is /dev/sdc, then value is sdc */
    char *dev_node;

    /* refers full path of node of usbk device.
     * e.g. usbk device node is /dev/sdc, then value is /dev/sdc */
    char *dev_node_path;

    /* refers node of backdisk device which plugged to usbk device.
     * e.g. device node is /dev/sdb, then valuse is sdb */
    char *backdisk_node;

    /* refers full path of node of backdisk device.
     * e.g. backdisk device node is /dev/sdb, then value is /dev/sdb */
    char *backdisk_node_path;

    /* refers if usbk device supported or not */
    bool is_supported;

    /* refers usbk product information
     * e.g. USBK CryptoBridge 2.0 */
    char *product;

    /* refers usbk product model
     * e.g. A103, A101 */
    char *model;

    /* refers serial number of the usbk device
     * e.g. 842CBDAA3952313832312020FF0913 */
    char *serial;

    /* FIXME: ???
     * */
    char *usb_serial;

    /* refers firmware version of the usbk device
     * e.g. 2.5.4 */
    char *firmware_ver;

    /* refers number of keys in the usbk device
     * e.g. 3 */
    uint8_t multikey_cap;

    /* refers device label of the usbk device
     * This label is just a block device label.
     *
     * e.g. "USBK CryptoBridge 2.0" */
    char *dev_label;

    /* refers status of usbk device
     * e.g. USBK_DS_ACTIVATE, USBK_DS_ACTIVATE_WITH_BACKDISK */
    usbk_ds_t dev_state;

    /* refers the key working on. usbk device may contains more than one
     * key (multikey_cap contains this number)
     *
     * e.g. 1 */
    int current_key;

    /* refers the autoactivated key of the usbk. there might a key which
     * can be used without a password.
     * */
    int autoact_keyno;

    /* refers the remaining retry number. User must enter true password,
     * if he/she couldn't enter true password, then retry_num value
     * decreses 1. When retry_num value reaches 0, then ALL THE KEYS
     * AND PASSWORDS removes from the device and the backdisk becomes
     * a mass raw data.
     *
     * retry_num is 3 on true entry. */
    int retry_num;

    /* refers key labels for a better user interaction. User sets this.
     *
     * default values are "" */
    char **key_names;

    /* FIXME: remove this double-linked list structure */
    struct USBK *next;
};
typedef struct USBK USBK;

#define USBK_ASSIGN_PASSWORD(usbk, new_pass) \
    usbk_changepassword(usbk, NULL, new_pass)
#define USBK_SETKEY_NAME(usbk, pass, key_no, key_name) \
    usbk_setkey_keyname(usbk, pass, key_no, key_name, USBK_KEYSIZE_NULL, NULL)
#define USBK_SETKEY_128BIT_HEX(usbk, pass, key_no, key) \
    usbk_setkey_hex(usbk, pass, key_no, USBK_KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_HEX(usbk, pass, key_no, key) \
    usbk_setkey_hex(usbk, pass, key_no, USBK_KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_HEX(usbk, pass, key_no, key) \
    usbk_setkey_hex(usbk, pass, key_no, USBK_KEYSIZE_256BIT, key)

#if 0
/* FIXME macros of the depreciated function usbk_setkey_decimal */
#define USBK_SETKEY_128BIT_DECIMAL(usbk, pass, key_no, key) \
    usbk_setkey_decimal(usbk, pass, key_no, USBK_KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_DECIMAL(usbk, pass, key_no, key) \
    usbk_setkey_decimal(usbk, pass, key_no, USBK_KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_DECIMAL(usbk, pass, key_no, key) \
    usbk_setkey_decimal(usbk, pass, key_no, USBK_KEYSIZE_256BIT, key)
#endif

#define USBK_SETKEY_128BIT_TEXT(usbk, pass, key_no, key) \
    usbk_setkey_text(usbk, pass, key_no, USBK_KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_TEXT(usbk, pass, key_no, key) \
    usbk_setkey_text(usbk, pass, key_no, USBK_KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_TEXT(usbk, pass, key_no, key) \
    usbk_setkey_text(usbk, pass, key_no, USBK_KEYSIZE_256BIT, key)
#define USBK_ENABLE_AUTACT(usbk, pass, key_no) \
    usbk_set_autact(usbk, pass, key_no)
#define USBK_DISABLE_AUTACT(usbk, pass) \
    usbk_set_autact(usbk, pass, 0)

extern USBK* usbk_new(const char* dev);
extern int usbk_release(USBK* usbk);

/*
 * All the getter functions
 * */

/* This is the helper macro for reaching the USBK structure */
#define USBK_GET_ELEMENT(usbk, element)   (usbk->element)

#if defined(__GNUC__)
#  define inline inline __attribute__((always_inline))
#endif

/* Debugging informations */
extern void usbk_debug_enable(void);
extern void usbk_debug_disable(void);
extern void usbk_debug(bool enable);
extern bool usbk_debug_check(void);

extern const char* usbk_libversion(void);

/*
 * And all the setter and other functions which interacts
 * directly with USBK
 *
 * */
extern int usbk_key_activate(USBK* usbk, const char* pass, uint8_t key_no);
extern int usbk_key_deactivate(USBK* usbk);
extern int usbk_change_password(USBK* usbk, const char* old_pass, const char* new_pass);
extern int usbk_set_devicelabel(USBK* usbk, const char* pass, const char* device_label);
extern int usbk_set_keyname(USBK* usbk, const char* pass, uint8_t key_no, const char* key_name);
extern int usbk_set_key_hex(USBK* usbk, const char* pass, uint8_t key_no, usbk_keysize_t key_size, const uint8_t* key);
extern int usbk_set_key_text(USBK* usbk, const char* pass, uint8_t key_no, usbk_keysize_t key_size, const char* key);

#if 0
/* FIXME depreciated function */
extern int usbk_set_key_decimal(USBK* usbk, const char* pass, uint8_t key_no, usbk_keysize_t key_size, const char* key);
#endif

extern int usbk_set_key_and_keyname(USBK* usbk, const char* pass, int key_no, const char* key_name, usbk_keysize_t key_size, const uint8_t* key);
extern int usbk_set_autact(USBK* usbk, const char* pass, int key_no);
extern int usbk_get_randomkey(USBK* usbk, uint8_t*random_key, usbk_keysize_t keysize);
extern int usbk_refresh_usbkinfo(USBK* usbk);

inline const char* usbk_get_dev(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, dev_node); }
inline const char* usbk_get_dev_path(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, dev_node_path); }
inline const char* usbk_get_backdisk(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, backdisk_node); }
inline const char* usbk_get_backdisk_path(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, backdisk_node_path); }
inline const char* usbk_get_product(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, product); }
inline const char* usbk_get_model(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, model); }
inline const char* usbk_get_serial(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, serial); }
inline const char* usbk_get_usb_serial(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, usb_serial); }
inline const char* usbk_get_firmware_ver(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, firmware_ver); }
inline const char* usbk_get_dev_label(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, dev_label); }
inline const char* usbk_get_keyname(USBK* usbk, int key_no)
    { return USBK_GET_ELEMENT(usbk, key_names[key_no]); }

inline int usbk_get_lastopr_status(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, lastopr); }

inline bool usbk_check_support(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, is_supported); }
inline int usbk_get_multikeycap(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, multikey_cap); }
inline int usbk_get_current_keyno(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, current_key); }
inline int usbk_get_autoactivation_keyno(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, autoact_keyno); }
inline int usbk_get_retry_number(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, retry_num); }
inline usbk_ds_t usbk_get_state(USBK* usbk)
    { return USBK_GET_ELEMENT(usbk, dev_state); }

inline int usbk_set_keyname(USBK* usbk, const char* pass, uint8_t key_no, const char* key_name)
	{ return usbk_set_key_and_keyname(usbk, pass, key_no, key_name, USBK_KEYSIZE_NULL, NULL); }

#if defined(__linux__)

extern int get_usbk_count();

extern USBK* usbk_list_new();
extern int usbk_list_release();
extern USBK* usbk_list_get_next(USBK* usbk);
extern int usbk_list_get_counter();

extern int usbk_list_refreshall();

#define usbk_list_entry_foreach(list_entry, first_entry) \
    for (list_entry = first_entry; \
         list_entry != NULL; \
         list_entry = usbk_list_get_next(list_entry))

#endif /* defined(__linux__) */

#ifdef __cplusplus
}
#endif

#endif /* LIBUSBK_H_ */

