/*
 * Jz4760 common routines
 *
 *  Copyright (c) 2006
 *  Ingenic Semiconductor, <cwjia@ingenic.cn>
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

#include <config.h>
#include <asm/mipsregs.h>


#include <common.h>
#include <command.h>
#include <asm/jzsoc.h>
/* #include <asm/jz4775_pll.h> */

#undef DEBUG
#ifdef DEBUG
#define dprintf(fmt,args...)	printf(fmt, ##args)
#else
#define dprintf(fmt,args...)
#endif
extern void board_early_init(void);
extern void sdram_init(void);
extern void serial_put_hex(unsigned int  d);

void jzmemset(void *dest,int ch,int len)
{
	unsigned int *d = (unsigned int *)dest;
	int i;
	int wd;

	wd = (ch << 24) | (ch << 16) | (ch << 8) | ch;

	for(i = 0;i < len / 32;i++)
	{
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
		*d++ = wd;
	}
}

#ifndef CONFIG_FPGA
/*
 * M = PLLM * 2, N = PLLN
 * NO = 2 ^ OD
 */
void apll_init(void)
{
	/** divisors,
	 *  for jz4775 ,P:H2:H0:L2:C
	 *  DIV should be one of [1, 2, 3, 4, 5, 6, ..., 14]
	 */
	int div[5] = {8, 4, 4, 2, 1};
	register unsigned int clk_ctrl;

	/* Init APLL */
//	serial_puts("CPAPCR = ");
//	serial_put_hex(CPAPCR_M_N_OD | CPM_CPAPCR_EN | 0x20);

	REG_CPM_CPAPCR = CPAPCR_M_N_OD | CPM_CPAPCR_EN | 0x20;
	while (!(REG_CPM_CPAPCR & CPM_CPAPCR_ON)) ;

	/* init CPU, L2CACHE, AHB0, AHB2, APB clock */
	clk_ctrl = REG_CPM_CPCCR & (0xff << 24);
	clk_ctrl |= (CPM_CPCCR_CE_CPU | CPM_CPCCR_CE_AHB0 | CPM_CPCCR_CE_AHB2);
	clk_ctrl |= ((div[0] - 1) << CPM_CPCCR_PDIV_BIT)  |
		   ((div[1] - 1) << CPM_CPCCR_H2DIV_BIT) |
		   ((div[2] - 1) << CPM_CPCCR_H0DIV_BIT) |
		   ((div[3] - 1) << CPM_CPCCR_L2DIV_BIT) |
		   ((div[4] - 1) << CPM_CPCCR_CDIV_BIT);

	REG_CPM_CPCCR = clk_ctrl;
	while (REG_CPM_CPCSR & (CPM_CPCSR_CDIV_BUSY | CPM_CPCSR_H0DIV_BUSY | CPM_CPCSR_H2DIV_BUSY)) ;

	clk_ctrl = REG_CPM_CPCCR;
	clk_ctrl &= ~(0xff << 24);

	clk_ctrl |= (CPM_SRC_SEL_APLL << CPM_CPCCR_SEL_SRC_BIT) | (CPM_PLL_SEL_SRC << CPM_CPCCR_SEL_CPLL_BIT) |
			(CPM_PLL_SEL_SRC << CPM_CPCCR_SEL_H0PLL_BIT) | (CPM_PLL_SEL_SRC << CPM_CPCCR_SEL_H2PLL_BIT);

	REG_CPM_CPCCR = clk_ctrl;
#if 0
	serial_puts("REG_CPM_CPCCR = ");
	serial_put_hex(REG_CPM_CPCCR);
	serial_puts("REG_CPM_CPAPCR = ");
	serial_put_hex(REG_CPM_CPAPCR);
#endif
}

