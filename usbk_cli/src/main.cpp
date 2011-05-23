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

//TODO: BackDisk mount edilmiş ise deactive işlemi sırasında kullanıcıya uyarı ver ve emin misin diye sor. FORCE EKLENEBİLİR

//PRIVATE HEADER
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <usb.h>
#include <sys/ioctl.h>
#include "libusbk.h"

using namespace std;

//PRIVATE DEFINES
//-MESSAGES OF LINUX CLI
#define NO_DEVICE_FOUND                     "No Device Found\n"
#define MSG_FABRIC_DEFAULT                  "Fabric default. Please first set your password."
#define MSG_MUST_REMOVE                     "Must remove. Please remove and re-plug the USBK.\n"
#define NOT_CREATE_RANDOM_KEY               "Random Key is unable to created.\n"
#define NOT_MALLOC                          "invalid space of memory\n"
#define ARG_PARSE_ERROR                     "Parse error\n"
#define MISSING_PARAMETER                   "Missing parameter(s)\n"
#define MAIN_OPERTAION_MORE                 "You may not specify more than one `-adcnmxXtTls' option\n"
#define TRY_HELP                            "Try `usbk --help' for more information.\n"
#define MUST_SUPERUSER                      "requested operation requires superuser privilege\n"
#define MISSING_PARAM_DEV                   "Dev paramter missing\n"
#define NO_DEVICE                           "No Device\n"
#define COMMAND_DONE                        "Done.\n"
#define DEVICE_IN_ACTIVATE                  "%s [%s] in activate.\n"
#define COMMAND_ERROR_AS_ACTIVATE           "This command is not done becuase %s [%s] is in activate.\n"
#define KEY_LEN_INCORRECT                   "The length of key is incorrect.\n"
#define KEY_DECIMAL_FORMAT_INCORRECT        "The format of decimal key is incorrect.\n"
#define KEY_TEXT_FORMAT_INCORRECT           "The format of text key is incorrect.\n"
#define UNKOWN_ERROR                        "Unknown Error!!!\n"
#define RANDOMKEY_SHORT                     "Random key is short. So, remain part is filled with '0'.\n"
#define SET_RANDOM_KEY                      "Set Random Key: "


//PRIVATE FUNCTIONS
static int _parse_options(int *argc, char** argv[]);

static void linuxcli_show_devices(void);
static void linuxcli_show_dev_info(const char* dev);
static int StatusChecker(const char *usbk_dev, int status);
static int linuxcli_GetRetryNumber(const char *usbk_dev);

static int check_key_decimal(string str, unsigned char *key, int key_size_byte);
static int check_key_text(string str, unsigned char *key, int key_size_byte);

static void print_help(int exval);
static void print_version(void);
static bool check_superuser(void);


//PRIVATE VARIABLES
//-FLAGS
int verbose_flag;
int uflag = 0;
int aflag = 0;
int dflag = 0;
int cflag = 0;
int nflag = 0;
int mflag = 0;
int xflag = 0;
int Xflag = 0;
int tflag = 0;
int Tflag = 0;
int lflag = 0;
int sflag = 0;
int kflag = 0;
int pflag = 0;
int fflag = 1;
int Fflag = 1;
int iflag = 0;
int vflag = 0;
int question_flag = 0;

int main_operation = 0;
//-OTHERS
int opt_key;
int opt_key_size_byte;
char *opt_parola;
char *opt_dev_label;
char *opt_aes_name;
char *opt_new_password;
char *opt_key_size_str;
char opt_key_format;
char usbk_dev[1024];
char opt_string_key[128];
unsigned char opt_aes_key[1024];
unsigned char set_key[1024];
USBK_INFO *usbk_info;
bool superuser_privileges = false;

static struct option long_options[] =
        {
        /* These options set a flag. */
        { "dev", required_argument, 0, 'u' },
        { "verbose", no_argument, &verbose_flag, 1 },
        { "brief", no_argument, &verbose_flag, 0 },
          /* These options don't set a flag. We distinguish them by their indices. */
        { "activate", no_argument, 0, 'a' },
        { "deactivate", no_argument, 0, 'd' },
        { "newpass", required_argument, 0, 'c' },
        { "label", required_argument, 0, 'n' },
        { "keyname", required_argument, 0, 'm' },
        { "change-key", required_argument, 0, 'x' },
        { "change-key-with-random", no_argument, 0, 'X' },
        { "enable-auto", no_argument, 0, 't' },
        { "disable-auto", no_argument, 0, 'T' },
        { "gen-key", no_argument, 0, 'l' },
        { "show-devices", no_argument, 0, 's' },
        { "key-no", required_argument, 0, 'k' },
        { "passwd", required_argument, 0, 'p' },
        { "key-format", required_argument, 0, 'f' },
        { "key-size", required_argument, 0, 'F' },
        { "show-info", no_argument, 0, 'i' },
        { "ver", no_argument, 0, 'v' },
        { "help", no_argument, 0, '?' },
        { NULL, 0, NULL, 0 }  // Last element must be NULL
};


