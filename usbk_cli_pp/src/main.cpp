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

#include "config.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <iostream>
#include <sstream>
#include <string.h>

#include "../../libusbk/src/libusbk.h"
#include "../../libusbk++/src/libusbk.hpp"

//PRIVATE FUNCTION DECLARATIONS
static int _parse_options(int *argc, char** argv[]);

// FIXME: remove these and find another way to deal with it.
usbk_keysize_t parse_keysize(char *s);
int parse_keysize_inbyte(char *s);

static void linuxcli_show_devices(void);
static void linuxcli_show_dev_info(UsbkDevice* device);

static void print_result(UsbkDevice* device);

static void print_help();
static void print_version();

//PRIVATE VARIABLES
//-FLAGS
int verbose_flag;
int uflag = 0;
int aflag = 0;
int dflag = 0;
int cflag = 0;
int Dflag = 0;
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
          /* These options don't set a flag. We distinguish them by their indices. */
        { "activate", no_argument, 0, 'a' },
        { "deactivate", no_argument, 0, 'd' },
        { "debug", no_argument, 0, 'D' },
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
        { "version", no_argument, 0, 'v' },
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
    	std::cerr << "Parse Error!";
        exit(EXIT_FAILURE);
    }

    if (question_flag) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    if (vflag) {
        print_version();
        exit(EXIT_SUCCESS);
    }

    if (Dflag) {
    	libusbk_enable_debug();
    	libusbk_plusplus_enable_debug();
    }

    if (main_operation > 1) {
    	std::cerr
    		<< "You may not specify more than one `-adcnmxXtTls' option\n"
    		<< "Try `usbk --help' for more information.\n";
        exit(EXIT_FAILURE);
    }

    if (!iflag && main_operation == 0) {
        exit(EXIT_SUCCESS);
    }

    if (geteuid() != 0) {
    	std::cerr << "Requested operation requires superuser privilege\n";
        exit(EXIT_FAILURE);
    }

    if (sflag) {
        linuxcli_show_devices();
        exit(EXIT_SUCCESS);
    }

    if (uflag == 0) {
    	std::cerr
    		<< "Dev paramter missing\n"
    		<< "Try `usbk --help' for more information.\n";
        exit(EXIT_SUCCESS);
    }

    /* FIXME: write exception handlers */
    UsbkDevice *device = new UsbkDevice(usbk_dev);

    if (device->lastOperation() != USBK_LO_PASS) {
    	print_result(device);
    	delete device;

        exit(0);
    }

    int ret;

    /////////////////////////////////////////////
    // ACTIVATE
    // FIXME: ask password from command line
    /////////////////////////////////////////////
    if (pflag & aflag & kflag) {
    	ret = device->activateKey(opt_parola, opt_key);
        printf("%d\n", ret);
        print_result(device);
    }

    /////////////////////////////////////////////
    // DEACTIVATE
    /////////////////////////////////////////////
    if (dflag) {
        ret = device->deactivate();
        printf("%d\n", ret);
        print_result(device);
    }

    /////////////////////////////////////////////
    // CHANGE PASSWORD
    /////////////////////////////////////////////
    if (cflag) {
        ret = device->changePassword(opt_parola, opt_new_password);
        printf("%d\n", ret);
        print_result(device);
    }

    /////////////////////////////////////////////
    // SET DEVICE NAME
    /////////////////////////////////////////////
    if (pflag & nflag) {
        ret = device->setDeviceLabel(opt_parola, opt_dev_label);
        printf("%d\n", ret);
        print_result(device);
    }

