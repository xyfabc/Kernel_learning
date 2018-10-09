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

#include <asm/jzsoc.h>

/*
 * NAND flash definitions
 */
#define NAND_DATAPORT	CFG_NAND_BASE
#define NAND_ADDRPORT   (CFG_NAND_BASE | NAND_ADDR_OFFSET)
#define NAND_COMMPORT   (CFG_NAND_BASE | NAND_CMD_OFFSET)

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

#define __tnand_read_perform() \
do { \
	REG_NEMC_TGWE |= NEMC_TGWE_DAE; \
	__tnand_dae_sync(); \
} while (0)

#define __nemc_pn_reset_and_enable() \
do { \
	REG_NEMC_PNCR = NEMC_PNCR_PNRST | NEMC_PNCR_PNEN; \
} while (0)

#define __nemc_pn_disable() \
do { \
	REG_NEMC_PNCR = 0x0; \
} while (0)

#define __cpm_set_bchdiv(sclk, div) \
do { \
	unsigned int regval; \
	regval = (sclk) | (((div) - 1) & CPM_BCHCDR_BCHCDR_MASK); \
	REG_CPM_BCHCDR = regval | CPM_BCHCDR_CE_BCH; \
	while (REG_CPM_BCHCDR & CPM_BCHCDR_BCH_BUSY); \
	REG_CPM_BCHCDR &= ~CPM_BCHCDR_CE_BCH; \
} while (0)

/*
 * NAND flash parameters
 */
static int bus_width; /* Bus width: 8 or 16 */
static int page_size;
static int free_size;
static int oob_size;
static int block_size;
static int row_cycle;
static int page_per_block;
static int bad_block_pos;

/* ECC args */
static int ecc_block;
static int ecc_parity;
static int ecc_count;

/* BCH operation args */
struct ops_bch {
	int bch_level;
	int bch_block_size;
	int bch_parity_size;
};

/*
 * External routines
 */
extern int serial_init(void);
extern void serial_puts(const char *s);
extern void serial_put_hex(unsigned int d);
extern void sdram_init(void);
extern void pll_init(void);
extern void flush_cache_all(void);

/**
 * spl_mem_copy - SPL Memory copy
 * @d:		dest address
 * @s:		source address
 * @count:	memory copy size(Byte)
 */
static u8 *spl_mem_copy(void *d, void *s, int count)
{
	int i;

	for (i = 0; i < count; i++)
		((u8 *)d)[i] = ((u8 *)s)[i];

	return (u8 *)d;
}

/**
 * nand_wait_ready - NAND Wait R/B
 */
static inline void nand_wait_ready(void)
{
	volatile unsigned int timeout = 200;

	while ((REG_GPIO_PXPIN(0) & 0x00100000) && timeout--);
	while (!(REG_GPIO_PXPIN(0) & 0x00100000));
}

/**
 * nand_read_buf - Read NAND data to memory
 * @buf:	buffer memory address
 * @count:	read data size(Byte)
 */
static void (*nand_read_buf)(void *buf, int count);
static void nand_read_buf8(void *buf, int count)
{
	int i;
	u8 *p = (u8 *)buf;

	for (i = 0; i < count; i++)
		*p++ = __nand_data8();
}

static void nand_read_buf16(void *buf, int count)
{
	int i;
	u16 *p = (u16 *)buf;

	for (i = 0; i < count; i += 2)
		*p++ = __nand_data16();
}

/**
 * bch_error_correct - BCH Correct the error bit in ECC_BLOCK bytes data
 * @data_buf:	BCH will correct data buffer
 * @err_bit:	BCH half word error number
 */
static void bch_error_correct(u16 *data_buf, u32 err_bit)
{
	u32 err_mask, idx; /* the index of half-word has error bit */

	idx = (REG_BCH_ERR(err_bit) & BCH_ERR_INDEX_MASK) >> BCH_ERR_INDEX_BIT;
	err_mask = (REG_BCH_ERR(err_bit) & BCH_ERR_MASK_MASK) >> BCH_ERR_MASK_BIT;

	data_buf[idx] ^= (u16)err_mask;
}

/**
 * nand_bch_correct - Do BCH decode
 * @data_buf:	data buffer of BCH block
 * @ecc_buf:	BCH parity buffer
 * @ops:	structure of BCH operat information
 *
 * NOTE:
 *	Return -1, if Check out BCH uncorrectable 
 */
