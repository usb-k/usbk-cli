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

#if defined(__linux__)
#  define USBK_ATTRIBUTE_PACKED __attribute__((__packed__))
#else
#  define USBK_ATTRIBUTE_PACKED
#endif

typedef enum USBK_ATTRIBUTE_PACKED __KEYSIZE
{
    KEYSIZE_NULL = 0,
    KEYSIZE_128BIT = 1,
    KEYSIZE_192BIT = 2,
    KEYSIZE_256BIT = 3,
} KEYSIZE;

//-USBK DEVICE STATES
typedef enum __USBK_DS
{
    USBK_DS_ACTIVATE                = 1,
    USBK_DS_ACTIVATE_WITH_BACKDISK  = 2,
    USBK_DS_DEACTIVATE              = 3,
    USBK_DS_FABRIC_DEFAULT          = 4,
    USBK_DS_MUST_REMOVE             = 5,
} USBK_DS;

typedef enum __USBK_LASTOPR
{
    USBK_LO_PASS                = 0,
    USBK_LO_GEN_FAIL            = 1,
    USBK_LO_FAILED_PASS         = 2,
    USBK_LO_FABRIC_RESET        = 3,
    USBK_LO_USBK_UNPLUGING      = 4,
    USBK_LO_INVALID_KEYNO       = 5,
    USBK_LO_INVALID_KEYSIZE     = 6,
    USBK_LO_INVALID_DEVICELABEL = 7,
    USBK_LO_INVALID_PASS        = 8,
    USBK_LO_INVALID_NEWPASS     = 9,

    USBK_LO_STATE_ERROR         = 10,
    USBK_LO_SCSI_ERROR          = 11,
    USBK_LO_UNSUPPORTED_USBK    = 12,
    USBK_LO_INVALID_KEY         = 13,

    USBK_LO_UDEV_ERROR          = 14,
    USBK_LO_MEM_ERROR           = 15,
} LIBUSBK_LASTOPR;

typedef struct __USBK USBK;

#define USBK_ASSIGN_PASSWORD(usbk, new_pass)                 usbk_changepassword(usbk, NULL, new_pass)
#define USBK_SETKEY_NAME(usbk, pass, key_no, key_name)       usbk_setkey_keyname(usbk, pass, key_no, key_name, KEYSIZE_NULL, NULL)
#define USBK_SETKEY_128BIT_HEX(usbk, pass, key_no, key)      usbk_setkey_hex(usbk, pass, key_no, KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_HEX(usbk, pass, key_no, key)      usbk_setkey_hex(usbk, pass, key_no, KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_HEX(usbk, pass, key_no, key)      usbk_setkey_hex(usbk, pass, key_no, KEYSIZE_256BIT, key)
#define USBK_SETKEY_128BIT_DECIMAL(usbk, pass, key_no, key)  usbk_setkey_decimal(usbk, pass, key_no, KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_DECIMAL(usbk, pass, key_no, key)  usbk_setkey_decimal(usbk, pass, key_no, KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_DECIMAL(usbk, pass, key_no, key)  usbk_setkey_decimal(usbk, pass, key_no, KEYSIZE_256BIT, key)
#define USBK_SETKEY_128BIT_TEXT(usbk, pass, key_no, key)     usbk_setkey_text(usbk, pass, key_no, KEYSIZE_128BIT, key)
#define USBK_SETKEY_192BIT_TEXT(usbk, pass, key_no, key)     usbk_setkey_text(usbk, pass, key_no, KEYSIZE_192BIT, key)
#define USBK_SETKEY_256BIT_TEXT(usbk, pass, key_no, key)     usbk_setkey_text(usbk, pass, key_no, KEYSIZE_256BIT, key)
#define USBK_ENABLE_AUTACT(usbk, pass, key_no)               usbk_setautact(usbk, pass, key_no)
#define USBK_DISABLE_AUTACT(usbk, pass)                      usbk_setautact(usbk, pass, 0)

USBK* usbk_new(const char* dev);
extern int usbk_release(USBK* usbk);

/*
 * All the getter functions
 * */

/* This is the helper macro for reaching the USBK structure */
#define USBK_GET_ELEMENT(usbk, element)   (usbk->element)

