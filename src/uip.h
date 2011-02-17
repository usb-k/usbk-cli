/*! \file *********************************************************************
 *
 * \brief UIP.h - Created on: Feb 7, 2011
 *
 * This file includes...
 *
 * - Compiler:          GNU GCC for AVR32
 * - Supported devices: DEMM and EVK1104
 * - AppNote:           USBK_UI_ Protocol_2v3r0
 *
 * \author              Timucin Anuslu
 *                      Tamara Elektronik : http://www.tamara.com.tr \n
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
//              In C# for windows,
//                      typedef enum : char{
//                          OPMSG_DISABLE           = 0,
//                      }t_UIP_OPRSTATUS_MSG;
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PROTOCOL_HEADER "USBK"

#ifdef PACKAGED_STRUCT
#  define ATTR_PACKED __attribute__((__packed__))
#  define ATTR_ENUM_PACKED __attribute__((__packed__))
#else
#  define ATTR_PACKED
#  define ATTR_ENUM_PACKED __attribute__((__packed__))
#endif


typedef U32 t_UIP_TAGNUMBER;

typedef struct ATTR_PACKED __UIP_FIRMVER
{
#define UIP_FIRMWAREVIR_SIZE        15
    char                s[UIP_FIRMWAREVIR_SIZE+1];
}t_UIP_FIRMVER;

typedef union ATTR_PACKED __UIP_SERIAL
{
#define UIP_SERIAL_TOTALSIZE        32
#define UIP_SERIAL_SIZE             15
    U8 u8[UIP_SERIAL_SIZE];
    U8 all[UIP_SERIAL_TOTALSIZE];
}t_UIP_SERIAL;

typedef U8 t_UIP_FABDEF;
typedef U8 t_UIP_LOGSTAT;
typedef U8 t_UIP_BACKDSTAT;

typedef struct ATTR_PACKED __UIP_MODEL
{
#define UIP_MODEL_SIZE          15
    char s[UIP_MODEL_SIZE+1];
}t_UIP_MODEL;


typedef union ATTR_PACKED __UIP_DEVLABEL
{
#define UIP_DEVLABEL_TOTALSIZE      65
#define UIP_DEVLABEL_SIZE           11
    char s[UIP_DEVLABEL_SIZE+1];
    U8 all[UIP_DEVLABEL_TOTALSIZE];
}t_UIP_DEVLABEL;

typedef enum ATTR_ENUM_PACKED __E_UIP_DEVSTATE // must be 1 byte
{
    ACTIVATE                = 1,
    ACTIVATE_WITH_BACKDISK  = 2,
    DEACTIVATE              = 3,
    FABRIC_DEFAULT          = 4,
    MUST_REMOVE             = 5,
}e_UIP_DEVSTATE;

typedef union ATTR_PACKED __UIP_DEVSTATE
{
#define UIP_DEVSTATE_TOTALSIZE  4
    e_UIP_DEVSTATE  me;
    U8              all[UIP_DEVSTATE_TOTALSIZE];
}t_UIP_DEVSTATE;


typedef U8 t_UIP_KEYNB;
typedef U8 t_UIP_NBRETRY;

typedef union ATTR_PACKED __UIP_KEY
{
#define KEY_LEN     32
      U32 u32[KEY_LEN/4];
      U16 u16[KEY_LEN/2];
      U8  u8 [KEY_LEN];
}t_UIP_KEY;

typedef union ATTR_PACKED __UIP_KEYNAME
{
#define UIP_KEYNAME_TOTALSIZE       65
#define UIP_KEYNAME_SIZE            12
    char s[UIP_KEYNAME_SIZE+1];
    U8 all [UIP_KEYNAME_TOTALSIZE];
}t_UIP_KEYNAME;

typedef union ATTR_PACKED __UIP_PAROLA
{
#define UIP_PAROLA_TOTALSIZE        65
#define UIP_PAROLA_SIZE             17
      char  s[UIP_PAROLA_SIZE];
      U8    all[UIP_PAROLA_TOTALSIZE];
} t_UIP_PAROLA;

typedef U8 t_UIP_OPTION;

typedef enum ATTR_ENUM_PACKED __E_UIP_OPRSTATUS     // must be 1 byte
{
    OPRS_PASS           = 1,
    OPRS_GEN_FAIL       = 2,
    OPRS_INVALID_PASS   = 3,
    OPRS_FABRIC_RESET   = 4,
    OPRS_USBK_UNPLUGING = 5
}e_UIP_OPRSTATUS;


typedef union ATTR_PACKED __UIP_OPRSTATUS
{
#define UIP_OPRSTATUS_TOTALSIZE     4
    e_UIP_OPRSTATUS     me;
    U8                  all[UIP_OPRSTATUS_TOTALSIZE];
}t_UIP_OPRSTATUS;

typedef enum ATTR_ENUM_PACKED __E_UIP_OPRSTATUS_MSG // must be 1 byte (int)
{
    OPMSG_DISABLE           = 1,
    OPMSG_INVALID_KEYNO     = 2,
    OPMSG_ACTIVE            = 3,
    OPMSG_DEACTIVE          = 4,

}e_UIP_OPRSTATUS_MSG;

typedef union ATTR_PACKED __UIP_OPRSTATUS_MSG
{
#define UIP_OPRSTATUS_MSG_TOTALSIZE     4
    e_UIP_OPRSTATUS_MSG     me;
    U8                      all[UIP_OPRSTATUS_MSG_TOTALSIZE];
}t_UIP_OPRSTATUS_MSG;


typedef struct ATTR_PACKED __UIP_HEADER
{
#define UIP_HEADER_SIZE     15
    char s[UIP_HEADER_SIZE+1];
}t_UIP_HEADER;


#define SETKEY_NAMEONLY     0x00    // 0b00000000
#define SETKEY_NAME_AND_KEY 0x01    // 0b00000001

#define SETKEY_MASK_OFFSET_KEY          0

typedef union ATTR_PACKED _UIP_KEYOPTION
{
#define UIP_KEYOPTION_TOTALSIZE     4
    U8 me;
    U8 all[UIP_KEYOPTION_TOTALSIZE];
}t_UIP_KEYOPTION;

typedef enum ATTR_PACKED __E_UIP_KEYSIZE
{
    KEYSIZE_128 = 1,
    KEYSIZE_192 = 2,
    KEYSIZE_256 = 3,
}e_UIP_KEYSIZE;


typedef union ATTR_PACKED __UIP_KEYSIZE
{
#define UIP_KEYSIZE_TOTALSIZE       4
    e_UIP_KEYSIZE me;
    U8 all[UIP_KEYSIZE_TOTALSIZE];
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

typedef struct ATTR_PACKED __UIP_GETSTATUS
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_OPRSTATUS     lastop;                     // 1 byte, type = enum[t_UIP_OPRSTATUS] + reserved
    t_UIP_OPRSTATUS_MSG lastop_msg;                 // 1 byte, type = enum[t_UIP_OPRSTATUS_MSG] + reserved
    t_UIP_NBRETRY       retry_num;                  // 1 byte binary [NB_RETRY:0]
}t_UIP_GETSTATUS;

typedef struct ATTR_PACKED __UIP_DEVINFO
{
    t_UIP_HEADER        header;                     // 15 byte string + null
    t_UIP_FIRMVER       firmware_ver;               // 15 byte string + null
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

typedef struct ATTR_PACKED __UIP_ACTIVATE
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary [1:NB_AESKEY]
}t_UIP_ACTIVATE;

typedef struct ATTR_PACKED __UIP_DEACTIVATE
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
}t_UIP_DEACTIVATE;

typedef struct ATTR_PACKED __UIP_SETDEVICELABEL
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_DEVLABEL      devlabel;                   // 11 byte string + null + reserved
}t_UIP_SETDEVICELABEL;

typedef struct ATTR_PACKED __UIP_CHPASS
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        old_password;               // 16 byte string + null + reserved
    t_UIP_PAROLA        new_password;               // 16 byte string + null + reserved
}t_UIP_CHPASS;

typedef struct ATTR_PACKED __UIP_SETAUTOACTIVATE
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary [0:NB_AESKEY]
}t_UIP_SETAUTOACTIVATE;

typedef struct ATTR_PACKED __UIP_SETKEY
{
    t_UIP_TAGNUMBER     tag;                        // 4 byte binary
    t_UIP_PAROLA        password;                   // 16 byte string + null + reserved
    t_UIP_KEYNB         keyno;                      // 1 byte binary
    t_UIP_KEYOPTION     options;                    // 1 byte, |x|x|x|x|x|x|x|key|
    t_UIP_KEYNAME       keyname;                    // 12 byte string + null
    t_UIP_KEYSIZE       keysize;                    // 1 byte, t_UIP_KEYSIZE + reserved
    t_UIP_KEY           key;                        // 32 byte binary
}t_UIP_SETKEY;


typedef struct ATTR_PACKED __UIP_GENERATEKEY
{
    t_UIP_KEY           key;                        // 32 byte binary
}t_UIP_GENERATEKEY;



#endif /* UIP_H_ */
