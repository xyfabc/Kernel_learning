/*
 * Copyright (C) 2007 Ingenic Semiconductor Inc.
 * Author: Regen Huang <lhhuang@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <nand.h>
#include <asm/io.h>

#if defined(CONFIG_JZ4780)
#include <asm/jz4780.h>
#endif

#if 0
#define NEMC_PNCR (NEMC_BASE+0x100)
#define NEMC_PNDR (NEMC_BASE+0x104)
#define REG_NEMC_PNCR REG32(NEMC_PNCR)
#define REG_NEMC_PNDR REG32(NEMC_PNDR)
#endif

#define __nemc_pn_reset_and_enable() \
do { \
	REG_NEMC_PNCR = 0x3; \
} while (0)
#define __nemc_pn_disable() \
do { \
	REG_NEMC_PNCR = 0x0; \
} while (0)

/*
 * NAND flash definitions
 */
#define NAND_DATAPORT	CFG_NAND_BASE
#define NAND_ADDRPORT   (CFG_NAND_BASE | NAND_ADDR_OFFSET)
#define NAND_COMMPORT   (CFG_NAND_BASE | NAND_CMD_OFFSET)

#define ECC_BLOCK	1024
static int par_size;

#define __nand_cmd(n)		(REG8(NAND_COMMPORT) = (n))
#define __nand_addr(n)		(REG8(NAND_ADDRPORT) = (n))
#define __nand_data8()		REG8(NAND_DATAPORT)
#define __nand_data16()		REG16(NAND_DATAPORT)

#define __nand_enable()		(REG_NEMC_NFCSR |= NEMC_NFCSR_NFE1 | NEMC_NFCSR_NFCE1)
#define __nand_disable()	(REG_NEMC_NFCSR &= ~(NEMC_NFCSR_NFE1 | NEMC_NFCSR_NFCE1))

#define __tnand_enable() \
do { \
	REG_NEMC_NFCSR |= NEMC_NFCSR_TNFE1 | NEMC_NFCSR_NFE1; \
	__tnand_dphtd_sync(1); \
	REG_NEMC_NFCSR |= NEMC_NFCSR_NFCE1 | NEMC_NFCSR_DAEC; \
} while (0)
	
#define __tnand_disable() \
do { \
	REG_NEMC_NFCSR &= ~NEMC_NFCSR_NFCE1; \
	__tnand_dphtd_sync(1); \
	REG_NEMC_NFCSR &= ~(NEMC_NFCSR_TNFE1 | NEMC_NFCSR_NFE1); \
} while (0)

#define __tnand_datard_perform() \
do { \
	REG_NEMC_TGWE |= NEMC_TGWE_DAE; \
	__tnand_dae_sync(); \
} while (0)

static inline void nand_wait_rb(void)
{
	volatile unsigned int timeout = 200;
	while ((REG_GPIO_PXPIN(0) & 0x00100000) && timeout--);
	while (!(REG_GPIO_PXPIN(0) & 0x00100000));
}

/*
 * NAND flash parameters
 */
static int bus_width = 8;
static int page_size = 8192;
static int oob_size = 436;
static int ecc_count = 4;
static int row_cycle = 3;
static int page_per_block = 128;
static int bad_block_pos = 0;
static int block_size = 1048576;
static int free_size = 512;

static unsigned char oob_buf[1024] = {0};

/*
 * External routines
 */
extern void flush_cache_all(void);
extern int serial_init(void);
extern void serial_puts(const char *s);
extern void serial_put_hex(unsigned int d);
extern void sdram_init(void);
extern void pll_init(void);

