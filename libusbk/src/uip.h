/*! \file *********************************************************************
 *
 * \brief UIP.h - Created on: Feb 18, 2011
 *
 * This file includes...
 *
 * - Compiler:              GNU GCC for AVR32, GNU GCC for Linux CLI, Visual Studio 2010 for Windows GUI
 * - Supported firmware:    USBK 2v5
 * - Supported software:    USBK Management Software GUI for Windows 2v3 and USBK Management Software CLI for Linux 1v2
 * - AppNote:               USBK_UI_Protocol_2v5r7
 *
 * \author                  Timucin Anuslu, Murat Kılıvan
 *                          Tamara Elektronik : http://www.tamara.com.tr \n
 *
 ******************************************************************************/

#ifndef UIP_H_
#define UIP_H_


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAUTION :
//  The enum of this protocol is 1 byte. The parameter to declare the type of enum as 1 byte must be defined for complier.
//
//  For example :
//              In C for AVR32,
//                      typedef enum __attribute__((__packed__))
//                      {
//                          OPMSG_DISABLE           = 0,
//                      }t_UIP_OPRSTATUS_MSG;
//
//              In C and C++ for Linux,
//                      typedef enum __attribute__((__packed__))
//                      {
//                          OPMSG_DISABLE           = 0,
//                      }t_UIP_OPRSTATUS_MSG;
//
//              In C# for windows,
//                      typedef enum : char{
//                          OPMSG_DISABLE           = 0,
//                      }t_UIP_OPRSTATUS_MSG;
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "libusbk.h"

#if defined(__AVR32__)
#  define ATTR_PACKED_BEGIN __attribute__((__packed__))
#  define ATTR_ENUM_PACKED_BEGIN __attribute__((__packed__))
#  define ATTR_PACKED_END
#  define ATTR_ENUM_PACKED_END
#elif  defined(__linux__)
#  define ATTR_PACKED_BEGIN
#  define ATTR_ENUM_PACKED_BEGIN __attribute__((__packed__))
#  define ATTR_PACKED_END
#  define ATTR_ENUM_PACKED_END
#elif defined(WIN32)
#  define ATTR_PACKED_BEGIN
#  define ATTR_ENUM_PACKED_BEGIN
#  define ATTR_PACKED_END
#  define ATTR_ENUM_PACKED_END : char
#else
#  error unsupported environment
#endif

#define PROTOCOL_HEADER "USBK"

typedef uint32_t t_UIP_TAGNUMBER;

typedef struct ATTR_PACKED_BEGIN __UIP_FIRMVER ATTR_PACKED_END
{
#define UIP_FIRMWAREVIR_SIZE        15
    char                s[UIP_FIRMWAREVIR_SIZE+1];
}t_UIP_FIRMVER;

typedef union ATTR_PACKED_BEGIN __UIP_SERIAL ATTR_PACKED_END
{
#define UIP_SERIAL_TOTALSIZE        32
#define UIP_SERIAL_SIZE             15
    uint8_t u8[UIP_SERIAL_SIZE];
    uint8_t all[UIP_SERIAL_TOTALSIZE];
}t_UIP_SERIAL;

typedef uint8_t t_UIP_FABDEF;
typedef uint8_t t_UIP_LOGSTAT;
typedef uint8_t t_UIP_BACKDSTAT;

typedef struct ATTR_PACKED_BEGIN __UIP_PRODUCT ATTR_PACKED_END
{
#define UIP_PRODUCT_SIZE          31
    char s[UIP_PRODUCT_SIZE+1];
}t_UIP_PRODUCT;

typedef struct ATTR_PACKED_BEGIN __UIP_MODEL ATTR_PACKED_END
{
#define UIP_MODEL_SIZE          15
    char s[UIP_MODEL_SIZE+1];
}t_UIP_MODEL;


typedef union ATTR_PACKED_BEGIN __UIP_DEVLABEL ATTR_PACKED_END
{
#define UIP_DEVLABEL_TOTALSIZE      65
#define UIP_DEVLABEL_SIZE           11
    char    s[UIP_DEVLABEL_SIZE+1];
    uint8_t all[UIP_DEVLABEL_TOTALSIZE];
}t_UIP_DEVLABEL;

typedef enum ATTR_ENUM_PACKED_BEGIN __E_UIP_DEVSTATE ATTR_ENUM_PACKED_END // must be 1 byte
{
    ACTIVATE                = 1,
    ACTIVATE_WITH_BACKDISK  = 2,
    DEACTIVATE              = 3,
    FABRIC_DEFAULT          = 4,
    MUST_REMOVE             = 5,
}e_UIP_DEVSTATE;

typedef union ATTR_PACKED_BEGIN __UIP_DEVSTATE ATTR_PACKED_END
{
#define UIP_DEVSTATE_TOTALSIZE  4
    e_UIP_DEVSTATE  me;
    uint8_t         all[UIP_DEVSTATE_TOTALSIZE];
}t_UIP_DEVSTATE;


