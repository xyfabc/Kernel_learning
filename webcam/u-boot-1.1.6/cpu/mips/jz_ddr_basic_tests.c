/*
 * Jz Memory Base Test
 */

#include <config.h>
#include <common.h>
#include <asm/jzsoc.h>
#include <asm/mipsregs.h>

#undef DEBUG
/* #define DEBUG */

#ifdef DEBUG
#define dprintf(fmt,args...)	printf(fmt, ##args)
#else
#define dprintf(fmt,args...)	{}
#endif

#define DDR_DMA_BASE  (0xa0000000)		/*un-cached*/
#define LOW_MEM_SPACE_SIZE 0x10000000 		/* 256M */

#define ADDRLINE_TEST
#define DATALINE_TEST
#define CPU_TEST
#define DMA_TEST
#define CPU_DMA
#define REMAP_TEST

extern void remap_swap(int, int);
extern void flush_dcache_all(void);
extern void serial_puts (const char *s);
extern void serial_put_dec(unsigned int d);
extern void serial_put_hex(unsigned int  d);

#ifdef DEBUG
static int ddr_dma_test(int);
static void dump_jz_dma_channel(unsigned int dmanr)
{
	serial_puts("====================================================================\n");
	dprintf("DMA%d Registers:\n", dmanr);
	dprintf("  DMACR  = 0x%08x\n", REG_DMAC_DMACR(dmanr));
	dprintf("  DMACPR = 0x%08x\n", REG_DMAC_DMACPR(0));
	dprintf("  DSAR   = 0x%08x\n", REG_DMAC_DSAR(dmanr));
	dprintf("  DTAR   = 0x%08x\n", REG_DMAC_DTAR(dmanr));
	dprintf("  DTCR   = 0x%08x\n", REG_DMAC_DTCR(dmanr));
	dprintf("  DRSR   = 0x%08x\n", REG_DMAC_DRSR(dmanr));
	dprintf("  DCCSR  = 0x%08x\n", REG_DMAC_DCCSR(dmanr));
	dprintf("  DCMD  = 0x%08x\n", REG_DMAC_DCMD(dmanr));
	dprintf("  DDA  = 0x%08x\n", REG_DMAC_DDA(dmanr));
	dprintf("  DMADBR = 0x%08x\n", REG_DMAC_DMADBR(dmanr));
	serial_puts("====================================================================\n");
}

