/*
 * @file lsusb.cpp
 *
 * Copyright (C) 2003-2009 D. Gilbert
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
 * This is a utility program for listing SCSI devices and hosts (HBAs)
 * in the Linux operating system. It is applicable to kernel versions
 * 2.6.1 and greater.
 *
 */

#define _LSUSBK_H
#include "lsusbk.h"

#define USBK_VENDOR   "USBK"
#define USBK_MODEL    "Cryptographer"

/* Compare <host:controller:target:lun> tuples (aka <h:c:t:l> or hctl) */
static int
cmp_hctl(const struct addr_hctl * le, const struct addr_hctl * ri)
{
        if (le->h == ri->h) {
                if (le->c == ri->c) {
                        if (le->t == ri->t)
                                return ((le->l == ri->l) ? 0 :
                                        ((le->l < ri->l) ? -1 : 1));
                        else
                                return (le->t < ri->t) ? -1 : 1;
                } else
                        return (le->c < ri->c) ? -1 : 1;
        } else
                return (le->h < ri->h) ? -1 : 1;
}

static void
invalidate_hctl(struct addr_hctl * p)
{
        if (p) {
                p->h = -1;
                p->c = -1;
                p->t = -1;
                p->l = -1;
        }
}

/* Return 1 for directory entry that is link or directory (other than
 * a directory name starting with dot). Else return 0.
 */
static int
first_scandir_select(const struct dirent * s)
{
        if (FT_OTHER != aa_first.ft)
                return 0;
        if ((DT_LNK != s->d_type) &&
            ((DT_DIR != s->d_type) || ('.' == s->d_name[0])))
                return 0;
        strncpy(aa_first.name, s->d_name, NAME_LEN_MAX);
        aa_first.ft = FT_CHAR;  /* dummy */
        aa_first.d_type =  s->d_type;
        return 1;
}

/* scan for directory entry that is either a symlink or a directory */
static int
scan_for_first(const char * dir_name)
{
        struct dirent ** namelist;
        int num, k;

        aa_first.ft = FT_OTHER;
        num = scandir(dir_name, &namelist, first_scandir_select, NULL);
        if (num < 0) {
                return -1;
        }
        for (k = 0; k < num; ++k)
                free(namelist[k]);
        free(namelist);
        return num;
}

static int
non_sg_scandir_select(const struct dirent * s)
{
        int len;

        if (FT_OTHER != non_sg.ft)
                return 0;
        if ((DT_LNK != s->d_type) &&
            ((DT_DIR != s->d_type) || ('.' == s->d_name[0])))
                return 0;
        if (0 == strncmp("scsi_changer", s->d_name, 12)) {
                strncpy(non_sg.name, s->d_name, NAME_LEN_MAX);
                non_sg.ft = FT_CHAR;
                non_sg.d_type =  s->d_type;
                return 1;
        } else if (0 == strncmp("block", s->d_name, 5)) {
                strncpy(non_sg.name, s->d_name, NAME_LEN_MAX);
                non_sg.ft = FT_BLOCK;
                non_sg.d_type =  s->d_type;
                return 1;
        } else if (0 == strcmp("tape", s->d_name)) {
                strcpy(non_sg.name, s->d_name);
                non_sg.ft = FT_CHAR;
                non_sg.d_type =  s->d_type;
                return 1;
        } else if (0 == strncmp("scsi_tape:st", s->d_name, 12)) {
                len = strlen(s->d_name);
                if (isdigit(s->d_name[len - 1])) {
                        /* want 'st<num>' symlink only */
                        strcpy(non_sg.name, s->d_name);
                        non_sg.ft = FT_CHAR;
                        non_sg.d_type =  s->d_type;
                        return 1;
                } else
                        return 0;
        } else if (0 == strncmp("onstream_tape:os", s->d_name, 16)) {
                strcpy(non_sg.name, s->d_name);
                non_sg.ft = FT_CHAR;
                non_sg.d_type =  s->d_type;
                return 1;
        } else
                return 0;
}

