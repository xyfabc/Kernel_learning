#ifndef __DDR2_CONFIG_H
#define __DDR2_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR2 info
 */
/* Chip Select */
#define DDR_CS1EN 1 // CSEN : whether a ddr chip exists 0 - un-used, 1 - used
#define DDR_CS0EN 1
#define DDR_DW32 1  /* 0 - 16-bit data width, 1 - 32-bit data width */

/* DDR2 paramters */
#define DDR_ROW  14  /* ROW : 12 to 14 row address */
#define DDR_COL  10  /* COL :  8 to 10 column address */
#define DDR_BANK8   1 /* Banks each chip: 0-4bank, 1-8bank */
#define DDR_CL   5  /* CAS latency: 1 to 7 */

/*
 * DDR2 controller timing1 register
 */
#define DDR_tRAS 45  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. 45-70000*/
#define DDR_tRTP 8   /* 7.5ns READ to PRECHARGE command period. */
#define DDR_tRP  13  /* tRP: PRECHARGE command period to the same bank */
#define DDR_tRCD 13  /* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC  58  /* ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD 8  /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR  15  /* WRITE Recovery Time defined by register MR of DDR2 memory */
#define DDR_tWTR 8   /* WRITE to READ command delay. */

/*
 * DDR2 controller timing2 register
 */
#define DDR_tRFC 128	/* ns,  AUTO-REFRESH command period. */
#define DDR_tMINSR 140	/* Minimum Self-Refresh / Deep-Power-Down , DDR2 no*/
#define DDR_tXP  2	/* tCK EXIT-POWER-DOWN to next valid command period: 1 to 8 tCK. */
#define DDR_tMRD 2	/* unit: tCK. Load-Mode-Register to next valid command period: DDR2 2 to 3 tCK */

/* new add */
#define DDR_BL	 8   /* Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/
#define DDR_tAL  0		/* Additive Latency, tCK*/
#define DDR_tRL (DDR_tAL + DDR_CL)	/* DDR2: Read Latency = tAL + tCL */
#define DDR_tWL (DDR_tRL - 1)		/* DDR2: Write Latency = tAL + tCL - 1*/
#define DDR_tCCD 2		/* CAS# to CAS# command delay , tCK*/
#define DDR_tRTW ((DDR_BL > 4) ? 6 : 4) /* 4 in case of BL=4, 6 in case of BL=8 */
#define DDR_tFAW 45		/* Four bank activate period, ns */
#define DDR_tCKE 3		/* CKE minimum pulse width, tCK */
#define DDR_tRDLAT	(DDR_tRL - 2)	
#define DDR_tWDLAT	(DDR_tWL - 1)
#define DDR_tXARD 2		/* DDR2 only: Exit active power down to read command , tCK*/
#define DDR_tXARDS (8 - DDR_tAL)/* DDR2 only: Exit active power down to read command (slow exit/low power mode), tCK */

#define DDR_tXSNR (DDR_tRFC + 10) 	/*DDR2 only: Exit self-refresh to a non-read command , ns */
#define DDR_tXSRD 	200	/* DDR2 only : Exit self-refresh to a read command , tCK */
#define DDR_tCKSRE  	10000 	/* DDR2 no:Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
#define DDR_tDLLLOCK	512	/* DDR2 no */	

/*
 * DDR2 controller refcnt register
 */
#define DDR_tREFI	7800	/* Refresh period: ns */

#define DDR_CLK_DIV 1    	/* Clock Divider. auto refresh
				 *	cnt_clk = memclk/(16*(2^DDR_CLK_DIV))
				 */

#endif /* __DDR2_CONFIG_H */
