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



#include <stdint.h>

#define INDIR   0X01
#define OUTDIR  0X00

#define MAX_SENSE_LEN 18 //Sense data max length

//SCSI return status codes.
#define STATUS_GOOD     0x00  // Status Good
#define STATUS_CHKCOND  0x02  // Check Condition
#define STATUS_CONDMET  0x04  // Condition Met
#define STATUS_BUSY     0x08  // Busy
#define STATUS_INTERM   0x10  // Intermediate
#define STATUS_INTCDMET 0x14  // Intermediate-condition met
#define STATUS_RESCONF  0x18  // Reservation conflict
#define STATUS_COMTERM  0x22  // Command Terminated
#define STATUS_QFULL    0x28  // Queue full
#define STATUS_ACA      0x30  // ACA active

typedef struct 
{
	uint8_t Length;
  uint8_t	ScsiStatus;
  uint8_t	PathId;
  uint8_t	TargetId;
  uint8_t	Lun;
  uint8_t	CdbLength;
  uint8_t	SenseInfoLength;
  uint8_t	DataIn;
  uint32_t	DataTransferLength;
  uint32_t	TimeOutValue;
  uint32_t	DataBufferOffset;
  uint32_t	SenseInfoOffset;
  uint8_t Cdb[16];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;



typedef struct
{
  uint16_t	Length;
  uint8_t	ScsiStatus;
  uint8_t	PathId;
  uint8_t	TargetId;
  uint8_t	Lun;
  uint8_t	CdbLength;
  uint8_t	SenseInfoLength;
  uint8_t	DataIn;
  uint32_t	DataTransferLength;
  uint32_t	TimeOutValue;
  uint8_t*	DataBuffer;
  uint32_t	SenseInfoOffset;
  uint8_t	Cdb[16];
} SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SCSI_PASS_THROUGH_DIRECT_AND_SENSE_BUFFER
{
    SCSI_PASS_THROUGH_DIRECT spt;
    uint8_t	SenseBuf[MAX_SENSE_LEN];
}T_SPDT_SBUF;

typedef struct __ST_PACKET_A {
    int  sg_fd;
    char cmdtype;
    int  cmddir;
    ST_CMD_T      *cmd;
    unsigned int   cmdlen;
//    unsigned char *data;
	uint8_t	*data;
    unsigned int   datalen;
} ST_PACKET_T_A;

typedef struct 
{
  SCSI_PASS_THROUGH spt;
  uint32_t	Filler;
  uint8_t	ucSenseBuf[32];
  uint8_t	ucDataBuf[512];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

typedef struct 
{
  SCSI_PASS_THROUGH_DIRECT spt;
  uint32_t	Filler;
  uint8_t	ucSenseBuf[32];
} SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

typedef struct 
{
  uint8_t	NumberOfLogicalUnits;
  uint8_t	InitiatorBusId;
  uint32_t	InquiryDataOffset;
} SCSI_BUS_DATA, *PSCSI_BUS_DATA;

typedef struct 
{
  uint8_t	NumberOfBusses;
  SCSI_BUS_DATA BusData[1];
} SCSI_ADAPTER_BUS_INFO, *PSCSI_ADAPTER_BUS_INFO;

typedef struct 
{
  uint8_t	PathId;
  uint8_t	TargetId;
  uint8_t	Lun;
  uint8_t	DeviceClaimed;
  uint32_t	InquiryDataLength;
  uint32_t	NextInquiryDataOffset;
  uint8_t	InquiryData[1];
} SCSI_INQUIRY_DATA, *PSCSI_INQUIRY_DATA;

typedef struct 
{
  uint32_t	Length;
  uint8_t	PortNumber;
  uint8_t	PathId;
  uint8_t	TargetId;
  uint8_t	Lun;
} SCSI_ADDRESS, *PSCSI_ADDRESS;

//method codes
#define  METHOD_BUFFERED     0
#define  METHOD_IN_DIRECT    1
#define  METHOD_OUT_DIRECT   2
#define  METHOD_NEITHER      3

#define IOCTL_SCSI_BASE    0x00000004

//constants for DataIn member of SCSI_PASS_THROUGH* structures
#define  SCSI_IOCTL_DATA_OUT          0
#define  SCSI_IOCTL_DATA_IN           1
#define  SCSI_IOCTL_DATA_UNSPECIFIED  2

//Standard IOCTL define
#define IOCTL_SCSI_PASS_THROUGH         CTL_CODE( IOCTL_SCSI_BASE, 0x0401, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_MINIPORT             CTL_CODE( IOCTL_SCSI_BASE, 0x0402, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_INQUIRY_DATA     CTL_CODE( IOCTL_SCSI_BASE, 0x0403, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_GET_CAPABILITIES     CTL_CODE( IOCTL_SCSI_BASE, 0x0404, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SCSI_PASS_THROUGH_DIRECT  CTL_CODE( IOCTL_SCSI_BASE, 0x0405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS )
#define IOCTL_SCSI_GET_ADDRESS          CTL_CODE( IOCTL_SCSI_BASE, 0x0406, METHOD_BUFFERED, FILE_ANY_ACCESS )




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