static int nand_bch_correct(u8 *data_buf, u8 *ecc_buf, struct ops_bch *ops)
{
	u32 err_cnt, status;
	int i, retval = 0;

	/* Init BCH for BCH decode */
	__bch_cnt_set(ops->bch_block_size, ops->bch_parity_size);
	__bch_decoding(ops->bch_level);

	/* Fill ecc_block and ecc_parity to BCH_DR */
	for (i = 0; i < ops->bch_block_size; i++)
		REG_BCH_DR = data_buf[i];
	for (i = 0; i < ops->bch_parity_size; i++)
		REG_BCH_DR = ecc_buf[i];

	/* Wait BCH decode finished */
	__bch_decode_sync();

	/* Handle BCH */
	status = REG_BCH_INTS;
	if (status & BCH_INTS_UNCOR) {
		/* BCH Uncorrectable error */
		retval = -1;
	} else if (status & BCH_INTS_ERR) {
		err_cnt = (status & BCH_INTS_ERRC_MASK) >>  BCH_INTS_ERRC_BIT;
		for(i = 0; i < err_cnt; i++)
			bch_error_correct((u16 *)data_buf, i);
	}

	/* Clear and disable BCH */
	__bch_decints_clean();
	__bch_disable();

	return retval;
}

/**
 * nand_read_oob - Read NAND spera area data
 * @page_addr:	NAND page address
 * @buf:	read data buffer
 * @size:	read data size(Byte)
 */
static int nand_read_oob(int page_addr, u8 *buf, int size)
{
	int col_addr;
	int i;

	if (page_size != 512) {
		if (bus_width == 8)
			col_addr = page_size;
		else
			col_addr = page_size / 2;
	} else
		col_addr = 0;

#ifdef CFG_NAND_TOGGLE
	__tnand_enable();
#endif
	if (page_size != 512)
		/* Send READ0 command */
		__nand_cmd(NAND_CMD_READ0);
	else
		/* Send READOOB command */
		__nand_cmd(NAND_CMD_READOOB);

	/* Send column address */
	__nand_addr(col_addr & 0xff);
	if (page_size != 512)
		__nand_addr((col_addr >> 8) & 0xff);

	/* Send page address */
	for (i = 0; i < row_cycle; i++) {
		__nand_addr(page_addr & 0xff);
		page_addr >>= 8;
	}

	/* Send READSTART command for 2048 or 4096 ps NAND */
	if (page_size != 512)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	nand_wait_ready();

#ifdef CFG_NAND_TOGGLE
	__tnand_read_perform();
#endif

#if CFG_NAND_USE_PN
	__nemc_pn_reset_and_enable();
#endif

	/* Read oob data */
	nand_read_buf((void *)buf, size);

#if CFG_NAND_USE_PN
	__nemc_pn_disable();
#endif

#ifdef CFG_NAND_TOGGLE
	__tnand_disable();
#endif

	if (page_size == 512)
		nand_wait_ready();

	return 0;
}

/**
 * nand_read_page - Read NAND page data with BCH
 * @page_addr:	NAND page address
 * @data_buf:	NAND data area buffer
 * @oob_buf:	NAND spera area buffer
 */
static int nand_read_page(int page_addr, u8 *data_buf, u8 *oob_buf)
{
	struct ops_bch read_ops;
	u8 *ecc_buf;
	int i, retval;

#ifdef CFG_NAND_TOGGLE
	__tnand_enable();
#endif
	/* Send READ0 command */
	__nand_cmd(NAND_CMD_READ0);

	/* Send column address */
	__nand_addr(0);
	if (page_size != 512)
		__nand_addr(0);

	/* Send page address */
	for (i = 0; i < row_cycle; i++) {
		__nand_addr(page_addr & 0xff);
		page_addr >>= 8;
	}

	/* Send READSTART command for 2048 or 4096 ps NAND */
	if (page_size != 512)
		__nand_cmd(NAND_CMD_READSTART);

	/* Wait for device ready */
	nand_wait_ready();

#ifdef CFG_NAND_TOGGLE
	__tnand_read_perform();
#endif

	/* Read data */
#if CFG_NAND_USE_PN
	__nemc_pn_reset_and_enable();
#endif
	nand_read_buf((void *)data_buf, page_size);

	/* Read oob */
#if CFG_NAND_USE_PN
	__nemc_pn_reset_and_enable();
#endif
	nand_read_buf((void *)oob_buf, oob_size);
#if CFG_NAND_USE_PN
	__nemc_pn_disable();
#endif

#ifdef CFG_NAND_TOGGLE
	__tnand_disable();
#endif

	spl_mem_copy(	oob_buf + oob_size,
			data_buf + page_size - free_size,
			free_size);

	read_ops.bch_level = CFG_NAND_BCH_BIT;
	read_ops.bch_block_size = ecc_block;
	read_ops.bch_parity_size = ecc_parity;

	ecc_count = (page_size - free_size) / ecc_block;
	ecc_buf = oob_buf + CFG_NAND_ECC_POS;
	/* Perform BCH decode */
	for (i = 0; i < ecc_count; i++) {
		retval = nand_bch_correct(data_buf + i * ecc_block,
					ecc_buf + i * ecc_parity,
					&read_ops);
		if (retval < 0) {
			/* BCH Uncorrectable */
			serial_puts("NAND: Uncorrectable ECC Error\n");
		}
	}

	return 0;
}

