#ifndef __CAMERA__

typedef unsigned int	__u32;
typedef signed int	__s32;
typedef unsigned short	__u16;
typedef signed short	__s16;
typedef unsigned char	__u8;
typedef signed char	__s8;


#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>
//

#include "../drivers/usb/gadget/uvc.h"


struct uvc_device
{
	int fd;

	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;

	int control;

	unsigned int fcc;
	unsigned int width;
	unsigned int height;

	void **mem;
	unsigned int nbufs;
	unsigned int bufsize;

	unsigned int bulk;
	unsigned char color;
	unsigned int imgsize;
	void *imgdata;
};




struct camera_param{
	__u32 tlb	: 1,
	      debug	: 1,
	      format_spc: 1,
	      save_jpeg_userptr :1,
	      planar	: 1,
	      use_vpu	: 1,
	      lcd_nbuf  : 2,
	      yuv422_format  : 2,
	      reserved	: 21;


	__s32 cim_id;
	__u32 width;
	__u32 height;
	__u32 bpp;

};

struct camera_format {
	__u32 fourcc;	// compatible with V4L2
	__u32 format;	// local definition, more detail
};

struct camera_info{
	__s32 fd_cim;
	__s32 capture_limit;
	__s32 squeue;

	__u8  file_name[64];
	__u8  video_dev_name[16];

	struct camera_param param;
	struct camera_format fmt;

	__u32 yoff;
	__u32 xoff;

	__u8 *ybuf;
	__u8 *ubuf;
	__u8 *vbuf;
	__u32 n_bytes;

	void *jz_jpeg;
	void *buf_vaddr;
	/***********for uvc*************/
	struct uvc_device *uvc_dev;
	struct v4l2_buffer *uvc_cur_buf;
};

enum io_method {
	IO_METHOD_READ = 0,
	IO_METHOD_MMAP = 1,
	IO_METHOD_USERPTR = 2,
};

enum camera_options {
	OPS_PREVIEW = 0,
	OPS_CAPTURE = 1,
};

enum capture_options {
	OPS_SAVE_BMP = 0,
	OPS_SAVE_JPG = 1,
	OPS_SAVE_RAW = 2,
};

enum yuv422_format{
	F_YUYV,
	F_YVYU,
	F_UYVY,
	F_VYUY,
};


struct cim_ops {
	void (*capture_picture)(struct camera_info *camera_info, struct camera_ctl *camera_ctl);
	int (*priview_picture)(struct camera_info *camera_info);
	void (*camera_fps)(struct camera_info *camera_inf);
};

struct camera_ctl{
	/*v4l2 support  mode*/
	enum io_method io_method;
	/*camera optiongs*/
	enum camera_options cam_opt;
	/*capture optiong*/
	enum capture_options cap_opt;

	struct cim_ops ops;
};



#define	__CAMERA__
#endif/*END OF __CAMERA__*/





