#if 0
    /////////////////////////////////////////////
    // SET KEY
    /////////////////////////////////////////////

    /* FIXME: setting keys will be rehandled! */

    if (pflag & xflag & kflag & Fflag) {
        switch (opt_key_format) {
        case 'd':
            printf("ERROR: Depreciated function!\n");
            ret=-1;

            //res = usbk_setkey_decimal(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), opt_string_key);
            break;
        case 't':
            ret = usbk_set_key_text(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), opt_string_key);
            break;
        default:
            break;
        }
        printf("%d\n", ret);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // SET KEY RANDOMLY
    /////////////////////////////////////////////
    /*if (pflag & Xflag & kflag & Fflag) {
        int i;
        uint8_t randomkey[32];
        ret = usbk_get_randomkey(myusbk, randomkey, parse_keysize(opt_key_size_str));

        if (ret == USBK_LO_PASS) {
            ret = usbk_set_key_hex(myusbk, opt_parola, opt_key, parse_keysize(opt_key_size_str), randomkey);
            if (ret == USBK_LO_PASS) {
                for (i = 0; i < parse_keysize_inbyte(opt_key_size_str); i++) {
                    fprintf(stdout, "%d", randomkey[i]);
                    if (i != (parse_keysize_inbyte(opt_key_size_str) - 1))
                        fprintf(stdout, ".");
                }
                printf("\n");
            }
        }
        printf("%d\n", ret);
        print_result(myusbk);
    }*/

    /////////////////////////////////////////////
    // SET KEY NAME ONLY
    /////////////////////////////////////////////
    if (mflag & !xflag & !Xflag) {
        if (pflag & kflag) {
            ret = usbk_set_keyname(myusbk, opt_parola, opt_key, opt_aes_name);
            printf("%d\n", ret);
            print_result(myusbk);
        }
    }

    /////////////////////////////////////////////
    // ENABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (pflag & tflag & kflag) {
        ret = USBK_ENABLE_AUTACT(myusbk, opt_parola, opt_key);
        printf("%d\n", ret);
        print_result(myusbk);
    }

    /////////////////////////////////////////////
    // DISABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (pflag & Tflag) {
        ret = USBK_DISABLE_AUTACT(myusbk, opt_parola);
        printf("%d\n", ret);
        print_result(myusbk);
    }
#endif

    /////////////////////////////////////////////
    // SHOW USBK INFORMATION
    /////////////////////////////////////////////
    if (iflag) {
        linuxcli_show_dev_info(device);
    }

    delete device;

    exit(0);
}

