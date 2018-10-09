/* along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>

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

#if defined(CONFIG_JZ4810)
#include <asm/jz4810.h>
#endif

#ifndef CONFIG_JZ4780
void ddr_mem_init(int msel, int hl, int tsel, int arg);

void ddr_mem_init(int msel, int hl, int tsel, int arg)
{
	volatile int tmp_cnt;
	register unsigned int cpu_clk, ddr_twr;
	register unsigned int ddrc_cfg_reg = 0, init_ddrc_mdelay = 0;

	cpu_clk = CFG_CPU_SPEED;

#if defined(CONFIG_FPGA)

	ddrc_cfg_reg = arg << 30 | DDRC_CFG_TYPE_DDR2 | (DDR_ROW - 12) << 10
		| (DDR_COL-8) << 8 | DDR_CS1EN << 7 | DDR_CS0EN << 6
		| ((DDR_CL-1) | 0x8)<<2 | DDR_BANK8 << 1 | DDR_DW32;

#else /* if defined(CONFIG_FPGA) */

	ddrc_cfg_reg = DDRC_CFG_TYPE_DDR2 | (DDR_ROW - 12) << 10
		| (DDR_COL - 8) << 8 | DDR_CS1EN << 7 | DDR_CS0EN << 6
		| ((DDR_CL - 1) | 0x8) << 2 | DDR_BANK8 << 1 | DDR_DW32;

#endif /* if defined(CONFIG_FPGA) */

	ddrc_cfg_reg |= DDRC_CFG_MPRT;

#if defined(CONFIG_FPGA)
	init_ddrc_mdelay = tsel << 18 | msel << 16 | hl << 15;
#else
	init_ddrc_mdelay = tsel << 18 | msel << 16 | hl << 15 | arg << 14;
#endif
	ddr_twr = ((REG_DDRC_TIMING1 & DDRC_TIMING1_TWR_MASK) >> DDRC_TIMING1_TWR_BIT) + 1;
	REG_DDRC_CFG = ddrc_cfg_reg;
	REG_DDRC_MDELAY = init_ddrc_mdelay | DDRC_MDELAY_MAUTO;

	/***** init ddrc registers & ddr memory regs ****/
	/* Wait for number of auto-refresh cycles */
	tmp_cnt = (cpu_clk / 1000000) * 10;
	while (tmp_cnt--);

	/* Set CKE High */
	REG_DDRC_CTRL = DDRC_CTRL_CKE; // ?

	/* Wait for number of auto-refresh cycles */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* PREA */
	REG_DDRC_LMR =  DDRC_LMR_CMD_PREC | DDRC_LMR_START; //0x1;

	/* Wait for DDR_tRP */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* EMR2: extend mode register2 */
	REG_DDRC_LMR = DDRC_LMR_BA_EMRS2 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;//0x221;

	/* EMR3: extend mode register3 */
	REG_DDRC_LMR = DDRC_LMR_BA_EMRS3 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;//0x321;

	/* EMR1: extend mode register1 */
#if defined(CONFIG_DDR2_DIFFERENTIAL)
	REG_DDRC_LMR = ((DDR_EMRS1_DIC_HALF) << 16) | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#else
	REG_DDRC_LMR = ((DDR_EMRS1_DIC_NORMAL | DDR_EMRS1_DQS_DIS) << 16) | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#endif
	/* wait DDR_tMRD */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* MR - DLL Reset A1A0 burst 2 */
	REG_DDRC_LMR = ((ddr_twr - 1) << 9 | DDR2_MRS_DLL_RST | DDR_CL << 4 | DDR_MRS_BL_4) << 16
		| DDRC_LMR_BA_MRS | DDRC_LMR_CMD_LMR | DDRC_LMR_START;

	/* wait DDR_tMRD */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* PREA */
	REG_DDRC_LMR =  DDRC_LMR_CMD_PREC | DDRC_LMR_START; //0x1;

	/* Wait for DDR_tRP */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* AR: auto refresh */
	REG_DDRC_LMR = DDRC_LMR_CMD_AUREF | DDRC_LMR_START; //0x11;
	/* Wait for DDR_tRP */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	REG_DDRC_LMR = DDRC_LMR_CMD_AUREF | DDRC_LMR_START; //0x11;

	/* Wait for DDR_tRP */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* MR - DLL Reset End */
	REG_DDRC_LMR = ((ddr_twr-1)<<9 | DDR_CL<<4 | DDR_MRS_BL_4)<< 16
		| DDRC_LMR_BA_MRS | DDRC_LMR_CMD_LMR | DDRC_LMR_START;

	/* wait 200 tCK */
	tmp_cnt = (cpu_clk / 1000000) * 2;
	while (tmp_cnt--);

	/* EMR1 - OCD Default */