static int
non_sg_scan(const char * dir_name)
{
        struct dirent ** namelist;
        int num, k;

        non_sg.ft = FT_OTHER;
        num = scandir(dir_name, &namelist, non_sg_scandir_select, NULL);
        if (num < 0) {
                return -1;
        }
        for (k = 0; k < num; ++k)
                free(namelist[k]);
        free(namelist);
        return num;
}

/*
static int
sg_scandir_select(const struct dirent * s)
{
        if (FT_OTHER != aa_sg.ft)
                return 0;
        if ((DT_LNK != s->d_type) &&
            ((DT_DIR != s->d_type) || ('.' == s->d_name[0])))
                return 0;
        if (0 == strncmp("scsi_generic", s->d_name, 12)) {
                strncpy(aa_sg.name, s->d_name, NAME_LEN_MAX);
                aa_sg.ft = FT_CHAR;
                aa_sg.d_type =  s->d_type;
                return 1;
        } else
                return 0;
}
*/
/*
static int
sg_scan(const char * dir_name)
{
        struct dirent ** namelist;
        int num, k;

        aa_sg.ft = FT_OTHER;
        num = scandir(dir_name, &namelist, sg_scandir_select, NULL);
        if (num < 0)
                return -1;
        for (k = 0; k < num; ++k)
                free(namelist[k]);
        free(namelist);
        return num;
}
*/

/* If 'dir_name'/'base_name' is a directory chdir to it. If that is successful
   return 1, else 0 */
static int
if_directory_chdir(const char * dir_name, const char * base_name)
{
        char buff[NAME_LEN_MAX];
        struct stat a_stat;

        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, base_name);
        if (stat(buff, &a_stat) < 0)
                return 0;
        if (S_ISDIR(a_stat.st_mode)) {
                if (chdir(buff) < 0)
                        return 0;
                return 1;
        }
        return 0;
}

/* If 'dir_name'/generic is a directory chdir to it. If that is successful
   return 1. Otherwise look a directory of the form
   'dir_name'/scsi_generic:sg<n> and if found chdir to it and return 1.
   Otherwise return 0. */
/*
static int
if_directory_ch2generic(const char * dir_name)
{
        char buff[NAME_LEN_MAX];
        struct stat a_stat;
        const char * old_name = "generic";

        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, old_name);
        if ((stat(buff, &a_stat) >= 0) && S_ISDIR(a_stat.st_mode)) {
                if (chdir(buff) < 0)
                        return 0;
                return 1;
        }
        // No "generic", so now look for "scsi_generic:sg<n>"
        if (1 != sg_scan(dir_name))
                return 0;
        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, aa_sg.name);
        if (stat(buff, &a_stat) < 0)
                return 0;
        if (S_ISDIR(a_stat.st_mode)) {
                if (chdir(buff) < 0)
                        return 0;
                return 1;
        }
        return 0;
}
*/
/* If 'dir_name'/'base_name' is found places corresponding value in 'value'
 * and returns 1 . Else returns 0.
 */
static int
get_value(const char * dir_name, const char * base_name, char * value, int max_value_len)
{
        char buff[NAME_LEN_MAX];
        FILE * f;
        int len;

        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, base_name);
        if (NULL == (f = fopen(buff, "r"))) {
                return 0;
        }
        if (NULL == fgets(value, max_value_len, f)) {
                /* assume empty */
                value[0] = '\0';
                fclose(f);
                return 1;
        }
        len = strlen(value);
        if ((len > 0) && (value[len - 1] == '\n'))
                value[len - 1] = '\0';
        fclose(f);
        return 1;
}