usbk_keysize_t parse_keysize(char *s){
    if (!strcmp(s, "128")) {
        return USBK_KEYSIZE_128BIT;
    } else if (!strcmp(s, "192")) {
        return USBK_KEYSIZE_192BIT;
    } else if (!strcmp(s, "256")) {
        return USBK_KEYSIZE_256BIT;
    } else {
      return USBK_KEYSIZE_NULL;
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

    if (*argc == 1) {
        question_flag = 1;
    } else {
        while (( opt = getopt_long(*argc, *argv, "u:adc:n:m:x:XtTlsk:p:F:f:ivD?", long_options, &option_index)) != -1) {
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
                	std::cerr << "Missing parameter!\n";
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

            case 'D':
                Dflag = 1;
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

void linuxcli_show_devices(void)
{
	UsbkDeviceList deviceList;

	std::cout << deviceList;
}

void linuxcli_show_dev_info(UsbkDevice* myusbk)
{
    std::cout << *myusbk;
}

static void print_result(UsbkDevice* device)
{
	std::string result;

    switch (device->lastOperation()) {
    case USBK_LO_PASS:
    	result = "Operation Successful";
        break;

    case USBK_LO_GEN_FAIL:
    	result = "Operation Failed!";
        break;

    case USBK_LO_FAILED_PASS:
    	{
    		std::stringstream s;

    		s << "Password Incorrect, Retry Number = "
    		  << device->retryNumber();

    		result = s.str();
    	}

        break;

    case USBK_LO_FABRIC_RESET:
        result =
        	"****************************************\n"
        	" All keys and your password is erased.  \n"
        	" Please, re-configure your device.      \n"
        	"****************************************";
        break;

    case USBK_LO_USBK_UNPLUGING:
    	result = "USBK must be removed and replugged.";
        break;

    case USBK_LO_INVALID_KEYNO:
        result = "Invalid Key Number.";
        break;

    case USBK_LO_INVALID_KEYSIZE:
        result = "Invalid Key Size.";
        break;

    case USBK_LO_INVALID_DEVICELABEL:
        result = "Invalid Device Label";
        break;

    case USBK_LO_INVALID_NEWPASS:
        result = "Invalid New Password.";
        break;

    case USBK_LO_STATE_ERROR:

        switch (device->deviceState()) {
        case USBK_DS_ACTIVATE:
            result = "USBK in active. This operation is not able to done in this state.";
            break;

        case USBK_DS_ACTIVATE_WITH_BACKDISK:
        	result = "The backdisk is plugged to USBK.  This operation is not able to done in this state.";
            break;

        case USBK_DS_DEACTIVATE:
            result = "USBK in deactive. This operation is not able to done in this state.";
            break;

        case USBK_DS_FABRIC_DEFAULT:
        	result = "USBK in fabric default. Please first set your password.";
            break;

        case USBK_DS_MUST_REMOVE:
            result = "USBK in must remove. This operation is not able to done in this state.";
            break;

        default:
        	result = "The state of USBK is unknown. This operation is not able to done in this state.";
            break;
        }

        break;
    case USBK_LO_SCSI_ERROR:
    	result = "SCSI Error! No Device Found!";
        break;

    case USBK_LO_UNSUPPORTED_USBK:
    	result = "Unsupported USBK.";
        break;

    case USBK_LO_INVALID_KEY:
    	result = "Invalid Key.";
        break;

    case USBK_LO_UDEV_ERROR:
        result = "UDEV Error! No Device Found.";
        break;

    case USBK_LO_MEM_ERROR:
    	result = "Mem Error! No Device Found.";
        break;

    case USBK_LO_INVALID_PASS:
        result = "Invalid Password.";
        break;

    default:
        result = "Unknown error!";
        break;
    }

    std::cout << result << std::endl;
}

static void print_help()
{
    print_version();
    std::cout
    	<< std::endl << "USBK CryptoBridge Configurator"
    	<< std::endl << "\tUsage: usbk [options]"
    	<< std::endl
    	<< std::endl << "Main operations:"
    	<< std::endl << "  -u, --dev                     device"
        << std::endl << "  -a, --activate                activate device"
        << std::endl << "  -d, --deactivate              deactivate device"
        << std::endl << "  -c, --newpasswd=NEWPASS       change the password to NEWPASS"
        << std::endl << "  -n, --label=LABEL             change the label to LABEL"
        << std::endl << "  -m, --keyname=KEYNAME         change then key nameto NAME"
        << std::endl << "  -x, --change-key=NEWKEY       change the key to NEWKEY"
        << std::endl << "  -X, --change-key-with-random  change the key to random key"
        << std::endl << "  -t, --enable-auto             enable auto activate"
        << std::endl << "  -T, --disable-auto            disable auto activate"
        << std::endl << "  -l, --gen-key                 generate and set random key"
        << std::endl << "  -s, --show-devices            show device list"
        << std::endl
        << std::endl << " Setting options:"
        << std::endl
        << std::endl << "  -k, --key-no=KEYNO            use KEYNO as key number"
        << std::endl << "  -p, --passwd=PASSWD           checks password with PASSWD"
        << std::endl << "  -F, --key-size=KEY_SIZE       KEYSIZE is 128, 192 or 256"
        << std::endl << "  -f, --key-format=FORMAT       FORMAT=t for text or FORMAT=d for"
        << std::endl
        << std::endl << " Other options:\n"
        << std::endl
        << std::endl << "  -i, --show-info               show device info"
        << std::endl << "  -D, --debug                   print debug output"
        << std::endl << "  -v, --version                 print program version"
        << std::endl << "  -?, --help                    give this help list"
        << std::endl
        << std::endl << "defaults for options:"
        << std::endl << "--key-size=256"
        << std::endl << "--key-format=d"
        << std::endl << "--key-no=1"
        << std::endl
        << std::endl << "Examples:"
        << std::endl << "  usbk -s                       # Show device list"
        << std::endl << "  usbk sdc -a -k 1 -p foo       # activate device with key 1"
        << std::endl << "  usbk sdc -d                   # deactivate device"
        << std::endl << std::endl;
}

static void print_version(void)
{
	std::cout << USBK_CLI_PACKAGE << " " << USBK_CLI_VERSION << std::endl;
}