typedef uint8_t t_UIP_KEYNB;
typedef uint8_t t_UIP_NBRETRY;

typedef union ATTR_PACKED_BEGIN __UIP_KEY ATTR_PACKED_END
{
#define KEY_LEN     32
      uint8_t  u8 [KEY_LEN];
}t_UIP_KEY;

typedef union ATTR_PACKED_BEGIN __UIP_KEYNAME ATTR_PACKED_END
{
#define UIP_KEYNAME_TOTALSIZE       65
#define UIP_KEYNAME_SIZE            12
    char        s[UIP_KEYNAME_SIZE+1];
    uint8_t all [UIP_KEYNAME_TOTALSIZE];
}t_UIP_KEYNAME;

typedef union ATTR_PACKED_BEGIN __UIP_PAROLA ATTR_PACKED_END
{
#define UIP_PAROLA_TOTALSIZE        65
#define UIP_PAROLA_SIZE             17
      char      s[UIP_PAROLA_SIZE];
      uint8_t   all[UIP_PAROLA_TOTALSIZE];
} t_UIP_PAROLA;

typedef uint8_t t_UIP_OPTION;

typedef enum ATTR_ENUM_PACKED_BEGIN __E_UIP_OPRSTATUS ATTR_ENUM_PACKED_END // must be 1 byte
{
    OPRS_PASS                   = 1,
    OPRS_GEN_FAIL               = 2,
    OPRS_FAILED_PASS            = 3,
    OPRS_FABRIC_RESET           = 4,
    OPRS_USBK_UNPLUGING         = 5,
    OPRS_INVALID_KEYNO          = 6,
    OPRS_INVALID_KEYSIZE        = 7,
    OPRS_INVALID_DEVICELABEL    = 8,
    OPRS_INVALID_PASS           = 9,
    OPRS_INVALID_NEWPASS        = 10,

}e_UIP_OPRSTATUS;


typedef union ATTR_PACKED_BEGIN __UIP_OPRSTATUS ATTR_PACKED_END
{
#define UIP_OPRSTATUS_TOTALSIZE     4
    e_UIP_OPRSTATUS     me;
    uint8_t             all[UIP_OPRSTATUS_TOTALSIZE];
}t_UIP_OPRSTATUS;

typedef enum ATTR_ENUM_PACKED_BEGIN __E_UIP_OPRSTATUS_MSG ATTR_ENUM_PACKED_END // must be 1 byte (int)
{
    OPMSG_DISABLE               = 1,
    //OPMSG_INVALID_KEYNO       = 2,    //move to opr status
    OPMSG_ACTIVE                = 3,
    OPMSG_DEACTIVE              = 4,
    //OPMSG_INVALID_KEYSIZE     = 5,    //move to opr status
    //OPMSG_INVALID_DEVICELABEL = 6,    //move to opr status
    //OPMSG_INVALID_PASS        = 7,    //move to opr status
    //OPMSG_INVALID_NEWPASS     = 8,    //move to opr status
}e_UIP_OPRSTATUS_MSG;

typedef union ATTR_PACKED_BEGIN __UIP_OPRSTATUS_MSG ATTR_PACKED_END
{
#define UIP_OPRSTATUS_MSG_TOTALSIZE     4
    e_UIP_OPRSTATUS_MSG     me;
    uint8_t                 all[UIP_OPRSTATUS_MSG_TOTALSIZE];
}t_UIP_OPRSTATUS_MSG;


typedef struct ATTR_PACKED_BEGIN __UIP_HEADER ATTR_PACKED_END
{
#define UIP_HEADER_SIZE     15
    char s[UIP_HEADER_SIZE+1];
}t_UIP_HEADER;


#define SETKEY_NAMEONLY     0x00    // 0b00000000
#define SETKEY_NAME_AND_KEY 0x01    // 0b00000001

typedef union ATTR_PACKED_BEGIN _UIP_KEYOPTION ATTR_PACKED_END
{
#define UIP_KEYOPTION_TOTALSIZE     4
    uint8_t me;
    uint8_t all[UIP_KEYOPTION_TOTALSIZE];
}t_UIP_KEYOPTION;

typedef enum ATTR_ENUM_PACKED_BEGIN __E_UIP_KEYSIZE ATTR_ENUM_PACKED_END //must be 1 byte
{
    KEYSIZE_NONE = 0,
    KEYSIZE_128  = 1,
    KEYSIZE_192  = 2,
    KEYSIZE_256  = 3,
}e_UIP_KEYSIZE;


typedef union ATTR_PACKED_BEGIN __UIP_KEYSIZE ATTR_PACKED_END
{
#define UIP_KEYSIZE_TOTALSIZE       4
    e_UIP_KEYSIZE   me;
    uint8_t         all[UIP_KEYSIZE_TOTALSIZE];
}t_UIP_KEYSIZE;



