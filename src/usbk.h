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