inline const char* usbk_get_dev(USBK* usbk)          { return USBK_GET_ELEMENT(usbk, dev); }
inline const char* usbk_get_dev_path(USBK* usbk)     { return USBK_GET_ELEMENT(usbk, dev_path); }
inline const char* usbk_get_backdisk(USBK* usbk)     { return USBK_GET_ELEMENT(usbk, backdisk); }
inline const char* usbk_get_backdisk_path(USBK* usbk) { return USBK_GET_ELEMENT(usbk, backdisk_path); }
inline const char* usbk_get_product(USBK* usbk)      { return USBK_GET_ELEMENT(usbk, product); }
inline const char* usbk_get_model(USBK* usbk)        { return USBK_GET_ELEMENT(usbk, model); }
inline const char* usbk_get_serial(USBK* usbk)       { return USBK_GET_ELEMENT(usbk, serial); }
inline const char* usbk_get_usb_serial(USBK* usbk)   { return USBK_GET_ELEMENT(usbk, usb_serial); }
inline const char* usbk_get_firmware_ver(USBK* usbk) { return USBK_GET_ELEMENT(usbk, firmware_ver); }
inline const char* usbk_get_dev_label(USBK* usbk)    { return USBK_GET_ELEMENT(usbk, dev_label); }
inline const char* usbk_get_keyname(USBK* usbk, int key_no) { return USBK_GET_ELEMENT(usbk, key_names[key_no]); }

inline int usbk_get_lastopr_status(USBK* usbk)       { return USBK_GET_ELEMENT(usbk, lastopr); }

inline bool usbk_check_support(USBK* usbk)           { return USBK_GET_ELEMENT(usbk, supported); }
inline int usbk_get_multikeycap(USBK* usbk)          { return USBK_GET_ELEMENT(usbk, multikey_cap); }
inline int usbk_get_current_keyno(USBK* usbk)        { return USBK_GET_ELEMENT(usbk, current_key); }
inline int usbk_get_autoactivation_keyno(USBK* usbk) { return USBK_GET_ELEMENT(usbk, autoact_keyno); }
inline int usbk_get_retry_number(USBK* usbk)         { return USBK_GET_ELEMENT(usbk, retry_num); }
inline USBK_DS usbk_get_state(USBK* usbk)            { return USBK_GET_ELEMENT(usbk, dev_state); }

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
extern int usbk_activatekey(USBK* usbk, const char* pass, uint8_t key_no);
extern int usbk_deactivatekey(USBK* usbk);
extern int usbk_changepassword(USBK* usbk, const char* old_pass, const char* new_pass);
extern int usbk_setdevicelabel(USBK* usbk, const char* pass, const char* device_label);
extern int usbk_setkeyname(USBK* usbk, const char* pass, uint8_t key_no, const char* key_name);
extern int usbk_setkey_keyname(USBK* usbk, const char* pass, int key_no, const char* key_name, KEYSIZE key_size, const uint8_t* key);
extern int usbk_setkey_hex(USBK* usbk, const char* pass, uint8_t key_no, KEYSIZE key_size, const uint8_t* key);
extern int usbk_setkey_decimal(USBK* usbk, const char* pass, uint8_t key_no, KEYSIZE key_size, const char* key);
extern int usbk_setkey_text(USBK* usbk, const char* pass, uint8_t key_no, KEYSIZE key_size, const char* key);
extern int usbk_setautact(USBK* usbk, const char* pass, int key_no);
extern int usbk_getrandomkey(USBK* usbk, uint8_t*random_key, KEYSIZE keysize);
extern int usbk_refreshusbkinfo(USBK* usbk);

#if defined(__linux__)

typedef struct __USBKS USBKS;

extern USBKS* usbk_list_new(void);
extern int usbk_list_release(USBKS* usbks);
extern USBK* usbk_list_get_entry(USBKS* usbks);
extern USBK* usbk_list_get_next(USBK* usbk);
extern USBK* usbk_list_get_previous(USBK* usbk);
extern int usbk_list_get_counter(USBKS* usbks);
extern int usbk_list_get_lastoprstatus(USBKS* usbks);

extern int usbk_list_refreshall(USBKS* usbks);

#define usbk_list_entry_foreach(list_entry, first_entry) \
    for (list_entry = first_entry; \
         list_entry != NULL; \
         list_entry = usbk_list_get_next(list_entry))

#endif /* defined(__linux__) */

#ifdef __cplusplus
}
#endif

#endif /* LIBUSBK_H_ */

