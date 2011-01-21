/*
 * @file main.cpp
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
 * Main function of USB-K crypro device
 *
 */

//TODO: BackDisk in mount noktasını bul ve 'Show device list' de göster. 'Device info'da BackDisk sutunu kaldir.
//TODO: BackDisk mount edilmiş ise deactive işlemi sırasında kullanıcıya uyarı ver ve emin misin diye sor.

#include <iostream>

#include "usbk.h"

#define USBK_PRODUCT_ID         0xa100
#define USBK_VENDOR_ID         0x2384

using namespace std;

/* Flag set by ‘--verbose’. */
static int verbose_flag;

t_usbk usbk;

int main(int argc, char *argv[])
{
    int c;
    t_UI_ACTIVATE act;
    t_UI_PAROLA opt_parola;
    t_UI_AESNB opt_key;
    t_UI_DEVNAME opt_dev_name;
    t_UI_SET_DEV_NAME set_dev_name;
    t_UI_SET_AES_KEY set_key;
    t_UI_KEYNAME opt_aes_name;
    t_UI_CHANGE_PASS change_pas;
    t_UI_PAROLA opt_new_password;
    tAESKEY opt_aes_key;
    t_UI_SET_AUTO_ACTIVATE set_auto;
    t_UI_OPTION opt_auto_activate;
    char opt_key_format;
    char opt_string_key[64];

    /* configs for cmd-args*/
    int aflag = 0;
    int dflag = 0;
    int cflag = 0;
    int nflag = 0;
    int mflag = 0;
    int xflag = 0;
    int tflag = 0;
    int Tflag = 0;
    int lflag = 0;
    int sflag = 0;
    int kflag = 0; opt_key = 1; // default key no 1
    int pflag = 0;
    int fflag = 1; opt_key_format = 'd'; // default key format decimal
    int iflag = 0;
    int vflag = 0;

    int main_operation = 0;

    while (1) {
        static struct option long_options[] = {
        /* These options set a flag. */
        { "verbose", no_argument, &verbose_flag, 1 },
        { "brief", no_argument, &verbose_flag, 0 },
        /* These options don't set a flag. We distinguish them by their indices. */
        { "activate", no_argument, 0, 'a' },
        { "deactivate", no_argument, 0, 'd' },
        { "newpass", required_argument, 0, 'c' },
        { "devname", required_argument, 0, 'n' },
        { "keyname", required_argument, 0, 'm' },
        { "change-key", required_argument, 0, 'x' },
        { "enable-auto", no_argument, 0, 't' },
        { "disable-auto", no_argument, 0, 'T' },
        { "gen-key", no_argument, 0, 'l' },
        { "show-devices", no_argument, 0, 's' },
        { "key-no", required_argument, 0, 'k' },
        { "passwd", required_argument, 0, 'p' },
        { "key-format", required_argument, 0, 'f' },
        { "show-info", no_argument, 0, 'i' },
        { "ver", no_argument, 0, 'v' },
        { "help", no_argument, 0, '?' },
        { 0, 0, 0, 0 }
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, ":adc:n:m:x:tTlsk:p:f:iv?", long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 'a':
            aflag = 1;
            main_operation++;
            break;

        case 'd':
            dflag = 1;
            main_operation++;
            break;

        case 'c':
            cflag = 1;
            main_operation++;
            if(strlen(optarg) > sizeof(t_UI_PAROLA)){
                printf("Uyarı: Parola %d karakterden uzun olamaz.\n", sizeof(t_UI_PAROLA));
                exit(1);
            }
            strncpy(opt_new_password.u8, optarg, sizeof(t_UI_PAROLA));
            break;

        case 'n':
            nflag = 1;
            main_operation++;
            strncpy(opt_dev_name.c, optarg, sizeof(t_UI_DEVNAME));
            break;

        case 'm':
            mflag = 1;
            //main_operation++;
            if(strlen(optarg) > sizeof(t_UI_KEYNAME)){
                printf("Uyarı: Key adi %d karakterden uzun olamaz.\n", sizeof(t_UI_KEYNAME));
                exit(1);
            }
            strncpy(opt_aes_name.c, optarg, sizeof(t_UI_KEYNAME));
            break;

        case 'x':
            xflag = 1;
            main_operation++;
            sprintf(opt_string_key, "%s", optarg);
            break;

        case 't':
            tflag = 1;
            main_operation++;
            opt_auto_activate = 'E';
            break;

        case 'T':
            Tflag = 1;
            main_operation++;
            opt_auto_activate = 'D';
            break;

        case 'l':
            lflag = 1;
            main_operation++;
            break;

        case 's':
            sflag = 1;
            main_operation++;
            break;

        case 'k':
            kflag = 1;
            opt_key = atoi(optarg);
            break;

        case 'p':
            pflag = 1;
            if(strlen(optarg) > sizeof(t_UI_PAROLA)){
                printf("Uyarı: Parola %d karakterden uzun olamaz.\n", sizeof(t_UI_PAROLA));
                exit(1);
            }
            strncpy(opt_parola.u8, optarg, sizeof(t_UI_PAROLA));
            break;

        case 'f':
            fflag = 1;
            if(!strcmp(optarg, "d")){
                opt_key_format = 'd';
            }
            else if(!strcmp(optarg, "t")){
                opt_key_format = 't';
            }
            else{
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(1);
            }
            break;

        case 'i':
            iflag = 1;
            break;

        case 'v':
            vflag = 1;
            break;

        case '?':
            print_help(0);
            break;

        default:
            break;
            //abort ();
        }
    }

    if(main_operation > 1){
        printf("usbk: You may not specify more than one `-adcnmxtTls' option\n");
        printf("usbk: Try `usbk --help' for more information.\n");
        exit(1);
    }

    if (vflag) {
        printf("%s version %s\n", PACKAGE, VERSION);
    }

    if (sflag) {
        lsusbk();
        exit(1);
    }

    if(!iflag && main_operation == 0){
        exit(0);
    }

    strncpy(usbk.dev, argv[argc - 1], SIZE_DEVNAME);
    if (find_sdevices(&usbk) == 0) {
        printf("Hata: %s üzerinde USBK cihazı bulunamadi\n", usbk.dev);
        exit(1);
    }
    usbk_get_dev_info(&usbk);

    /////////////////////////////////////////////
    // ACTIVATE
    /////////////////////////////////////////////
    if (aflag) {
        if (kflag) {
            if (pflag) {
                if (usbk.info.login == 0) {
                    printf("%s [%s] cihazı zaten aktif.\n", usbk.dev, usbk.info.devname.c);
                    exit(0);
                } else {
                    memset(&act, 0, sizeof(t_UI_ACTIVATE));
                    strncpy(act.parola.u8, opt_parola.u8, sizeof(act.parola));
                    memcpy(&act.SelectedKeyNo, &opt_key, sizeof(act.SelectedKeyNo));
                    send_scsi_command(&usbk, (unsigned char*) &act, ACTIVATE_KEY, sizeof(t_UI_ACTIVATE), WRITE_SCSI);
                    usbk_check_last_opr(&usbk);
                    if (iflag) {
                        usbk_get_dev_info(&usbk);
                        usbk_show_show_dev_info(&usbk);
                    }
                    printf("Done.\n");
                    exit(0);
                }
            }
            else{
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // DEACTIVATE
    /////////////////////////////////////////////
    if (dflag) {
        if (usbk.info.login == 1) {
            printf("%s [%s] cihazı zaten deaktif.\n", usbk.dev, usbk.info.devname.c);
            exit(0);
        } else {
            send_scsi_command(&usbk, (unsigned char*) NULL, DEACTIVATE_KEY, 0, WRITE_SCSI);
            usbk_check_last_opr(&usbk);
            if (iflag) {
                usbk_get_dev_info(&usbk);
                usbk_show_show_dev_info(&usbk);
            }
            printf("Done.\n");
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // CHANGE PASSWORD
    /////////////////////////////////////////////
    if (cflag) {
        if (pflag) {
            if (usbk.info.login == 0) {
                printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devname.c);
                exit(0);
            } else {
                memset(&change_pas, 0, sizeof(t_UI_CHANGE_PASS));
                strncpy(change_pas.old_password.u8, opt_parola.u8, sizeof(change_pas.old_password));
                strncpy(change_pas.new_password.u8, opt_new_password.u8, sizeof(change_pas.new_password));
                send_scsi_command(&usbk, (unsigned char*) &change_pas, CHANGE_PASS, sizeof(t_UI_CHANGE_PASS), WRITE_SCSI);
                usbk_check_last_opr(&usbk);
                if (iflag) {
                    usbk_get_dev_info(&usbk);
                    usbk_show_show_dev_info(&usbk);
                }
                printf("Done.\n");
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // SET DEVICE NAME
    /////////////////////////////////////////////
    if (nflag) {
        if (pflag) {
            if (usbk.info.login == 0) {
                printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n",
                        usbk.dev, usbk.info.devname.c);
                exit(0);
            } else {
                memset(&set_dev_name, 0, sizeof(t_UI_SET_DEV_NAME));
                strncpy(set_dev_name.parola.u8, opt_parola.u8, sizeof(set_dev_name.parola));
                strncpy(set_dev_name.devname.c, opt_dev_name.c, sizeof(set_dev_name.devname));
                send_scsi_command(&usbk, (unsigned char*) &set_dev_name, SET_DEV_NAME, sizeof(t_UI_SET_DEV_NAME), WRITE_SCSI);
                usbk_check_last_opr(&usbk);
                if (iflag) {
                    usbk_get_dev_info(&usbk);
                    usbk_show_show_dev_info(&usbk);
                }
                printf("Done.\n");
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // SET KEY NAME ONLY
    /////////////////////////////////////////////
    if (mflag && !xflag) {
        if (kflag) {
            if (pflag) {
                if (usbk.info.login == 0) {
                    printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n",
                            usbk.dev, usbk.info.devname.c);
                    exit(0);
                } else {
                    memset(&set_key, 0, sizeof(t_UI_SET_AES_KEY));
                    strncpy(set_key.parola.u8, opt_parola.u8, sizeof(set_key.parola));
                    memcpy(&set_key.aes_key_no, &opt_key, sizeof(set_key.aes_key_no));
                    strncpy(set_key.aes_name.c, opt_aes_name.c, sizeof(set_key.aes_name));
                    memset(&set_key.nameonly, 1, sizeof(set_key.nameonly));
                    send_scsi_command(&usbk, (unsigned char*) &set_key, SET_KEY, sizeof(t_UI_SET_AES_KEY), WRITE_SCSI);
                    usbk_check_last_opr(&usbk);
                    if (iflag) {
                        usbk_get_dev_info(&usbk);
                        usbk_show_show_dev_info(&usbk);
                    }
                    printf("Done.\n");
                    exit(0);
                }
            }
            else{
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // SET KEY
    /////////////////////////////////////////////
    if (xflag) {
        if (pflag) {
            if (kflag) {
                if (usbk.info.login == 0) {
                    printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n",
                            usbk.dev, usbk.info.devname.c);
                    exit(0);
                } else {
                    memset(&set_key, 0, sizeof(t_UI_SET_AES_KEY));
                    strncpy(set_key.parola.u8, opt_parola.u8, sizeof(set_key.parola));
                    memcpy(&set_key.aes_key_no, &opt_key, sizeof(set_key.aes_key_no));
                    strncpy(set_key.aes_key.key, opt_aes_key.key, sizeof(set_key.aes_key));
                    memset(&set_key.nameonly, 0, sizeof(set_key.nameonly));
                    if (mflag) {
                        strncpy(set_key.aes_name.c, opt_aes_name.c, sizeof(set_key.aes_name));
                    }

                    //printf("key:%s\n", opt_string_key);
                    string str;
                    str = opt_string_key;
                    if (opt_key_format == 'd') {
                        if(check_key_decimal(str, set_key.aes_key.key) == -1){
                            printf("Hata: key format yanlis.\n");
                            exit(0);
                        }
                    }else if(opt_key_format == 't'){
                        if(check_key_text(str, set_key.aes_key.key) == -1){
                            printf("Hata: key format yanlis.\n");
                            exit(0);
                        }
                    }else{
                        cout << "Beklenmeyen hata!" << endl;
                    }

                    send_scsi_command(&usbk, (unsigned char*) &set_key, SET_KEY, sizeof(t_UI_SET_AES_KEY), WRITE_SCSI);
                    usbk_check_last_opr(&usbk);
                    if (iflag) {
                        usbk_get_dev_info(&usbk);
                        usbk_show_show_dev_info(&usbk);
                    }
                    printf("Done.\n");
                    exit(0);
                }
            }
            else{
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }
    /////////////////////////////////////////////
    // ENABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (tflag) {
        if (pflag) {
            if (kflag) {
                if (usbk.info.login == 0) {
                    printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n",
                            usbk.dev, usbk.info.devname.c);
                    exit(0);
                } else {
                    memset(&set_auto, 0, sizeof(t_UI_SET_AUTO_ACTIVATE));
                    strncpy(set_auto.parola.u8, opt_parola.u8, sizeof(set_auto.parola));
                    memcpy(&set_auto.SelectedKeyNo, &opt_key, sizeof(set_auto.SelectedKeyNo));
                    memcpy(&set_auto.AutoActivate, &opt_auto_activate, sizeof(set_auto.AutoActivate));
                    send_scsi_command(&usbk, (unsigned char*) &set_auto, SET_AUTO_ACTIVE, sizeof(t_UI_SET_AUTO_ACTIVATE), WRITE_SCSI);
                    usbk_check_last_opr(&usbk);
                    if (iflag) {
                        usbk_get_dev_info(&usbk);
                        usbk_show_show_dev_info(&usbk);
                    }
                    printf("Done.\n");
                    exit(0);
                }
            }
            else{
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // DISABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (Tflag) {
        if (pflag) {
            if (usbk.info.login == 0) {
                printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devname.c);
                exit(0);
            } else {
                memset(&set_auto, 0, sizeof(t_UI_SET_AUTO_ACTIVATE));
                strncpy(set_auto.parola.u8, opt_parola.u8, sizeof(set_auto.parola));
                memcpy(&set_auto.SelectedKeyNo, &opt_key, sizeof(set_auto.SelectedKeyNo));
                memcpy(&set_auto.AutoActivate, &opt_auto_activate,
                        sizeof(set_auto.AutoActivate));
                send_scsi_command(&usbk, (unsigned char*) &set_auto, SET_AUTO_ACTIVE, sizeof(t_UI_SET_AUTO_ACTIVATE), WRITE_SCSI);
                usbk_check_last_opr(&usbk);
                if (iflag) {
                    usbk_get_dev_info(&usbk);
                    usbk_show_show_dev_info(&usbk);
                }
                printf("Done.\n");
                exit(0);
            }
        }
        else{
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    if (iflag) {
        usbk_get_dev_info(&usbk);
        usbk_show_show_dev_info(&usbk);
        printf("Done.\n");
    }

    /* Instead of reporting ‘--verbose’
     and ‘--brief’ as they are encountered,
     we report the final status resulting from them. */
    if (verbose_flag)
        puts("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
    /*
     if (optind < argc)
     {
     printf ("non-option ARGV-elements: ");
     while (optind < argc)
     printf ("%s ", argv[optind++]);
     putchar ('\n');
     }
     */

    exit(0);
}

void print_help(int exval) {
    printf("Usage: usbk [OPTION...]\n\n");

    printf("Examples:\n");
    printf("  usbk -s                       # Show device list\n");
    printf("  usbk /dev/sdc -a -k 1 -p foo  # activate device with key 1\n");
    printf("  usbk /dev/sdc -d              # deactivate device\n\n");

    printf(" Main operation mode:\n\n");
    printf("  -a, --activate                activate device\n");
    printf("  -d, --deactivate              deactivate device\n");
    printf("  -c, --newpasswd=NEWPASS       change the password to NEWPASS\n");
    printf("  -n, --devname=DEVNAME         change the device name to DEVNAME\n");
    printf("  -m, --keyname=KEYNAME         change then key nameto NAME\n");
    printf("  -x, --change-key=NEWKEY       change the key to NEWKEY\n");
    printf("  -t, --enable-auto             enable auto activate\n");
    printf("  -T, --disable-auto            disable auto activate\n");
    printf("  -l, --gen-key                 generate and set random key\n");
    printf("  -s, --show-devices            show device list\n\n");

    printf(" Setting options:\n\n");
    printf("  -k, --key-no=KEYNO            use KEYNO as key number\n");
    printf("  -p, --passwd=PASSWD           checks password with PASSWD\n");
    printf("  -f, --key-format=FORMAT       FORMAT=t for text or FORMAT=d for\n");
    printf("                                decimal input. default is decimal\n\n");

    printf(" Other options:\n\n");
    printf("  -i, --show-info               show device info\n");
    printf("  -v, --ver                     print program version\n");
    printf("  -?, --help                    give this help list\n\n");

    printf("defaults for options:\n");
    printf("--key-format=d\n");
    printf("--key-no=1\n\n");

    exit(exval);
}


void print_help_eski(int exval) {
    printf("%s version %s\n\n", PACKAGE, VERSION);

    printf(" Main operation mode:\n\n");
    printf("  -a, --activate                Activate device\n");
    printf("  -d, --deactivate              Deactivate device\n");
    printf("  -c, --newpasswd=[PASS]        Change password\n");
    printf("  -n, --devname=NAME            Change device name\n");
    printf("  -m, --keyname=NAME            Change key name\n");
    printf("  -x, --key=KEY                 Change key\n");
    printf("  -t, --enable-auto             Enable auto activate\n");
    printf("  -T, --disable-auto            Disable auto activate\n");
    printf("  -?, --gen-key-decimal         Generate and set random decimal key\n");
    printf("  -?, --gen-key-text            Generate and set random text key\n");
    printf("  -f, --show-devices            Show device list\n\n");


    printf("USAGE:\n");
    printf("  Activate device               %s <device> -ak <key no> -p <password> \n", PACKAGE);
    printf("  Deactivate device             %s <device> -d \n", PACKAGE);
    printf("  Change password               %s <device> -c <new password> -p <password> \n", PACKAGE);
    printf("  Change device name            %s <device> -n <name> -p <password> \n", PACKAGE);
    printf("  Change key name               %s <device> -m <key name> -k <key no> -p <password> \n", PACKAGE);
    printf("  Change key                    %s <device> -x <key> -k <key no> -p <password> \n", PACKAGE);
    printf("  Enable auto activate          %s <device> -tk <key no> -p <password> \n", PACKAGE);
    printf("  Disable auto activate         %s <device> -Tp <password> \n", PACKAGE);
    printf("  Generate and set random key   %s <device> -k <key no> -g <format> -p <password> \n", PACKAGE);
    printf("  Show device info              %s <device> -i \n", PACKAGE);
    printf("  Show device list              %s -f\n\n", PACKAGE);

    printf("OPTIONS:\n");
    printf("  -a                            Activate device with <key no>\n");
    printf("  -d                            Deactivate device.\n");
    printf("  -p <password>                 Password entry\n");
    printf("  -n <device name>              Set <device name>\n");
    printf("  -c <new password>             Set <new password>\n");
    printf("  -k <key no>                   Set <key no>\n");
    printf("  -m <key name>                 Set <key name>\n");
    printf("  -x <key>                      Set <key>\n");
    printf("  -g <format>                   Generate random key. <format> list:[text], [decimal] \n");
    printf("  -t                            Enable auto activate\n");
    printf("  -T                            Disable auto activate\n");
    printf("  -f                            Show usbk encryption device list\n");
    printf("  -i                            Print device info\n");
    printf("  -v                            Print version\n\n");
    exit(exval);
}

void scan_usb() {
    struct usb_bus *busses;
    struct usb_bus *bus;
    struct usb_device *ec3dev;
    struct usb_dev_handle *ec3;
    char serial_num[255], manufacturer[255], product[255];

    printf("USBK encryption device list\n");

    usb_init();
    usb_find_busses();
    usb_find_devices();
    busses = usb_get_busses();

    ec3dev = 0;
    for (bus = busses; bus; bus = bus->next) {
        struct usb_device *dev;
        for (dev = bus->devices; dev; dev = dev->next) {
            if ((dev->descriptor.idVendor == USBK_VENDOR_ID)
                    && (dev->descriptor.idProduct == USBK_PRODUCT_ID)) {
                ec3 = usb_open(dev);
                usb_get_string_simple(ec3, dev->descriptor.iSerialNumber,
                        serial_num, sizeof(serial_num));

                usb_get_string_simple(ec3, dev->descriptor.iManufacturer,
                        manufacturer, sizeof(manufacturer));

                usb_get_string_simple(ec3, dev->descriptor.iProduct, product,
                        sizeof(product));

                usb_release_interface(ec3, 0);
                usb_close(ec3);

                printf("USB:%s\t%s\t%s\n", serial_num, product, manufacturer);
            }
        }
    }
}

void usbk_show_show_dev_info(t_usbk *usbk) {
    //t_UI_DEVINFO dev_info;
    int i;
    char status[13];
    char backdisk[8];
    char autoactive[9];
    char model[32];

    sprintf(model, "%s %s", usbk->model, usbk->rev);

    if (usbk->info.login == 1) {
        sprintf(status, "Deactive");
        sprintf(backdisk, "-");
    } else {
        sprintf(status, "active [%d]", usbk->info.currentkeynum);
        if (usbk->info.backdisk == 1) {
            sprintf(backdisk, "none");
        } else {
            sprintf(backdisk, "exist");
        }
    }

    if (usbk->info.autologin_key_num == 0) {
        sprintf(autoactive, "disable");
    } else {
        sprintf(autoactive, "key %d", usbk->info.autologin_key_num);
    }

    //        123456789012 1234567890123456 1234 12345678901 123456 12345678 1234567890 12345 12345678 123456789 123456789012
    printf("device       Model            Rev  Name        Serial Firmware Status     Retry BackDisk Auto Act. KeyNames    \n");
    //                                                 deactive                  disable
    //                                                 active [1                 key 1
    printf(
            "%-12.12s %-16.16s %-4.4s %-11.11s %-6.6s %-8.8s %-10.10s %-5d %-8.8s %-9.9s [1] %-12s\n",
            usbk->dev, usbk->model, usbk->rev, usbk->info.devname.c,
            usbk->info.serial, usbk->info.firmver.u8, status,
            usbk->info.retry_num, backdisk, autoactive, usbk->info.keyname[0].c);
    for (i = 1; i < usbk->info.multikeycap; i++)
        printf("%99.s[%d] %-12s\n", " ", i + 1, usbk->info.keyname[i].c);

}

int usbk_get_dev_info(t_usbk *usbk) {
    send_scsi_command(usbk, (unsigned char*) &usbk->info, GET_DEV_INFO, sizeof(t_UI_DEVINFO), READ_SCSI);
    //memcpy((char*) &usbk->info, buffer, sizeof(t_UI_DEVINFO));
    return 0;
}

int usbk_check_last_opr(t_usbk *usbk){

    send_scsi_command(usbk, (unsigned char*) &usbk->status, GET_STATUS, sizeof(t_UI_STATUSALL), READ_SCSI);
    if(usbk->status.lastopt == 2){
        printf("Hata: Parola yanlis. RetryNum:%d\n", usbk->status.retry_num);
        exit(1);
    }
    else if(usbk->status.lastopt == 3){
        printf("\n*******************************************\n");
        printf("Uyari: USBK cihazinizdaki bilgiler silindi.\n");
        printf("Yeni parolaniz 123456 dir.\n");
        printf("Lutfen cihazinizi tekrar konfigure ediniz.\n");
        printf("*******************************************\n\n");
        return 1;
    }
    else if(usbk->status.lastopt != 0){
        printf("Hata: Islem basarisiz. MSG_CODE:0x%02X MSG:0x%02X RetryNum:%d\n", usbk->status.msg_code, usbk->status.msg, usbk->status.retry_num);
        exit(1);
    }
    return 0;
}

// 1) string icinde bosluk karakteri var mi?
// 2) string icerisindeki '.' karakterinin sayisi 15 mi?
// 3) '.' karakterlerini ' ' karakterine donustur
// 4) string icerisinde bosluklar haric decimal olmayan karakter var mi?
// 5) string to integer islemini yap
// 6) 16 adet sayı cikartabildin mi?
// 7) sayılar 0 ile 255 arasinda mi?
int check_key_decimal(string str, char *key){
    int ikey[16];
    size_t found;
    string::iterator it;
    int i;

    // string icinde bosluk karakteri var mi?
    if(str.find(" ") != string::npos) return -1;

    // string icerisindeki '.' karakterinin sayisi 15 mi?
    // '.' karakterlerini ' ' karakterine donustur
    i = 0;
    found=str.find_first_of(".");
    while (found!=string::npos)
    {
        i++;
        str[found]=' ';
        found=str.find_first_of(".",found+1);
    }
    if( i != 15) return -1;
    //cout << str << endl;

    // string icerisinde bosluklar haric decimal olmayan karakter var mi?
    i = 0;
    for ( it=str.begin() ; it < str.end(); it++ ){
        if(*it != ' '){
            if(isdigit(*it) == false){
                i++;
            }
        }
    }
    if( i != 0){
        return -1;
    }

    // string to integer islemini yap
    // sayılar 0 ile 255 arasinda mi?
    istringstream iss(str,istringstream::in);
    i = 0;
    for (int n=0; n<16; n++)
    {
        iss >> ikey[n];

        if( iss.fail() || iss.bad() )
            return -1;

        if( iss.eof() && n != 15 )
            return -1;

        if( ikey[n] > 255 || ikey[n] < 0 ){
            i++;
        }
        key[n] = ikey[n];
        //cout << n+1 << "." << ikey[n] << endl;
    }
    if( i != 0){
        return -1;
    }

    return 0;
}


int check_key_text(string str, char *key){
    if (str.size() > 16){
        return -1;
    }
    strncpy(key, str.c_str(), 16);
    return 0;
}

