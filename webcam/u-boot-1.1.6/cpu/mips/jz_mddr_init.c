/* along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/jzsoc.h>

#if !defined(CONFIG_JZ4780) && !defined(CONFIG_JZ4775)
void ddr_mem_init(int msel, int hl, int tsel, int arg);

void ddr_mem_init(int msel, int hl, int tsel, int arg)
{
	volatile int tmp_cnt;
	register unsigned int cpu_clk, ddr_twr;
	register unsigned int ddrc_cfg_reg=0, init_ddrc_mdelay=0;

	cpu_clk = CFG_CPU_SPEED;

#if defined(CONFIG_FPGA)

	ddrc_cfg_reg = arg << 30 | DDRC_CFG_BTRUN | DDRC_CFG_TYPE_MDDR				//??? 30bit convert
		| (DDR_ROW - 12) << 10 | (DDR_COL - 8) << 8 | DDR_CS1EN << 7 | DDR_CS0EN << 6
		| ((DDR_CL - 1) | 0x8) << 2 | DDR_BANK8 << 1 | DDR_DW32;

#else /* CONFIG_FPGA */

//	ddrc_cfg_reg = DDRC_CFG_BTRUN | DDRC_CFG_TYPE_MDDR
	ddrc_cfg_reg = DDRC_CFG_TYPE_MDDR
		| (DDR_ROW - 12) << 10 | (DDR_COL - 8) << 8 | DDR_CS1EN << 7 | DDR_CS0EN << 6
		| ((DDR_CL - 1) | 0x8) << 2 | DDR_BANK8 << 1 | DDR_DW32;

#endif /* CONFIG_FPGA */

	ddrc_cfg_reg |= DDRC_CFG_MPRT;
#if defined(CONFIG_FPGA)
	init_ddrc_mdelay = tsel << 18 | msel << 16 | hl << 15;
#else
	init_ddrc_mdelay = tsel << 18 | msel << 16 | hl << 15 | arg << 14;
#endif
	ddr_twr = ((REG_DDRC_TIMING1 & DDRC_TIMING1_TWR_MASK) >> DDRC_TIMING1_TWR_BIT) + 1;
	REG_DDRC_CFG     = ddrc_cfg_reg;
//	REG_DDRC_CFG     = 0x0000b5e9;
//	REG_DDRC_CFG     &= ~(1 << 21);
	REG_DDRC_MDELAY = init_ddrc_mdelay | DDRC_MDELAY_MAUTO;
	/***** init ddrc registers & ddr memory regs ****/

	/* Wait for number of auto-refresh cycles */
	tmp_cnt = (cpu_clk / 1000000) * 10;
	while (tmp_cnt--);

	REG_DDRC_CTRL = DDRC_CTRL_CKE; // auto-refresh , otherwise self-refresh

	/* Wait for number of auto-refresh cycles */
	tmp_cnt = (cpu_clk / 1000000) * 20;
	while (tmp_cnt--);

	/* PREA */
	REG_DDRC_LMR =  DDRC_LMR_CMD_PREC | DDRC_LMR_START; //0x1;

	/* Wait for DDR_tRP */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* AR: auto refresh */
	REG_DDRC_LMR = DDRC_LMR_CMD_AUREF | DDRC_LMR_START; //0x11;

	/* wait DDR_tRFC */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* AR: auto refresh */
	REG_DDRC_LMR = DDRC_LMR_CMD_AUREF | DDRC_LMR_START; //0x11;
	/* wait DDR_tRFC */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* MR */
	REG_DDRC_LMR = (DDR_CL<<4 | DDR_MRS_BL_4)<< 16
		| DDRC_LMR_BA_M_MRS | DDRC_LMR_CMD_LMR | DDRC_LMR_START;

	/* wait DDR_tMRD */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);

	/* EMR: extend mode register */
	REG_DDRC_LMR = (DDR_EMRS_DS_FULL | DDR_EMRS_PRSR_ALL) << 16
		| DDRC_LMR_BA_M_EMRS | DDRC_LMR_CMD_LMR | DDRC_LMR_START;

	/* wait DDR_tMRD */
	tmp_cnt = (cpu_clk / 1000000) * 1;
	while (tmp_cnt--);
}

#else	/* CONFIG_JZ4780 */
/* new add */
void ddr_cfg_init(void)
{
	register unsigned int ddrc_cfg_reg = 0, tmp;

	tmp = DDR_CL - 1;
	if (tmp < 0)
		tmp = 0;
	if (tmp > 4)
		tmp = 4;
	ddrc_cfg_reg = DDRC_CFG_TYPE_MDDR | DDRC_CFG_IMBA | DDR_DW32 | DDRC_CFG_MPRT | (tmp | 0x8) << 2
		| (DDR_ROW - 12) << 11  | (DDR_COL - 8) << 8   | DDR_CS0EN << 6 | DDR_BANK8 << 1
		| (DDR_ROW - 12) << 27 | (DDR_COL - 8) << 24 | DDR_CS1EN << 7 | DDR_BANK8 << 23 ;
	if (DDR_BL > 4)
		ddrc_cfg_reg |= 1 << 21;

	REG_DDRC_CFG = ddrc_cfg_reg;
}

