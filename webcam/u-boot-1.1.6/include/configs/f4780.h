/*
 * (C) Copyright 2008  Ingenic Semiconductor
 *
 *  Author: <cwjia@ingenic.cn>
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

/*
 * This file contains the configuration parameters for the fuwa board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_MIPS32		1  /* MIPS32 CPU core */
#define CONFIG_JzRISC		1  /* JzRISC core */
#define CONFIG_JZSOC		1  /* Jz SoC */
#define CONFIG_JZ4780		1  /* Jz4780 SoC */
//#define CONFIG_FPGA		1  /* f4780 is an FPGA board */
#define CONFIG_F4780		1  /* f4780 validation board */
#define CONFIG_DDRC		1  /* use ddr controller */

/* memory group */
// [MAY CHANGE] RAM
#ifdef CONFIG_SDRAM_MDDR
//#include "asm/jz_mem_nand_configs/MDDR_H5MS5122DFR-J3M.h"
#include "asm/jz_mem_nand_configs/MCP_H9DA4GH2GJAMCR.h"
#elif defined CONFIG_SDRAM_LPDDR2
#include "asm/jz_mem_nand_configs/LPDDR2_H9TC.h"
#elif defined CONFIG_SDRAM_DDR2
//#include "asm/jz_mem_nand_configs/DDR2_H5PS1G63EFR-G7C.h"
//#include "asm/jz_mem_nand_configs/DDR2_demo.h"
#include "asm/jz_mem_nand_configs/DDR2_H5PS1G63EFR-Y5C.h"
#elif defined CONFIG_SDRAM_DDR3
//#include "asm/jz_mem_nand_configs/DDR3_KTA-MB1066.h"
//#include "asm/jz_mem_nand_configs/DDR3_KMD3S1600V4G.h"
//#include "asm/jz_mem_nand_configs/DDR3_M471B5273CH0-CF8.h"
//#include "asm/jz_mem_nand_configs/DDR3_GSKILLF-10666CL9.h"
//#include "asm/jz_mem_nand_configs/DDR3_TSD34096M1333C9-E.h"
//#include "asm/jz_mem_nand_configs/DDR3_M473B5773DH0-YK0.h"
#include "asm/jz_mem_nand_configs/DDR3_H5TQ2G63BFR.h"
#else
#include "asm/jz_mem_nand_configs/SDRAM_.h"
#endif

// [MAY CHANGE] NAND
#include "asm/jz_mem_nand_configs/NAND_K9GAG08U0D.h"
//#include "asm/jz_mem_nand_configs/NAND_K9GAG08U0M.h"
//#include "asm/jz_mem_nand_configs/NAND_K9GBG08U0A.h"
//#include "asm/jz_mem_nand_configs/NAND_K9GBG08U0M.h"
//#include "asm/jz_mem_nand_configs/NAND_K9GBGD8U0B.h"
//#include "asm/jz_mem_nand_configs/NAND_K9F1G08U0B.h"
//#include "asm/jz_mem_nand_configs/NAND_K9F1208U0C.h"
//#include "asm/jz_mem_nand_configs/NAND_TC58NVG6DCJTA00.h"
//#include "asm/jz_mem_nand_configs/NAND_TC58TEG6DCJTA00.h"


#define JZ4760_NORBOOT_CFG	JZ4760_NORBOOT_8BIT	/* NOR Boot config code */

//#define CFG_EXTAL		12000000	/* EXTAL freq: 12  */
#define CFG_EXTAL		24000000	/* EXTAL freq: 24  */
#define CFG_CPU_SPEED		(800 * 1000000)	/* CPU clock: 400M */
//#define CFG_CPU_SPEED		(1300 * 1000000)
//#define CFG_CPU_SPEED		(48 * 1000000)	
#define CFG_DDR_SPEED		CFG_CPU_SPEED
#define CFG_DIV                 6		/* for ddr div */
//#define CFG_DDR_SPEED		(48 * 1000000)
#define	CFG_HZ			(CFG_EXTAL/256) /* incrementer freq */