#if defined(CONFIG_DDR2_DIFFERENTIAL)
	REG_DDRC_LMR = (DDR_EMRS1_DIC_HALF | DDR_EMRS1_OCD_DFLT) << 16 | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#else
	REG_DDRC_LMR = (DDR_EMRS1_DIC_NORMAL | DDR_EMRS1_DQS_DIS | DDR_EMRS1_OCD_DFLT) << 16 | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#endif


	/* EMR1 - OCD Exit */
#if defined(CONFIG_DDR2_DIFFERENTIAL)
	REG_DDRC_LMR = ((DDR_EMRS1_DIC_HALF) << 16) | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#else
	REG_DDRC_LMR = ((DDR_EMRS1_DIC_NORMAL | DDR_EMRS1_DQS_DIS) << 16) | DDRC_LMR_BA_EMRS1 | DDRC_LMR_CMD_LMR | DDRC_LMR_START;
#endif

	/* wait DDR_tMRD */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

}

#else	/* CONFIG_JZ4780*/
void ddr_cfg_init(void)
{
	register unsigned int ddrc_cfg_reg = 0, tmp;

	tmp = DDR_CL - 1;
	if (tmp < 0)
		tmp = 0;
	if (tmp > 4)
		tmp = 4;
	ddrc_cfg_reg = DDRC_CFG_TYPE_DDR2 | DDRC_CFG_IMBA | DDR_DW32 | DDRC_CFG_ODT_EN | DDRC_CFG_MPRT | (tmp | 0x8) << 2
		| (DDR_ROW - 12) << 11  | (DDR_COL - 8) << 8   | DDR_CS0EN << 6 | DDR_BANK8 << 1
		| (DDR_ROW - 12) << 27 | (DDR_COL - 8) << 24 | DDR_CS1EN << 7 | DDR_BANK8 << 23 ;
	if (DDR_BL > 4)
		ddrc_cfg_reg |= 1 << 21;

	REG_DDRC_CFG = ddrc_cfg_reg;
}