static int buf_check(u8 *org, u8 *obj, int size)
{
	int i, j, ecnt = 0;

	for (i = 0; i < size; i++) {
#if 1
		switch(org[i] ^ obj[i]) {
		case 0x00:
			break;
		case 0x01:
		case 0x02:
		case 0x04:
		case 0x08:
		case 0x10:
		case 0x20:
		case 0x40:
		case 0x80:
			ecnt++;
			break;
		default:
			serial_puts("\n--Buf check: IN Error!\n");
			serial_puts("--local:");
			serial_put_hex(i);

			for (j = i - 2; j < i + 3; j++) {
				serial_put_hex(j);
				serial_puts("--  : 0x");
				serial_put_hex(org[j]);
				serial_puts("--vs: 0x");
				serial_put_hex(obj[j]);
				serial_puts("\n");
			}
			ecnt++;
		}
#else
		if (org[i] != obj[i]) {
			serial_puts("\n--Buf check: IN Error!\n");
			serial_puts("--local:");
			serial_put_hex(i);

			for (j = i - 2; j < i + 3; j++) {
				serial_put_hex(j);
				serial_puts("--  : 0x");
				serial_put_hex(org[j]);
				serial_puts("--vs: 0x");
				serial_put_hex(obj[j]);
				serial_puts("\n");
			}
			ecnt++;

		}
#endif
	}
 
	if (ecnt == 0)
		serial_puts("--buf check: NO Error!\n");
	serial_puts("----err count:");
	serial_put_hex(ecnt);

	return ecnt ? -1 : 0;;
}

static void buf_init(u8 *buffer, int len)
{
	u8 *buf = buffer;
	int i;

	for (i = 0; i < len; i++) {
		buf[i] = i % 256;
	}
}

static void mem_dump(u8 *buf, int len)
{
	int i;

	serial_puts("\n--buf addr: ");

	for (i = 0; i < len; i++){
		serial_puts(" ");
		serial_put_hex(buf[i]);
	}
}

#if 0
static void nand_read_base(unsigned char *data_buf, int col,
			int page, int size)
{
	int i, j;
	REG32(0xb3410050) = 0x80010001; // enable
	while(!(REG32(0xb341013c) & 0x1)); // wait DPHTD
	REG32(0xb3410050) = 0x00010003; // set CE#

	REG8(0xbb000000 | 0x00400000) = 0x00; // nand cmd
	REG8(0xbb000000 | 0x00800000) = col & 0xff; // nand addr
	col >>= 8;
	REG8(0xbb000000 | 0x00800000) = col & 0xff; // nand addr
	for (i = 0; i < 3; i++) {
		REG8(0xbb000000 | 0x00800000) = page & 0xff; // nand addr
		page >>= 8;
	}
	REG8(0xbb000000 | 0x00400000) = 0x30; // nand cmd

	nand_wait_rb();

	REG32(0xb341010c) = 0x80000000;
	while(!(REG32(0xb341010c) & 0x80000000));

	for (i = 0; i < size/1024; i++) {
		for (j = 0; j < 1024; j++)
			data_buf[i * 1024 + j] = REG8(0xbb000000);
	}
#if 0
	for (i = 0; i < size; i++) {
		data_buf[i] = REG8(0xbb000000);
	}
#endif
	REG32(0xb3410050) = 0x00010001; // clear CE#
	while(!(REG32(0xb341013c) & 0x1));
	REG32(0xb3410050) = 0x0;
}

#else

static void nand_read_base(unsigned char *data_buf, int col,
			int page, int size)
{
	int i, j;

	__tnand_enable();

	__nand_cmd(0x00);
	__nand_addr(col& 0xff);
	col >>= 8;
	__nand_addr(col & 0xff); // nand addr
	for (i = 0; i < 3; i++) {
		__nand_addr(page & 0xff); // nand addr
		page >>= 8;
	}
	__nand_cmd(0x30); // nand cmd

	nand_wait_rb();
	__tnand_datard_perform();

	for (i = 0; i < size/1024; i++) {
		for (j = 0; j < 1024; j++)
			data_buf[i * 1024 + j] = REG8(0xbb000000);
	}
#if 0
	for (i = 0; i < size; i++) {
		data_buf[i] = REG8(0xbb000000);
	}
#endif
	__tnand_disable();
}
#endif

static void gpio_init(void)
{
	switch (CFG_UART_BASE) {
	case UART0_BASE:
		__gpio_as_uart0();
		__cpm_start_uart0();
		break;
	case UART1_BASE:
		__gpio_as_uart1();
		__cpm_start_uart1();
		break;
	case UART2_BASE:
		__gpio_as_uart2();
		__cpm_start_uart2();
		break;
	case UART3_BASE:
		__gpio_as_uart3();
		__cpm_start_uart3();
		break;
	}

#ifdef CONFIG_FPGA
	__gpio_as_nor();

        /* if the delay isn't added on FPGA, the first line that uart
	 * to print will not be normal.
	 */
	{
		volatile int i=1000;
		while(i--);
	}
#endif
}

