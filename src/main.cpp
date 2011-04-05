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

//TODO: BackDisk mount edilmiş ise deactive işlemi sırasında kullanıcıya uyarı ver ve emin misin diye sor.

#include "usbk.h"

//Msgs of DeviceControl
#define MISSING_PARAMETER    "Missing parameter"
#define WARNING              "Warning:"
#define MSG_FABRIC_DEFAULT   "Fabric default. Please first set your password."
#define MSG_MUST_REMOVE      "Must remove. Please remove and re-plug the USBK."
#define NOT_CREATE_RANDOM_KEY "Key Random Olarak Uretilemiyor."
#define NOT_MALLOC            "memory'de yer yok!\n"

using namespace std;

/* Flag set by ‘--verbose’. */
static int verbose_flag;

static int
_parse_options(int *argc, char** argv[]);


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

        /* Last element must be NULL */
        { NULL, 0, NULL, 0 } };

/* configs for cmd-args*/
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
int kflag = 0;
int pflag = 0;
int fflag = 1;
int Fflag = 1;
int iflag = 0;

int main_operation = 0;

t_UIP_ACTIVATE act;
t_UIP_PAROLA opt_parola;
t_UIP_KEYNB opt_key;
t_UIP_DEVLABEL opt_dev_name;
t_UIP_SETDEVICELABEL set_dev_name;
t_UIP_SETKEY set_key;
t_UIP_KEYNAME opt_aes_name;
t_UIP_CHPASS change_pas;
t_UIP_PAROLA opt_new_password;
t_UIP_KEY opt_aes_key;
t_UIP_SETAUTOACTIVATE set_auto;
char opt_key_format;

e_UIP_KEYSIZE opt_key_size;
int opt_key_size_byte;

char opt_string_key[128];

USBK usbk;

void linuxcli_show_devices(USBK_List* p_usbklink);

/*! \brief main function
 *
 * \param argc command line argument count
 * \param argv argument list
 * \return 0 normal exit
 * \return 1 abnormal exit
 */

bool StatusChecker(int status) {
    switch ((e_UIP_OPRSTATUS) status) {
    case OPRS_PASS:
        return true;
        break;
    case OPRS_INVALID_PASS:
        printf("Hata: Parola yanlis. RetryNum:%d\n", 0);//usbk->status.retry_num);
        return false;
        break;
    case OPRS_FABRIC_RESET:
        printf("\n*********************************************\n");
        printf(" Uyari: USBK cihazinizdaki bilgiler silindi. \n");
        printf(" Lutfen cihazinizi tekrar konfigure ediniz.  \n");
        printf("*********************************************\n\n");
        return true;
        break;
    case OPRS_USBK_UNPLUGING:
        printf("%s\n", MSG_MUST_REMOVE);
        return false;
        break;
    default:
        printf("Hata: Islem basarisiz. MSG_CODE:0x%02X RetryNum:%d\n", status, 0);//usbk->status.retry_num);    //todo
        return false;
        break;
    }
    return false;
}