/* this must be included AFTER CFG_EXTAL and CFG_CPU_SPEED */
#define CFG_UART_BASE  		UART1_BASE	/* Base of the UART channel */
#define CONFIG_BAUDRATE		57600
#define CFG_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_COMMANDS		(CONFIG_CMD_DFL | \
				 CFG_CMD_ASKENV | \
				 CFG_CMD_NAND   | \
				 CFG_CMD_MSC    | \
				 CFG_CMD_DHCP	| \
				 CFG_CMD_DIAG	| \
				 CFG_CMD_PING)
#define CONFIG_BOOTP_MASK	( CONFIG_BOOTP_DEFAUL )

#define CONFIG_POST		(CFG_JZ_POST_MEMORY)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>
//#ifndef CONFIG_FPGA
//#include "jz4780_common.h"
//#endif

// [MAY CHANGE] Boot Arguments
#define CONFIG_BOOTDELAY	1
#define CONFIG_BOOTFILE	        "tjiang/uImage"	/* file to load */

#if defined(CONFIG_SDRAM_DDR2)
#define LINUX_CMDLINE0 \
	"mem=256M console=ttyS0,9600n8 ip=192.168.4.58:192.168.3.56:192.168.1.1:255.255.248.0 nfsroot=192.168.3.56:/nfsroot/root26/home/tjiang/root rw"
#else
#define LINUX_CMDLINE0 \
	"mem=256M mem=256M@0x30000000 console=ttyS3,9600n8 ip=192.168.4.58:192.168.3.56:192.168.1.1:255.255.248.0 nfsroot=192.168.3.56:/nfsroot/root26/home/xlsu/fpaga_root rw"
#endif
#define LINUX_CMDLINE1 \
	"mem=256M console=ttyS3,57600n8 ip=off root=/dev/ram0 rw rdinit=/linuxrc"
//	"mem=256M console=ttyS0,57600n8 ip=off root=/dev/ram0 rw rdinit=/linuxrc"
#define LINUX_CMDLINE2 \
	"mem=255M mem=256M@0x30000000 console=ttyS3,57600n8 ubi.mtd=2 root=ubi0:ubifs rootfstype=ubifs rw"
//	"mem=255M console=ttyS3,57600n8 ubi.mtd=2 root=ubi0:ubifs rootfstype=ubifs rw"
#define LINUX_CMDLINE3 \
	"mem=255M mem=256M@0x30000000 console=ttyS0,57600n8 ip=off rootfstype=yaffs2 root=/dev/mtdblock2 rw"

#if defined(CONFIG_NAND_U_BOOT)
#define CONFIG_BOOTARGS		LINUX_CMDLINE2
#if (CFG_NAND_PAGE_SIZE < 8192 )
#define CONFIG_BOOTCOMMAND	"nand read 0x80600000 0x400000 0x400000;bootm"
#else
#define CONFIG_BOOTCOMMAND	"nand read 0x80600000 0x800000 0x400000;bootm"
#endif
#elif defined(CONFIG_MSC_U_BOOT)
#define CONFIG_BOOTARGS		LINUX_CMDLINE1
#define CONFIG_BOOTCOMMAND	"msc read 0x80600000 0x400000 0x500000;bootm"
#else
#define CONFIG_BOOTARGS		LINUX_CMDLINE0
#define CONFIG_BOOTCOMMAND	"tftpboot;bootm"
#endif

#define CFG_AUTOLOAD		"n"		/* No autoload */

#define CONFIG_SERVERIP		192.168.3.56
#define CONFIG_IPADDR		192.168.4.58
#define CONFIG_NET_MULTI
#define CONFIG_ETHADDR		00:2a:c6:2c:bd:fc    /* Ethernet address */

/*
 * Serial download configuration
 *
 */
#define CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory      */
#define	CFG_PROMPT		"F4780 # "	/* Monitor Command Prompt    */
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size   */
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16)  /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args*/

#define CFG_MALLOC_LEN		896*1024
#define CFG_BOOTPARAMS_LEN	128*1024

#define CFG_SDRAM_BASE		0x80000000     /* Cached addr */

#define CFG_INIT_SP_OFFSET	0x400000

#define	CFG_LOAD_ADDR		0x80600000     /* default load address	*/