/* Allocate dev_node_list & collect info on every node in /dev. */
static void
collect_dev_nodes ()
{
        DIR *dirp;
        struct dirent *dep;
        char device_path[ PATH_MAX + 1];
        struct stat stats;
        struct dev_node_list *cur_list, *prev_list;
        struct dev_node_entry *cur_ent;

        if (dev_node_listhead) return; /* already collected nodes */

        dev_node_listhead = (struct dev_node_list*)malloc (sizeof(struct dev_node_list));
        if (!dev_node_listhead) return;

        cur_list = dev_node_listhead;
        cur_list->next = NULL;
        cur_list->count = 0;

        dirp = opendir (dev_dir);
        if (dirp == NULL) return;

        while (1)
        {
                dep = readdir (dirp);
                if (dep == NULL) break;

                snprintf (device_path, PATH_MAX, "%s/%s",
                          dev_dir, dep->d_name);
                device_path [PATH_MAX] = '\0';

                if (lstat(device_path, &stats))
                        continue;

                /* Skip non-block/char files. */
                if ( (!S_ISBLK(stats.st_mode)) && (!S_ISCHR(stats.st_mode)) )
                        continue;

                /* Add to the list. */
                if (cur_list->count >= DEV_NODE_LIST_ENTRIES)
                {
                        prev_list = cur_list;
                        cur_list = (struct dev_node_list*)malloc (sizeof(struct dev_node_list));
                        if (!cur_list) break;
                        prev_list->next = cur_list;
                        cur_list->next = NULL;
                        cur_list->count = 0;
                }

                cur_ent = &cur_list->nodes[cur_list->count];
                cur_ent->maj = major(stats.st_rdev);
                cur_ent->min = minor(stats.st_rdev);
                if (S_ISBLK(stats.st_mode))
                        cur_ent->type = BLK_DEV;
                else if (S_ISCHR(stats.st_mode))
                        cur_ent->type = CHR_DEV;
                cur_ent->mtime = stats.st_mtime;
                strncpy(cur_ent->name, device_path, NAME_MAX);

                cur_list->count++;
        }

        closedir(dirp);
}

/* Free dev_node_list. */
static void
free_dev_node_list ()
{
        if (dev_node_listhead)
        {
                struct dev_node_list *cur_list, *next_list;

                cur_list = dev_node_listhead;
                while (cur_list)
                {
                        next_list = cur_list->next;
                        free(cur_list);
                        cur_list = next_list;
                }

                dev_node_listhead = NULL;
        }
}

/* Given a path to a class device, find the most recent device node with
   matching major/minor. */

static int
get_dev_node (char *wd, char *node, enum dev_type type)
{
        struct dev_node_list *cur_list;
        struct dev_node_entry *cur_ent;
        char value[NAME_LEN_MAX];
        unsigned int maj, min;
        time_t newest_mtime = 0;
        int match_found = 0;
        unsigned int i;

        strcpy(node,"-");

        if (dev_node_listhead == NULL)
        {
                collect_dev_nodes();
                if (dev_node_listhead == NULL) goto exit;
        }

        // Get the major/minor for this device.
        if (!get_value(wd, "dev", value, NAME_LEN_MAX)) goto exit;
        sscanf(value, "%u:%u", &maj, &min);

        // Search the node list for the newest match on this major/minor.
        cur_list = dev_node_listhead;
        i = 0;

        while (1)
        {
                if (i >= cur_list->count)
                {
                        cur_list = cur_list->next;
                        if (!cur_list) break;
                        i = 0;
                }

                cur_ent = &cur_list->nodes[i];
                i++;

                if ( (maj == cur_ent->maj) &&
                     (min == cur_ent->min) &&
                     (type == cur_ent->type) )
                {
                        if ( (!match_found) ||
                             (difftime(cur_ent->mtime,newest_mtime) > 0) )
                        {
                                newest_mtime = cur_ent->mtime;
                                strncpy(node, cur_ent->name, NAME_MAX);
                        }
                        match_found = 1;
                }
        }

exit:
        return match_found;
}


/*  Parse colon_list into host/channel/target/lun ("hctl") array,
 *  return 1 if successful, else 0.
 */