#ifndef CFG_NAND_BADBLOCK_PAGE
#define CFG_NAND_BADBLOCK_PAGE	0 /* NAND bad block was marked at this page in a block, starting from 0 */
#endif

/**
 * nand_load - Load u-boot data to memory
 * @offs:	nand offset
 * @uboot_size:	load u-boot size
 * @dst:	memory address for load u-boot
 *
 * NOTE:
 *	u-boot data buffer address is argument "dst" witch point to DDR memory,
 *	and NAND oob buffer modified to use DDR memeory after "dst" plus double
 *	of "uboot_size".
 */
static void nand_load(int offs, int uboot_size, unsigned char *dst)
{
	int page;
	int pagecopy_count;
	unsigned char *oob_buf = dst + uboot_size * 2; /* Get oob_buf space on DDR memory */

#if defined(CFG_NAND_TOGGLE) && !defined(CONFIG_FPGA)
	serial_puts("Toggle NAND DQS Init ... ");
#ifndef CFG_NAND_TOGGLE_RDQS
	__tnand_dqsdelay_probe();
	if (__tnand_dqsdelay_checkerr())
		serial_puts("failed\n\n");
	else
		serial_puts("ok\n\n");
#else
	__tnand_dqsdelay_init(CFG_NAND_TOGGLE_RDQS);
	serial_puts("ok\n\n");
#endif
#if 0
	serial_puts("NEMC_TGDR:\t");
	serial_put_hex(REG32(NEMC_TGDR));
#endif
#endif

#ifndef CFG_NAND_TOGGLE
	__nand_enable();
#endif

	page = offs / page_size;
	pagecopy_count = 0;
	while (pagecopy_count * page_size < uboot_size) {
		if (page % page_per_block == 0) {
			nand_read_oob(page + CFG_NAND_BADBLOCK_PAGE, oob_buf, oob_size);
			if (oob_buf[bad_block_pos] != 0xff) {
				page += page_per_block;
				/* Skip bad block */
				continue;
			}
		}

		/* Load this page to dst, do the ECC */
		nand_read_page(page, dst, oob_buf);

		dst += (page_size - free_size);
		page++;
		pagecopy_count++;
	}

#ifndef CFG_NAND_TOGGLE
	__nand_disable();
#endif
}

/**
 * gpio_init - Init UART gpio
 */ 
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
        /* if the delay isn't added on FPGA, the first line that uart
	 * to print will not be normal.
	 */
	{
		volatile int i=1000;
		while(i--);
	}
#endif
}

/**
 * calc_free_size - Caculate freesize
 */
static int calc_free_size(int oobsize,
			  int ecccount,
			  int ecc_block_size,
			  int ecc_parity_size)
{
	int freesize;
	
	if (ecccount * ecc_parity_size + CFG_NAND_ECC_POS > oobsize)
		freesize = ecc_block_size;
	else
		freesize = 0;

	return freesize;
} 

/**
 * spl_boot - Second programme loader boot
 */
