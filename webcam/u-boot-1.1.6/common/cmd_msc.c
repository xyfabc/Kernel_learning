/*
 * Driver for NAND support, Rick Bronson
 * borrowed heavily from:
 * (c) 1999 Machine Vision Holdings, Inc.
 * (c) 1999, 2000 David Woodhouse <dwmw2@infradead.org>
 *
 * Added 16-bit nand support
 * (C) 2004 Texas Instruments
 */

#include <common.h>

#include <asm/io.h>
#include <command.h>

#include <asm/jzsoc.h>

#define BUS_WIDTH      2
#define AUTO_CMD12     1

#if defined(AUTO_CMD12)
#define AUTO_CMD23    0
#else
#define AUTO_CMD23    1
#endif

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
static int rca;
static int highcap = 0;

/*
 * GPIO definition
 */
#define MMC_IRQ_MASK()				\
do {						\
	REG_MSC_IMASK = 0xffffffff;			\
	REG_MSC_IREG = 0xffffffff;			\
} while (0)

/* Stop the MMC clock and wait while it happens */
static inline int jz_mmc_stop_clock(void)
{
	return 0;
}

/* Start the MMC clock and operation */
static inline int jz_mmc_start_clock(void)
{
	REG_MSC_STRPCL = MSC_STRPCL_START_OP;
	return 0;
}

