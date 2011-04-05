/*
 * @file usbk.h
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

#ifndef USBK_H_
#define USBK_H_

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <usb.h>
#include <sys/ioctl.h>

#include "general.h"
#include "usbk_scsi.h"
#include "libusbk.h"


void print_help(int exval);
void print_version(void);
void linuxcli_show_dev_info(USBK *usbk);
//int usbk_get_dev_info(USBK_T *usbk);
//int usbk_check_last_opr(USBK_T *usbk);
int check_key_decimal(std::string str, U8 *key, int key_size_byte);
int check_key_text(std::string str, U8 *key, int key_size_byte);
#endif // USBK_H_

