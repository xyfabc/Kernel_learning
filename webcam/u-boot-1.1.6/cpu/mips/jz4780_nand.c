/*
 * Platform independend driver for JZ4770.
 *
 * Copyright (c) 2007 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) &&	defined(CONFIG_JZ4780)
#include <nand.h>
#if defined(CONFIG_JZ4780)
#include <asm/jz4780.h>
#endif

/* Size of ecc parities per 512 bytes, in half-byte */
static struct nand_oobinfo nand_oob_bch = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 52,
	.eccpos = {
		3,  4,  5,  6,  7,  8,  9,  10,
		11, 12, 13, 14, 15, 16, 17, 18,
		19, 20, 21, 22, 23, 24, 25, 26,
		27, 28, 29, 30, 31, 32, 33, 34,
		35, 36, 37, 38, 39, 40, 41, 42,
		43, 44, 45, 46, 47, 48, 49, 50,
		51, 52, 53, 54},
	.oobfree = {{2, 1}, {55, 9}}
};

#if 0
static void bch_register_dump()
{
}
#endif

#ifndef CFG_NAND_TOGGLE
static void jz_hwcontrol_common(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	switch (cmd) {
	case NAND_CTL_SETNCE:
		REG_NEMC_NFCSR |= NEMC_NFCSR_NFCE1;
		break;

	case NAND_CTL_CLRNCE:
		REG_NEMC_NFCSR &= ~NEMC_NFCSR_NFCE1;
		break;

	case NAND_CTL_SETCLE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) | NAND_CMD_OFFSET);
		break;

	case NAND_CTL_CLRCLE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) & ~NAND_CMD_OFFSET);
		break;

	case NAND_CTL_SETALE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) | NAND_ADDR_OFFSET);
		break;

	case NAND_CTL_CLRALE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) & ~NAND_ADDR_OFFSET);
		break;
	}
}

#else

static void jz_hwcontrol_toggle(struct mtd_info *mtd, int cmd)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	switch (cmd) {
	case NAND_CTL_SETNCE:
		__tnand_dphtd_sync(1);
		REG_NEMC_NFCSR |= NEMC_NFCSR_DAEC | NEMC_NFCSR_NFCE1;	
		__tnand_dae_clr();
		break;

	case NAND_CTL_CLRNCE:
		REG_NEMC_NFCSR &= ~NEMC_NFCSR_NFCE1;
		__tnand_dphtd_sync(1);
		break;

	case TNAND_READ_PERFORM:
		REG_NEMC_TGWE |= NEMC_TGWE_DAE;
		__tnand_dae_sync();
		break;

	case TNAND_WRITE_PERFORM:
		REG_NEMC_TGWE |= NEMC_TGWE_DAE | NEMC_TGWE_SDE1;
		__tnand_dae_sync();
		__tnand_wcd_sync();
		break;

	case NAND_CTL_SETCLE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) | NAND_CMD_OFFSET);
		break;

	case NAND_CTL_CLRCLE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) & ~NAND_CMD_OFFSET);
		break;

	case NAND_CTL_SETALE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) | NAND_ADDR_OFFSET);
		break;

	case NAND_CTL_CLRALE:
		this->IO_ADDR_W = (void __iomem *)((unsigned long)(this->IO_ADDR_W) & ~NAND_ADDR_OFFSET);
		break;
	}
}
#endif /* CFG_NAND_TOGGLE */

static int jz_device_ready(struct mtd_info *mtd)
{
	int ready;

	udelay(20);	/* FIXME: add 20us delay */
	ready = (REG_GPIO_PXPIN(0) & 0x00100000)? 1 : 0;

	return ready;
}

/*
 * NEMC setup
 */
static void jz_device_setup(void)
{
	/* Set NAND GPIO and Timings */
#if CFG_NAND_BW8 == 1
	__gpio_as_nand_8bit(1);
	REG_NEMC_SMCR1 = CFG_NAND_SMCR1;
#else /* 16 bit */
	__gpio_as_nand_16bit(1);
	REG_NEMC_SMCR1 = CFG_NAND_SMCR1 | 0x40;
#endif

#ifdef CFG_NAND_TOGGLE
	__gpio_as_nand_toggle();
	REG_NEMC_TGCR1 = CFG_NAND_TGCR1;
//	__tnand_dqsdelay_probe();
	/* Set NFE bit */
	REG_NEMC_NFCSR |= NEMC_NFCSR_TNFE1 | NEMC_NFCSR_NFE1;
#else
	/* Set NFE bit */
	REG_NEMC_NFCSR |= NEMC_NFCSR_NFE1;
#endif

#if 1
	/* disable NAND WP# */
	__gpio_as_output1(182);
#endif
}

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	/*
	 * Don't use "chip" to address the NAND device,
	 * generate the cs from the address where it is encoded.
	 */
}

/**
 * jzsoc_nand_enable_bch_hwecc - Config BCH
 */
static void jzsoc_nand_enable_bch_hwecc(struct mtd_info* mtd, int mode)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	int ecc_size = this->eccsize;
	int ecc_bytes = this->eccbytes;

