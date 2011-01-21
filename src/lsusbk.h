/*
 * @file lsusbk.h
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
 * Header of lsusbk.cpp
 *
 */

#ifndef LSUSBK_H_
#define LSUSBK_H_

//#define _XOPEN_SOURCE 500
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#ifndef _BSD_SOURCE
#  define _BSD_SOURCE
#endif
#ifndef _SVID_SOURCE
#  define _SVID_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <dirent.h>
#include <libgen.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <linux/major.h>

#include <features.h>

#include "general.h"

#define NAME_LEN_MAX 260
#define FT_OTHER 0
#define FT_BLOCK 1
#define FT_CHAR 2

typedef struct __ADDR_HCTL {
    int h;
    int c;
    int t;
    int l;
} ADDR_HCTL_T;

/* Device node list: contains the information needed to match a node with a
   sysfs class device. */
#define DEV_NODE_LIST_ENTRIES 16

enum dev_type { BLK_DEV, CHR_DEV };

#if 0
struct dev_node_list {
       struct dev_node_list *next;
       unsigned int count;
       struct dev_node_entry {
               unsigned int maj, min;
               enum dev_type type;
               time_t mtime;
               char name [ NAME_MAX + 1];
       } nodes[DEV_NODE_LIST_ENTRIES];
};
#endif

struct dev_node_entry {
    unsigned int maj;
    unsigned int min;
    enum dev_type type;
    time_t mtime;
    char name [ NAME_MAX + 1];
};

struct dev_node_list {
    struct dev_node_list *next;
    unsigned int count;
    struct dev_node_entry nodes[DEV_NODE_LIST_ENTRIES];
};


struct item_t {
    char name[NAME_LEN_MAX];
    int ft;
    int d_type;
};

extern int lsusbk(void);
extern int find_sdevices(USBK_T *usbk);

#endif // LSUSBK_H_