#define CFG_MEMTEST_START	0x80100000
#define CFG_MEMTEST_END		0x80800000

#define CFG_RX_ETH_BUFFER	16	/* use 16 rx buffers on jz47xx eth */

/*
 * Configurable options for zImage if SPL is to load zImage instead of u-boot.
 */
#define CONFIG_LOAD_UBOOT       /* If it's defined, then spl load u-boot instead of zImage, and following options isn't used */
#define PARAM_BASE		0x80004000      /* The base of parameters which will be sent to kernel zImage */
#define CFG_ZIMAGE_SIZE	        (2 << 20)	/* Size of kernel zImage */
#define CFG_ZIMAGE_DST	        0x80100000	/* Load kernel zImage to this addr */
#define CFG_ZIMAGE_START	CFG_ZIMAGE_DST	/* Start kernel zImage from this addr	*/
#define CFG_CMDLINE		CONFIG_BOOTARGS
#define CFG_NAND_ZIMAGE_OFFS	(CFG_NAND_BLOCK_SIZE*4) /* NAND offset of zImage being loaded */
#define CFG_SPI_ZIMAGE_OFFS	(256 << 10) /* NAND offset of zImage being loaded */

/*-----------------------------------------------------------------------
 * Environment
 *----------------------------------------------------------------------*/
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL) && !defined(CONFIG_MSC_U_BOOT) && !defined(CONFIG_MSC_SPL)
#define CFG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars	*/
#elif defined(CONFIG_MSC_U_BOOT)
#define CFG_ENV_IS_IN_MSC	1	/* use MSC  for environment vars	*/
#else /* CONFIG_NAND_U_BOOT */
#define CFG_ENV_IS_IN_NAND	1	/* use NAND for environment vars	*/
#endif


/*-----------------------------------------------------------------------
 * NAND FLASH configuration
 */
#undef CFG_NAND_BCH_BIT
#define CFG_NAND_BCH_BIT	8	/* Specify the hardware BCH algorithm for 4780 (4|8) */
#define CFG_NAND_ECC_POS	24	/* Ecc offset position in oob area, its default value is 3 if it isn't defined. */
//#define CFG_NAND_SMCR1		0x13444400      /* 0x0fff7700 is slowest */
#define CFG_NAND_SMCR1		0x0fff7700      /* 0x0fff7700 is slower */
//#define CFG_NAND_SMCR1		0x1fffff00      /* 0x0fff7700 is slowest */
#define CFG_NAND_TGCR1		0x13333407
//#define CFG_NAND_TGCR1		0x0FFF770A	/* slower */
//#define CFG_NAND_TOGGLE_RDQS	0x9	/* Toggle NAND DQSdelay use user's set */
#define CFG_NAND_USE_PN		0	/* Use PN in jz4780 for TLC NAND */
#define CFG_NAND_BACKUP_NUM	1	/* TODO */
#define CONFIG_NAND_FREE	1	/* use NAND freesize */

