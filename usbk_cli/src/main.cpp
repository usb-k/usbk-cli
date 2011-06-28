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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sstream>
#include <string.h>
#include "libusbk.h"

using namespace std;


#define MSG_PASS                        "Pass.\n"
#define MSG_GEN_FAIL                    "Fail.\n"
#define MSG_FAILED_PASS                 "Failed Password, Retry Number = %d.\n", myusbk->info.retry_num
#define MSG_FABRIC_RESET                "\n"\
                                        "***********************************************\n"\
                                        " All keys and your password is erased.         \n"\
                                        " Please, re-configure your device.             \n"\
                                        "***********************************************\n"\
                                        "\n"
#define MSG_USBK_UNPLUGING              "Must remove. Please remove and re-plug the USBK.\n"
#define MSG_INVALID_KEYNO               "Invalid Key Number.\n"
#define MSG_INVALID_KEYSIZE             "Invalid Key Size.\n"
#define MSG_INVALID_DEVICELABEL         "Invalid Device Label\n"
#define MSG_INVALID_NEWPASS             "Invalid New Password.\n"

#define MSG_DSE_ACTIVATE                "USBK in active. This operation is not able to done in this state.\n"
#define MSG_DSE_ACTIVATE_WITH_BACKDISK  "The backdisk is plugged to USBK.  This operation is not able to done in this state.\n"
#define MSG_DSE_DEACTIVATE              "USBK in deactive. This operation is not able to done in this state.\n"
#define MSG_DSE_FABRIC_DEFAULT          "USBK in fabric default. Please first set your password.\n"
#define MSG_DSE_MUST_REMOVE             "USBK in must remove. This operation is not able to done in this state.\n"
#define MSG_DSE_UNKNOWN                 "The state of USBK  is unknown. This operation is not able to done in this state.\n"

#define MSG_SCSI_ERROR                  "No Device Found (scsi error).\n"
#define MSG_UNSUPPORTED_USBK            "Unsupported USBK.\n"
#define MSG_INVALID_KEY                 "Invalid Key.\n"
#define MSG_UDEV_ERROR                  "No Device Found (udev error).\n"
#define MSG_MEM_ERROR                   "No Device Found (mem error).\n"
#define MSG_UNKOWN_ERROR                "Unkown Error.\n"


//MESSAGES OF LINUX CLI
#define ARG_PARSE_ERROR                     "Parse error\n"
#define MISSING_PARAMETER                   "Missing parameter(s)\n"
#define MAIN_OPERTAION_MORE                 "You may not specify more than one `-adcnmxXtTls' option\n"
#define TRY_HELP                            "Try `usbk --help' for more information.\n"
#define MUST_SUPERUSER                      "requested operation requires superuser privilege\n"
#define MISSING_PARAM_DEV                   "Dev paramter missing\n"


//PRIVATE FUNCTION DECLARATIONS
static int _parse_options(int *argc, char** argv[]);
KEYSIZE parse_keysize(char *s);
int parse_keysize_inbyte(char *s);

static void linuxcli_show_devices(void);
static void linuxcli_show_dev_info(USBK* myusbk);
static void print_result(USBK* myusbk);

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
int fflag = 0;
int Fflag = 0;
int iflag = 0;
int vflag = 0;
int question_flag = 0;

