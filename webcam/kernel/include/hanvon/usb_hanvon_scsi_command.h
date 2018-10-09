#ifndef __USB_HANVON_SCSI_COMMAND__
#define __USB_HANVON_SCSI_COMMAND__

/* the write function for the usb file_storage */
extern int hanvon_usbd_write(void *buf, size_t count);

/* the read function for the usb file_storage */
extern int hanvon_usbd_read(void *buf, size_t count);

extern void hanvon_usbd_ioctl(void);

#endif	/* __USB_HANVON_SCSI_COMMAND__ */