static u8 * mmc_cmd(u16 cmd, unsigned int arg, unsigned int cmdat, u16 rtype)
{
	static u8 resp[20];
	u32 timeout = 0x3fffff;
	int words, i;

	jz_mmc_stop_clock();
	REG_MSC_CMD   = cmd;
	REG_MSC_ARG   = arg;
	REG_MSC_CMDAT = cmdat;

	REG_MSC_IMASK = ~MSC_IMASK_END_CMD_RES;
	jz_mmc_start_clock();

	/* printf("cmd:%d, cmdat:%x\n",REG_MSC_CMD, REG_MSC_CMDAT); */
	while (timeout-- && !(REG_MSC_STAT & MSC_STAT_END_CMD_RES))
		;
	REG_MSC_IREG = MSC_IREG_END_CMD_RES;
	if(timeout == 0)
		printf("wait end cmd res timeout\n");
	switch (rtype) {
		case MSC_CMDAT_RESPONSE_R1:
		case MSC_CMDAT_RESPONSE_R3:
			words = 3;
			break;

		case MSC_CMDAT_RESPONSE_R2:
			words = 8;
			break;

		default:
			return 0;
	}
	
	for (i = words-1; i >= 0; i--) {
		u16 res_fifo = REG_MSC_RES;
		int offset = i << 1;

		resp[offset] = ((u8 *)&res_fifo)[0];
		resp[offset+1] = ((u8 *)&res_fifo)[1];
	}
	return resp;
}
static int mmc_block_writem(u32 src, u32 num, u8 *dst)
{
	u8 *resp;
	u32 stat, timeout, cnt, nob, sorm;
	u32 *wbuf = (u32 *)dst;

	resp = mmc_cmd(16, 0x200, 0x1, MSC_CMDAT_RESPONSE_R1);
	REG_MSC_BLKLEN = 0x200;
	REG_MSC_NOB = num / 512;
	nob  = num / 512;

	if (highcap)
		resp = mmc_cmd(25, src, 0x19 | (BUS_WIDTH << 9) | (AUTO_CMD23 << 18) | (AUTO_CMD12 << 16), MSC_CMDAT_RESPONSE_R1);
	else
		resp = mmc_cmd(25, src * 512, 0x19 | (BUS_WIDTH << 9) | (AUTO_CMD23 << 18) | (AUTO_CMD12 << 16), MSC_CMDAT_RESPONSE_R1);
#if 1
	for (nob; nob >= 1; nob--) {
		timeout = 0x3FFFFFF;
		while (timeout) {
			timeout--;
			stat = REG_MSC_STAT;
			if (stat & (MSC_STAT_CRC_WRITE_ERROR | MSC_STAT_CRC_WRITE_ERROR_NOSTS)) {
				serial_puts("\n MSC_STAT_CRC_WRITE_ERROR\n\n");
				return -1;
			}
			else if (!(stat & MSC_STAT_DATA_FIFO_FULL)) {
				/* Ready to write data */
				break;
			}

			udelay(1);
		}

		if (!timeout)
			return -1;

		/* Write data to TXFIFO */
		cnt = 128;
		while (cnt) {
			while (REG_MSC_STAT & MSC_STAT_DATA_FIFO_FULL)
				;
			REG_MSC_TXFIFO = *wbuf++;
			cnt--;
		}
	}
#endif
#if defined(AUTO_CMD12)
	while(!(REG_MSC_STAT & (1 << 31)))
		;
	REG_MSC_IREG = (1 << 15);
	serial_puts("\n MSC AUTO_CMD12 DONE!");
#endif

	while (!(REG_MSC_STAT & MSC_STAT_DATA_TRAN_DONE))
		;
	REG_MSC_IREG = MSC_IREG_DATA_TRAN_DONE;	
	serial_puts("\n MSC DATA TRANSFER DONE!");

	while (!(REG_MSC_STAT & MSC_STAT_PRG_DONE))
		;
	REG_MSC_IREG = (MSC_IREG_PRG_DONE );	
	serial_puts("\n MSC PROGRAM DONE!");

	jz_mmc_stop_clock();

	return 0;
}
static int mmc_block_readm(u32 src, u32 num, u8 *dst)
{
	u8 *resp;
	u32 stat, timeout, data, cnt, nob, sorm;

	resp = mmc_cmd(16, 0x200, 0x1, MSC_CMDAT_RESPONSE_R1);
	REG_MSC_BLKLEN = 0x200;
	REG_MSC_NOB = num / 512;
	nob  = num / 512;

	if (highcap)
		resp = mmc_cmd(18, src, 0x9 | (BUS_WIDTH << 9) | (AUTO_CMD23 << 18) | (AUTO_CMD12 << 16), MSC_CMDAT_RESPONSE_R1);
	else
		resp = mmc_cmd(18, src * 512, 0x9 | (BUS_WIDTH << 9) | (AUTO_CMD23 << 18) | (AUTO_CMD12 << 16), MSC_CMDAT_RESPONSE_R1);
	udelay(1000000);
	for (nob; nob >= 1; nob--) {
		timeout = 0x3ffffff;

		while (timeout) {
			timeout--;
			stat = REG_MSC_STAT;
			if (stat & MSC_STAT_TIME_OUT_READ) {
				serial_puts("\n MSC_STAT_TIME_OUT_READ\n\n");
				return -1;
			}
			else if (stat & MSC_STAT_CRC_READ_ERROR) {
				serial_puts("\n MSC_STAT_CRC_READ_ERROR\n\n");
				return -1;
			}
			else if (!(stat & MSC_STAT_DATA_FIFO_EMPTY)) {
				/* Ready to read data */
				break;
			}
			udelay(1);
		}
		if (!timeout) {
			serial_puts("\n mmc/sd read timeout\n");
			return -1;
		}

		/* Read data from RXFIFO. It could be FULL or PARTIAL FULL */
		cnt = 128;
		while (cnt) {
			while (cnt && (REG_MSC_STAT & MSC_STAT_DATA_FIFO_EMPTY))
				;
			cnt --;

			data = REG_MSC_RXFIFO;
			{

				*dst++ = (u8)(data >> 0);
				*dst++ = (u8)(data >> 8);
				*dst++ = (u8)(data >> 16);
				*dst++ = (u8)(data >> 24);

			}
		}
	}
	while (!(REG_MSC_STAT & MSC_STAT_DATA_TRAN_DONE))
		;
	
	REG_MSC_IREG = MSC_IREG_DATA_TRAN_DONE;	

#if defined(AUTO_CMD12)
	while(!(REG_MSC_STAT & (1 << 31)))
		;
	REG_MSC_IREG = (1 << 15);
#endif

	jz_mmc_stop_clock();
	return 0;
}