int main_operation = 0;
//-OTHERS
int opt_key;
int opt_key_size_byte;
char *opt_parola = NULL;
char *opt_dev_label;
char *opt_aes_name;
char *opt_new_password;
char *opt_key_size_str;
char opt_key_format;
char usbk_dev[1024];
char opt_string_key[128];
char opt_aes_key[1024];
char set_key[1024];
//USBK_INFO *usbk_info;

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

    kflag = 1; opt_key = 1; // default key no 1
    fflag = 1; opt_key_format = 'd'; // default key format decimal
    Fflag = 1; opt_key_size_str = strdup("256");

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

    if (check_superuser() == false) {
        fprintf(stderr, MUST_SUPERUSER);
        exit(0);
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

    USBK *myusbk = usbk_new(usbk_dev);
    if (myusbk == NULL)
    {
        fprintf(stderr, "myusbk not created");
        exit(0);
    }

    if (usbk_get_lastopr_status(myusbk) != USBK_LO_PASS)
    {
        print_result(myusbk);
        usbk_release(myusbk);
        exit(0);
    }

    int res;

    /////////////////////////////////////////////
    // ACTIVATE
    /////////////////////////////////////////////
    if (pflag & aflag & kflag) {
        res = usbk_activatekey(myusbk, opt_parola, (int) opt_key);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // DEACTIVATE
    /////////////////////////////////////////////
    if (dflag) {
        res = usbk_deactivatekey(myusbk);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // CHANGE PASSWORD
    /////////////////////////////////////////////
    if (cflag) {
        res = usbk_changepassword(myusbk, opt_parola, opt_new_password);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SET DEVICE NAME
    /////////////////////////////////////////////
    if (pflag & nflag) {
        res = usbk_setdevicelabel(myusbk, opt_parola, opt_dev_label);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SET KEY
    /////////////////////////////////////////////
    if (pflag & xflag & kflag & Fflag) {
        switch (opt_key_format) {
        case 'd':
            res = usbk_setkey_decimal(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), opt_string_key);
            break;
        case 't':
            res = usbk_setkey_text(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), opt_string_key);
            break;
        default:
            break;
        }
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SET KEY RANDOMLY
    /////////////////////////////////////////////
    if (pflag & Xflag & kflag & Fflag) {
        int i;
        uint8_t randomkey[32];
        res = usbk_getrandomkey(myusbk, randomkey, parse_keysize(opt_key_size_str));

        if (res == USBK_LO_PASS) {
            res =
                    usbk_setkey_hex(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), randomkey);
            if (res == USBK_LO_PASS) {
                for (i = 0; i < parse_keysize_inbyte(opt_key_size_str); i++) {
                    fprintf(stdout, "%d", randomkey[i]);
                    if (i != (parse_keysize_inbyte(opt_key_size_str) - 1))
                        fprintf(stdout, ".");
                }
                printf("\n");
            }
        }
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SET KEY NAME ONLY
    /////////////////////////////////////////////
    if (mflag & !xflag & !Xflag) {
        if (pflag & kflag) {
            res = usbk_setkeyname(myusbk, opt_parola, opt_key, opt_aes_name);
            printf("%d\n", res);
            print_result(myusbk);
        }
    }

    /////////////////////////////////////////////
    // ENABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (pflag & tflag & kflag) {
        res = usbk_enableautact(myusbk, opt_parola, opt_key);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // DISABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (pflag & Tflag) {
        res = usbk_disableautact(myusbk, opt_parola);
        printf("%d\n", res);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SHOW USBK INFORMATION
    /////////////////////////////////////////////
    if (iflag) {
        linuxcli_show_dev_info(myusbk);
    }

    usbk_release(myusbk);

    exit(0);
}

KEYSIZE parse_keysize(char *s){
    if (!strcmp(s, "128")) {
        return KEYSIZE_128BIT;
    } else if (!strcmp(s, "192")) {
        return KEYSIZE_192BIT;
    } else if (!strcmp(s, "256")) {
        return KEYSIZE_256BIT;
    } else {
      return KEYSIZE_NULL;
    }
}

int parse_keysize_inbyte(char *s){
    if (!strcmp(s, "128")) {
        return 16;
    } else if (!strcmp(s, "192")) {
        return 24;
    } else if (!strcmp(s, "256")) {
        return 32;
    } else {
      return 0;
    }
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
                if (!Xflag && !xflag) main_operation++;
                opt_aes_name = strdup(optarg);
                break;

            case 'x':
                xflag = 1;
                if (!mflag) main_operation++;
                sprintf(opt_string_key, "%s", optarg);
                break;

            case 'X':
                Xflag = 1;
                if (!mflag) main_operation++;
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

void print_device(USBK* myusbk)
{
    if (usbk_check_support(myusbk) == true)
    {
        printf("  %s\n",                            usbk_get_dev_label(myusbk));
        printf("    Device   : %s\n",               usbk_get_dev(myusbk));
        printf("    BackDisk : %s\n",               usbk_get_backdisk(myusbk));
        printf("    Product  : %s\n",               usbk_get_product(myusbk));
        printf("    Model    : %s\n",               usbk_get_model(myusbk));
        printf("    Serial   : %s\n",               usbk_get_serial(myusbk));
        printf("    Firmware : %s\n",               usbk_get_firmware_ver(myusbk));
        printf("\n");
    }
    else
    {
        printf("  %s(UNSUPPORTED DEVICE)\n",        usbk_get_dev_label(myusbk));
        printf("    Device   : %s\n",               usbk_get_dev(myusbk));
        printf("    Product  : %s\n",               usbk_get_product(myusbk));
        printf("    Model    : %s\n",               usbk_get_model(myusbk));
        printf("    Serial   : %s\n",               usbk_get_serial(myusbk));
        printf("    Firmware : %s\n",               usbk_get_firmware_ver(myusbk));
        printf("\n");
    }
}

void linuxcli_show_devices(void) {
    int i;
    int rtn;

    USBKS *usbks = usbk_list_new();

    for (USBK_LIST* list_entry = usbks->usbk; list_entry != NULL; list_entry = list_entry->previous)
    {
        print_device(list_entry->me);
    }
    (usbks->counter > 0) ? printf ("%d",usbks->counter):printf ("None");
    printf (" USBK Found.\n\n");

    usbk_list_release(usbks);
}

void linuxcli_show_dev_info(USBK* myusbk) {


    //USBK_INFO *usbk_infos;
    if (usbk_check_support(myusbk) == true)
    {
        //UI_DEVINFO_T dev_info;
        int i;
        char status[64];
        char backdisk[8];
        char autoactive[64];
        char model[32];

        usbk_refreshusbkinfo(myusbk);

        sprintf(backdisk, "-");

        switch (usbk_get_state(myusbk)) {
        case USBK_DS_ACTIVATE:
            sprintf(status, "active [%d]", usbk_get_current_keyno(myusbk));
            sprintf(backdisk, "none");
            break;
        case USBK_DS_ACTIVATE_WITH_BACKDISK:
            sprintf(status, "active [%d]", usbk_get_current_keyno(myusbk));
            sprintf(backdisk, "exist");
            break;
        case USBK_DS_DEACTIVATE:
            sprintf(status, "deactive");
            break;
        case USBK_DS_FABRIC_DEFAULT:
            sprintf(status, "%s", MSG_DSE_FABRIC_DEFAULT);
            break;
        case USBK_DS_MUST_REMOVE:
            sprintf(status, "%s", MSG_USBK_UNPLUGING);
            break;
        default:
            sprintf(status, "%d:unknown", usbk_get_state(myusbk));
            break;
        }

        if (usbk_get_autoactivation_keyno(myusbk) == 0) {
            sprintf(autoactive, "disable");
        } else {
            sprintf(autoactive, "enable with key #%d", usbk_get_autoactivation_keyno(myusbk));
        }

        printf("\n  usbk information\n");
        printf("    device path             %s\n", usbk_get_dev_path(myusbk));
        printf("    back disk path          %s\n", usbk_get_backdisk_path(myusbk));
        printf("    product                 %s\n", usbk_get_product(myusbk));
        printf("    model                   %s\n", usbk_get_model(myusbk));
        printf("    serial number           %s\n", usbk_get_serial(myusbk));
        printf("    firmware verison        %s\n", usbk_get_firmware_ver(myusbk));
        printf("    label                   %s\n", usbk_get_dev_label(myusbk));
        printf("    status                  %s\n", status);
        printf("    retry number            %d\n", usbk_get_retry_number(myusbk));
        printf("    back disk               %s\n", backdisk);
        printf("    auto activation         %s\n", autoactive);
        printf("    max. key capacity       %d\n", usbk_get_multikeycap(myusbk));
        for (i = 0; i < usbk_get_multikeycap(myusbk); i++) {
            printf("      key %d name            %s\n", i+1, usbk_get_keyname(myusbk,i));
        }
    }
    else
    {
        printf("\n  usbk information\n");
        printf("    logic name              %s\n", usbk_get_dev_path(myusbk));
        printf("    product                 %s\n", usbk_get_product(myusbk));
        printf("    model                   %s\n", usbk_get_model(myusbk));
        printf("    serial number           %s\n", usbk_get_serial(myusbk));
        printf("    firmware verison        %s\n", usbk_get_firmware_ver(myusbk));
        printf("    label                   %s\n", usbk_get_dev_label(myusbk));
        printf("    UNSUPPORTED DEVICE!\n");
    }
}


static void print_result(USBK* myusbk) {
    switch (usbk_get_lastopr_status(myusbk)) {
    case USBK_LO_PASS:
        fprintf(stdout, "Pass.\n");
        break;
    case USBK_LO_GEN_FAIL:
        fprintf(stderr, "Fail.\n");
        break;
    case USBK_LO_FAILED_PASS:
        fprintf(stderr, "Failed Password, Retry Number = %d.\n", usbk_get_retry_number(myusbk));
        break;
    case USBK_LO_FABRIC_RESET:
        printf("\n");
        printf("***********************************************\n");
        printf(" All keys and your password is erased.         \n");
        printf(" Please, re-configure your device.             \n");
        printf("***********************************************\n");
        printf("\n");
        break;
    case USBK_LO_USBK_UNPLUGING:
        fprintf(stderr, "Must remove. Please remove and re-plug the USBK.\n");
        break;
    case USBK_LO_INVALID_KEYNO:
        fprintf(stderr, "Invalid Key Number.\n");
        break;
    case USBK_LO_INVALID_KEYSIZE:
        fprintf(stderr, "Invalid Key Size.\n");
        break;
    case USBK_LO_INVALID_DEVICELABEL:
        fprintf(stderr, "Invalid Device Label\n");
        break;
    case USBK_LO_INVALID_NEWPASS:
        fprintf(stderr, "Invalid New Password.\n");
        break;
    case USBK_LO_STATE_ERROR:

        switch (usbk_get_state(myusbk)) {
        case USBK_DS_ACTIVATE:
            fprintf(stderr, "USBK in active. This operation is not able to done in this state.\n");
            break;
        case USBK_DS_ACTIVATE_WITH_BACKDISK:
            fprintf(stderr, "The backdisk is plugged to USBK.  This operation is not able to done in this state.\n");
            break;
        case USBK_DS_DEACTIVATE:
            fprintf(stderr, "USBK in deactive. This operation is not able to done in this state.\n");
            break;
        case USBK_DS_FABRIC_DEFAULT:
            fprintf(stderr, "USBK in fabric default. Please first set your password.\n");
            break;
        case USBK_DS_MUST_REMOVE:
            fprintf(stderr, "USBK in must remove. This operation is not able to done in this state.\n");
            break;
        default:
            fprintf(stderr, "The state of USBK  is unknown. This operation is not able to done in this state.\n");
            break;
        }

        break;
    case USBK_LO_SCSI_ERROR:
        fprintf(stderr, "No Device Found (scsi error).\n");
        break;
    case USBK_LO_UNSUPPORTED_USBK:
        fprintf(stderr, "Unsupported USBK.\n");
        break;
    case USBK_LO_INVALID_KEY:
        fprintf(stderr, "Invalid Key.\n");
        break;
    case USBK_LO_UDEV_ERROR:
        fprintf(stderr, "No Device Found (udev error).\n");
        break;
    case USBK_LO_MEM_ERROR:
        fprintf(stderr, "No Device Found (mem error).\n");
        break;
    case USBK_LO_INVALID_PASS:
    default:
        fprintf(stderr, "Unkown Error.\n");
        break;
    }
}

static void print_help(int exval) {
    print_version();
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
