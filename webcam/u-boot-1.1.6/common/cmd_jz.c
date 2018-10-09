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

#if (CONFIG_COMMANDS & CFG_CMD_JZ)

#if defined(CONFIG_JZ4750) || defined(CONFIG_JZ4750D) || defined(CONFIG_JZ4750L) || defined(CONFIG_JZ4760) || defined(CONFIG_JZ4760B) || defined(CONFIG_JZ4770)	|| defined(CONFIG_JZ4780)

#if defined(CONFIG_JZ4750)
#include <asm/jz4750.h>
#endif
#if defined(CONFIG_JZ4750D)
#include <asm/jz4750d.h>
#endif
#if defined(CONFIG_JZ4750L)
#include <asm/jz4750l.h>
#endif
#if defined(CONFIG_JZ4760)
#include <asm/jz4760.h>
#endif
#if defined(CONFIG_JZ4760B)
#include <asm/jz4760b.h>
#endif
#if defined(CONFIG_JZ4770)
#include <asm/jz4770.h>
#endif
#if defined(CONFIG_JZ4780)
#include <asm/jz4780.h>
#endif

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char

#if (CONFIG_COMMANDS & CFG_CMD_FAT) && (CONFIG_COMMANDS & CFG_CMD_NAND)
extern int do_fat_fsload(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_nand(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

int do_regread(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 addr;

	if (argc != 2)
		goto fail_read;

	addr = (u32)simple_strtoul(argv[1], NULL, 16);
	printf("addr: %#x, val: %#x\n", addr, REG32(addr));

	return 1;

fail_read:
	printf("Usage:\n%s\n", cmdtp->help);
	return 1;
}

U_BOOT_CMD(
	regread, 2, 1, do_regread,
	"regread - user special read register command\n",
	"regread - addr\n");

int do_regwrite(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u32 addr, val;
	
	if (argc != 3)
		goto fail_write;

	addr = (u32)simple_strtoul(argv[1], NULL, 16);
	val = (u32)simple_strtoul(argv[2], NULL, 16);
	REG32(addr) = val;

	return 1;

fail_write:
	printf("Usage:\n%s\n", cmdtp->help);
	return 1;
}

U_BOOT_CMD(
	regwrite, 3, 1, do_regwrite,
	"regwrite - user special write register command\n",
	"regwrite - [addr : value]\n");

#if defined(CONFIG_JZ4760) || defined(CONFIG_JZ4760B)
int do_setdiv(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u16 div_i, div_h, div_h2, div_p, div_m, div_s;
	register u32 cpccr = REG_CPM_CPCCR;

	int n2FR[9] = {
                0, 0, 1, 2, 3, 0, 4, 0, 5
        };

	if (argc != 7)
		goto fail_setdiv;

	div_i = (u32)simple_strtoul(argv[1], NULL, 10);
	div_h = (u32)simple_strtoul(argv[2], NULL, 10);
	div_h2 = (u32)simple_strtoul(argv[3], NULL, 10);
	div_p = (u32)simple_strtoul(argv[4], NULL, 10);
	div_m = (u32)simple_strtoul(argv[5], NULL, 10);
	div_s = (u32)simple_strtoul(argv[6], NULL, 10);

	printf("DIV: %d:%d:%d:%d:%d:%d\n", div_i, div_h, div_h2, div_p, div_m, div_s);

	cpccr &= ~(0xfffff | 0xf << 24);
	cpccr |=  CPM_CPCCR_PCS | CPM_CPCCR_CE |
		(n2FR[div_i] << CPM_CPCCR_CDIV_BIT) |
		(n2FR[div_h] << CPM_CPCCR_HDIV_BIT) |
		(n2FR[div_h2] << CPM_CPCCR_H2DIV_BIT) |
		(n2FR[div_p] << CPM_CPCCR_PDIV_BIT) |
		(n2FR[div_m] << CPM_CPCCR_MDIV_BIT) |
		(n2FR[div_s] << CPM_CPCCR_SDIV_BIT);

	REG_CPM_CPCCR = cpccr;
	printf("REG_CPM_CPCCR = %#x\n", cpccr);

	return 1;

fail_setdiv:
	printf("Usage:\n%s\n", cmdtp->help);

	return 1;
}

U_BOOT_CMD(
	setdiv, 7, 1, do_setdiv,
	"setdiv  - reset the pll div[I:H:H2:P:M:S]",
	"setdiv - I H H2 P M S\n");

int do_setspeed(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u16 val;
	register u32 cppcr;

	if (argc != 2)
		goto fail_setspeed;

	cppcr = REG_CPM_CPPCR;
	val = (u32)simple_strtoul(argv[1], NULL, 10);
	if (val < 72 || val > 1500) {
		printf("Error: val must be 72 ~ 1500.\n");
		return 1;
	}

	val /= 6;
	cppcr &= ~(0xffff0000 | CPM_CPPCR_PLLS | CPM_CPPCR_PLLEN | CPM_CPPCR_PLLST_MASK);
	cppcr |= (val << 24) | (2 << 18) | (1 << 16) | 0xff; // for 12M

	REG_CPM_CPPSR &= ~CPM_CPPSR_PS;
        REG_CPM_CPPSR &= ~CPM_CPPSR_CS;
        REG_CPM_CPPSR &= ~CPM_CPPSR_FS;

        REG_CPM_CPPSR |= CPM_CPPSR_PM;

	REG_CPM_CPPCR = cppcr | CPM_CPPCR_PLLEN;
	while(!(REG_CPM_CPPCR & CPM_CPPCR_PLLS));

	printf("Set CPU Speed: %d\n", val*6);
	printf("REG_CPM_CPPCR: %#x\n", REG_CPM_CPPCR);

	return 1;

fail_setspeed:
	printf("Usage:\n%s\n", cmdtp->help);
}

U_BOOT_CMD(
	setspeed, 2, 1, do_setspeed,
	"setspeed- reset the CPU Speed.\n",
	"setspeed - val\n");
#endif

#if (CONFIG_COMMANDS & CFG_CMD_FAT) && (CONFIG_COMMANDS & CFG_CMD_NAND)
int do_nandload(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i, ret;
	ulong len;
	char *size, *act[14];

	if(argc < 6){
		printf("Usage: %s %s\n", argv[0], cmdtp->help);
		return 1;
	}
	if(argc != 6 && strlen(argv[6]) > 7){
		printf("** wrong format: %s **\n", argv[6]);
		return 1;
	}

	/* Image read from other device, like MMC */
	char *argv_fat[5] = {argv[0], argv[1], argv[2],
			     argv[3], argv[4]};

	ret = do_fat_fsload(cmdtp, flag, 5, argv_fat);
	if(ret)
		return 1;

	/* NAND Erase before write Image to the NAND */
	len = simple_strtoul(getenv("filesize"), NULL, 16);
	len = len < 0x300000 ? 0x300000 : len;
	sprintf(size, "%x", len); /* nand erase size */

	char *argv_ncl[4] = {"nand", "erase", argv[5], size};
	ret = do_nand(cmdtp, flag, 4, argv_ncl);
	if(ret)
		return 1;

	/* Write Image to NAND at the addess of offset */
	strcpy(act, "write");
	if(argc == 7){
		sprintf(act, "%s.%s", act, argv[6]);
	}

	char *argv_nwr[5] = {"nand", act, argv[3], argv[5], getenv("filesize")};
	ret = do_nand(cmdtp, flag, 5, argv_nwr);
	if(ret)
		return 1;

	return 0;
}
	
U_BOOT_CMD(
	nupdate, 7, 0, do_nandload,
	"nupdate - update file to nand from MMC/SD Card\n",
	"<interface> <dev[:part]> <addr> <filename> <off> [yaffs|yaffs2]\n"
	"  -load binary file 'filename' from 'dev' on 'interface'\n"
	"  to address 'addr' from dos filesystem\n");
#endif

#endif  /* CONFIG_JZ4750 CONFIG_JZ4760 ... */
#endif  /* if CONFIG_COMMANDS & CFG_JZ_CMD */