static void sd_init(void)
{
	int retries;
	u8 *resp;
	unsigned int cardaddr;
//	serial_puts("cmd_msc: SD init\n");
	resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
	retries = 500;
	while (retries-- && resp && !(resp[4] & 0x80)) {
		resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(41, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
		udelay(1000);
		udelay(1000);
	}

	if (resp[4] & 0x80) 
		;//serial_puts("SD init ok\n");
	else 
		serial_puts("SD init fail\n");

	/* try to get card id */
	resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
	resp = mmc_cmd(3, 0, 0x6, MSC_CMDAT_RESPONSE_R1);
	cardaddr = (resp[4] << 8) | resp[3]; 
	rca = cardaddr << 16;
	resp = mmc_cmd(9, rca, 0x2, MSC_CMDAT_RESPONSE_R2);
#if 0
	if (resp[1] & (1 << 5)) {
		        serial_puts("\nThis SD card  support WRITE_PARTIAL!\n");
	}else
		        serial_puts("\nThis SD card don't support WRITE_PARTIAL!\n");

	if (resp[8] & (1 << 7)) {
		        serial_puts("\nThis SD card  support READ_PARTIAL!\n");
	}else
		        serial_puts("\nThis SD card don't support READ_PARTIAL!\n");
#endif
	
	highcap = (resp[14] & 0xc0) >> 6;
//	printf("cmd_msc: highcap == %d \n",highcap);

#ifndef CONFIG_FPGA
	REG_MSC_CLKRT = 2;
#else
	REG_MSC_CLKRT = 4;
#endif
	resp = mmc_cmd(7, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
	if(BUS_WIDTH == 2){	
		resp = mmc_cmd(55, rca, 0x1, MSC_CMDAT_RESPONSE_R1);
		resp = mmc_cmd(6, 0x2, 0x401, MSC_CMDAT_RESPONSE_R1);
	}
}
/* init mmc/sd card we assume that the card is in the slot */
static int  mmc_init(void)
{
	int retries, msc_cdr;
	u8 *resp;

	__gpio_as_msc();

	__msc_reset();
	MMC_IRQ_MASK();	

//	msc_cdr = CPM_MSCCDR_MPCS_SRC | CPM_MSCCDR_CE;
/*
	msc_cdr = CPM_MSCCDR_MPCS_MPLL | CPM_MSCCDR_CE;
	msc_cdr |= __cpm_get_pllout2() % (25000000 / 2) ? __cpm_get_pllout2() / 25000000 / 2 : __cpm_get_pllout2() / 25000000 / 2 - 1;
	REG_CPM_MSCCDR = msc_cdr;
	while (REG_CPM_MSCCDR & CPM_MSCCDR_MSC_BUSY) ;
*/
	REG_MSC_CLKRT = 7;

	REG_MSC_RDTO = 0xffffffff;
	REG_MSC_RESTO = 0xffff;
	REG_MSC_LPM = 1;

	resp = mmc_cmd(12, 0, 0x41, MSC_CMDAT_RESPONSE_R1);

	/* reset */
	resp = mmc_cmd(0, 0, 0x80, 0);
	resp = mmc_cmd(8, 0x1aa, 0x1, MSC_CMDAT_RESPONSE_R1);
	resp = mmc_cmd(55, 0, 0x1, MSC_CMDAT_RESPONSE_R1);
	if(!(resp[0] & 0x20) && (resp[5] != 0x37)) { 
		resp = mmc_cmd(1, 0x40ff8000, 0x3, MSC_CMDAT_RESPONSE_R3);
		retries = 500;
		while (retries-- && resp && !(resp[4] & 0x80)) {
			resp = mmc_cmd(1, 0x40300000, 0x3, MSC_CMDAT_RESPONSE_R3);
			udelay(1000);
			udelay(1000);
		}

		if ((resp[4] & 0x80 ) == 0x80) 
			serial_puts("MMC init ok\n");
		else 
			serial_puts("MMC init fail\n");

		if((resp[4] & 0x60) == 0x40)
			highcap = 1;
		else
			highcap = 0;
		printf("cmd_msc: highcap == %d \n",highcap);
		/* try to get card id */
		resp = mmc_cmd(2, 0, 0x2, MSC_CMDAT_RESPONSE_R2);
		resp = mmc_cmd(3, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);

#ifndef CONFIG_FPGA
		REG_MSC_CLKRT = 2;
#else
		REG_MSC_CLKRT = 4;
#endif
		resp = mmc_cmd(7, 0x10, 0x1, MSC_CMDAT_RESPONSE_R1);
		if(BUS_WIDTH == 2){
			resp = mmc_cmd(6, 0x3b70101, 0x441, MSC_CMDAT_RESPONSE_R1);
			
			while(!(REG_MSC_STAT & MSC_STAT_PRG_DONE))
				;
			REG_MSC_IREG |= MSC_IREG_PRG_DONE;
		}
	}
	else
		sd_init();
	return 0;
}

/*
 * Load kernel image from MMC/SD into RAM
 */
int msc_read(ulong start_byte, u8 *dst, size_t len)
{
	int start_sect;

	start_sect = start_byte / 512;
/*
	serial_puts("\n===READ FROM START SECTOR: ");
	serial_put_hex(start_sect);
	serial_puts("\n===READ OUT TO ADDRESS: ");
	serial_put_hex(dst);
*/
	mmc_init();
	mmc_block_readm(start_sect, len, dst);

	return 0;
}

int msc_write(ulong start_byte, u8 *dst, size_t len)
{
	int start_sect;

	start_sect = start_byte / 512;
/*
	serial_puts("\n===WRITE TO START SECTOR: ");
	serial_put_hex(start_sect);
	serial_puts("\n===WRITE FROM ADDRESS: ");
	serial_put_hex(dst);
*/
	mmc_block_writem(start_sect, len, dst);
	return 0;
}

static inline int str2long(char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return (*p != '\0' && *endptr == '\0') ? 1 : 0;
}

int do_msc(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, ret=0;
	ulong addr, off, size;
	char *cmd;
	int quiet = 1;
	const char *quiet_str = getenv("quiet");
	char buf[512];

	int data_pin = 32*4 + 20;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	if (quiet_str)
		quiet = simple_strtoul(quiet_str, NULL, 0) != 0;

	cmd = argv[1];

	if (strncmp(cmd, "read", 4) == 0) {
		int read;

		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		off = (ulong)simple_strtoul(argv[3], NULL, 16);
		size = (ulong)simple_strtoul(argv[4], NULL, 16);

		read = strncmp(cmd, "read", 4) == 0; /* 1 = read, 0 = write */
		ret = msc_read(off, (u_char *)addr, size);
		printf(" %d bytes : %s\n", size, ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "write", 7) == 0) {

		if (argc < 4)
			goto usage;

		addr = (ulong)simple_strtoul(argv[2], NULL, 16);
		off = (ulong)simple_strtoul(argv[3], NULL, 16);
		size = (ulong)simple_strtoul(argv[4], NULL, 16);

		printf("off = %x, addr = %x, size = %x\n", off, addr, size);


		ret = msc_write(off, (u_char *)addr, size);

		printf(" %d bytes : %s\n", size, ret ? "ERROR" : "OK");
		return ret == 0 ? 0 : 1;
	}


usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(msc, 5, 1, do_msc,
	"msc	- MMC/SD sub-system\n",
	"msc read	- addr off|partition size\n");

