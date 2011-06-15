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

#include <inttypes.h>

typedef enum __attribute__((__packed__)) __KEYSIZE
{
    KEYSIZE_NULL = 0,
    KEYSIZE_128BIT = 1,
    KEYSIZE_192BIT = 2,
    KEYSIZE_256BIT = 3,
}KEYSIZE;

//-USBK DEVICE STATES
typedef enum __USBK_DS
{
    USBK_DS_ACTIVATE                = 1,
    USBK_DS_ACTIVATE_WITH_BACKDISK  = 2,
    USBK_DS_DEACTIVATE              = 3,
    USBK_DS_FABRIC_DEFAULT          = 4,
    USBK_DS_MUST_REMOVE             = 5,
}USBK_DS;

//-USBK DEVICE INFORMATION STRUCTURE
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

class usbk
{
private:
    // Variables
    int debug_enable;
    //Functions
    int commandstatus();
    int lastop();
    int getdeviceinfo();
    int getudevbackdisk();
    int getudevinfo(const char *device);
    int getstate();

    int keysize_byte(KEYSIZE keysize);
    int convertkey_decimal2hex(uint8_t *key_hex, const char* key_decimal, KEYSIZE keysize);
    int convertkey_text2hex(uint8_t *key_hex, const char* key_text, KEYSIZE keysize);

    int setkey_keyname(const char *pass, int key_no, const char* key_name, KEYSIZE key_size, const uint8_t* key);
    int setautact(const char *pass, int key_no);

public:
    // Variable
    USBK_INFO info;
    int lastopr;

    // Function
    usbk(const char* dev);
    ~usbk(){};

    int activatekey(const char* pass, uint8_t key_no);
    int deactivatekey();
    int assignpassword(const char* new_pass);
    int changepassword(const char* old_pass, const char* new_pass);
    int setdevicelabel(const char* pass, const char* device_label);

    int setkeyname(const char *pass, uint8_t key_no, const char* key_name);


    int setkey_hex(const char *pass, uint8_t key_no, KEYSIZE key_size, const uint8_t* key);
    int setkey_decimal(const char *pass, uint8_t key_no, KEYSIZE key_size, const char* key);
    int setkey_text(const char *pass, uint8_t key_no, KEYSIZE key_size, const char* key);

    int setkey_128bit_hex(const char *pass, uint8_t key_no, const uint8_t * key);
    int setkey_192bit_hex(const char *pass, uint8_t key_no, const uint8_t * key);
    int setkey_256bit_hex(const char *pass, uint8_t key_no, const uint8_t * key);

    int setkey_128bit_decimal(const char *pass, uint8_t key_no, char* key);
    int setkey_192bit_decimal(const char *pass, uint8_t key_no, char* key);
    int setkey_256bit_decimal(const char *pass, uint8_t key_no, char* key);

    int setkey_128bit_text(const char *pass, uint8_t key_no, const char* key);
    int setkey_192bit_text(const char *pass, uint8_t key_no, const char* key);
    int setkey_256bit_text(const char *pass, uint8_t key_no, const char* key);

    int enableautact(const char *pass, uint8_t key_no);
    int disableautact(const char *pass);

    int getrandomkey(uint8_t *random_key, KEYSIZE keysize);

    int refreshusbkinfo();
};

typedef struct __USBK_LIST
{
    usbk*               me;
    struct __USBK_LIST* next;
    struct __USBK_LIST* previous;
}USBK_LIST;

class usbklist
{
private:
    //variables
    int debug_enable;
    int counter;
    //functions
public:
    //variables
    USBK_LIST* usbks;
    int lastopr;
    //functions
    usbklist();
    ~usbklist();
    int refreshall();
};

#endif

