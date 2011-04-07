/*
 * @file usbk_sg_ctl.h
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

#ifndef USBK_SG_CTL_H_
#define USBK_SG_CTL_H_

//PUBLIC DEFINES
//*! \name -RETURN VALUES OF SCSI GENERIC LAYER
//! @{
#define rtnUSBK_SG_PASS                  0      //!< Pass
#define rtnUSBK_SG_ERR_GENERAL          -1      //!< General Error
#define rtnUSBK_SG_ERR_NOT_DEVICE       -2      //!< Device can not be found
#define rtnUSBK_SG_ERR_IOCTL_SCSI       -3      //!< IOCTL Error
#define rtnUSBK_SG_ERR_COMMAND_FAIL     -4      //!< SCSI Command Fail
//! @}

//PUBLIC STRUCTURES
typedef struct __ST_CMD {
    char opcode;
    char lun;
    char v_opcode;
    char dummy1;
    char dummy2;
    char dummy3;
    char reserved;
    char len1;
    char len0;
    char cntrl;
} ST_CMD_T;

typedef struct __ST_PACKET {
    int  sg_fd;
    char cmdtype;
    int  cmddir;
    ST_CMD_T      *cmd;
    unsigned int   cmdlen;
    unsigned char *data;
    unsigned int   datalen;
} ST_PACKET_T;


// PUBLIC FUNCTION DECLERATIONS
/**
 * usbk_sg_ctl. h
 * \brief Open Devices
 * <pre>int usbk_sg_open(char* DevicePath);</pre>
 *
 * Open USBK-Host System Channel over USB&SCSI
 *
 * @param DevicePath : USBK device node path.
 *
 * \return \ref rtnUSBK_SG_PASS or \ref rtnUSBK_SG_ERR_NOT_DEVICE
 *
 * \page usbk_open usbk_open
 * \ingroup ScsiGeneric
 *
 */
int usbk_sg_open(const char* DevicePath);


/**
 * usbk_sg_ctl. h
 * \brief Close Device
 * <pre>void usbk_sg_close(char* DevicePath);</pre>
 *
 * Close USBK-Host System Channel over USB&SCSI
 *
 * \page usbk_close usbk_close
 * \ingroup ScsiGeneric
 *
 */
void usbk_sg_close(void);

/**
 * usbk_sg_ctl. h
 * \brief Data Transfer over SCSI
 * <pre>int usbk_sg_tansfer(ST_PACKET_T *scsi_packet);</pre>
 *
 * Transfer packets over USBK-Host System Channel
 *
 * @param ST_PACKET_T *scsi_packet : Packet Structure.
 *
 * \return \ref rtnUSBK_SG_ERR_IOCTL_SCSI or \ref rtnUSBK_SG_ERR_COMMAND_FAIL
 *
 * \page usbk_sg_tansfer usbk_sg_tansfer
 * \ingroup ScsiGeneric
 *
 */
int usbk_sg_tansfer(ST_PACKET_T *scsi_packet);

#endif // USBK_SG_CTL_H_