static int
parse_colon_list(const char * colon_list, struct addr_hctl * outp)
{
        const char * elem_end;

        if ((! colon_list) || (! outp))
                return 0;
        if (1 != sscanf(colon_list, "%d", &outp->h))
                return 0;
        if (NULL == (elem_end = strchr(colon_list, ':')))
                return 0;
        colon_list = elem_end + 1;
        if (1 != sscanf(colon_list, "%d", &outp->c))
                return 0;
        if (NULL == (elem_end = strchr(colon_list, ':')))
                return 0;
        colon_list = elem_end + 1;
        if (1 != sscanf(colon_list, "%d", &outp->t))
                return 0;
        if (NULL == (elem_end = strchr(colon_list, ':')))
                return 0;
        colon_list = elem_end + 1;
        if (1 != sscanf(colon_list, "%d", &outp->l))
                return 0;
        return 1;
}

static void
one_sdev_entry(const char * dir_name, const char * devname)
{
        char buff[NAME_LEN_MAX];
        char value[NAME_LEN_MAX];

        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, devname);
/*        
        snprintf(value, NAME_LEN_MAX, "[%s]", devname);
        printf("%-13s", value);

        if (! get_value(buff, "type", value, NAME_LEN_MAX)) {
                printf("type?   ");
        } else if (1 != sscanf(value, "%d", &type)) {
                printf("type??  ");
        } else if ((type < 0) || (type > 31)) {
                printf("type??? ");
        } else
                printf("%s ", scsi_short_device_types[type]);
*/
        if (get_value(buff, "vendor", value, NAME_LEN_MAX)){
                if(strncmp(USBK_VENDOR, value, strlen(USBK_VENDOR)))
                  return;
        }
        else
                  return;

        if (get_value(buff, "model", value, NAME_LEN_MAX)){
        		//if(strcmp(USBK_MODEL, value))
        		if(strncmp(USBK_MODEL, value, strlen(USBK_MODEL)))
                  return;
        }
        else
                  return;



        if (1 == non_sg_scan(buff)) {
                char wd[NAME_LEN_MAX];
                char extra[NAME_LEN_MAX];

                if (DT_DIR == non_sg.d_type) {
                        strcpy(wd, buff);
                        strcat(wd, "/");
                        strcat(wd, non_sg.name);
                        if (1 == scan_for_first(wd))
                                strcpy(extra, aa_first.name);
                        else {
                                printf("unexpected scan_for_first error");
                                wd[0] = '\0';
                        }
                } else {
                        strcpy(wd, buff);
                        strcpy(extra, non_sg.name);
                }
                if (wd[0] && (if_directory_chdir(wd, extra))) {
                        if (NULL == getcwd(wd, NAME_LEN_MAX)) {
                                printf("getcwd error");
                                wd[0] = '\0';
                        }
                }
                if (wd[0]) {
                        char dev_node[NAME_MAX + 1] = "";
                        enum dev_type typ;

                        typ = (FT_BLOCK == non_sg.ft) ? BLK_DEV : CHR_DEV;
                        
                        if (!get_dev_node(wd, dev_node, typ))
                                snprintf(dev_node, NAME_MAX, "-               ");

                        printf("%-16s", dev_node);
                }
        } else
                printf("-       ");

        if (get_value(buff, "vendor", value, NAME_LEN_MAX))
                printf("%-8s ", value);
        else
                printf("vendor?  ");

        if (get_value(buff, "model", value, NAME_LEN_MAX))
                printf("%-16s ", value);
        else
                printf("model?           ");

        if (get_value(buff, "rev", value, NAME_LEN_MAX))
                printf("%-4s  ", value);
        else
                printf("rev?  ");

