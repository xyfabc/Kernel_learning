#ifndef __CIM_HEADER_H__
#define __CIM_HEADER_H__
/*
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <linux/i2c.h>
*/
#include "camera.h"
#include "format.h"


typedef unsigned int	__u32;
typedef signed int	__s32;
typedef unsigned short	__u16;
typedef signed short	__s16;
typedef unsigned char	__u8;
typedef signed char	__s8;

#undef cim_dbg
#define cim_dbg(x...)				\
	({					\
	 fprintf(stdout, x);	\
	})
/* #define Debug */
#ifdef Debug
#define _DEBUG  1
#else
#define _DEBUG  0
#endif
#define debug_cond(cond, fmt, args...)      \
	do {                    \
		if (cond)           \
		printf(fmt, ##args);    \
	} while (0)

#define debug(fmt, args...)         \
	debug_cond(_DEBUG, fmt, ##args)

// function declaration


/* video */
extern int open_device(const char *dev_name);

extern void init_device(struct camera_info *camera_inf, struct camera_ctl *camera_ctl);
extern void uninit_device(struct camera_ctl *camera_ctl, struct camera_info *camera_inf);

extern unsigned int set_format(struct camera_info *camera_inf);
extern int process_framebuf(struct camera_info *camera_inf, struct camera_ctl *camera_ctl);

extern void start_capturing(struct camera_ctl *camera_ctl);
extern void stop_capturing(struct camera_ctl *camera_ctl);
extern void get_one_img(struct camera_info *camera_inf, struct camera_ctl *camera_ctl);



#endif // __CIM_HEADER_H__