#define DDRP_PTR0_tDLLSRST 	50	// 50ns
#define DDRP_tDLLLOCK 		5120 	// 5.12us
#define DDRP_PTR0_ITMSRST_8 	8	// 8tck
#define DDRP_PTR1_DINIT0_MDDR	200 * 1000	//200us
#define DDRP_PTR1_DINIT1_MDDR	100	// reverse
#define DDRP_PTR2_DINIT2_MDDR	100	// reverse
#define DDRP_PTR2_DINIT3_MDDR	100	// reverse
void ddr_phy_init(unsigned long ps, unsigned int dtpr0_reg)
{
	register unsigned int tmp;
	unsigned int ptr0_reg = 0, ptr1_reg = 0, ptr2_reg = 0, dtpr1_reg = 0, dtpr2_reg = 0;
	unsigned int count = 0, i = 0, ck = 0;

	REG_DDRP_DCR = DDRP_DCR_TYPE_MDDR | (DDR_BANK8 << 3);	// 0x0

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

	tmp = DDR_GET_VALUE(DDRP_PTR1_DINIT0_MDDR, ps);
	if (tmp > 0x7ffff)
		tmp = 0x7ffff;
	ptr1_reg |= tmp;
	tmp = DDR_GET_VALUE(DDRP_PTR1_DINIT1_MDDR, ps);
	if (tmp > 0xff)
		tmp = 0xff;
	ptr1_reg |= tmp << 19;
	REG_DDRP_PTR1 = ptr1_reg;

	tmp = DDR_GET_VALUE(DDRP_PTR2_DINIT2_MDDR, ps);
	if (tmp > 0x1ffff)
		tmp = 0x1ffff;
	ptr2_reg |= tmp;
	tmp = DDR_GET_VALUE(DDRP_PTR2_DINIT3_MDDR, ps);
	if (tmp > 0x3ff)
		tmp = 0x3ff;
	ptr2_reg |= tmp << 17;
	REG_DDRP_PTR2 = ptr2_reg;

	REG_DDRP_MR0 = DDR_CL << 4 | 3; /* brust length must be 8 when training*/

	dtpr0_reg |= DDR_tMRD;    // valid values: 2 - 3
	if (DDR_tCCD > DDR_BL / 2)
		dtpr0_reg |= 1 << 31;
	REG_DDRP_DTPR0 = dtpr0_reg;

	tmp = DDR_GET_VALUE(DDR_tFAW, ps); //new time ,ddr3: unit - tCK
	if (tmp < 2) tmp = 2;
	if (tmp > 31) tmp = 31;
	dtpr1_reg |= tmp << 3;
	tmp = DDR_GET_VALUE(DDR_tRFC, ps);
	if (tmp > 255) tmp = 255;
	dtpr1_reg |= tmp << 16;
	REG_DDRP_DTPR1 = dtpr1_reg;

	tmp = DDR_GET_VALUE(DDR_tXS, ps);
	if (tmp < 2) tmp = 2;
	if (tmp > 0x3ff) tmp = 0x3ff;
	dtpr2_reg |= tmp;
	tmp = DDR_tXP;
	if (tmp < 2) tmp = 2;
	if (tmp > 0x1f) tmp = 0x1f;
	dtpr2_reg |= DDR_tXP << 10;
	tmp = DDR_tCKE;
	tmp = DDR_GET_VALUE(DDRP_tDLLLOCK, ps);
	if (tmp > 1023) tmp = 1023;
	dtpr2_reg |= tmp << 19;
	REG_DDRP_DTPR2 = dtpr2_reg;

	REG_DDRP_PGCR = DDRP_PGCR_ITMDMD | DDRP_PGCR_DQSCFG | 7 << DDRP_PGCR_CKEN_BIT | 2 << DDRP_PGCR_CKDV_BIT | (DDR_CS0EN | DDR_CS1EN << 1) << DDRP_PGCR_RANKEN_BIT | DDRP_PGCR_PDDISDX; // 0x18c2e03

	REG_DDRP_ACIOCR = 0x30c00813;
	REG_DDRP_DXCCR = 0x4802;

	serial_puts("MDDR Init PHY\n");
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE)) {
		if (REG_DDRP_PGSR == 0x1f)
			break;
	}

	/* REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DRAMINT | DDRP_PIR_DLLBYP; // 0x20041 */
	REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DRAMINT; // 0x20041
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE)) {
		if (REG_DDRP_PGSR == 0x1f)
			break;
	}

	/* REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_QSTRN | DDRP_PIR_DLLBYP; // 0x20085 */
	REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DLLLOCK | DDRP_PIR_QSTRN;
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE)) {
		if ((count++ == CFG_CPU_SPEED) || (REG_DDRP_PGSR & (DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR))) {
			serial_puts("Init PHY: DDR TRAIN DONE\n");
			serial_puts("REG_DDP_PGSR: ");
			serial_put_hex(REG_DDRP_PGSR);
			for (i = 0; i < 4; i++) {
				serial_puts("REG_DDP_GXnGSR: ");
				serial_put_hex(REG_DDRP_DXGSR0(i));
			}
			while (1);
		}
	}

	tmp = 1;
	while (DDR_BL >> tmp)
		tmp++;
	REG_DDRP_MR0 = DDR_CL << 4 | (tmp - 1); // use appropriate BL
	/* REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DRAMINT | DDRP_PIR_DLLBYP; // 0x20041 */
	REG_DDRP_PIR = DDRP_PIR_INIT | DDRP_PIR_DRAMINT; // 0x20041
	while (REG_DDRP_PGSR != (DDRP_PGSR_IDONE | DDRP_PGSR_DLDONE | DDRP_PGSR_ZCDONE | DDRP_PGSR_DIDONE | DDRP_PGSR_DTDONE))
		;
}
#endif