/*
 * /dev/sg4 yazditan rutin
 * 
        if (if_directory_ch2generic(buff)) {
                char wd[NAME_LEN_MAX];

                if (NULL == getcwd(wd, NAME_LEN_MAX))
                        printf("  generic_dev error");
                else {
                        char dev_node[NAME_MAX + 1] = "";

                        if (!get_dev_node(wd, dev_node, CHR_DEV))
                                snprintf(dev_node, NAME_MAX, "-");

                        printf("  %s", dev_node);
                }
        }
        else
                printf("  -");
*/
        printf("\n");
}

int match_sdev_entry(const char * dir_name, const char * devname, t_usbk *usbk) // char *dev_find, char *sg_dev)
{
        char buff[NAME_LEN_MAX];
        char value[NAME_LEN_MAX];

        strcpy(buff, dir_name);
        strcat(buff, "/");
        strcat(buff, devname);

        if (get_value(buff, "vendor", value, NAME_LEN_MAX)){
                if(strncmp(USBK_VENDOR, value, strlen(USBK_VENDOR)))
                  return 0;
        }
        else
                  return 0;

        if (get_value(buff, "vendor", value, NAME_LEN_MAX))
                sprintf(usbk->vendor, "%s ", value);
        else
                sprintf(usbk->vendor, "vendor?  ");

        if (get_value(buff, "model", value, NAME_LEN_MAX))
                sprintf(usbk->model, "%s ", value);
        else
                sprintf(usbk->model, "model?           ");

        if (get_value(buff, "rev", value, NAME_LEN_MAX))
                sprintf(usbk->rev, "%s  ", value);
        else
                sprintf(usbk->rev, "rev?  ");

        if (1 == non_sg_scan(buff)) {
                char wd[NAME_LEN_MAX];
                char extra[NAME_LEN_MAX];

                if (DT_DIR == non_sg.d_type) {
                        strcpy(wd, buff);
                        strcat(wd, "/");
                        strcat(wd, non_sg.name);
                        if (1 == scan_for_first(wd))
                                strcpy(extra, aa_first.name);
                        else {
                                return 0;
                        }
                } else {
                        strcpy(wd, buff);
                        strcpy(extra, non_sg.name);
                }
                if (wd[0] && (if_directory_chdir(wd, extra))) {
                        if (NULL == getcwd(wd, NAME_LEN_MAX)) {
                                return 0;
                        }
                }
                if (wd[0]) {
                        char dev_node[NAME_MAX + 1] = "";
                        enum dev_type typ;

                        typ = (FT_BLOCK == non_sg.ft) ? BLK_DEV : CHR_DEV;
                        
                        if (!get_dev_node(wd, dev_node, typ))
                                snprintf(dev_node, NAME_MAX, "-               ");

                        if(strcmp(dev_node, usbk->dev) != 0){
                          return 0;
                        }
                        
                }
        } else
                return 0;
/*
        if (if_directory_ch2generic(buff)) {
                char wd[NAME_LEN_MAX];

                if (NULL == getcwd(wd, NAME_LEN_MAX))
                        return 0;
                else {
                        char dev_node[NAME_MAX + 1] = "";

                        if (!get_dev_node(wd, dev_node, CHR_DEV))
                                snprintf(dev_node, NAME_MAX, "-");

                        //printf("  %s", dev_node);
                        sprintf(usbk->sg_dev, "%s", dev_node);
                        return 1;
                }
        }
        else
                return 0;
*/
        return 1;
}


