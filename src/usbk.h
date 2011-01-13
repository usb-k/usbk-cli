/*
 * @file usbk.h
 *
 * Copyright (C) 2010
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

#ifdef _USBK_H

#include "general.h"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <usb.h>
#include <sys/ioctl.h>
#include "usbk_scsi.h"
#include "lsusbk.h"


void print_help(int exval);
void scan_usb();
void usbk_show_show_dev_info(t_usbk *usbk);
int usbk_get_dev_info(t_usbk *usbk);
int usbk_check_last_opr(t_usbk *usbk);
int check_key_decimal(string str, char *key);
int check_key_text(string str, char *key);

#endif
