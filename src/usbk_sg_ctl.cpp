#define _USBK_SG_CTL_H
#include "usbk_sg_ctl.h"

int sg_fd;

int usbk_open(char* DevicePath)
{

    sg_fd  = open(DevicePath , O_RDWR);// | O_EXCL)

    if (sg_fd < 0)
    {
        fprintf(stderr,NOTDEVICE);
        return 1;
    }
    return 0;
}

void usbk_close(void)
{
	close(sg_fd);
}

int usbk_sg_tansfer(st_packet *scsi_packet)
{
    sg_io_hdr_t  io_hdr;
    int i;
    unsigned char  sense_buffer[32];

    memset(sense_buffer, 0, sizeof(sense_buffer));
    memset(&io_hdr, 0, sizeof(sg_io_hdr_t));

    io_hdr.interface_id = 'S';

    if (scsi_packet->cmddir == 0x01)
    {
        io_hdr.dxfer_direction =  SG_DXFER_FROM_DEV ; //packet.cmddir;
    }
    else if (scsi_packet->cmddir == 0x00)
    {
        io_hdr.dxfer_direction =  SG_DXFER_TO_DEV ; //packet.cmddir;
    }

    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.sbp = sense_buffer;
    io_hdr.dxfer_len = scsi_packet->datalen;
    io_hdr.dxferp = scsi_packet->data;
    io_hdr.cmd_len = scsi_packet->cmdlen;
    io_hdr.cmdp = (unsigned char*) (scsi_packet->cmd);
    io_hdr.timeout = 1000;

//    printf("Sent CMD: ");
//    for (i = 0; i < io_hdr.cmd_len; i++)
//    {
//        printf("0x%02X ", io_hdr.cmdp[i]);
//    }
//    printf("\n");


    if (ioctl(sg_fd, SG_IO, &io_hdr) < 0)
    {
        fprintf(stderr,MSGERRIOCTL);
        return 1;
    }
    else if ((io_hdr.info & SG_INFO_OK_MASK) != SG_INFO_OK)
    {
        fprintf(stderr, CMDFAIL);
        if (io_hdr.sb_len_wr > 0)
        {
        	i=0;
        	printf(MSGSENSEDATA);
           while(i < io_hdr.sb_len_wr)
           {
               printf("0x%02X ", sense_buffer[i]);
               i++;
               if(i%16 == 0)
               {
                   int j=0;
                   for(j=0;j<16;j++)
                   {
                       printf("%c", sense_buffer[i-16+j]);
                   }
                   printf("\n");
               }
               else if(i%8 == 0) printf("- ");
           }
           int j ;
           for (j=0;j<i%16;j++)
           {
               printf("%c", sense_buffer[i-i%16+j]);
           }
           printf("\n");
        }
        scsi_packet->datalen = 0;
        return 1;
    }

	//printf(MSGRETBYTE, io_hdr.dxfer_len);
	scsi_packet->datalen = io_hdr.dxfer_len;
	return 0;
}

void usbk_sg_show_packet(st_packet *scsi_packet)
{
     if(scsi_packet->datalen)
        {
            printf("Received/Sent data: \n");

            int i=0;
            while(i < scsi_packet->datalen)
            {
                printf("0x%02X ", scsi_packet->data[i]);
                i++;

                if(i%16 == 0)
                {
                    int j=0;
                    for(j=0;j<16;j++)
                    //while(j<16)
                    {
                        printf("%c", scsi_packet->data[i-16+j]);
                      //  j++;
                    }
                    printf("\n");
                }
                else if(i%8 == 0) printf("- ");
            }
            int j ;
            for (j=0;j<i%16;j++)
            {
                printf("%c", scsi_packet->data[i-i%16+j]);
            }
            printf("\n\n");
        }
}