static int
sdev_scandir_select(const struct dirent * s)
{
/* Following no longer needed but leave for early lk 2.6 series */
        if (strstr(s->d_name, "mt"))
                return 0;       /* st auxiliary device names */
        if (strstr(s->d_name, "ot"))
                return 0;       /* osst auxiliary device names */
        if (strstr(s->d_name, "gen"))
                return 0;
/* Above no longer needed but leave for early lk 2.6 series */
        if (!strncmp(s->d_name, "host", 4)) /* SCSI host */
                return 0;
        if (!strncmp(s->d_name, "target", 6)) /* SCSI target */
                return 0;
        if (strchr(s->d_name, ':')) {
                if (filter_active) {
                        struct addr_hctl s_hctl;

                        if (! parse_colon_list(s->d_name, &s_hctl)) {
                                fprintf(stderr, "sdev_scandir_select: parse "
                                        "failed\n");
                                return 0;
                        }
                        if (((-1 == filter.h) || (s_hctl.h == filter.h)) &&
                            ((-1 == filter.c) || (s_hctl.c == filter.c)) &&
                            ((-1 == filter.t) || (s_hctl.t == filter.t)) &&
                            ((-1 == filter.l) || (s_hctl.l == filter.l)))
                                return 1;
                        else
                                return 0;
                } else
                        return 1;
        }
        /* Still need to filter out "." and ".." */
        return 0;
}

/* Returns -1 if (a->d_name < b->d_name) ; 0 if they are equal
 * and 1 otherwise.
 * Function signature was more generic before version 0.23 :
 * static int sdev_scandir_sort(const void * a, const void * b)
 */

#if(__GLIBC_MINOR__ >= 9)
static int sdev_scandir_sort(const struct dirent ** a, const struct dirent ** b){
#else
static int sdev_scandir_sort(const void* p1, const void *p2){
const struct dirent * const * a = reinterpret_cast<const struct dirent * const *>(p1); 
const struct dirent * const * b = reinterpret_cast<const struct dirent * const *>(p2); 
//return (*d1)->d_namlen-(*d2)->d_namlen; 
#endif
        const char * lnam = (*a)->d_name;
        const char * rnam = (*b)->d_name;
        struct addr_hctl left_hctl;
        struct addr_hctl right_hctl;

        if (! parse_colon_list(lnam, &left_hctl)) {
                fprintf(stderr, "sdev_scandir_sort: left parse failed\n");
                return -1;
        }
        if (! parse_colon_list(rnam, &right_hctl)) {
                fprintf(stderr, "sdev_scandir_sort: right parse failed\n");
                return 1;
        }
        return cmp_hctl(&left_hctl, &right_hctl);
}

/* List SCSI devices (LUs). */
static void
list_sdevices(void)
{
        char buff[NAME_LEN_MAX];
        char name[NAME_LEN_MAX];
        struct dirent ** namelist;
        int num, k;

        strcpy(buff, sysfsroot);
        strcat(buff, bus_scsi_devs);

        num = scandir(buff, &namelist, sdev_scandir_select,
                      sdev_scandir_sort);
        if (num < 0) {  /* scsi mid level may not be loaded */
                return;
        }
//              1234567890123456 12345678 1234567890123456 1234
        printf("device          product  model            ver\n");

        for (k = 0; k < num; ++k) {
                strncpy(name, namelist[k]->d_name, NAME_LEN_MAX);
                //printf("[%s %s]", name, buff);
                one_sdev_entry(buff, name);
                free(namelist[k]);
        }
        printf("\n");
        free(namelist);
}

int
lsusbk(void)
{
        invalidate_hctl(&filter);
        list_sdevices();
        free_dev_node_list();
        return 0;
}


int find_sdevices(t_usbk *usbk){
        char buff[NAME_LEN_MAX];
        char name[NAME_LEN_MAX];
        struct dirent ** namelist;
        int num, k;
        int ret_val = 0;

        strcpy(buff, sysfsroot);
        strcat(buff, bus_scsi_devs);

        num = scandir(buff, &namelist, sdev_scandir_select,
                      sdev_scandir_sort);
        if (num < 0) {  /* scsi mid level may not be loaded */
                return 0;
        }

        for (k = 0; k < num; ++k) {
                strncpy(name, namelist[k]->d_name, NAME_LEN_MAX);
                if(match_sdev_entry(buff, name, usbk) == 1){
                  ret_val = 1;
                  free(namelist[k]);
                  break;
                }
                free(namelist[k]);
        }
        free(namelist);
        return ret_val;  
}