static void ddr_control_regs_print(void)
{
	serial_puts("====================================================================\n");
	dprintf("DDRC REGS:\n");
	dprintf("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
	dprintf("REG_DDRC_CFG \t\t= 0x%08x\n", REG_DDRC_CFG);
	dprintf("REG_DDRC_CTRL \t\t= 0x%08x\n", REG_DDRC_CTRL);
	dprintf("REG_DDRC_LMR \t\t= 0x%08x\n", REG_DDRC_LMR);
	dprintf("REG_DDRC_TIMING1 \t= 0x%08x\n", REG_DDRC_TIMING(1));
	dprintf("REG_DDRC_TIMING2 \t= 0x%08x\n", REG_DDRC_TIMING(2));
	dprintf("REG_DDRC_TIMING3 \t= 0x%08x\n", REG_DDRC_TIMING(3));
	dprintf("REG_DDRC_TIMING4 \t= 0x%08x\n", REG_DDRC_TIMING(4));
	dprintf("REG_DDRC_TIMING5 \t= 0x%08x\n", REG_DDRC_TIMING(5));
	dprintf("REG_DDRC_TIMING6 \t= 0x%08x\n", REG_DDRC_TIMING(6));
	dprintf("REG_DDRC_MMAP0 \t\t= 0x%08x\n", REG_DDRC_MMAP0);
	dprintf("REG_DDRC_MMAP1 \t\t= 0x%08x\n", REG_DDRC_MMAP1);
	dprintf("REG_DDRC_REFCNT \t= 0x%08x\n", REG_DDRC_REFCNT);
	dprintf("REG_DDRC_REMAP1 \t= 0x%08x\n", REG_DDRC_REMAP(1));
	dprintf("REG_DDRC_REMAP2 \t= 0x%08x\n", REG_DDRC_REMAP(2));
	dprintf("REG_DDRC_REMAP3 \t= 0x%08x\n", REG_DDRC_REMAP(3));
	dprintf("REG_DDRC_REMAP4 \t= 0x%08x\n", REG_DDRC_REMAP(4));
	dprintf("REG_DDRC_REMAP5 \t= 0x%08x\n", REG_DDRC_REMAP(5));
	serial_puts("====================================================================\n");
}

static void ddr_phy_regs_print(void)
{
	serial_puts("====================================================================\n");
	dprintf("DDR PHY REGS:\n");
	dprintf("REG_DDRP_PGCR \t\t= 0x%08x\n", REG_DDRP_PGCR);
	dprintf("REG_DDRP_PGSR \t\t= 0x%08x\n", REG_DDRP_PGSR);
	dprintf("REG_DDRP_DCR \t\t= 0x%08x\n", REG_DDRP_DCR);
	dprintf("REG_DDRP_PTR0 \t\t= 0x%08x\n", REG_DDRP_PTR0);
	dprintf("REG_DDRP_PTR1 \t\t= 0x%08x\n", REG_DDRP_PTR1);
	dprintf("REG_DDRP_PTR2 \t\t= 0x%08x\n", REG_DDRP_PTR2);
	dprintf("REG_DDRP_DTPR0 \t\t= 0x%08x\n", REG_DDRP_DTPR0);
	dprintf("REG_DDRP_DTPR1 \t\t= 0x%08x\n", REG_DDRP_DTPR1);
	dprintf("REG_DDRP_DTPR2 \t\t= 0x%08x\n", REG_DDRP_DTPR2);
	dprintf("REG_DDRP_MR0 \t\t= 0x%08x\n", REG_DDRP_MR0);
	dprintf("REG_DDRP_MR1 \t\t= 0x%08x\n", REG_DDRP_MR1);
	dprintf("REG_DDRP_MR2 \t\t= 0x%08x\n", REG_DDRP_MR2);
	dprintf("REG_DDRP_MR3 \t\t= 0x%08x\n", REG_DDRP_MR3);
	dprintf("REG_DDRP_DTAR \t\t= 0x%08x\n", REG_DDRP_DTAR);
	dprintf("REG_DDRP_ACIOCR \t= 0x%08x\n", REG_DDRP_ACIOCR);
	dprintf("REG_DDRP_DXCCR \t\t= 0x%08x\n", REG_DDRP_DXCCR);
	dprintf("REG_DDRP_DSGCR \t\t= 0x%08x\n", REG_DDRP_DSGCR);
	dprintf("REG_DDRP_DXDQSTR(0) \t= 0x%08x\n", REG_DDRP_DXDQSTR(0));
	dprintf("REG_DDRP_DXGCR(0) \t= 0x%08x\n", REG_DDRP_DXDQSTR(0));
	dprintf("REG_DDRP_DXDQSTR(1) \t= 0x%08x\n", REG_DDRP_DXDQSTR(1));
	dprintf("REG_DDRP_DXGCR(1) \t= 0x%08x\n", REG_DDRP_DXDQSTR(1));
	serial_puts("====================================================================\n");
}

#define DDR_16M (16 * 1024 * 1024)
static void map_ddr_memory(unsigned long vbase, unsigned long pbase, unsigned long meg) {
	int i, entrys, pfn0, pfn1, vadd, padd;
	unsigned long entrylo0, entrylo1, entryhi, pagemask;

	entrys = meg / 16;
	pagemask = PM_16M;

	for (i = 0; i < entrys; i+=2) {
		vadd = vbase + i * DDR_16M;
		padd = pbase + i * DDR_16M;
		entryhi = vadd;
		pfn0 = (padd >> 6) | (2 << 3);
		pfn1 = (padd + DDR_16M) >> 6 | (2 << 3);
		entrylo0 = (pfn0 | 0x6) & 0x3ffffff;
		entrylo1 = (pfn1 | 0x6) & 0x3ffffff;
		add_wired_entry(entrylo0, entrylo1, entryhi, pagemask);
	}
}
#endif /* DEBUG */

static void jzmemset(void *dest,int ch,int len)
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

static unsigned int gen_verify_data(unsigned int addr, int test_cnt)
{
#if 0
	if (test_cnt == 0)
		i = i >> 2;
	else if (test_cnt == 1)
		i = 0xa5a5a5a5;
	else
		i = 0x5a5a5a5a;
#endif
	if (addr >= 0xa0000000)
		return addr;
	else
		return ~addr;
}

#ifdef DMA_TEST
static int dma_check_result(void *src, void *dst, int size,int print_flag, int test_cnt)
{
	unsigned int addr1, addr2, i, err = 0, count = 0;
	unsigned int data_expect,dsrc,ddst;

	addr1 = (unsigned int)src;
	addr2 = (unsigned int)dst;
	dprintf("check addr: 0x%08x\n", addr1);

	for (i = 0; i < size; i += 4) {
		data_expect = gen_verify_data(addr1, test_cnt);
		dsrc = REG32(addr1);
		ddst = REG32(addr2);
		if ((dsrc != data_expect)
				|| (ddst != data_expect)) {
#if 1
			serial_puts("\nDMA SRC ADDR:");
			serial_put_hex(addr1);
			serial_puts("DMA DST ADDR:");
			serial_put_hex(addr2);
			serial_puts("expect data:");
			serial_put_hex(data_expect);
			serial_puts("src data:");
			serial_put_hex(dsrc);
			serial_puts("dst data:");
			serial_put_hex(ddst);

#endif
			err = 1;
			if(print_flag) { 
				if (count == 30)
					return 1;
				count++;
			} else {
				return 1;
			}
		}

		addr1 += 4;
		addr2 += 4;
	}

	return err;
}

static void dma_nodesc_test(int dma_chan, int dma_src_addr, int dma_dst_addr, int size)
{
	int dma_src_phys_addr, dma_dst_phys_addr;

	/* Allocate DMA buffers */
	dma_src_phys_addr = dma_src_addr & ~0xa0000000;
	dma_dst_phys_addr = dma_dst_addr & ~0xa0000000;

	/* Init DMA module */
	REG_DMAC_DCCSR(dma_chan) = 0;
	REG_DMAC_DRSR(dma_chan) = DMAC_DRSR_RS_AUTO;	//DRSR: DMA Request Types
	REG_DMAC_DSAR(dma_chan) = dma_src_phys_addr;
	REG_DMAC_DTAR(dma_chan) = dma_dst_phys_addr;
	REG_DMAC_DTCR(dma_chan) = size / 32;
	REG_DMAC_DCMD(dma_chan) = DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_32BYTE | DMAC_DCMD_TIE;
	REG_DMAC_DCCSR(dma_chan) = DMAC_DCCSR_NDES | DMAC_DCCSR_EN;
}

static int ddr_dma_test(int print_flag)
{
	int i, err = 0, banks;
	int times, test_cnt;
	unsigned int addr, DDR_DMA0_SRC, DDR_DMA0_DST;
	volatile unsigned int tmp;
	register unsigned int cpu_clk;
	long int memsize, banksize, testsize;

	serial_puts("### ddr dma test ###\n");
	__cpm_start_pdma();
	banks = (DDR_BANK8 ? 8 : 4) *(DDR_CS0EN + DDR_CS1EN);
	memsize = initdram(0);
	if (memsize > LOW_MEM_SPACE_SIZE)
		memsize = LOW_MEM_SPACE_SIZE;
	//dprintf("memsize = 0x%08x\n", memsize);
	banksize = memsize/banks;
	//testsize = 32 * 1024;	
	testsize = banksize / 2;	

	for(test_cnt = 0; test_cnt < 1; test_cnt++) {
		for(times = 0; times < banks; times++) {
			DDR_DMA0_SRC = DDR_DMA_BASE + banksize * times;
			DDR_DMA0_DST = DDR_DMA_BASE + banksize * (times + 1) - testsize;
			serial_puts("dma src addr: ");
			serial_put_hex(DDR_DMA0_SRC);
			serial_puts("dma dst addr: ");
			serial_put_hex(DDR_DMA0_DST);

			cpu_clk = CFG_CPU_SPEED;

			addr = DDR_DMA0_SRC;

			for (i = 0; i < testsize; i += 4) {
				*(volatile unsigned int *)(addr + i) = gen_verify_data(addr + i, test_cnt);
			}

			REG_DMAC_DMACR(3) = 0;
			REG_DMAC_DMACPR(3) = 0;

			/* Init target buffer */
			jzmemset((void *)DDR_DMA0_DST, 0, testsize);
			dma_nodesc_test(3, DDR_DMA0_SRC, DDR_DMA0_DST, testsize);

			REG_DMAC_DMACR(3) = DMAC_DMACR_DMAE; /* global DMA enable bit */

//			while(REG_DMAC_DTCR(3));
			while(!(REG_DMAC_DCCSR(3) & 0x8));

			tmp = (cpu_clk / 1000000) * 1;
			while (tmp--);

			err = dma_check_result((void *)DDR_DMA0_SRC, (void *)DDR_DMA0_DST, testsize,print_flag, test_cnt);

			//REG_DMAC_DCCSR(0) &= ~DMAC_DCCSR_EN;  /* disable DMA */
			REG_DMAC_DMACR(3) = 0;
			REG_DMAC_DCCSR(3) = 0;
			REG_DMAC_DCMD(3) = 0;
			REG_DMAC_DRSR(3) = 0;

			if (err != 0) {
				return err;
			}
		}
	}

	serial_puts("ddr dma test over!\n");
	return err;
}
#endif

#ifdef CPU_TEST
static int check_result(void *src, int size, int print_flag)
{
	unsigned int addr, i, err = 0, count = 0;
	unsigned int data_expect, dsrc;

	addr = (unsigned int)src;

	dprintf("check addr: 0x%08x\n", addr);

	for (i = 0; i < size; i += 4) {
		data_expect = gen_verify_data(addr + i, 0);
		dsrc = REG32(addr + i);
		if (dsrc != data_expect) {
#if 1
			serial_puts("\nWRONG DATA AT:");
			serial_put_hex(addr + i);
			serial_puts("exp:");
			serial_put_hex(data_expect);
			serial_puts("src:");
			serial_put_hex(dsrc);
			serial_puts("NOW:");
			serial_put_hex(REG32(addr + i));
			serial_puts("UNC:");
			serial_put_hex(REG32((addr + i) | 0xa0000000));
#endif
			err = 1;
			if(print_flag) { 
				if (count == 30)
					return 1;
				count++;
			} else {
				return 1;
			}
		}
	}

	return err;
}

static int ddr_test(int print_flag, int cache_flag)
{
	int i, err = 0, banks;
	int times;
	unsigned int addr;
	register unsigned int cpu_clk;
	long int memsize, banksize, testsize;

	serial_puts("### ddr cpu test ###\n");
	cpu_clk = CFG_CPU_SPEED;
	banks = (DDR_BANK8 ? 8 : 4) *(DDR_CS0EN + DDR_CS1EN);
	memsize = initdram(0);
	if (memsize > LOW_MEM_SPACE_SIZE)
		memsize = LOW_MEM_SPACE_SIZE;
	banksize = memsize/banks;
	testsize = 8 * 1024;	
	//testsize = banksize;	

	dprintf("testsize: 0x%08x\n", testsize);
	dprintf("banks: 0x%08x\n", banks);

	if (cache_flag == 0 || cache_flag == 2) { 
		serial_puts("uncache test start!\n");
		for (times = 0; times < banks; times++) {
			addr = DDR_DMA_BASE + banksize * times;

			dprintf("start phy uncache addr: 0x%08x\n", addr);
			for (i = 0; i < testsize; i += 4) {
				REG32(addr + i) = gen_verify_data(addr + i, 0);
			}
		}

#if 1
#if (CONFIG_JZ4780)
		/* open CIM TVE LCD and VPU clock gate for sleep signal */
		REG_CPM_CLKGR0 &= ~((1 << 26) | (1 << 27) | (1 << 28));
		REG_CPM_CLKGR1 &= ~(1 << 4);
		REG_CPM_CLKGR0 |= ((1 << 26) | (1 << 27) | (1 << 28));
		REG_CPM_CLKGR1 |= (1 << 4);
#endif
		serial_puts("start enter self refresh\n");

#if defined(CONFIG_FPGA)
		/* enable ddr controller and phy low power protocol, use self refresh */
		REG32(0xb30100bc) = 0;	
#endif
		asm volatile ("sync");
		REG_DDRC_CTRL |=  DDRC_CTRL_SR;		
		while (!(REG_DDRC_ST & 0x4)) ;
		while (REG_DDRC_ST & 0x1) ;

		serial_puts("enter self refresh test, please wait a moment ...\n");
#if 0
		__asm__ __volatile__(
				".set noreorder\n\t"
				"1:\n\t"
				"bne %0, 0, 1b\n\t"
				"addi %0, %0, -1\n\t"
				".set reorder\n\t"
				: :"r"(0x1fffff));
#endif
		dprintf("REG_DDRC_ST \t\t= 0x%08x, bit2\n", REG_DDRC_ST);
		dprintf("REG_DDRC_CTRL \t\t= 0x%08x, bit5\n", REG_DDRC_CTRL);
		serial_getc();
		serial_puts("exit self refresh mode\n");

		REG_DDRC_CTRL &=  ~DDRC_CTRL_SR;		
		while (REG_DDRC_ST & 0x4) ;
		while (!(REG_DDRC_ST & 0x1)) ;

		/* Reset ddr */
		* (volatile unsigned *) 0xb00000d0 = 0x3;
		volatile unsigned tmp = 0xffff;
		while (tmp--) ;
		serial_puts("rst over\n");
		* (volatile unsigned *) 0xb00000d0 = 0x1;
		tmp = 0xffff;
		while (tmp--) ;

		dprintf("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
		dprintf("REG_DDRC_CTRL \t\t= 0x%08x\n", REG_DDRC_CTRL);
#endif

		for (times = 0; times < banks; times++) {
			addr = DDR_DMA_BASE + banksize * times;
			err = check_result((void *)addr, testsize, print_flag);

			if (err != 0) {
				serial_puts("uncache test err!\n");
				return err;
			}
		}
		serial_puts("uncache test over!\n");
	}

	if (cache_flag == 1 || cache_flag == 2) {
		serial_puts("cache test start!\n");
		/* at least 2MB */
		if (testsize < 1 * 1024 * 1024 / banks)
			testsize = 1 * 1024 * 1024 / banks;
		for (times = 0; times < banks; times++) {
			addr = DDR_DMA_BASE + banksize * times;
			addr &= ~0x20000000;

			dprintf("start cache addr: 0x%08x\n", addr);

			for (i = 0; i < testsize; i += 4) {
				REG32(addr + i) = gen_verify_data(addr + i, 0);
			}
			err = check_result((void *)addr, testsize, print_flag);
		}

		for (times = 0; times < banks; times++) {
			addr = DDR_DMA_BASE + banksize * times;
			addr &= ~0x20000000;
			err = check_result((void *)addr, testsize, print_flag);

		}

		if (err != 0) {
			serial_puts("cache test err!\n");
			return err;
		}
		serial_puts("cache test over!\n");
	}

	return err;
}
#endif

#ifdef DATALINE_TEST
static void dataline_test(void)
{
	unsigned * testaddr = (unsigned *)0xa0001000;
	unsigned int i = 0, src = 0;	

	serial_puts("### dataline test ###\n");
	REG32(testaddr) = 0;
	for (i = 0; i < 32; i++) {
		REG32(testaddr) = (1 << i);
		src = REG32(testaddr);
		if (src != (1 << i)) {
			serial_puts("===========================================\n");
			serial_puts("dataline test: error data line = ");
			serial_put_dec(i);
			serial_puts("\nmemory data = ");
			serial_put_hex(src);
			serial_puts("expect data = ");
			serial_put_hex(1 << i);
			serial_puts("===========================================\n");
		}
	}
}
#endif

#ifdef ADDRLINE_TEST
static void addrline_test(void)
{
	unsigned int testaddr = 0xa0000000;
	unsigned int i = 0, src = 0, testsize = 0, ext = 1;	
	unsigned memsize = initdram(0);

	serial_puts("### addrline test ###\n");
	if (memsize > LOW_MEM_SPACE_SIZE) {
		testsize = LOW_MEM_SPACE_SIZE;
		while ((memsize >> ext) > testsize)
			ext++;

		for (i = 0; i < ext; i++) {
			if (i + 16 == 17)
				continue;
			remap_swap(0 + i, 16 + i);
		}
		serial_puts("REMAP(1) = ");
		serial_put_hex(REG_DDRC_REMAP(1));
		serial_puts("REMAP(5) = ");
		serial_put_hex(REG_DDRC_REMAP(5));

		for (i = 0; i < ext; i++) { 
			/* serial_puts("test addr = "); */
			/* serial_put_hex(testaddr + (1 << (i + 12))); */
			REG32(testaddr + (1 << (i + 12))) = (1 << (i + 12));
			src = REG32(testaddr + (1 << (i + 12)));
			if (src != (1 << (i + 12))) {
				serial_puts("===========================================\n");
				serial_puts("addrline test: err addr line = ");
				serial_put_dec(i + 28);
				serial_puts("\nmemory data = ");
				serial_put_hex(src);
				serial_puts("expect data = ");
				serial_put_hex(1 << (i + 12));
				serial_puts("ST = ");
				serial_put_hex(REG_DDRC_ST);
				serial_puts("===========================================\n");
			}
		}

		REG_DDRC_REMAP(1) = (3 & 0x1f) << 24 | (2 & 0x1f) << 16 | (1 & 0x1f) << 8 | (0 & 0x1f);
		REG_DDRC_REMAP(5) = (19 & 0x1f) << 24 | (18 & 0x1f) << 16 | (17 & 0x1f) << 8 | (16 & 0x1f);

		/* need a delay */
		i = 100;
		while (i--)
			asm volatile ("nop");	
	} else {
		testsize = memsize;
	}

#if 0
	REG32(testaddr) = 0x5a5a3c3c;
	src = REG32(testaddr);
	if (src != 0x5a5a3c3c) {
		serial_puts("===========================================\n");
		serial_puts("addrline test: err addr line = ");
		serial_put_dec(0);
		serial_puts("addr = ");
		serial_put_hex(testaddr);
		serial_puts("memory data = ");
		serial_put_hex(src);
		serial_puts("expect data = ");
		serial_put_hex(0x5a5a3c3c);
		serial_puts("ST = ");
		serial_put_hex(REG_DDRC_ST);
		serial_puts("===========================================\n");
	}
#endif

	for (i = 2; (1 << i) < testsize; i++) {
		REG32(testaddr + (1 << i)) = (1 << i);
		src = REG32(testaddr + (1 << i));
		if (src != (1 << i)) {
			serial_puts("===========================================\n");
			serial_puts("addrline test: err addr line = ");
			serial_put_dec(i);
			serial_puts("addr = ");
			serial_put_hex(testaddr + (1 << i));
			serial_puts("memory data = ");
			serial_put_hex(src);
			serial_puts("expect data = ");
			serial_put_hex(1 << i);
			serial_puts("ST = ");
			serial_put_hex(REG_DDRC_ST);
			serial_puts("===========================================\n");
		}
	}
}
#endif

#ifdef REMAP_TEST
static int memory_post_remap_test(void)
{
	unsigned int i;

	serial_puts("### start remap test ###\n");
	for (i = 0; i < 10; i++) {
		REG_DDRC_REMAP(1) = (3 & 0x1f) << 24 | (2 & 0x1f) << 16 | (1 & 0x1f) << 8 | (0 & 0x1f);
		REG_DDRC_REMAP(2) = (7 & 0x1f) << 24 | (6 & 0x1f) << 16 | (5 & 0x1f) << 8 | (4 & 0x1f);
		REG_DDRC_REMAP(3) = (11 & 0x1f) << 24 | (10 & 0x1f) << 16 | (9 & 0x1f) << 8 | (8 & 0x1f);
		REG_DDRC_REMAP(4) = (15 & 0x1f) << 24 | (14 & 0x1f) << 16 | (13 & 0x1f) << 8 | (12 & 0x1f);
		REG_DDRC_REMAP(5) = (19 & 0x1f) << 24 | (18 & 0x1f) << 16 | (17 & 0x1f) << 8 | (16 & 0x1f);

		REG32(0xa0000000 | (1 << (i + 12))) = 0x123456;
		REG32(0xa0000000 | (1 << (i + 1 + 12))) = 0x778899aa;
		REG32(0xa0000000 | (1 << (i + 1 + 12)) | (1 << (i + 12))) = 0xbbccddee;
		remap_swap(i, 10);
#ifndef DEBUG
		remap_swap(i + 1, 11);
		serial_puts("===> swap: ");
		serial_put_dec(i);
		serial_puts("<=>10 ");
		serial_put_dec(i + 1);
		serial_puts("<=>11 :\n");
#else
		dprintf ("===> %d<=>10 %d<=>11 :\n", i, i + 1);
#endif
		if (REG32(0xa0400000) != 0x123456) {
#ifndef DEBUG
			serial_puts("\tERROR: should: 0x123456, act: ");
			serial_put_hex(REG32(0xa0400000));
#else
			dprintf ("\tERROR: should: 0x123456, act: 0x%08x, src: 0x%08x\n", REG32(0xa0400000), REG32(0xa0000000 | 
						(1 << (i + 12))));
#endif
		}
		if (REG32(0xa0800000) != 0x778899aa) {
#ifndef DEBUG
			serial_puts("\tERROR: should: 0x778899aa, act: ");
			serial_put_hex(REG32(0xa0800000));
#else
			dprintf ("\tERROR: should: 0x778899aa, act: 0x%08x, src: 0x%08x\n", REG32(0xa0800000), REG32(0xa0000000 | 
						(1 << (i + 1 + 12))));
#endif
		}
		if (REG32(0xa0c00000) != 0xbbccddee) {
#ifndef DEBUG
			serial_puts("\tERROR: should: 0xbbccddee, act: ");
			serial_put_hex(REG32(0xa0c00000));
#else
			dprintf ("\tERROR: should: 0xbbccddee, act: 0x%08x, src: 0x%08x\n", REG32(0xa0c00000), REG32(0xa0000000 | 
						(1 << (i + 12)) | (1 << (i + 1 + 12))));
#endif
		}
#ifndef DEBUG
		serial_puts("done\n");
#else
		dprintf ("done\n");
#endif
	}

	REG_DDRC_REMAP(1) = (3 & 0x1f) << 24 | (2 & 0x1f) << 16 | (1 & 0x1f) << 8 | (0 & 0x1f);
	REG_DDRC_REMAP(2) = (7 & 0x1f) << 24 | (6 & 0x1f) << 16 | (5 & 0x1f) << 8 | (4 & 0x1f);
	REG_DDRC_REMAP(3) = (11 & 0x1f) << 24 | (10 & 0x1f) << 16 | (9 & 0x1f) << 8 | (8 & 0x1f);
	REG_DDRC_REMAP(4) = (15 & 0x1f) << 24 | (14 & 0x1f) << 16 | (13 & 0x1f) << 8 | (12 & 0x1f);
	REG_DDRC_REMAP(5) = (19 & 0x1f) << 24 | (18 & 0x1f) << 16 | (17 & 0x1f) << 8 | (16 & 0x1f);
}
#endif

#ifdef CPU_DMA
typedef struct {
	volatile u32 dcmd;      /* DCMD value for the current transfer */
	volatile u32 dsadr;     /* DSAR value for the current transfer */
	volatile u32 dtadr;     /* DTAR value for the current transfer */
	volatile u32 ddadr;     /* Points to the next descriptor + transfer count */
	volatile u32 dstrd;     /* DMA source and target stride address */
	volatile u32 dreqt;     /* DMA request type for current transfer */
	volatile u32 reserved0; /* Reserved */
	volatile u32 reserved1; /* Reserved */
} jz_dma_desc;

#if 0
#define UART_BASE  CFG_UART_BASE
#include <asm/jz_serial.h>
void uart_dma(void)
{
	int i;
	//	unsigned char *buf = (unsigned char *)0x80000000;
	unsigned char *buf = (unsigned char *)0xb3422020;

	serial_puts("### start uart dma ###\n");

	__gpio_as_uart3();
	__cpm_start_uart3();

	REG8(UART3_IER) = 0;
	REG8(UART3_FCR) = ~UART_FCR_UUE;
	REG8(UART3_SIRCR) = ~(SIRCR_RSIRE | SIRCR_TSIRE);
	REG8(UART3_LCR) =  UART_LCR_WLEN_8 | UART_LCR_STOP_1;

	REG8(UART3_LCR) |= UART_LCR_DLAB;
	REG8(UART3_UMR) =  UMR_BEST;
	REG8(UART3_UACR) =  UACR_BEST;
	REG8(UART3_DLHR) =  (DIV_BEST >> 8) & 0xff;
	REG8(UART3_DLLR) =  DIV_BEST & 0xff;

	REG8(UART3_LCR) &= ~UART_LCR_DLAB;
	//	REG8(UART3_IER) &= ~(3 << 1);
	//	REG8(UART3_IER) |= (1 << 2) | (4 << 1);
	REG8(UART3_FCR) = (1 << 3) | UART_FCR_UUE | UART_FCR_FE | UART_FCR_TFLS | UART_FCR_RFLS;

	for (i = 0; i < 26; i += 4) 
		buf[i] = 'a' + i;

	//	jz_dma_desc *desc = (struct jz_dma_desc *)0x8ffff000;
	jz_dma_desc *desc = (struct jz_dma_desc *)0xb3422000;
	jz_dma_desc *next = desc;

	REG_DMAC_DMACR(0) = 0;
	REG_DMAC_DMACPR(0) = 0;
	REG_DMAC_DCCSR(3) |= DMAC_DCCSR_DES8;
	REG_DMAC_DCCSR(3) &= ~DMAC_DCCSR_NDES;
	REG_DMAC_DRSR(3) = 0xe;

	REG_DMAC_DTCR(3) = 0;
	REG_DMAC_DCMD(3) = DMAC_DCMD_LINK;

	desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_8 | DMAC_DCMD_DWDH_8 |
		DMAC_DCMD_DS_8BIT | DMAC_DCMD_LINK | DMAC_DCMD_TIE;
	desc->dsadr = (unsigned int)buf & 0x1fffffff;
	desc->dtadr = UART3_TDR & 0x1fffffff;
	desc->dreqt = 0xe;
	desc->ddadr = (((unsigned int)next >> 4) << 24) | 26;

	flush_cache_all();

	REG_DMAC_DDA(3) = (unsigned int)desc & 0xfffffff;
	REG_DMAC_DMADBSR(0) = 1 << 3;
	REG_DMAC_DMACR(0) |= 1;

	REG_DMAC_DCCSR(3) |= 1;
}
#else

void start_dma(void)
{
	int i;

	serial_puts("dma access...\n");

	__cpm_start_pdma();
	for (i = 0; i < 0x4000000; i += 4)
		REG32(0x80000000 + i) = 0x80000000 + i;

	/* at mcu */
	jz_dma_desc *desc = (jz_dma_desc *)0xb3422000;
	jz_dma_desc *next = desc;
	/* jz_dma_desc *next = desc + 1; */

	REG_DMAC_DMACR(0) = 0;
	REG_DMAC_DMACPR(0) = 0;
	REG_DMAC_DMADBSR(0) = 0;
	REG_DMAC_DDA(3) = 0;
	REG_DMAC_DCCSR(3) = 0;

	REG_DMAC_DCCSR(3) |= DMAC_DCCSR_DES8;
	REG_DMAC_DCCSR(3) &= ~DMAC_DCCSR_NDES;
	REG_DMAC_DRSR(3) = DMAC_DRSR_RS_AUTO;

	REG_DMAC_DTCR(3) = 0;
	REG_DMAC_DCMD(3) = DMAC_DCMD_LINK;

	desc->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 |
		//DMAC_DCMD_DS_128BYTE | DMAC_DCMD_TIE;
		DMAC_DCMD_DS_128BYTE | DMAC_DCMD_LINK | DMAC_DCMD_TIE;
	desc->dsadr = 0x00000000;
	desc->dtadr = 0x04000000;
	desc->dreqt = 0x8;
	desc->ddadr = (((unsigned int)next >> 4) << 24) | 0x80000;

#if 0
	next->dcmd = DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_RDIL_IGN | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 |
		DMAC_DCMD_DS_128BYTE | DMAC_DCMD_LINK | DMAC_DCMD_TIE;
	next->dsadr = 0x00000000;
	next->dtadr = 0x04000000;
	next->dreqt = 0x8;
	next->ddadr = (((unsigned int)desc >> 4) << 24) | 0x80000;
#endif

	flush_dcache_all();

	REG_DMAC_DDA(3) = (unsigned int)desc & 0x1fffffff;
	REG_DMAC_DMADBSR(0) = 1 << 3;
	REG_DMAC_DMACR(0) |= 1;

	REG_DMAC_DCCSR(3) |= 1;

#if 0
#if 0
	while (1) {
		if (REG_DMAC_DTCR(3) < 0x40000)
			__gpio_as_output0(32 * 4 + 2);
		else
			__gpio_as_output1(32 * 4 + 2);
	}
#endif

	while(!(REG_DMAC_DCCSR(3) & 0x8)) {
#if 0
		serial_puts("=================================================\n");
		serial_puts("DTCR = ");
		serial_put_hex(REG_DMAC_DTCR(3));
		serial_puts("DDA = ");
		serial_put_hex(REG_DMAC_DDA(3));
		serial_puts("DSAR = ");
		serial_put_hex(REG_DMAC_DSAR(3));
		serial_puts("DTAR = ");
		serial_put_hex(REG_DMAC_DTAR(3));
		serial_puts("DTCR = ");
		serial_put_hex(REG_DMAC_DTCR(3));
		serial_puts("DRSR = ");
		serial_put_hex(REG_DMAC_DRSR(3));
		serial_puts("DCMD = ");
		serial_put_hex(REG_DMAC_DCMD(3));
		serial_puts("DCCSR = ");
		serial_put_hex(REG_DMAC_DCCSR(3));
		serial_puts("=================================================\n");
#endif
		;
	}


	serial_puts("Check...\n");
	unsigned int bad;
	for (i = 0; i < 0x4000000; i += 4)
		if ((bad = REG32(0x84000000 + i)) != 0x80000000 + i) {
			serial_puts("*************************************************\n");
			serial_puts("Error:");
			serial_put_hex(0x84000000 + i);
			serial_puts("Act:");
			serial_put_hex(bad);
			serial_puts("Now:");
			serial_put_hex(REG32(0x84000000 + i));
			serial_puts("Src:");
			serial_put_hex(REG32(0x80000000 + i));
			serial_puts("Now:");
			serial_put_hex(REG32(0x84000000 + i));
			serial_puts("*************************************************\n");
		}
	serial_puts("Check Over\n");
#endif
}
#endif

static int cpu_and_dma(void)
{
	int i;	
	unsigned int count = 0, cnt = 0;

	serial_puts("### cpu and dma test ###\n");
	jzmemset((void *)0x80000000, 0, 0x10000000);

	/* 0x80000000 - 0x880000000 */
	//uart_dma();	
	start_dma();	

#if 1
	while (1) {
		serial_puts("cpu access...\n");
		__gpio_as_output1(32 * 4 + 2);
		for (i = 0; i < 0x8000000; i += 4) {
			REG32(0x88000000 + i) = 0x88000000 + i;
		}

		__gpio_as_output0(32 * 4 + 2);
#if 1
		for (i = 0; i < 0x8000000; i += 4) {
			if (REG32(0x88000000 + i) != 0x88000000 + i) {
				serial_puts("=======================================\n");
				serial_puts("Error:");
				serial_put_hex(0x88000000 + i);
				serial_puts("Act:");
				serial_put_hex(REG32(0x88000000 + i));
				serial_puts("=======================================\n");
				cnt++;
				if (cnt == 10)
					break;
			}
		}

		__gpio_as_output1(32 * 4 + 2);
		if((REG_DMAC_DCCSR(3) & 0x8) || REG_DMAC_DTCR(3) > 0x80000 || REG_DMAC_DDA(3) != 0x13422000) {
			serial_puts("DDA = ");
			serial_put_hex(REG_DMAC_DDA(3));
			serial_puts("DSAR = ");
			serial_put_hex(REG_DMAC_DSAR(3));
			serial_puts("DTAR = ");
			serial_put_hex(REG_DMAC_DTAR(3));
			serial_puts("DTCR = ");
			serial_put_hex(REG_DMAC_DTCR(3));
		}
#endif
		jzmemset((void *)0x88000000, 0, 0x8000000);
		__gpio_as_output0(32 * 4 + 2);
		serial_puts("cpu access over, ");
		serial_put_hex(count);
		count++;
		cnt = 0;
	}
#endif

	return 0;
}
#endif

void ddr_basic_tests(void)
{
	serial_puts("ddr basic tests\n");
#ifdef ADDRLINE_TEST
	addrline_test();
#endif
#ifdef DATALINE_TEST
	dataline_test();
#endif
	
#ifdef CPU_TEST
	ddr_test(1, 2);
#endif
#ifdef DMA_TEST
	ddr_dma_test(1);
#endif

#ifdef CPU_DMA
	cpu_and_dma();
#endif

#ifdef REMAP_TEST
	memory_post_remap_test();
#endif
}