void spl_boot(void)
{
	unsigned char *databuf = (unsigned char *)0xa0001000;
	unsigned char *tempbuf = (unsigned char *)0xa0004000;
	int i, j, ret;

	/*
	 * Init hardware
	 */

	__cpm_start_dmac();
	__cpm_start_ddr();
	/* enable mdmac's clock */
	REG_MDMAC_DMACKES = 0x3;

	//set bch divider
	__cpm_sw_bchm();
	__cpm_set_bchdiv(3);
	__cpm_start_bch();

	gpio_init();
	serial_init();

	serial_puts("\n\nNAND Secondary Program Loader\n\n");
	serial_puts("---NEMC_NFCSR:");
	serial_put_hex(REG_NEMC_NFCSR);
	serial_puts("---NEMC_SMCR1:");
	serial_put_hex(REG_NEMC_SMCR1);
	serial_puts("---NEMC_TGDR:");
	serial_put_hex(REG_NEMC_TGDR);
	serial_puts("\n");
        serial_puts("--GPIO_A_PXPIN:");
	serial_put_hex(REG_GPIO_PXPIN(0));
	serial_puts("--GPIO_A_PXINT:");
	serial_put_hex(REG_GPIO_PXINT(0));
	serial_puts("--GPIO_A_PXMASK:");
	serial_put_hex(REG_GPIO_PXMASK(0));
	serial_puts("--GPIO_A_PXPART1:");
	serial_put_hex(REG_GPIO_PXPAT1(0));
	serial_puts("--GPIO_A_PXPART0:");
	serial_put_hex(REG_GPIO_PXPAT0(0));
	serial_puts("--GPIO_A_PXFLG:");
	serial_put_hex(REG_GPIO_PXFLG(0));
	serial_puts("--GPIO_A_PXPEN:");
	serial_put_hex(REG_GPIO_PXPEN(0));

	serial_puts("--GPIO_B_PXPIN:");
	serial_put_hex(REG_GPIO_PXPIN(1));
	serial_puts("--GPIO_B_PXINT:");
	serial_put_hex(REG_GPIO_PXINT(1));
	serial_puts("--GPIO_B_PXMASK:");
	serial_put_hex(REG_GPIO_PXMASK(1));
	serial_puts("--GPIO_B_PXPART1:");
	serial_put_hex(REG_GPIO_PXPAT1(1));
	serial_puts("--GPIO_B_PXPART0:");
	serial_put_hex(REG_GPIO_PXPAT0(1));
	serial_puts("--GPIO_B_PXFLG:");
	serial_put_hex(REG_GPIO_PXFLG(1));
	serial_puts("--GPIO_B_PXPEN:");
	serial_put_hex(REG_GPIO_PXPEN(1));


#ifndef CONFIG_FPGA
	pll_init();
#endif
	sdram_init();

	bus_width = (CFG_NAND_BW8==1) ? 8 : 16;
	page_size = CFG_NAND_PAGE_SIZE;
	row_cycle = CFG_NAND_ROW_CYCLE;
	block_size = CFG_NAND_BLOCK_SIZE;
	page_per_block =  CFG_NAND_BLOCK_SIZE / CFG_NAND_PAGE_SIZE;
	bad_block_pos = (page_size == 512) ? 5 : 0;
	oob_size = CFG_NAND_OOB_SIZE;
	par_size = CFG_NAND_BCH_BIT * 14 / 8;

	ecc_count = page_size / ECC_BLOCK;
	
	REG_NEMC_SMCR1 = CFG_NAND_SMCR1;

	buf_init(databuf, 8192);
	serial_puts("--buf init\n");
	mem_dump(databuf, 128);

	serial_puts("--nand test read\n");
	for (i = 0; i < 2000; i++) {
		for  (j = 0; j < 8192; j++)
			tempbuf[i] = 0x00;
		serial_puts("--nand read:");
		serial_put_hex(i);
	
		nand_read_base(tempbuf, 0, 1025, 8192);
		ret = buf_check(databuf, tempbuf, 8192);
	}

	serial_puts("--tempbuf dump\n");
	mem_dump(tempbuf, 128);
}