void mpll_init(void)
{
	/* Init MPLL */
//	serial_puts("CPMPCR = ");
//	serial_put_hex(CPMPCR_M_N_OD | CPM_CPMPCR_EN);

	REG_CPM_CPMPCR = CPMPCR_M_N_OD | CPM_CPMPCR_EN;
	while (!(REG_CPM_CPMPCR & CPM_CPMPCR_ON)) ;

	/*
	 * Init DDR clock
	 */
	REG_CPM_DDCDR = CPM_DDRCDR_DCS_MPLL | CPM_DDRCDR_CE_DDR | (CFG_DDR_DIV - 1);
	/* REG_CPM_DDCDR = CPM_DDRCDR_DCS_SRC | CPM_DDRCDR_CE_DDR | (CFG_DDR_DIV - 1); */
	while (REG_CPM_DDCDR & CPM_DDRCDR_DDR_BUSY) ;
#if 0
	serial_puts("REG_CPM_CPMPCR = ");
	serial_put_hex(REG_CPM_CPMPCR);
	serial_puts("===> REG_CPM_DDCDR = ");
	serial_put_hex(REG_CPM_DDCDR);
	serial_puts("mpll init over\n");
#endif

	/*
	 * Reset DDR clock
	 */
	* (volatile unsigned *) 0xb00000d0 = 0x3;
	volatile unsigned tmp = 0xffff;
	while (tmp--) ;
//	serial_puts("rst over\n");
	* (volatile unsigned *) 0xb00000d0 = 0x1;
	tmp = 0xffff;
	while (tmp--) ;
//	serial_puts("===> 0xb00000d0=");
//	serial_put_hex(*(volatile unsigned *)0xb00000d0);
}

void core_voltage_regulate_act8600(int vol);
void core_voltage_regulate(int vol)
{
	/* TODO: this should be move to core_voltage.c in future. 2012-11-10  */
#ifdef CONFIG_ACT8600
	__gpio_as_output0(GPIO_SDA);
	__gpio_as_output0(GPIO_SCL);
	core_voltage_regulate_act8600(vol);
#else
#warning "core_voltage_regulate() else CONFIG_ACT8600"
#endif
}
void pll_init(void)
{
#ifdef CONFIG_ACT8600
	core_voltage_regulate(1250);
#endif

	mpll_init();
	apll_init();
}

void gpio_debug(int n)
{
	unsigned int p, o;

	p = (n) / 32;
	o = (n) % 32;

	REG_GPIO_PXINTC(p) = (1 << (o));
	REG_GPIO_PXMASKS(p) = (1 << (o));
	REG_GPIO_PXPAT1C(p) = (1 << (o));

	while (1) {
		REG_GPIO_PXPAT0C(p) = (1 << (o));
		REG_GPIO_PXPAT0S(p) = (1 << (o));
	}
}

#endif


//----------------------------------------------------------------------
// U-Boot common routines

#if !defined(CONFIG_NAND_SPL) && !defined(CONFIG_SPI_SPL) && !defined(CONFIG_MSC_SPL)
static void calc_clocks(void)
{
	DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_FPGA
	gd->cpu_clk = __cpm_get_cclk();
	gd->sys_clk = __cpm_get_hclk();
	gd->per_clk = __cpm_get_pclk();
	gd->mem_clk = __cpm_get_mclk();
	gd->dev_clk = CFG_EXTAL;
#else
	gd->cpu_clk = CFG_CPU_SPEED;
	gd->sys_clk = gd->per_clk = gd->dev_clk //= gd->mem_clk
		= CFG_EXTAL / CFG_DIV;
	gd->mem_clk = CFG_DDR_SPEED;
#endif
}
#ifndef CONFIG_FPGA
static void rtc_init(void)
{
#if 0
#define RTC_UNLOCK()			\
	do {					\
		while ( !__rtc_write_ready());	\
		__rtc_write_enable();		\
		while (!__rtc_write_enabled()) ;\
	} while (0)


	serial_puts("rtc_init ~~~~~~~~~~ ++\n");

	RTC_UNLOCK();

	__rtc_enable_alarm();	/* enable alarm */

	RTC_UNLOCK();

	REG_RTC_RGR   = 0x00007fff; /* type value */

	RTC_UNLOCK();

	REG_RTC_HWFCR = 0x0000ffe0; /* Power on delay 2s */

	RTC_UNLOCK();

	REG_RTC_HRCR  = 0x00000fe0; /* reset delay 125ms */

	serial_puts("rtc_init ~~~~~~~~~~ --\n");
#endif
}
#endif

//----------------------------------------------------------------------
// jz4760 board init routine