int main(int argc, char *argv[]) {

    int c;
    int status;

    USBK_List* p_usbk = NULL;

    opt_key = 1; // default key no 1
    opt_key_format = 'd'; // default key format decimal
    opt_key_size = KEYSIZE_256;

    if (!_parse_options(&argc, &argv)) {
        printf("Parse error\n");
        exit(0);
    }

    if (main_operation > 1) {
        printf("usbk: You may not specify more than one `-adcnmxXtTls' option\n");
        printf("usbk: Try `usbk --help' for more information.\n");
        exit(1);
    }

    if (!iflag && main_operation == 0) {
        exit(0);
    }

    if (uflag == 0) {
        printf("Uyarı: Device name parametresi eksik.\n");
        exit(0);
    }

    //usbk_get_device_info(&usbk);
    LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info));

    /////////////////////////////////////////////
    // ACTIVATE
    /////////////////////////////////////////////
    if (aflag) {
        if (kflag) {
            if (pflag) {
                switch (usbk.info.devstate.me) {
                case ACTIVATE:
                case ACTIVATE_WITH_BACKDISK:
                    printf("%s [%s] cihazı zaten aktif.\n", usbk.dev, usbk.info.devlabel.s);
                    break;
                case DEACTIVATE:
                    status = LibUSBK__ActivateKey(usbk.dev_path, opt_parola.s, (int)opt_key);

                    if (StatusChecker(status) != true){
                        exit(1);
                    }
                    if (iflag) {
                        if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info)) >= 0)
                        {
                            linuxcli_show_dev_info(&usbk);
                            printf("Done.\n");
                        }
                    }
                    exit(0);
                    break;
                case FABRIC_DEFAULT:
                    printf("%s\n", MSG_FABRIC_DEFAULT);
                    exit(0);
                    break;
                case MUST_REMOVE:
                    printf("%s\n", MSG_MUST_REMOVE);
                    exit(0);
                    break;
                default:
                    break;
                }
            } else {
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // DEACTIVATE
    /////////////////////////////////////////////
    if (dflag) {
        switch (usbk.info.devstate.me) {
        case ACTIVATE:
        case ACTIVATE_WITH_BACKDISK:
            status = LibUSBK__DeActivateKey(usbk.dev_path);
            if (StatusChecker(status) != true){
                exit(1);
            }
            if (iflag) {
                if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0)
                {
                    linuxcli_show_dev_info(&usbk);
                    printf("Done.\n");
                }
            }
            exit(0);
            break;
        case DEACTIVATE:
            printf("%s [%s] cihazı zaten deaktif.\n", usbk.dev, usbk.info.devlabel.s);
            exit(0);
            break;
        case FABRIC_DEFAULT:
            printf("%s\n", MSG_FABRIC_DEFAULT);
            exit(0);
            break;
        case MUST_REMOVE:
            printf("%s\n", MSG_MUST_REMOVE);
            exit(0);
            break;
        default:
            break;
        }
    }

    /////////////////////////////////////////////
    // CHANGE PASSWORD
    /////////////////////////////////////////////
    if (cflag) {
        switch (usbk.info.devstate.me) {
        case ACTIVATE:
        case ACTIVATE_WITH_BACKDISK:
            printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
            exit(0);
            break;
        case DEACTIVATE:
            if (pflag) {
                status = LibUSBK__ChangePassword(usbk.dev_path, opt_parola.s, opt_new_password.s);
                if (StatusChecker(status) != true){
                    exit(1);
                }

                if (iflag) {
                    if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                        linuxcli_show_dev_info(&usbk);
                        printf("Done.\n");
                    }
                }
                exit(0);
            } else {
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
            break;
        case FABRIC_DEFAULT:

            status = LibUSBK__ChangePassword(usbk.dev_path, NULL, opt_new_password.s);
            if (StatusChecker(status) != true){
                exit(1);
            }

            if (iflag) {
                if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                    linuxcli_show_dev_info(&usbk);
                    printf("Done.\n");
                }
            }

            exit(0);
            break;
        case MUST_REMOVE:
            printf("%s\n", MSG_MUST_REMOVE);
            exit(0);
            break;
        default:
            break;
        }
    }

    /////////////////////////////////////////////
    // SET DEVICE NAME
    /////////////////////////////////////////////
    if (nflag) {
        if (pflag) {
            switch (usbk.info.devstate.me) {
            case ACTIVATE:
            case ACTIVATE_WITH_BACKDISK:
                printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
                exit(0);
                break;
            case DEACTIVATE:
                memset(&set_dev_name, 0, sizeof(set_dev_name));
                strncpy(set_dev_name.password.s, opt_parola.s, sizeof(set_dev_name.password.s));
                strncpy(set_dev_name.devlabel.s, opt_dev_name.s, sizeof(set_dev_name.devlabel.s));

                status = LibUSBK__SetDeviceName(&usbk, (unsigned char*) &set_dev_name, sizeof(set_dev_name));
                if (StatusChecker(status) != true){
                    exit(1);
                }

                if (iflag) {
                    if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0)
                    {
                        linuxcli_show_dev_info(&usbk);
                        printf("Done.\n");
                    }
                }
                exit(0);
                break;
            case FABRIC_DEFAULT:
                printf("%s\n", MSG_FABRIC_DEFAULT);
                exit(0);
                break;
            case MUST_REMOVE:
                printf("%s\n", MSG_MUST_REMOVE);
                exit(0);
                break;
            default:
                break;
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // SET KEY NAME ONLY
    /////////////////////////////////////////////
    if (mflag && !xflag && !Xflag) {
        if (kflag) {
            if (pflag) {
                switch (usbk.info.devstate.me) {
                case ACTIVATE:
                case ACTIVATE_WITH_BACKDISK:
                    printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
                    exit(0);
                    break;
                case DEACTIVATE:
                    memset(&set_key, 0, sizeof(set_key));
                    strncpy(set_key.password.s, opt_parola.s, sizeof(set_key.password.s));
                    memcpy(&set_key.keyno, &opt_key, sizeof(set_key.keyno));
                    strncpy(set_key.keyname.s, opt_aes_name.s, sizeof(set_key.keyname.s));
                    memset(&set_key.options.me, SETKEY_NAMEONLY, sizeof(set_key.options.me));

                    status = LibUSBK__SetKey (&usbk, (unsigned char*) &set_key, sizeof(set_key));
                    if (StatusChecker(status) != true){
                        exit(1);
                    }

                    if (iflag) {
                        if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                            linuxcli_show_dev_info(&usbk);
                            printf("Done.\n");
                        }
                    }

                    exit(0);
                    break;
                case FABRIC_DEFAULT:
                    printf("%s\n", MSG_FABRIC_DEFAULT);
                    exit(0);
                    break;
                case MUST_REMOVE:
                    printf("%s\n", MSG_MUST_REMOVE);
                    exit(0);
                    break;
                default:
                    printf("default error\n");
                    break;
                }
            } else {
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
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
                    switch (usbk.info.devstate.me) {
                    case ACTIVATE:
                    case ACTIVATE_WITH_BACKDISK:
                        printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
                        exit(0);
                        break;
                    case DEACTIVATE:
                        memset(&set_key, 0, sizeof(set_key));
                        strncpy(set_key.password.s, opt_parola.s, sizeof(set_key.password.s));
                        memcpy(&set_key.keyno, &opt_key, sizeof(set_key.keyno));
                        memset(&set_key.options.me, SETKEY_NAME_AND_KEY, sizeof(set_key.options.me));
                        set_key.keysize.me = opt_key_size;

                        if (mflag) {
                            strncpy(set_key.keyname.s, opt_aes_name.s, sizeof(set_key.keyname.s));
                        } else {
                            strncpy(set_key.keyname.s, usbk.info.keyname[opt_key].s, sizeof(set_key.keyname.s));
                        }


                        switch (opt_key_size)
                        {
                        case KEYSIZE_128:
                            opt_key_size_byte = 16;
                            break;
                        case KEYSIZE_192:
                            opt_key_size_byte = 24;
                            break;
                        case KEYSIZE_256:
                        default:
                            opt_key_size_byte = 32;
                            break;

                        }

                        if (xflag) {

                            //printf("key:%s\n", opt_string_key);
                            if (opt_key_format == 'd') {
                                if (check_key_decimal((string) opt_string_key, opt_aes_key.u8, opt_key_size_byte)
                                        == -1) {
                                    printf("Hata: key decimal format yanlis.\n");
                                    exit(0);
                                }
                            } else if (opt_key_format == 't') {
                                if (check_key_text((string) opt_string_key, set_key.key.u8, opt_key_size_byte)
                                        == -1) {
                                    printf("Hata: key text format yanlis.\n");
                                    exit(0);
                                }
                            } else {
                                cout << "Beklenmeyen hata!" << endl;
                            }

                        }
                        else if (Xflag) {
                            t_UIP_SETKEY dummy_set_key;
                            status = LibUSBK__GetRandomKey(&usbk, (unsigned char*) dummy_set_key.key.u8, sizeof(dummy_set_key.key));
                            if (StatusChecker(status) != true){
                                fprintf (stderr, NOT_CREATE_RANDOM_KEY);
                                exit(1);
                            }
                            memcpy (set_key.key.u8, dummy_set_key.key.u8, opt_key_size_byte);
                        }

                        status = LibUSBK__SetKey (&usbk, (unsigned char*) &set_key, sizeof(set_key));
                        if (StatusChecker(status) != true){
                            exit(1);
                        }

                        if (Xflag) {
                            int i;
                            printf("Set Edilen Key : ");
                            for (i=0; i<opt_key_size_byte; i++)
                            {
                                printf("%d",set_key.key.u8[i]);

                                if (i != (opt_key_size_byte-1)) printf(".");
                            }
                            printf ("\n");
                        }

                        if (iflag) {
                            if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                                linuxcli_show_dev_info(&usbk);
                                printf("Done.\n");
                            }
                        }

                        exit(0);
                        break;
                    case FABRIC_DEFAULT:
                        printf("%s\n", MSG_FABRIC_DEFAULT);
                        exit(0);
                        break;
                    case MUST_REMOVE:
                        printf("%s\n", MSG_MUST_REMOVE);
                        exit(0);
                        break;
                    default:
                        break;
                    }
                } else {
                    printf("%s%s\n", WARNING, MISSING_PARAMETER);
                    exit(0);
                }
            } else {
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /*
     switch(usbk.info.devstate.me){
     case ACTIVATE:
     case ACTIVATE_WITH_BACKDISK:
     break;
     case DEACTIVATE:
     break;
     case FABRIC_DEFAULT:
     printf("%s\n", MSG_FABRIC_DEFAULT);
     exit(0);
     break;
     case MUST_REMOVE:
     printf("%s\n", MSG_MUST_REMOVE);
     exit(0);
     break;
     default:
     break;
     }

     */

    /////////////////////////////////////////////
    // ENABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (tflag) {
        if (pflag) {
            if (kflag) {
                switch (usbk.info.devstate.me) {
                case ACTIVATE:
                case ACTIVATE_WITH_BACKDISK:
                    printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
                    exit(0);
                    break;
                case DEACTIVATE:
                    memset(&set_auto, 0, sizeof(set_auto));
                    strncpy(set_auto.password.s, opt_parola.s, sizeof(set_auto.password.s));
                    memcpy(&set_auto.keyno, &opt_key, sizeof(set_auto.keyno));
                    //set_auto.keyno = 3;

                    status = LibUSBK__SetAutoAct(&usbk, (unsigned char*) &set_dev_name, sizeof(set_dev_name));
                    if (StatusChecker(status) != true){
                        exit(1);
                    }

                    if (iflag) {
                        if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                            linuxcli_show_dev_info(&usbk);
                            printf("Done.\n");
                        }
                    }

                    exit(0);
                    break;
                case FABRIC_DEFAULT:
                    printf("%s\n", MSG_FABRIC_DEFAULT);
                    exit(0);
                    break;
                case MUST_REMOVE:
                    printf("%s\n", MSG_MUST_REMOVE);
                    exit(0);
                    break;
                default:
                    break;
                }
            } else {
                printf("%s%s\n", WARNING, MISSING_PARAMETER);
                exit(0);
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    /////////////////////////////////////////////
    // DISABLE AUTO ACTIVATE
    /////////////////////////////////////////////
    if (Tflag) {
        if (pflag) {
            switch (usbk.info.devstate.me) {
            case ACTIVATE:
            case ACTIVATE_WITH_BACKDISK:
                printf("%s [%s] cihazı aktif iken bu işlem yapılamaz.\n", usbk.dev, usbk.info.devlabel.s);
                exit(0);
                break;
            case DEACTIVATE:
                memset(&set_auto, 0, sizeof(set_auto));
                strncpy(set_auto.password.s, opt_parola.s, sizeof(set_auto.password.s));
                memset(&set_auto.keyno, 0, sizeof(set_auto.keyno));

                status = LibUSBK__SetAutoAct(&usbk, (unsigned char*) &set_dev_name, sizeof(set_dev_name));
                if (StatusChecker(status) != true){
                    exit(1);
                }

                if (iflag) {
                    if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
                        linuxcli_show_dev_info(&usbk);
                        printf("Done.\n");
                    }
                }

                exit(0);
                break;
            case FABRIC_DEFAULT:
                printf("%s\n", MSG_FABRIC_DEFAULT);
                exit(0);
                break;
            case MUST_REMOVE:
                printf("%s\n", MSG_MUST_REMOVE);
                exit(0);
                break;
            default:
                break;
            }
        } else {
            printf("%s%s\n", WARNING, MISSING_PARAMETER);
            exit(0);
        }
    }

    if (iflag) {
        if (LibUSBK__GetDeviceInfo(&usbk, (unsigned char*) &usbk.info, sizeof(usbk.info))>=0){
            linuxcli_show_dev_info(&usbk);
            printf("Done.\n");
        }

    }

    exit(0);
}

void print_help(int exval) {
    print_version();
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

    exit(exval);
}

/*!
 *
 *murat@Murat:~/Tamara/Projeler/USB-K/linux/workspace/usbk-1.2/build$ sudo src/usbk /dev/sdd -i
 * logic name              /dev/sdd
 * model                   CryptoBridge
 * revision                Beta
 * serial number           0300000001035553424B0000000000
 * firmware verison        v2.5 beta
 * label
 * status                  0
 * retry number            0
 * back disk               -
 * auto activation         disable
 * max. key capacity       0
 *Done.
 *
 */
void linuxcli_show_dev_info(USBK *usbk) {
    //UI_DEVINFO_T dev_info;
    int i;
    char status[64];
    char backdisk[8];
    char autoactive[64];
    char model[32];

    //sprintf(model, "%s %s", usbk->model, usbk->rev);

    sprintf(backdisk, "-");

    switch (usbk->info.devstate.me) {
    case ACTIVATE:
        sprintf(status, "active [%d]", usbk->info.current_keyno);
        sprintf(backdisk, "none");
        break;
    case ACTIVATE_WITH_BACKDISK:
        sprintf(status, "active [%d]", usbk->info.current_keyno);
        sprintf(backdisk, "exist");
        break;
    case DEACTIVATE:
        sprintf(status, "deactive");
        break;
    case FABRIC_DEFAULT:
        sprintf(status, "%s", MSG_FABRIC_DEFAULT);
        break;
    case MUST_REMOVE:
        sprintf(status, "%s", MSG_MUST_REMOVE);
        break;
    default:
        sprintf(status, "%d:unknown", usbk->info.devstate.me);
        break;
    }

    if (usbk->info.autoactivate_keyno == 0) {
        sprintf(autoactive, "disable");
    } else {
        sprintf(autoactive, "enable with key #%d", usbk->info.autoactivate_keyno);
    }

    printf("\n  usbk information\n");
    printf("    logic name              %s\n", usbk->dev);
    printf("    back disk name          %s\n", usbk->backdisk);
    printf("    product                 %s\n", usbk->info.product.s);
    printf("    model                   %s\n", usbk->info.model.s);
    printf("    serial number           ");
    for (i = 0; i < 15; i++) {
        printf("%2.2X", usbk->info.serial.u8[i]);
    }
    printf("\n");
    printf("    firmware verison        %s\n", usbk->info.firmware_ver.s);
    printf("    label                   %s\n", usbk->info.devlabel.s);
    printf("    status                  %s\n", status);
    printf("    retry number            %d\n", usbk->info.retry_num);
    printf("    back disk               %s\n", backdisk);
    printf("    auto activation         %s\n", autoactive);
    printf("    max. key capacity       %d\n", usbk->info.multikeycap);
    for (i = 0; i < usbk->info.multikeycap; i++) {
        printf("      key %d name            %s\n", i, usbk->info.keyname[i].s);
    }
    /*

     //      123456789012 1234567890123456 1234 12345678901 123456 12345678 1234567890 12345 12345678 123456789 123456789012
     printf("device       Model            Rev  Name        Serial Firmware Status     Retry BackDisk Auto Act. KeyNames    \n");
     //                                                 deactive                  disable
     //                                                 active [1                 key 1
     printf(
     "%-12.12s %-16.16s %-4.4s %-11.11s %-6.6s %-8.8s %-10.10s %-5d %-8.8s %-9.9s [1] %-12s\n",
     usbk->dev, usbk->model, usbk->rev, usbk->info.devlabel.s,
     usbk->info.serial.u8, usbk->info.firmware_ver.s, status,
     usbk->info.retry_num, backdisk, autoactive, usbk->info.keyname[0].s);
     for (i = 1; i < usbk->info.multikeycap; i++)
     printf("%99.s[%d] %-12s\n", " ", i + 1, usbk->info.keyname[i].s);

     */

}

// 1) string icinde bosluk karakteri var mi?
// 2) string icerisindeki '.' karakterinin sayisi 15 mi?
// 3) '.' karakterlerini ' ' karakterine donustur
// 4) string icerisinde bosluklar haric decimal olmayan karakter var mi?
// 5) string to integer islemini yap
// 6) 16 adet sayı cikartabildin mi?
// 7) sayılar 0 ile 255 arasinda mi?
int check_key_decimal(string str, U8 *key, int key_size_byte) {
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

int check_key_text(string str, U8 *key, int key_size_byte) {
    if (str.size() > key_size_byte) {
        return -1;
    }
    strncpy((char*) key, str.c_str(), key_size_byte);
    return 0;
}

void print_version(void) {
    printf("%s version %s\n", PACKAGE, VERSION);
}

static int _parse_options(int *argc, char** argv[]) {
    /* getopt_long stores the option index here. */
    int option_index = 0;
    int opt;

    if (*argc == 1)
    {
        print_help(0);
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
                usbk.dev = strdup(optarg);
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
                if (strlen(optarg) > sizeof(opt_new_password.s)) {
                    printf("Uyarı: Parola %d karakterden uzun olamaz.\n", sizeof(opt_new_password.s));
                    exit(1);
                }
                strncpy(opt_new_password.s, optarg, sizeof(opt_new_password.s));
                break;

            case 'n':
                nflag = 1;
                main_operation++;
                strncpy(opt_dev_name.s, optarg, sizeof(opt_dev_name.s));
                break;

            case 'm':
                mflag = 1;
                main_operation++;
                if (strlen(optarg) > sizeof(opt_aes_name.s)) {
                    printf("Uyarı: Key adi %d karakterden uzun olamaz.\n", sizeof(opt_aes_name.s));
                    exit(1);
                }
                strncpy(opt_aes_name.s, optarg, sizeof(opt_aes_name.s));
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

            case 's': {
                USBK_List* p_usbklink = NULL;
                p_usbklink = LibUSBK__list_devices();
                if (p_usbklink == NULL){
                    fprintf(stderr, NOT_MALLOC);
                    exit(1);
                }
                linuxcli_show_devices(p_usbklink);
                LibUSBK__list_devices_release(p_usbklink);
            }
                exit(0);
                break;

            case 'k':
                kflag = 1;
                opt_key = atoi(optarg);
                break;

            case 'p':
                pflag = 1;
                if (strlen(optarg) > sizeof(opt_parola.s)) {
                    printf("Uyarı: Parola %d karakterden uzun olamaz.\n", sizeof(opt_parola.s));
                    exit(1);
                }
                strncpy(opt_parola.s, optarg, sizeof(opt_parola.s));
                break;

            case 'f':
                fflag = 1;
                if (!strcmp(optarg, "d")) {
                    opt_key_format = 'd';
                } else if (!strcmp(optarg, "t")) {
                    opt_key_format = 't';
                } else {
                    printf("%s%s\n", WARNING, MISSING_PARAMETER);
                    exit(1);
                }
                break;

            case 'F':
                Fflag = 1;
                if (!strcmp(optarg, "128")) {
                    opt_key_size = KEYSIZE_128;
                } else if (!strcmp(optarg, "192")) {
                    opt_key_size = KEYSIZE_192;
                } else if (!strcmp(optarg, "256")) {
                    opt_key_size = KEYSIZE_256;
                } else {
                    printf("%s%s\n", WARNING, MISSING_PARAMETER);
                    exit(1);
                }
                break;
            case 'i':
                iflag = 1;
                break;

            case 'v':
                print_version();
                break;

            case '?':
                print_help(0);
                break;
            default:
                break;
            }
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
    }

    return 1;
}

void linuxcli_show_devices(USBK_List* p_usbklink) {
    int i;
    int counter = 0;
    USBK_List *dummy_usbklink;

    for (dummy_usbklink = p_usbklink; dummy_usbklink != NULL; dummy_usbklink = dummy_usbklink->next) {
        printf("  %s\n",             dummy_usbklink->usbk.info.devlabel.s);
        printf("    Device   : %s\n",             dummy_usbklink->usbk.dev);
        printf("    BackDisk : %s\n",             dummy_usbklink->usbk.backdisk);
        printf("    Product  : %s\n",             dummy_usbklink->usbk.info.product.s);
        printf("    Model    : %s\n",             dummy_usbklink->usbk.info.model.s);
        printf("    Serial   : "); for (i = 0; i < 15; i++) printf("%2.2X", dummy_usbklink->usbk.info.serial.u8[i]); printf("\n");
        printf("    Firmware : %s\n",             dummy_usbklink->usbk.info.firmware_ver.s);
        printf("\n");
        counter++;
    }

    (counter > 0) ? printf ("%d",counter):printf ("None");
    printf (" USBK Found.\n\n");
}