// ------------------------------------------------------------------------------------
// USER INTERFACE PROTOCOL
// ------------------------------------------------------------------------------------


// VENDOR SPECIFIC COMMAND FOR USBK
// ------------------------------------------------------------------------------------
#define SBC_CMD_VENDOR_SPECIFIC_10      0xFE    // Vendor Specific Command For Tamara USB.K Crypto Tool

// Vendor Specific Command for Tamara USB.K Crypto Tool
// 0xFE (vendor specfic) | 0x00 | command | ....
#define SCSI_USBK_GET_STATUS        0x01
#define SCSI_USBK_GET_DEV_INFO      0x02
#define SCSI_USBK_ACTIVATEKEY       0x03
#define SCSI_USBK_DEACTIVATEKEY     0x04
#define SCSI_USBK_CHANGE_PASS       0x05
#define SCSI_USBK_SET_DEVICENAME    0x06
#define SCSI_USBK_SET_AUTOACTIVATE  0x07
#define SCSI_USBK_SET_KEY           0x08
#define SCSI_USBK_GENERATE_256KEY   0x09

#define ALL_SCSI_USBK_CMD_LEN   512     //0x0200


// STRUCTURE OF DATA PHASE FOR VENDOR SPECIFIC COMMAND
// ------------------------------------------------------------------------------------

typedef struct ATTR_PACKED_BEGIN __UIP_GETSTATUS ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_OPRSTATUS     lastop;                     // 1 byte, type = enum[t_UIP_OPRSTATUS] + reserved
    t_UIP_OPRSTATUS_MSG lastop_msg;                 // 1 byte, type = enum[t_UIP_OPRSTATUS_MSG] + reserved
    t_UIP_NBRETRY       retry_num;                  // 1 byte binary [NB_RETRY:0]
}t_UIP_GETSTATUS;

typedef struct ATTR_PACKED_BEGIN __UIP_DEVINFO ATTR_PACKED_END
{
    t_UIP_HEADER        header;                     // 15 byte string + null
    t_UIP_FIRMVER       firmware_ver;               // 15 byte string + null
    t_UIP_PRODUCT       product;                    // 31 byte string + null
    t_UIP_MODEL         model;                      // 15 byte string + null
    t_UIP_KEYNB         multikeycap;                // 1 byte binary [NB_AESKEY]
    t_UIP_SERIAL        serial;                     // 15 byte binary + reserved
    t_UIP_DEVSTATE      devstate;                   // 1 byte, type = enum[t_UIP_DEV_STATE] + reserved
    t_UIP_KEYNB         current_keyno;              // 1 byte binary [1:NB_AESKEY]
    t_UIP_NBRETRY       retry_num;                  // 1 byte binary [NB_RETRY:0]
    t_UIP_DEVLABEL      devlabel;                   // 11 byte string + null + reserved
    t_UIP_KEYNB         autoactivate_keyno;         // 1 byte binary [0:NB_AESKEY] (0:disable)
    t_UIP_KEYNAME       keyname[NB_AESKEY];         // (12 byte string + null + reserved) x NB_AESKEY
}t_UIP_DEVINFO;

typedef struct ATTR_PACKED_BEGIN __UIP_ACTIVATE ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary [1:NB_AESKEY]
}t_UIP_ACTIVATE;

typedef struct ATTR_PACKED_BEGIN __UIP_DEACTIVATE ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
}t_UIP_DEACTIVATE;

typedef struct ATTR_PACKED_BEGIN __UIP_SETDEVICELABEL ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_DEVLABEL      devlabel;                   // 11 byte string + null + reserved
}t_UIP_SETDEVICELABEL;

typedef struct ATTR_PACKED_BEGIN __UIP_CHPASS ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        old_password;               // 16 byte string + null + reserved
    t_UIP_PAROLA        new_password;               // 16 byte string + null + reserved
}t_UIP_CHPASS;

typedef struct ATTR_PACKED_BEGIN __UIP_SETAUTOACTIVATE ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary [0:NB_AESKEY]
}t_UIP_SETAUTOACTIVATE;

typedef struct ATTR_PACKED_BEGIN __UIP_SETKEY ATTR_PACKED_END
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary
    t_UIP_KEYOPTION     options;                    // 1 byte, |x|x|x|x|x|x|x|key|
    t_UIP_KEYNAME       keyname;                    // 12 byte string + null
    t_UIP_KEYSIZE       keysize;                    // 1 byte, t_UIP_KEYSIZE + reserved
    t_UIP_KEY           key;                        // 32 byte binary
}t_UIP_SETKEY;


typedef struct ATTR_PACKED_BEGIN __UIP_GENERATEKEY ATTR_PACKED_END
{
    t_UIP_KEY           key;                        // 32 byte binary
}t_UIP_GENERATEKEY;

#endif /* UIP_H_ */