#define DDRP_PTR0_tDLLSRST 	50	// 50ns
#define DDRP_tDLLLOCK 		5120	// 5.12us
#define DDRP_PTR0_ITMSRST_8 	8		// 8tck
#define DDRP_PTR1_DINIT0_DDR2	200 * 1000	//200us
#define DDRP_PTR1_DINIT1_DDR2	400	 	//400ns
#define DDRP_PTR2_DINIT2_DDR2	100 * 1000	// reverse
#define DDRP_PTR2_DINIT3_DDR2	100 * 1000	// reverse
void ddr_phy_init(unsigned long ps, unsigned int dtpr0_reg)
{
	register unsigned int tmp, tmp1;
	unsigned int ptr0_reg = 0, ptr1_reg = 0,  ptr2_reg = 0, dtpr1_reg = 0, dtpr2_reg = 0;
	unsigned int count = 0, i = 0, ck = 0;

	REG_DDRP_DCR = DDRP_DCR_TYPE_DDR2 | (DDR_BANK8 << 3);

	tmp = DDR_GET_VALUE(DDR_tWR, ps);
	if (tmp < 2)
		tmp = 2;
	if (tmp > 6)
		tmp = 6;
	REG_DDRP_MR0 = (tmp - 1) << 9 | DDR_CL << 4 | ((DDR_BL == 4) ? 2 : 3); // BL 8 = 11, 0xa53
	REG_DDRP_MR1 = DDR_EMRS1_DLL_EN | DDR_EMRS1_DIC_HALF | DDR_EMRS1_RTT_75; // 0x4

	/* DLL Soft Rest time */
	tmp = DDR_GET_VALUE(DDRP_PTR0_tDLLSRST, ps);
	if (tmp > 63)
		tmp = 63;
	ptr0_reg |= tmp;
	/* DLL Lock time */
	tmp = DDR_GET_VALUE(DDRP_tDLLLOCK, ps);
	if (tmp > 0xfff)
		tmp = 0xfff;
	ptr0_reg |= tmp << 6;
	ptr0_reg |= DDRP_PTR0_ITMSRST_8 << 18 ;
	REG_DDRP_PTR0 = ptr0_reg;

	tmp = DDR_GET_VALUE(DDRP_PTR1_DINIT0_DDR2, ps);
	if (tmp > 0x7ffff)
		tmp = 0x7ffff;
	ptr1_reg |= tmp;
	tmp = DDR_GET_VALUE(DDRP_PTR1_DINIT1_DDR2, ps / 1000);
	if (tmp > 0xff)
		tmp = 0xff;
	ptr1_reg |= tmp << 19;
	REG_DDRP_PTR1 = ptr1_reg;

#if 0
	tmp = DDR_GET_VALUE(DDRP_PTR2_DINIT2_DDR2, ps);
	if (tmp > 0x1ffff)
		tmp = 0x1ffff;
	ptr2_reg |= tmp;
	tmp = DDR_GET_VALUE(DDRP_PTR2_DINIT3_DDR2, ps);
	if (tmp > 0x3ff)
		tmp = 0x3ff;
	ptr2_reg |= tmp << 17;
#endif

	dtpr0_reg |= (DDR_tMRD - 2);    // valid values: 2 - 3
	if (DDR_tCCD > (DDR_BL / 2))
		dtpr0_reg |= 1 << 31;
	REG_DDRP_DTPR0 = dtpr0_reg;

	tmp = DDR_GET_VALUE(DDR_tFAW, ps);
	if (tmp < 2) tmp = 2;
	if (tmp > 31) tmp = 31;
	dtpr1_reg |= tmp << 3;
	tmp = DDR_GET_VALUE(DDR_tRFC, ps);
	if (tmp > 255) tmp = 255;
	dtpr1_reg |= tmp << 16;
	REG_DDRP_DTPR1 = dtpr1_reg;

	tmp = DDR_GET_VALUE(DDR_tXSNR, ps);
	tmp1 = DDR_GET_VALUE(DDR_tXRD * ps, ps);
	tmp = (tmp > tmp1) ? tmp : tmp1;
	if (tmp > 1023) tmp = 1023;
	dtpr2_reg |= tmp;

	tmp = DDR_GET_VALUE(DDR_tXARD, ps);
	tmp1 = DDR_GET_VALUE(DDR_tARDS, ps);
	tmp = (tmp > tmp1) ? tmp : tmp1;
	tmp1 = DDR_GET_VALUE(DDR_tXP, ps);
	tmp = (tmp > tmp1) ? tmp : tmp1;
	if (tmp > 31) tmp = 31;
	dtpr2_reg |= tmp << 10;
	tmp = DDR_tCKE;
	if (tmp > 15) tmp = 15;
	dtpr2_reg |= tmp << 15;
	tmp = DDR_tDLLLOCK;
	if (tmp > 1023) tmp = 1023;
	dtpr2_reg |= tmp << 19;
	REG_DDRP_DTPR2 = dtpr2_reg;

	REG_DDRP_PGCR = DDRP_PGCR_DQSCFG | 7 << DDRP_PGCR_CKEN_BIT | 2 << DDRP_PGCR_CKDV_BIT | (DDR_CS0EN | DDR_CS1EN << 1)
			<< DDRP_PGCR_RANKEN_BIT | DDRP_PGCR_ZCKSEL_32 | DDRP_PGCR_PDDISDX;

	serial_puts("DDR2 Init PHY\n");
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE)) {
		if (REG_DDRP_PGSR == 0x1f)
			break;
		if (count++ == 3000) {
			serial_puts("Init PHY: PHY INIT\n");
			serial_puts("REG_DDP_PGSR: ");
			serial_put_hex(REG_DDRP_PGSR);
			while (1) ;
		}
	}

	count = 0;
	REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DRAMINT;
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE)) {
		if (REG_DDRP_PGSR == 0x1f)
			break;
		if (count++ == 3000) {
			serial_puts("Init PHY: DDR INIT DONE\n");
			serial_puts("REG_DDP_PGSR: ");
			serial_put_hex(REG_DDRP_PGSR);
			while (1) ;
		}
	}

	REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_QSTRN;
	count = 0;
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE)) {
		if (count++ == 50000) {
			serial_puts("Init PHY: DDR TRAIN DONE\n");
			serial_puts("REG_DDP_PGSR: ");
			serial_put_hex(REG_DDRP_PGSR);
			for (i = 0; i < 4; i++) {
				serial_puts("REG_DDP_GXnGSR: ");
				serial_put_hex(REG_DDRP_DXGSR0(i));
			}
			if (REG_DDRP_PGSR & (DDRP_PGSR_DTDONE | DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR)) {
				serial_puts("REG_DDP_PGSR: ");
				serial_put_hex(REG_DDRP_PGSR);
				while (1) ;
			}
			count = 0;
		}
	}
}
#endif