#if 0
	if (mode == NAND_ECC_READ) {
		__bch_cnt_set(ecc_size, ecc_bytes);
		__bch_decoding(CFG_NAND_BCH_BIT);
	}

	if (mode == NAND_ECC_WRITE) {
		__bch_cnt_set(ecc_size, ecc_bytes);
		__bch_encoding(CFG_NAND_BCH_BIT);
	}
#endif
	switch (mode) {
	case NAND_ECC_READ_OOB :
		ecc_size = CFG_NAND_ECC_POS;
	case NAND_ECC_READ :
		__bch_cnt_set(ecc_size, ecc_bytes);
		__bch_decoding(CFG_NAND_BCH_BIT);
		break;

	case NAND_ECC_WRITE_OOB :
		ecc_size = CFG_NAND_ECC_POS;
	case NAND_ECC_WRITE :
		__bch_cnt_set(ecc_size, ecc_bytes);
		__bch_encoding(CFG_NAND_BCH_BIT);
	}
}

static int jzsoc_nand_calculate_bch_ecc(struct mtd_info *mtd, const u_char * dat, u_char * ecc_code)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	volatile u8 *paraddr = (volatile u8 *)BCH_PAR0;
	short i;

	/* Write data to REG_BCH_DR */
	for (i = 0; i < this->eccsize; i++) {
		REG_BCH_DR8 = dat[i];
	}

	/* Wait for BCH completion */
	__bch_encode_sync();

	for (i = 0; i < this->eccbytes; i++) {
		ecc_code[i] = *paraddr++;
	}

	__bch_encints_clean();
	__bch_disable();

	return 0;
}

/**
 * bch_correct
 * @dat:        data to be corrected
 * @idx:        the index of error bit in an eccsize
 */
static void bch_error_correct(u16 *data, int err_bit)
{
	u32 idx; /* indicates an error half_word */
	u16 err_mask;

	idx = (REG_BCH_ERR(err_bit) & BCH_ERR_INDEX_MASK) >> BCH_ERR_INDEX_BIT;
	err_mask = (REG_BCH_ERR(err_bit) & BCH_ERR_MASK_MASK) >> BCH_ERR_MASK_BIT;

	data[idx] ^= err_mask;
}

/**
 * jzsoc_nand_bch_correct_data:  calc_ecc points to oob_buf for us
 * @mtd:	mtd info structure
 * @dat:        data to be corrected
 * @read_ecc:   pointer to ecc buffer calculated when nand writing
 * @calc_ecc:   no used
 */
static int jzsoc_nand_bch_correct_data(struct mtd_info *mtd, u_char *data, u_char *read_ecc, u_char *calc_ecc)
{
	struct nand_chip *this = (struct nand_chip *)(mtd->priv);
	short i;
	u32 state, errcnt;
	int ret_val = 0;

	/* Write data to REG_BCH_DR */
	for (i = 0; i < this->eccsize; i++) {
		REG_BCH_DR = data[i];
	}

	/* Write parities to REG_BCH_DR */
	for (i = 0; i < this->eccbytes; i++) {
		REG_BCH_DR = read_ecc[i];
	}

	/* Wait for completion */
	__bch_decode_sync();

	/* Check decoding */
	state = REG_BCH_INTS;

	if (state & BCH_INTS_UNCOR) {
		printk("NAND: Uncorrectable ECC error--\n");
		printk("REG_BCH_CR=0x%08x, REG_BCH_CNT=0x%08x, state=0x%08x\n", REG_BCH_CR, REG_BCH_CNT, state);
		ret_val = -1;
	} else if (state & BCH_INTS_ERR) {
		/* Error occurred */
		errcnt = (state & BCH_INTS_ERRC_MASK) >> BCH_INTS_ERRC_BIT;

		for (i = 0; i < errcnt; i++)
			bch_error_correct((u16 *)data, i);
	}

	__bch_decints_clean();
	__bch_disable();

	return ret_val;
}

/*
 * Main initialization routine
 */
void board_nand_init(struct nand_chip *nand)
{
	jz_device_setup();

	/* Set 0 to nand_chip */
	memset(nand, 0x00, sizeof(struct nand_chip));

	/* set ECC Mode */
	nand->eccmode = NAND_ECC_HW_BCH64;

#ifdef CFG_NAND_TOGGLE
        nand->hwcontrol = jz_hwcontrol_toggle;
#else
        nand->hwcontrol = jz_hwcontrol_common;
#endif
        nand->dev_ready = jz_device_ready;
	
	nand->correct_data  = jzsoc_nand_bch_correct_data;
	nand->enable_hwecc  = jzsoc_nand_enable_bch_hwecc;
	nand->calculate_ecc = jzsoc_nand_calculate_bch_ecc;

        /* Set address of NAND IO lines */
        nand->IO_ADDR_R = (void __iomem *) CFG_NAND_BASE;
        nand->IO_ADDR_W = (void __iomem *) CFG_NAND_BASE;

        /* 20 us command delay time */
        nand->chip_delay = 20;

	nand->options &= ~NAND_BUSWIDTH_16;
#if CFG_NAND_BW8 == 0
	nand->options |= NAND_BUSWIDTH_16;
#endif
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_NAND) */
