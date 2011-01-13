/*
 * @file lsusbk.h
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
 * Header of lsusbk.cpp
 *
 */

#ifdef _LSUSBK_H

//#define _XOPEN_SOURCE 500
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <linux/major.h>
#include <time.h>
#include "general.h"

#define NAME_LEN_MAX 260
#define FT_OTHER 0
#define FT_BLOCK 1
#define FT_CHAR 2

static const char * sysfsroot = "/sys";
static const char * bus_scsi_devs = "/bus/scsi/devices";
static const char * dev_dir = "/dev";


struct addr_hctl {
        int h;
        int c;
        int t;
        int l;
};

struct addr_hctl filter;
static int filter_active = 0;

/* Device node list: contains the information needed to match a node with a
   sysfs class device. */
#define DEV_NODE_LIST_ENTRIES 16
enum dev_type { BLK_DEV, CHR_DEV};
/*
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
*/
struct dev_node_entry {
        unsigned int maj, min;
        enum dev_type type;
        time_t mtime;
        char name [ NAME_MAX + 1];
};

struct dev_node_list {
       struct dev_node_list *next;
       unsigned int count;
       struct dev_node_entry nodes[DEV_NODE_LIST_ENTRIES];
};


static struct dev_node_list* dev_node_listhead = NULL;

struct item_t {
        char name[NAME_LEN_MAX];
        int ft;
        int d_type;
};

static struct item_t non_sg;
//static struct item_t aa_sg;
static struct item_t aa_first;

static int cmp_hctl(const struct addr_hctl * le, const struct addr_hctl * ri);
static void invalidate_hctl(struct addr_hctl * p);
static int first_scandir_select(const struct dirent * s);
static int scan_for_first(const char * dir_name);
static int non_sg_scandir_select(const struct dirent * s);
static int non_sg_scan(const char * dir_name);
static int if_directory_chdir(const char * dir_name, const char * base_name);
static int get_value(const char * dir_name, const char * base_name, char * value, int max_value_len);
static void collect_dev_nodes ();
static void free_dev_node_list ();
static int get_dev_node (char *wd, char *node, enum dev_type type);
static int parse_colon_list(const char * colon_list, struct addr_hctl * outp);
static void one_sdev_entry(const char * dir_name, const char * devname);
int match_sdev_entry(const char * dir_name, const char * devname, t_usbk *usbk);
static int sdev_scandir_select(const struct dirent * s);
static int sdev_scandir_sort(const struct dirent ** a, const struct dirent ** b);
static void list_sdevices(void);
int lsusbk(void);
int find_sdevices(t_usbk *usbk);


#else

extern int lsusbk(void);
extern int find_sdevices(t_usbk *usb);

#endif
