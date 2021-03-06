/*
 * @file usbk_scsi.h
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
 *
 */

#ifndef USBK_SCSI_H_
#define USBK_SCSI_H_

#ifdef __cplusplus
extern "C" {
#endif

//PUBLIC DEFINES
//-DIRECTIONS
#define INDIR             0X01
#define OUTDIR            0X00

//-SCSI CMD DIRECTION
#define WRITE_SCSI           OUTDIR
#define READ_SCSI            INDIR

//!\name VENDOR SPECIFIC SCSI COMMAND FOR USBK
//! @{
#define GET_STATUS           0x01      //!< Get Status of Current Command
#define GET_DEV_INFO         0x02      //!< Get Detail Information about USBK
#define ACTIVATE_KEY         0x03      //!< Activate USBK with Selected Key
#define DEACTIVATE_KEY       0x04      //!< Deactivate USBK
#define CHANGE_PASS          0x05      //!< Change Password
#define SET_DEV_NAME         0x06      //!< Set Device Label
#define SET_AUTO_ACTIVE      0x07      //!< Change AutoActivation Setting (Enable with Selected Key / Disable)
#define SET_KEY              0x08      //!< Set Key
#define GENERATE_KEY         0x09      //!< Generate 256-Bit Random Key
//! @}


//!\name -RETURN VALUES OF SCSI LAYER
//! @{
enum usbk_scsi_ret {
    USBK_SCSI_PASS          =  0,      //!< Pass
    USBK_SCSI_OPEN_FAIL     = -1,      //!< Device Open Error
    USBK_SCSI_TRANSFER_FAIL = -2,      //!< Transfer Error
};
//! @}


//PUBLIC FUNCTIONS DECLERATIONS
/**
 * usbk_scsi. h
 * \brief Send Scsi Command
 * <pre>int send_scsi_command(USBK* abcde, unsigned char *buff, int cmd_index, int len, char rw);</pre>
 *
 * Send Scsi Command to USBK.
 *
 * @param *abcde         : abcde structure pointer
 * @param *buff         : Transfer buffer
 * @param cmd_index     : Scsi Command OpCode
 * @param len           : Transfer Len
 * @param rw            : Direction of Read/Write
 *
 * \return \ref rtnUSBK_SCSI_PASS or \ref rtnUSBK_SCSI_OPEN_FAIL or \ref rtnUSBK_SCSI_TRANSFER_FAIL
 *
 * \page send_scsi_command send_scsi_command
 * \ingroup Scsi
 *
 */
int send_scsi_command(const char *usbk_path, unsigned char *buff, int cmd_index, int len, char rw);

#ifdef __cplusplus
}
#endif

#endif