#define CFG_MAX_NAND_DEVICE     1
#define NAND_MAX_CHIPS          1
#define CFG_NAND_BASE           0xBB000000
#define NAND_ADDR_OFFSET        0x00800000
#define NAND_CMD_OFFSET         0x00400000
#define CFG_NAND_SELECT_DEVICE  1       /* nand driver supports mutipl. chips   */

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 8k from NAND (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (NUB) from NAND and execute it. This SPL
 * has to fit into 8kByte. It sets up the CPU and configures the SDRAM
 * controller and the NAND controller so that the special U-Boot image can be
 * loaded from NAND to SDRAM.
 *
 * NUB (NAND U-Boot)
 * This NAND U-Boot (NUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 */
#define CFG_NAND_U_BOOT_DST	0xa0100000	/* Load NUB to this addr	*/
#define CFG_NAND_U_BOOT_START	0x80100000 //CFG_NAND_U_BOOT_DST /* Start NUB from this addr	*/

/*
 * Define the partitioning of the NAND chip (only RAM U-Boot is needed here)
 */
#define CFG_NAND_U_BOOT_OFFS	(CFG_NAND_BLOCK_SIZE * (CFG_NAND_BACKUP_NUM + 1))	/* Offset to U-Boot image */

/* Size of U-Boot image */
#define CFG_NAND_U_BOOT_SIZE	(512 << 10)

#ifdef CFG_ENV_IS_IN_NAND
#define CFG_ENV_SIZE		0x10000
//#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_NAND_U_BOOT_SIZE)	/* environment starts here  */
#define CFG_ENV_OFFSET		(CFG_NAND_U_BOOT_OFFS + CFG_NAND_BLOCK_SIZE)	/* environment starts here  */
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET + CFG_NAND_BLOCK_SIZE)
#endif

/*
 * IPL (Initial Program Loader, integrated inside CPU)
 * Will load first 8k from MSC (SPL) into cache and execute it from there.
 *
 * SPL (Secondary Program Loader)
 * Will load special U-Boot version (MSUB) from MSC and execute it. This SPL
 * has to fit into 8kByte. It sets up the CPU and configures the SDRAM
 * controller and the MSC controller so that the special U-Boot image can be
 * loaded from MSC to SDRAM.
 *
 * MSUB (MMC/SD U-Boot)
 * This MSC U-Boot (MSUB) is a special U-Boot version which can be started
 * from RAM. Therefore it mustn't (re-)configure the SDRAM controller.
 *
 */
#define CFG_MSC_U_BOOT_DST	0x80100000	/* Load MSUB to this addr	 */
#define CFG_MSC_U_BOOT_START	CFG_MSC_U_BOOT_DST /* Start MSUB from this addr */

/*
 * Define the partitioning of the MMC/SD card (only RAM U-Boot is needed here)
 */
#define CFG_MSC_U_BOOT_OFFS	(16 << 10)	/* Offset to RAM U-Boot image	*/
#define CFG_MSC_U_BOOT_SIZE	(275 << 10)	/* Size of RAM U-Boot image	*/

#define CFG_MSC_BLOCK_SIZE	512

#ifdef CFG_ENV_IS_IN_MSC
#define CFG_ENV_SIZE		CFG_MSC_BLOCK_SIZE
#define CFG_ENV_OFFSET		((CFG_MSC_BLOCK_SIZE * 16) + CFG_MSC_U_BOOT_SIZE + (CFG_MSC_BLOCK_SIZE * 16))	/* environment starts here  */
#endif

/*-----------------------------------------------------------------------
 * SPI NOR FLASH configuration
 */
#define CFG_SPI_MAX_FREQ        1000000
#define CFG_SPI_U_BOOT_DST	0x80100000	/* Load NUB to this addr	*/
#define CFG_SPI_U_BOOT_START	CFG_SPI_U_BOOT_DST
#define CFG_SPI_U_BOOT_OFFS     (14 << 10)
#define CFG_SPI_U_BOOT_SIZE	(256 << 10)

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(128)	/* max number of sectors on one chip */

#define PHYS_FLASH_1		0xB8000000 /* Flash Bank #1 */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(256*1024)  /* Reserve 256 kB for Monitor */

#define CFG_FLASH_BASE		PHYS_FLASH_1
/* Environment settings */
#ifdef CFG_ENV_IS_IN_FLASH

#define CFG_ENV_SECT_SIZE	0x20000 /* Total Size of Environment Sector */
#define CFG_ENV_SIZE		CFG_ENV_SECT_SIZE
#endif
#define CFG_ENV_ADDR		0xB8040000

#define CFG_DIRECT_FLASH_TFTP	1	/* allow direct tftp to flash */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
/*
#define CFG_DCACHE_SIZE		16384
#define CFG_ICACHE_SIZE		16384
#define CFG_CACHELINE_SIZE	32
*/
#define CFG_DCACHE_SIZE		(32*1024)
#define CFG_ICACHE_SIZE		(32*1024)
#define CFG_CACHELINE_SIZE	32

/*======================================================================
 * GPIO
 */
#define GPIO_LCD_PWM   		(32*2+14) /* GPE14 PWM4 */

/*-----------------------------------------------------------------------
 * Partition info
 */

#if defined(CONFIG_NAND_U_BOOT)
/*
 * Nand Partition info
 */
 /*======== Partition size ============ */
#define PTN_UBOOT_SIZE				(3* 0x100000)
#define PTN_MISC_SIZE               (1* 0x100000)
#define PTN_KERNEL_SIZE             (4* 0x100000)
#define PTN_RECOVERY_SIZE           (4* 0x100000)
#define PTN_SYSTEM_SIZE             (100* 0x100000)
#define PTN_USERDATA_SIZE           (256* 0x100000)
#define PTN_CACHE_SIZE              (32* 0x100000)  /* optional */
/*======== Partition offset ============ */
#define PTN_UBOOT_OFFSET			(0)
#define PTN_MISC_OFFSET             (PTN_UBOOT_OFFSET+ PTN_UBOOT_SIZE)
#define PTN_KERNEL_OFFSET           (PTN_MISC_OFFSET + PTN_MISC_SIZE)
#define PTN_RECOVERY_OFFSET       	(PTN_KERNEL_OFFSET + PTN_KERNEL_SIZE)
#define PTN_SYSTEM_OFFSET         	(PTN_RECOVERY_OFFSET+PTN_RECOVERY_SIZE)
#define PTN_USERDATA_OFFSET			(PTN_SYSTEM_OFFSET+PTN_SYSTEM_SIZE)
#define PTN_CACHE_OFFSET			(PTN_USERDATA_OFFSET +PTN_USERDATA_SIZE)  /* optional */

#elif defined(CONFIG_MSC_U_BOOT)
/*
 * MSC Partition info
 */
/*======== Partition size ============ */
#define PTN_UBOOT_SIZE				(3* 0x100000)	/*include MBR(512bytes) before u-boot*/
#define PTN_MISC_SIZE               (1* 0x100000)
#define PTN_KERNEL_SIZE             (4* 0x100000)
#define PTN_RECOVERY_SIZE           (4* 0x100000)
#define PTN_SYSTEM_SIZE             (256* 0x100000)
#define PTN_USERDATA_SIZE           (500* 0x100000)
#define PTN_CACHE_SIZE              (32* 0x100000)	/* optional */
/*======== Partition offset ============ */
#define PTN_UBOOT_OFFSET			(0)		/* 0 is MBR offset,MBR in combination with UBOOT */
#define PTN_MISC_OFFSET             (PTN_UBOOT_OFFSET+ PTN_UBOOT_SIZE)
#define PTN_KERNEL_OFFSET           (PTN_MISC_OFFSET + PTN_MISC_SIZE)
#define PTN_RECOVERY_OFFSET       	(PTN_KERNEL_OFFSET + PTN_KERNEL_SIZE)
#define PTN_SYSTEM_OFFSET         	(PTN_RECOVERY_OFFSET+PTN_RECOVERY_SIZE)
#define PTN_USERDATA_OFFSET			(PTN_SYSTEM_OFFSET+PTN_SYSTEM_SIZE)  
#define PTN_CACHE_OFFSET			(PTN_USERDATA_OFFSET +PTN_USERDATA_SIZE)  /* optional */

/*-----------------------------------------------------------------------
 * MBR Partition info
 */
#define JZ_MBR_TABLE		/* configure the MBR below if JZ_MBR_TABLE defined*/
#define LINUX_FS_TYPE	0x83
#define VFAT_FS_TYPE	0x0B
/*======== Partition table ============ */
#define MBR_P1_OFFSET 	PTN_SYSTEM_OFFSET
#define MBR_P1_SIZE 	PTN_SYSTEM_SIZE
#define MBR_P1_TYPE 	LINUX_FS_TYPE

#define MBR_P2_OFFSET 	PTN_CACHE_OFFSET
#define MBR_P2_SIZE 	PTN_CACHE_SIZE
#define MBR_P2_TYPE 	LINUX_FS_TYPE

#define MBR_P3_OFFSET 	0x3a400000
#define MBR_P3_SIZE 	0x2000000
#define MBR_P3_TYPE 	LINUX_FS_TYPE

#define MBR_P4_OFFSET 	0x40000000
#define MBR_P4_SIZE 	0xa8c00000
#define MBR_P4_TYPE 	VFAT_FS_TYPE

#endif

#endif