void spl_boot(void)
{
#ifndef CONFIG_FPGA
	u32 bch_sclk, bch_div;
#endif

#ifdef CONFIG_LOAD_UBOOT
	void (*uboot)(void);
#else
	int i;
	u32 *param_addr = 0;
	u8 *tmpbuf = 0;
	u8 cmdline[256] = CFG_CMDLINE;
	void (*kernel)(int, char **, char *);
#endif

#ifndef CONFIG_FPGA
	/* Get External crystal freqence */
	jz_extal = (*(volatile unsigned int *)0xF40007F4) * 12000000;
#endif

	/*
	 * Init hardware
	 */
	gpio_init();
	serial_init();

	serial_puts("\n\nNAND Secondary Program Loader\n");
#if 0
	serial_puts("NEMC_TGDR:\t");
	serial_put_hex(REG32(NEMC_TGDR));
	serial_puts("BCH BIT:\t");
	serial_put_hex(CFG_NAND_BCH_BIT);
	serial_puts("\n");
#endif

#ifndef CONFIG_FPGA
	pll_init();

	/* set bch divider */
	bch_sclk = __cpm_get_h2pll() << CPM_BCHCDR_BPCS_BIT;
	bch_div = __cpm_get_h2div() + 1;
	__cpm_set_bchdiv(bch_sclk, bch_div);
	//__cpm_set_bchdiv(CPM_BCHCDR_BPCS_MPLL, 4);
#endif

	/* Init SDRAM */
	sdram_init();

	/* Init Paraments */
	bus_width = (CFG_NAND_BW8 == 1) ? 8 : 16;
	page_size = CFG_NAND_PAGE_SIZE;
	oob_size = CFG_NAND_OOB_SIZE;
	row_cycle = CFG_NAND_ROW_CYCLE;
	block_size = CFG_NAND_BLOCK_SIZE;

	page_per_block =  CFG_NAND_BLOCK_SIZE / CFG_NAND_PAGE_SIZE;
	bad_block_pos = (page_size == 512) ? 5 : 0;
	ecc_block = (page_size == 512) ? 512 : 1024;
	ecc_parity = CFG_NAND_BCH_BIT * 14 / 8;

	ecc_count = page_size / ecc_block;
	free_size = calc_free_size(oob_size, ecc_count,
				ecc_block, ecc_parity);

	if (bus_width == 8)
		nand_read_buf = nand_read_buf8;
	else
		nand_read_buf = nand_read_buf16;

#if CFG_NAND_BW8 == 1
	REG_NEMC_SMCR1 = CFG_NAND_SMCR1;
#else
	REG_NEMC_SMCR1 = CFG_NAND_SMCR1 | 0x40;
#endif

#ifdef CFG_NAND_TOGGLE
	REG_NEMC_TGCR1 = CFG_NAND_TGCR1;
#endif

#ifdef CONFIG_LOAD_UBOOT
	/*
	 * Load U-Boot image from NAND into RAM
	 */
	nand_load(CFG_NAND_U_BOOT_OFFS, CFG_NAND_U_BOOT_SIZE,
		  (unsigned char *)CFG_NAND_U_BOOT_DST);

	uboot = (void (*)(void))CFG_NAND_U_BOOT_START;
	serial_puts("Starting U-Boot ...\n");
#else
	/*
	 * Load kernel image from NAND into RAM
	 */
	nand_load(CFG_NAND_ZIMAGE_OFFS, CFG_ZIMAGE_SIZE, (unsigned char *)CFG_ZIMAGE_DST);

	/*
	 * Prepare kernel parameters and environment
	 */
	param_addr = (u32 *)PARAM_BASE;
	param_addr[0] = 0;	/* might be address of ascii-z string: "memsize" */
	param_addr[1] = 0;	/* might be address of ascii-z string: "0x01000000" */
	param_addr[2] = 0;
	param_addr[3] = 0;
	param_addr[4] = 0;
	param_addr[5] = PARAM_BASE + 32;
	param_addr[6] = CFG_ZIMAGE_START;
	tmpbuf = (u8 *)(PARAM_BASE + 32);

	for (i = 0; i < 256; i++)
		tmpbuf[i] = cmdline[i];  /* linux command line */

	kernel = (void (*)(int, char **, char *))CFG_ZIMAGE_START;
	serial_puts("Starting kernel ...\n");
#endif
	/*
	 * Flush caches
	 */
	flush_cache_all();

#ifndef CONFIG_LOAD_UBOOT
	/*
	 * Jump to kernel image
	 */
	(*kernel)(2, (char **)(PARAM_BASE + 16), (char *)PARAM_BASE);
#else
	/*
	 * Jump to U-Boot image
	 */
	(*uboot)();
#endif
}