//ALL FUNCTIONS

//!-\brief Main Function
//!     \param argc command line argument count
//!     \param argv argument list
//!     \return 0 normal exit
//!     \return 1 abnormal exit
int main(int argc, char *argv[]) {

    int c;
    int status;

    opt_key = 1; // default key no 1
    opt_key_format = 'd'; // default key format decimal
    opt_key_size_str = strdup("256");

    if (check_superuser() == true){
        superuser_privileges = true;
    }
    else{
        superuser_privileges = false;
    }


    if (!_parse_options(&argc, &argv)) {
        fprintf(stderr, ARG_PARSE_ERROR);
        exit(0);
    }

    if (question_flag) {
        print_help(0);
        exit(0);
    }

    if (vflag) {
        print_version();
        exit(0);
    }

    if (main_operation > 1) {
        fprintf(stderr, MAIN_OPERTAION_MORE);
        fprintf(stderr, TRY_HELP);
        exit(1);
    }

    if (!iflag && main_operation == 0) {
        exit(0);
    }

    if (superuser_privileges == false){
        fprintf(stderr,MUST_SUPERUSER);
        exit (0);
    }

    if (sflag) {
        linuxcli_show_devices();
        exit(0);
    }

    if (uflag == 0) {
        fprintf(stderr, MISSING_PARAM_DEV);
        fprintf(stderr, TRY_HELP);
        exit(0);
    }

    USBK_INFO *usbk_infos;
    int rtn = LibUSBK__GetDeviceInfo(usbk_dev, &usbk_infos);
    if (rtn<0){

        switch(rtn)
        {
        case LIBUSBK_RTN_UNSUPPORTED_USBK:
            break;
        default:
            fprintf(stderr,NO_DEVICE);
            exit(0);
            break;
        }

    }

    if (iflag) {
        linuxcli_show_dev_info(usbk_dev);
        fprintf(stdout, COMMAND_DONE);
    }


    if (usbk_infos->supported == true)
    {
        /////////////////////////////////////////////
        // ACTIVATE
        /////////////////////////////////////////////
        if (aflag) {
            if (kflag) {
                if (pflag) {
                    switch (usbk_infos->dev_state) {
                    case LIBSUBK_DEVSTATE_ACTIVATE:
                    case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                        fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                        break;
                    case LIBSUBK_DEVSTATE_DEACTIVATE:
                        status = LibUSBK__ActivateKey(usbk_infos->dev_path, opt_parola, (int)opt_key);

                        if (StatusChecker(usbk_dev ,status) < 0){
                            exit(1);
                        }
                        if (iflag) {
                            linuxcli_show_dev_info(usbk_dev);
                        }
                        fprintf(stdout, COMMAND_DONE);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                        fprintf(stderr, MSG_FABRIC_DEFAULT);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_MUST_REMOVE:
                        fprintf(stderr, MSG_MUST_REMOVE);
                        exit(0);
                        break;
                    default:
                        fprintf(stderr, UNKOWN_ERROR);
                        exit(0);
                        break;
                    }
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    fprintf(stderr, TRY_HELP);
                    exit(0);
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                fprintf(stderr, TRY_HELP);
                exit(0);
            }
        }

        /////////////////////////////////////////////
        // DEACTIVATE
        /////////////////////////////////////////////
        if (dflag) {
            switch (usbk_infos->dev_state) {
            case LIBSUBK_DEVSTATE_ACTIVATE:
            case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                status = LibUSBK__DeActivateKey(usbk_infos->dev_path);
                if (StatusChecker(usbk_dev ,status) < 0){
                    exit(1);
                }
                if (iflag) {
                    linuxcli_show_dev_info(usbk_dev);
                }
                fprintf(stdout, COMMAND_DONE);
                exit(0);
                break;
            case LIBSUBK_DEVSTATE_DEACTIVATE:
                fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                exit(0);
                break;
            case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                fprintf(stderr, MSG_FABRIC_DEFAULT);
                exit(0);
                break;
            case LIBSUBK_DEVSTATE_MUST_REMOVE:
                fprintf(stderr, MSG_MUST_REMOVE);
                exit(0);
                break;
            default:
                fprintf(stderr, UNKOWN_ERROR);
                exit(0);
                break;
            }
        }

        /////////////////////////////////////////////
        // CHANGE PASSWORD
        /////////////////////////////////////////////
        if (cflag) {
            switch (usbk_infos->dev_state) {
            case LIBSUBK_DEVSTATE_ACTIVATE:
            case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                exit(0);
                break;
            case LIBSUBK_DEVSTATE_DEACTIVATE:
                if (pflag) {
                    status = LibUSBK__ChangePassword(usbk_infos->dev_path, opt_parola, opt_new_password);
                    if (StatusChecker(usbk_dev ,status) < 0){
                        exit(1);
                    }

                    if (iflag) {
                        linuxcli_show_dev_info(usbk_dev);
                    }
                    fprintf(stderr, COMMAND_DONE);
                    exit(0);
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    exit(0);
                }
                break;
            case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:

                status = LibUSBK__ChangePassword(usbk_infos->dev_path, opt_new_password, opt_new_password);
                if (StatusChecker(usbk_dev ,status) < 0){
                    exit(1);
                }

                if (iflag) {
                    linuxcli_show_dev_info(usbk_dev);
                }
                fprintf(stdout, COMMAND_DONE);
                exit(0);
                break;
            case LIBSUBK_DEVSTATE_MUST_REMOVE:
                fprintf(stderr, MSG_MUST_REMOVE);
                exit(0);
                break;
            default:
                fprintf(stderr, UNKOWN_ERROR);
                exit(0);
                break;
            }
        }

        /////////////////////////////////////////////
        // SET DEVICE NAME
        /////////////////////////////////////////////
        if (nflag) {
            if (pflag) {
                switch (usbk_infos->dev_state) {
                case LIBSUBK_DEVSTATE_ACTIVATE:
                case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                    fprintf (stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_DEACTIVATE:
                    status = LibUSBK__SetDeviceName(usbk_infos->dev_path, opt_parola, opt_dev_label);
                    if (StatusChecker(usbk_dev ,status) < 0){
                        exit(1);
                    }

                    if (iflag) {
                        linuxcli_show_dev_info(usbk_dev);
                    }
                    fprintf(stdout, COMMAND_DONE);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                    fprintf(stderr, MSG_FABRIC_DEFAULT);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_MUST_REMOVE:
                    fprintf( stderr, MSG_MUST_REMOVE);
                    exit(0);
                    break;
                default:
                    fprintf(stderr, UNKOWN_ERROR);
                    exit(0);
                    break;
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                exit(0);
            }
        }

        /////////////////////////////////////////////
        // SET KEY NAME ONLY
        /////////////////////////////////////////////
        if (mflag && !xflag && !Xflag) {
            if (kflag) {
                if (pflag) {
                    switch (usbk_infos->dev_state) {
                    case LIBSUBK_DEVSTATE_ACTIVATE:
                    case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                        fprintf (stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_DEACTIVATE:
                        status = LibUSBK__SetKey (usbk_infos->dev_path, opt_parola, opt_key, true, opt_aes_name, NULL, NULL);
                        if (StatusChecker(usbk_dev ,status) < 0){
                            exit(1);
                        }

                        if (iflag) {
                            linuxcli_show_dev_info(usbk_dev);
                        }
                        fprintf(stdout, COMMAND_DONE);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                        fprintf(stderr, MSG_FABRIC_DEFAULT);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_MUST_REMOVE:
                        fprintf(stderr, MSG_MUST_REMOVE);
                        exit(0);
                        break;
                    default:
                        fprintf(stderr, UNKOWN_ERROR);
                        exit(0);
                        break;
                    }
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    exit(0);
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                exit(0);
            }
        }

        /////////////////////////////////////////////
        // SET KEY
        /////////////////////////////////////////////
        if (xflag | Xflag) {
            if (pflag) {
                if (kflag) {
                    if (Fflag)
                    {
                        switch (usbk_infos->dev_state) {
                        case LIBSUBK_DEVSTATE_ACTIVATE:
                        case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                            fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                            exit(0);
                            break;
                        case LIBSUBK_DEVSTATE_DEACTIVATE:

                            if (!mflag) {
                                opt_aes_name = usbk_infos->key_names[opt_key];
                            }

                            if (!strcmp(opt_key_size_str, "128")) {
                                opt_key_size_byte = 16;
                            } else if (!strcmp(opt_key_size_str, "192")) {
                                opt_key_size_byte = 24;
                            } else if (!strcmp(opt_key_size_str, "256")) {
                                opt_key_size_byte = 32;
                            } else {
                                fprintf (stderr, KEY_LEN_INCORRECT);
                                exit(0);
                            }

                            if (xflag) {
                                if (opt_key_format == 'd') {
                                    if (check_key_decimal((string) opt_string_key, opt_aes_key, opt_key_size_byte)
                                            == -1) {

                                        fprintf(stderr, KEY_DECIMAL_FORMAT_INCORRECT);
                                        exit(0);
                                    }
                                } else if (opt_key_format == 't') {
                                    if (check_key_text((string) opt_string_key, opt_aes_key, opt_key_size_byte)
                                            == -1) {
                                        fprintf(stderr, KEY_TEXT_FORMAT_INCORRECT);
                                        exit(0);
                                    }
                                } else {
                                    fprintf(stderr, UNKOWN_ERROR);
                                }

                            }
                            else if (Xflag) {
                                unsigned char *dummy_set_key;
                                status = LibUSBK__GetRandomKey (usbk_infos->dev_path, &dummy_set_key, opt_key_size_byte);

                                switch (StatusChecker(usbk_dev ,status))
                                {
                                case LIBUSBK_RTN_OPRS_PASS:
                                    memcpy (set_key, dummy_set_key, opt_key_size_byte);
                                    break;
                                case LIBUSBK_RTN_SHORT_GENERATEDKEY:
                                    fprintf(stderr, RANDOMKEY_SHORT);
                                    memcpy (set_key, dummy_set_key, opt_key_size_byte);
                                    break;
                                default:
                                    fprintf (stderr, NOT_CREATE_RANDOM_KEY);
                                    LibUSBK__GetRandomKey_Release(&dummy_set_key);
                                    exit(1);
                                    break;
                                }

                                LibUSBK__GetRandomKey_Release(&dummy_set_key);
                            }

                            status = LibUSBK__SetKey (usbk_infos->dev_path, opt_parola, opt_key, false, opt_aes_name, opt_key_size_str, opt_aes_key);
                            if (StatusChecker(usbk_dev ,status) < 0 ){
                                exit(1);
                            }

                            if (Xflag) {
                                int i;
                                fprintf (stdout, SET_RANDOM_KEY);
                                for (i=0; i<opt_key_size_byte; i++)
                                {
                                    fprintf (stdout, "%d",set_key[i]);

                                    if (i != (opt_key_size_byte-1)) fprintf (stdout, ".");
                                }
                                fprintf (stdout, "\n");
                            }

                            if (iflag) {
                                linuxcli_show_dev_info(usbk_dev);
                            }
                            fprintf (stdout, COMMAND_DONE);
                            exit(0);
                            break;
                        case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                            fprintf(stderr, MSG_FABRIC_DEFAULT);
                            exit(0);
                            break;
                        case LIBSUBK_DEVSTATE_MUST_REMOVE:
                            fprintf(stderr, MSG_MUST_REMOVE);
                            exit(0);
                            break;
                        default:
                            fprintf(stderr, UNKOWN_ERROR);
                            exit(0);
                            break;
                        }
                    } else {
                        fprintf(stderr, MISSING_PARAMETER);
                        exit(0);
                    }
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    exit(0);
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                exit(0);
            }
        }

        /////////////////////////////////////////////
        // ENABLE AUTO ACTIVATE
        /////////////////////////////////////////////
        if (tflag) {
            if (pflag) {
                if (kflag) {
                    switch (usbk_infos->dev_state) {
                    case LIBSUBK_DEVSTATE_ACTIVATE:
                    case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                        fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_DEACTIVATE:
                        status = LibUSBK__SetAutoAct(usbk_infos->dev_path, opt_parola, true, opt_key);
                        if (StatusChecker(usbk_dev ,status) < 0){
                            exit(1);
                        }

                        if (iflag) {
                            linuxcli_show_dev_info(usbk_dev);
                        }
                        fprintf(stdout, COMMAND_DONE);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                        fprintf(stderr, MSG_FABRIC_DEFAULT);
                        exit(0);
                        break;
                    case LIBSUBK_DEVSTATE_MUST_REMOVE:
                        fprintf(stderr, MSG_MUST_REMOVE);
                        exit(0);
                        break;
                    default:
                        fprintf(stderr, UNKOWN_ERROR);
                        exit(0);
                        break;
                    }
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    exit(0);
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                exit(0);
            }
        }

        /////////////////////////////////////////////
        // DISABLE AUTO ACTIVATE
        /////////////////////////////////////////////
        if (Tflag) {
            if (pflag) {
                switch (usbk_infos->dev_state) {
                case LIBSUBK_DEVSTATE_ACTIVATE:
                case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
                    fprintf(stderr, COMMAND_ERROR_AS_ACTIVATE, usbk_dev, usbk_infos->dev_label);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_DEACTIVATE:
                    status = LibUSBK__SetAutoAct(usbk_infos->dev_path, opt_parola, false, 0);
                    if (StatusChecker(usbk_dev ,status) < 0){
                        exit(1);
                    }

                    if (iflag) {
                        linuxcli_show_dev_info(usbk_dev);
                    }
                    fprintf (stdout, COMMAND_DONE);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
                    fprintf(stderr, MSG_FABRIC_DEFAULT);
                    exit(0);
                    break;
                case LIBSUBK_DEVSTATE_MUST_REMOVE:
                    fprintf(stderr, MSG_MUST_REMOVE);
                    exit(0);
                    break;
                default:
                    break;
                }
            } else {
                fprintf(stderr, MISSING_PARAMETER);
                exit(0);
            }
        }
    }


    LibUSBK__GetDeviceInfo_Release(usbk_infos);
    free (opt_parola);

    exit(0);
}

static int _parse_options(int *argc, char** argv[]) {
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int opt;

    if (*argc == 1)
    {
        question_flag = 1;
    }
    else
    {

        while (( opt = getopt_long(*argc, *argv, "u:adc:n:m:x:XtTlsk:p:F:f:iv?", long_options, &option_index)) != -1) {
            switch (opt) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf("option %s", long_options[option_index].name);
                if (optarg) printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'u':
                uflag = 1;
                strncpy (usbk_dev ,optarg, sizeof (usbk_dev));
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
                opt_new_password = strdup(optarg);
                break;

            case 'n':
                nflag = 1;
                main_operation++;
                opt_dev_label = strdup(optarg);
                break;

            case 'm':
                mflag = 1;
                main_operation++;
                opt_aes_name = strdup(optarg);
                break;

            case 'x':
                xflag = 1;
                main_operation++;
                sprintf(opt_string_key, "%s", optarg);
                break;

            case 'X':
                Xflag = 1;
                main_operation++;
                break;

            case 't':
                tflag = 1;
                main_operation++;
                break;

            case 'T':
                Tflag = 1;
                main_operation++;
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
                opt_parola = strdup(optarg);
                break;

            case 'f':
                fflag = 1;
                if (!strcmp(optarg, "d")) {
                    opt_key_format = 'd';
                } else if (!strcmp(optarg, "t")) {
                    opt_key_format = 't';
                } else {
                    fprintf(stderr, MISSING_PARAMETER);
                    exit(1);
                }
                break;

            case 'F':
                Fflag = 1;
                opt_key_size_str = strdup(optarg);
                break;
            case 'i':
                iflag = 1;
                break;

            case 'v':
                vflag = 1;
                break;

            case '?':
                question_flag = 1;
                break;
            default:
                break;
            }
        }

        /* Instead of reporting ‘--verbose’
         and ‘--brief’ as they are encountered,
         we report the final status resulting from them. */
        if (verbose_flag)
            fprintf(stdout, "verbose flag is set");

    }

    return 1;
}

void linuxcli_show_devices(void) {
    int i;
    int counter = 0;
    int rtn;

    USBK_List* p_usbklink = NULL;

    rtn = LibUSBK__list_devices(&p_usbklink);

    if (rtn < 0)
    {
        if (rtn == LIBUSBK_RTN_NO_DEVICE_FOUND)
        {
            fprintf(stderr, NO_DEVICE_FOUND);
            exit(1);
        }
        else
        {
            fprintf(stderr, NOT_MALLOC);
            exit(1);
        }
    }


    USBK_List *dummy_usbklink;
    for (dummy_usbklink = p_usbklink; dummy_usbklink != NULL; dummy_usbklink = dummy_usbklink->next) {

        if (dummy_usbklink->usbk_info.supported == true)
        {
            printf("  %s\n",                            dummy_usbklink->usbk_info.dev_label);
            printf("    Device   : %s\n",               dummy_usbklink->dev);
            printf("    BackDisk : %s\n",               dummy_usbklink->usbk_info.backdisk);
            printf("    Product  : %s\n",               dummy_usbklink->usbk_info.product);
            printf("    Model    : %s\n",               dummy_usbklink->usbk_info.model);
            printf("    Serial   : %s\n",               dummy_usbklink->usbk_info.serial);
            printf("    Firmware : %s\n",               dummy_usbklink->usbk_info.firmware_ver);
            printf("\n");
        }
        else
        {
            printf("  %s(UNSUPPORTED DEVICE)\n",        dummy_usbklink->usbk_info.dev_label);
            printf("    Device   : %s\n",               dummy_usbklink->dev);
            printf("    Product  : %s\n",               dummy_usbklink->usbk_info.product);
            printf("    Model    : %s\n",               dummy_usbklink->usbk_info.model);
            printf("    Serial   : %s\n",               dummy_usbklink->usbk_info.serial);
            printf("    Firmware : %s\n",               dummy_usbklink->usbk_info.firmware_ver);
            printf("\n");
        }

        counter++;
    }

    (counter > 0) ? printf ("%d",counter):printf ("None");
    printf (" USBK Found.\n\n");

    LibUSBK__list_devices_release(&p_usbklink);
}

void linuxcli_show_dev_info(const char* dev) {


    USBK_INFO *usbk_infos;
    if (LibUSBK__GetDeviceInfo(dev, &usbk_infos)<0)
    {
        LibUSBK__GetDeviceInfo_Release(usbk_infos);
        fprintf(stderr,NO_DEVICE);
        return;
    }


    if (usbk_infos->supported == true)
    {
        //UI_DEVINFO_T dev_info;
        int i;
        char status[64];
        char backdisk[8];
        char autoactive[64];
        char model[32];

        sprintf(backdisk, "-");

        switch (usbk_infos->dev_state) {
        case LIBSUBK_DEVSTATE_ACTIVATE:
            sprintf(status, "active [%d]", usbk_infos->current_key);
            sprintf(backdisk, "none");
            break;
        case LIBSUBK_DEVSTATE_ACTIVATE_WITH_BACKDISK:
            sprintf(status, "active [%d]", usbk_infos->current_key);
            sprintf(backdisk, "exist");
            break;
        case LIBSUBK_DEVSTATE_DEACTIVATE:
            sprintf(status, "deactive");
            break;
        case LIBSUBK_DEVSTATE_FABRIC_DEFAULT:
            sprintf(status, "%s", MSG_FABRIC_DEFAULT);
            break;
        case LIBSUBK_DEVSTATE_MUST_REMOVE:
            sprintf(status, "%s", MSG_MUST_REMOVE);
            break;
        default:
            sprintf(status, "%d:unknown", usbk_infos->dev_state);
            break;
        }

        if (usbk_infos->autoact_keyno == 0) {
            sprintf(autoactive, "disable");
        } else {
            sprintf(autoactive, "enable with key #%d", usbk_infos->autoact_keyno);
        }

        printf("\n  usbk information\n");
        printf("    logic name              %s\n", dev);
        printf("    back disk name          %s\n", usbk_infos->backdisk);
        printf("    product                 %s\n", usbk_infos->product);
        printf("    model                   %s\n", usbk_infos->model);
        printf("    serial number           %s\n", usbk_infos->serial);
        printf("    firmware verison        %s\n", usbk_infos->firmware_ver);
        printf("    label                   %s\n", usbk_infos->dev_label);
        printf("    status                  %s\n", status);
        printf("    retry number            %d\n", usbk_infos->retry_num);
        printf("    back disk               %s\n", backdisk);
        printf("    auto activation         %s\n", autoactive);
        printf("    max. key capacity       %d\n", usbk_infos->multikey_cap);
        for (i = 0; i < usbk_infos->multikey_cap; i++) {
            printf("      key %d name            %s\n", i, usbk_infos->key_names[i]);
        }

        /*

         //      123456789012 1234567890123456 1234 12345678901 123456 12345678 1234567890 12345 12345678 123456789 123456789012
         printf("device       Model            Rev  Name        Serial Firmware Status     Retry BackDisk Auto Act. KeyNames    \n");
         //                                                 deactive                  disable
         //                                                 active [1                 key 1
         printf(
         "%-12.12s %-16.16s %-4.4s %-11.11s %-6.6s %-8.8s %-10.10s %-5d %-8.8s %-9.9s [1] %-12s\n",
         abcde->dev, abcde->model, abcde->rev, abcde->info.devlabel.s,
         abcde->info.serial.u8, abcde->info.firmware_ver.s, status,
         abcde->info.retry_num, backdisk, autoactive, abcde->info.keyname[0].s);
         for (i = 1; i < abcde->info.multikeycap; i++)
         printf("%99.s[%d] %-12s\n", " ", i + 1, abcde->info.keyname[i].s);

         */
    }
    else
    {
        printf("\n  usbk information\n");
        printf("    logic name              %s\n", dev);
        printf("    product                 %s\n", usbk_infos->product);
        printf("    model                   %s\n", usbk_infos->model);
        printf("    serial number           %s\n", usbk_infos->serial);
        printf("    firmware verison        %s\n", usbk_infos->firmware_ver);
        printf("    label                   %s\n", usbk_infos->dev_label);
        printf("    UNSUPPORTED DEVICE!\n");
    }





    LibUSBK__GetDeviceInfo_Release(usbk_infos);

}

int StatusChecker(const char *usbk_dev, int status) {
    switch ((LIBUSBK_OPRSTATUS) status) {
    case LIBUSBK_RTN_OPRS_INVALID_PASS:
        fprintf(stderr, "Password is incorrect. Retry Number: %d", linuxcli_GetRetryNumber(usbk_dev));
        return false;
        break;
    case LIBUSBK_RTN_OPRS_FABRIC_RESET:
        printf("\n");
        printf("***********************************************\n");
        printf(" All keys and your password is erased.         \n");
        printf(" Please, re-configure your device.             \n");
        printf("***********************************************\n");
        printf("\n");
        return true;
        break;
    case LIBUSBK_RTN_OPRS_USBK_UNPLUGING:
        fprintf(stderr, MSG_MUST_REMOVE);
        return false;
        break;
    case LIBUSBK_RTN_OPRS_PASS:
    default:
        if (status < 0 ){
            fprintf(stderr, UNKOWN_ERROR "MSG_CODE:0x%02X and RetryNum:%d\n", status, linuxcli_GetRetryNumber(usbk_dev));
        }
        return status;
        break;
    }
    return status;
}

int linuxcli_GetRetryNumber(const char *usbk_dev)
{
    USBK_INFO *usbk_infos;
    if (LibUSBK__GetDeviceInfo(usbk_dev, &usbk_infos)<0) {
        fprintf(stderr,"device yok");
        return -1;
    }
    else{
        int retry_number = usbk_infos->retry_num;
        LibUSBK__GetDeviceInfo_Release(usbk_infos);
        return retry_number;
    }
}


// 1) string icinde bosluk karakteri var mi?
// 2) string icerisindeki '.' karakterinin sayisi 15 mi?
// 3) '.' karakterlerini ' ' karakterine donustur
// 4) string icerisinde bosluklar haric decimal olmayan karakter var mi?
// 5) string to integer islemini yap
// 6) 16 adet sayı cikartabildin mi?
// 7) sayılar 0 ile 255 arasinda mi?
int check_key_decimal(string str, unsigned char *key, int key_size_byte) {
    int ikey[key_size_byte];
    size_t found;
    string::iterator it;
    int i;

    // string icinde bosluk karakteri var mi?
    if (str.find(" ") != string::npos)
        return -1;

    // string icerisindeki '.' karakterinin sayisi 15 mi?
    // '.' karakterlerini ' ' karakterine donustur
    i = 0;
    found = str.find_first_of(".");
    while (found != string::npos) {
        i++;
        str[found] = ' ';
        found = str.find_first_of(".", found + 1);
    }
    if (i != (key_size_byte-1))
        return -1;
    //cout << str << endl;

    // string icerisinde bosluklar haric decimal olmayan karakter var mi?
    i = 0;
    for (it = str.begin(); it < str.end(); it++) {
        if (*it != ' ') {
            if (isdigit(*it) == false) {
                i++;
            }
        }
    }
    if (i != 0) {
        return -1;
    }

    // string to integer islemini yap
    // sayılar 0 ile 255 arasinda mi?
    istringstream iss(str, istringstream::in);
    i = 0;
    for (int n = 0; n < key_size_byte; n++) {
        iss >> ikey[n];

        if (iss.fail() || iss.bad())
            return -1;

        if (iss.eof() && n != (key_size_byte-1))
            return -1;

        if (ikey[n] > 255 || ikey[n] < 0) {
            i++;
        }
        key[n] = ikey[n];
        //cout << n+1 << "." << ikey[n] << endl;
    }
    if (i != 0) {
        return -1;
    }

    return 0;
}

int check_key_text(string str, unsigned char *key, int key_size_byte) {
    if (str.size() > key_size_byte) {
        return -1;
    }
    strncpy((char*) key, str.c_str(), key_size_byte);
    return 0;
}

static void print_help(int exval) {
    print_version();
/*
    printf("Usage: usbk [OPTION...]\n\n");

    printf("Examples:\n");
    printf("  usbk -s                       # Show device list\n");
    printf("  usbk sdc -a -k 1 -p foo       # activate device with key 1\n");
    printf("  usbk sdc -d                   # deactivate device\n\n");

    printf(" Main operation mode:\n\n");
    printf("  -u, --dev                     device\n");
    printf("  -a, --activate                activate device\n");
    printf("  -d, --deactivate              deactivate device\n");
    printf("  -c, --newpasswd=NEWPASS       change the password to NEWPASS\n");
    printf("  -n, --label=LABEL             change the label to LABEL\n");
    printf("  -m, --keyname=KEYNAME         change then key nameto NAME\n");
    printf("  -x, --change-key=NEWKEY       change the key to NEWKEY\n");
    printf("  -X, --change-key-with-random  change the key to random key\n");
    printf("  -t, --enable-auto             enable auto activate\n");
    printf("  -T, --disable-auto            disable auto activate\n");
    printf("  -l, --gen-key                 generate and set random key\n");
    printf("  -s, --show-devices            show device list\n\n");

    printf(" Setting options:\n\n");
    printf("  -k, --key-no=KEYNO            use KEYNO as key number\n");
    printf("  -p, --passwd=PASSWD           checks password with PASSWD\n");
    printf("  -F, --key-size=KEY_SIZE       KEYSIZE is 128, 192 or 256\n");
    printf("  -f, --key-format=FORMAT       FORMAT=t for text or FORMAT=d for\n");
    printf("                                decimal input. default is decimal\n\n");

    printf(" Other options:\n\n");
    printf("  -i, --show-info               show device info\n");
    printf("  -v, --ver                     print program version\n");
    printf("  -?, --help                    give this help list\n\n");

    printf("defaults for options:\n");
    printf("--key-size=256\n");
    printf("--key-format=d\n");
    printf("--key-no=1\n\n");
*/
    printf(""
            "Usage: usbk [OPTION...]\n"
            "\n"
            "Examples:\n"
            "  usbk -s                       # Show device list\n"
            "  usbk sdc -a -k 1 -p foo       # activate device with key 1\n"
            "  usbk sdc -d                   # deactivate device\n"
            "\n"
            " Main operation mode:\n"
            "\n"
            "  -u, --dev                     device\n"
            "  -a, --activate                activate device\n"
            "  -d, --deactivate              deactivate device\n"
            "  -c, --newpasswd=NEWPASS       change the password to NEWPASS\n"
            "  -n, --label=LABEL             change the label to LABEL\n"
            "  -m, --keyname=KEYNAME         change then key nameto NAME\n"
            "  -x, --change-key=NEWKEY       change the key to NEWKEY\n"
            "  -X, --change-key-with-random  change the key to random key\n"
            "  -t, --enable-auto             enable auto activate\n"
            "  -T, --disable-auto            disable auto activate\n"
            "  -l, --gen-key                 generate and set random key\n"
            "  -s, --show-devices            show device list\n"
            "\n"
            " Setting options:\n"
            "\n"
            "  -k, --key-no=KEYNO            use KEYNO as key number\n"
            "  -p, --passwd=PASSWD           checks password with PASSWD\n"
            "  -F, --key-size=KEY_SIZE       KEYSIZE is 128, 192 or 256\n"
            "  -f, --key-format=FORMAT       FORMAT=t for text or FORMAT=d for\n"
            "                                decimal input. default is decimal\n"
            "\n"
            " Other options:\n"
            "\n"
            "  -i, --show-info               show device info\n"
            "  -v, --ver                     print program version\n"
            "  -?, --help                    give this help list\n"
            "\n"
            "defaults for options:\n"
            "--key-size=256\n"
            "--key-format=d\n"
            "--key-no=1\n"
            "\n");
    exit(exval);
}

static void print_version(void) {
    fprintf(stdout, "%s version %s\n", PACKAGE, VERSION);
}



static bool check_superuser(void){
    if(geteuid() == 0){
        return true;
    }else{
        return false;
    }
}