int jz_board_init(void)
{
	board_early_init();  /* init gpio, pll etc. */

#if !defined(CONFIG_FPGA) && !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_SPI_U_BOOT) && !defined(CONFIG_MSC_U_BOOT)
	pll_init();          /* init PLL, do it when nor boot or defined(CONFIG_MSC_U_BOOT) */
#endif

#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_SPI_U_BOOT) && !defined(CONFIG_MSC_U_BOOT)
	serial_init();
	sdram_init();        /* init sdram memory */
#endif

	calc_clocks();       /* calc the clocks */
#ifndef CONFIG_FPGA
	rtc_init();		/* init rtc on any reset: */
#endif
	return 0;
}

//----------------------------------------------------------------------
// Timer routines

#define TIMER_CHAN  0
#define TIMER_FDATA 0xffff  /* Timer full data value */
#define TIMER_HZ    CFG_HZ

#define READ_TIMER  REG_TCU_TCNT(TIMER_CHAN)  /* macro to read the 16 bit timer */

static ulong timestamp;
static ulong lastdec;

void	reset_timer_masked	(void);
ulong	get_timer_masked	(void);
void	udelay_masked		(unsigned long usec);

/*
 * timer without interrupts
 */

int timer_init(void)
{
	REG_TCU_TCSR(TIMER_CHAN) = TCU_TCSR_PRESCALE256 | TCU_TCSR_EXT_EN;
	REG_TCU_TCNT(TIMER_CHAN) = 0;
	REG_TCU_TDHR(TIMER_CHAN) = 0;
	REG_TCU_TDFR(TIMER_CHAN) = TIMER_FDATA;

	REG_TCU_TMSR = (1 << TIMER_CHAN) | (1 << (TIMER_CHAN + 16)); /* mask irqs */
	REG_TCU_TSCR = (1 << TIMER_CHAN); /* enable timer clock */
	REG_TCU_TESR = (1 << TIMER_CHAN); /* start counting up */

	lastdec = 0;
	timestamp = 0;

	return 0;
}

void reset_timer(void)
{
	reset_timer_masked ();
}

ulong get_timer(ulong base)
{
	return get_timer_masked () - base;
}

void set_timer(ulong t)
{
	timestamp = t;
}

void udelay (unsigned long usec)
{
	ulong tmo,tmp;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= TIMER_HZ;
		tmo /= 1000;
	}
	else {
		if (usec >= 1) {
			tmo = usec * TIMER_HZ;
			tmo /= (1000*1000);
		}
		else
			tmo = 1;
	}

	/* check for rollover during this delay */
	tmp = get_timer (0);
	if ((tmp + tmo) < tmp )
		reset_timer_masked();  /* timer would roll over */
	else
		tmo += tmp;

	while (get_timer_masked () < tmo);
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER;
	timestamp = 0;
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;

	if (lastdec <= now) {
		/* normal mode */
		timestamp += (now - lastdec);
	} else {
		/* we have an overflow ... */
		timestamp += TIMER_FDATA + now - lastdec;
	}
	lastdec = now;

	return timestamp;
}

void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	/* normalize */
	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= TIMER_HZ;
		tmo /= 1000;
	} else {
		if (usec > 1) {
			tmo = usec * TIMER_HZ;
			tmo /= (1000*1000);
		} else {
			tmo = 1;
		}
	}

	endtime = get_timer_masked () + tmo;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On MIPS it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On MIPS it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	return TIMER_HZ;
}
#else
void udelay (unsigned long usec)
{
	unsigned int i = usec * (CFG_CPU_SPEED / 2000000);

	__asm__ __volatile__ (
			"\t.set noreorder\n"
			"1:\n\t"
			"bne\t%0, $0, 1b\n\t"
			"addi\t%0, %0, -1\n\t"
			".set reorder\n"
			: "=r" (i)
			: "0" (i)
			);
}
void mdelay(unsigned long msec)
{
	unsigned int i;

	for (i = 0; i < msec; i++)
		udelay(1000);
}

#endif /* !defined(CONFIG_NAND_SPL) && !defined(CONFIG_SPI_SPL) && !defined(CONFIG_MSC_SPL) */

//---------------------------------------------------------------------
// End of timer routine.
//---------------------------------------------------------------------
