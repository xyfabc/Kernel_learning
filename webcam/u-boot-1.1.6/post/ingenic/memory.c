/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
#include <asm/jz4780.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>

/* Memory test
 *
 * General observations:
 * o The recommended test sequence is to test the data lines: if they are
 *   broken, nothing else will work properly.  Then test the address
 *   lines.  Finally, test the cells in the memory now that the test
 *   program knows that the address and data lines work properly.
 *   This sequence also helps isolate and identify what is faulty.
 *
 * o For the address line test, it is a good idea to use the base
 *   address of the lowest memory location, which causes a '1' bit to
 *   walk through a field of zeros on the address lines and the highest
 *   memory location, which causes a '0' bit to walk through a field of
 *   '1's on the address line.
 *
 * o Floating buses can fool memory tests if the test routine writes
 *   a value and then reads it back immediately.  The problem is, the
 *   write will charge the residual capacitance on the data bus so the
 *   bus retains its state briefely.  When the test program reads the
 *   value back immediately, the capacitance of the bus can allow it
 *   to read back what was written, even though the memory circuitry
 *   is broken.  To avoid this, the test program should write a test
 *   pattern to the target location, write a different pattern elsewhere
 *   to charge the residual capacitance in a differnt manner, then read
 *   the target location back.
 *
 * o Always read the target location EXACTLY ONCE and save it in a local
 *   variable.  The problem with reading the target location more than
 *   once is that the second and subsequent reads may work properly,
 *   resulting in a failed test that tells the poor technician that
 *   "Memory error at 00000000, wrote aaaaaaaa, read aaaaaaaa" which
 *   doesn't help him one bit and causes puzzled phone calls.  Been there,
 *   done that.
 *
 * Data line test:
 * ---------------
 * This tests data lines for shorts and opens by forcing adjacent data
 * to opposite states. Because the data lines could be routed in an
 * arbitrary manner the must ensure test patterns ensure that every case
 * is tested. By using the following series of binary patterns every
 * combination of adjacent bits is test regardless of routing.
 *
 *     ...101010101010101010101010
 *     ...110011001100110011001100
 *     ...111100001111000011110000
 *     ...111111110000000011111111
 *
 * Carrying this out, gives us six hex patterns as follows:
 *
 *     0xaaaaaaaaaaaaaaaa
 *     0xcccccccccccccccc
 *     0xf0f0f0f0f0f0f0f0
 *     0xff00ff00ff00ff00
 *     0xffff0000ffff0000
 *     0xffffffff00000000
 *
 * To test for short and opens to other signals on our boards, we
 * simply test with the 1's complemnt of the paterns as well, resulting
 * in twelve patterns total.
 *
 * After writing a test pattern. a special pattern 0x0123456789ABCDEF is
 * written to a different address in case the data lines are floating.
 * Thus, if a byte lane fails, you will see part of the special
 * pattern in that byte lane when the test runs.  For example, if the
 * xx__xxxxxxxxxxxx byte line fails, you will see aa23aaaaaaaaaaaa
 * (for the 'a' test pattern).
 *
 * Address line test:
 * ------------------
 *  This function performs a test to verify that all the address lines
 *  hooked up to the RAM work properly.  If there is an address line
 *  fault, it usually shows up as two different locations in the address
 *  map (related by the faulty address line) mapping to one physical
 *  memory storage location.  The artifact that shows up is writing to
 *  the first location "changes" the second location.
 *
 * To test all address lines, we start with the given base address and
 * xor the address with a '1' bit to flip one address line.  For each
 * test, we shift the '1' bit left to test the next address line.
 *
 * In the actual code, we start with address sizeof(ulong) since our
 * test pattern we use is a ulong and thus, if we tried to test lower
 * order address bits, it wouldn't work because our pattern would
 * overwrite itself.
 *
 * Example for a 4 bit address space with the base at 0000:
 *   0000 <- base
 *   0001 <- test 1
 *   0010 <- test 2
 *   0100 <- test 3
 *   1000 <- test 4
 * Example for a 4 bit address space with the base at 0010:
 *   0010 <- base
 *   0011 <- test 1
 *   0000 <- (below the base address, skipped)
 *   0110 <- test 2
 *   1010 <- test 3
 *
 * The test locations are successively tested to make sure that they are
 * not "mirrored" onto the base address due to a faulty address line.
 * Note that the base and each test location are related by one address
 * line flipped.  Note that the base address need not be all zeros.
 *
 * Memory tests 1-4:
 * -----------------
 * These tests verify RAM using sequential writes and reads
 * to/from RAM. There are several test cases that use different patterns to
 * verify RAM. Each test case fills a region of RAM with one pattern and
 * then reads the region back and compares its contents with the pattern.
 * The following patterns are used:
 *
 *  1a) zero pattern (0x00000000)
 *  1b) negative pattern (0xffffffff)
 *  1c) checkerboard pattern (0x55555555)
 *  1d) checkerboard pattern (0xaaaaaaaa)
 *  2)  bit-flip pattern ((1 << (offset % 32))
 *  3)  address pattern (offset)
 *  4)  address pattern (~offset)
 *
 * Being run in normal mode, the test verifies only small 4Kb
 * regions of RAM around each 1Mb boundary. For example, for 64Mb
 * RAM the following areas are verified: 0x00000000-0x00000800,
 * 0x000ff800-0x00100800, 0x001ff800-0x00200800, ..., 0x03fff800-
 * 0x04000000. If the test is run in slow-test mode, it verifies
 * the whole RAM.
 */

#ifdef CONFIG_POST

#include <jz_post.h>
//#include <watchdog.h>

#if CONFIG_POST & CFG_JZ_POST_MEMORY

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)
#undef TEST_CS1
//#define TEST_CS1

DECLARE_GLOBAL_DATA_PTR;

/*
 * Define INJECT_*_ERRORS for testing error detection in the presence of
 * _good_ hardware.
 */
#undef  INJECT_DATA_ERRORS
#undef  INJECT_ADDRESS_ERRORS

#ifdef INJECT_DATA_ERRORS
#warning "Injecting data line errors for testing purposes"
#endif

#ifdef INJECT_ADDRESS_ERRORS
#warning "Injecting address line errors for testing purposes"
#endif


/*
 * This function performs a double word move from the data at
 * the source pointer to the location at the destination pointer.
 * This is helpful for testing memory on processors which have a 64 bit
 * wide data bus.
 *
 * On those PowerPC with FPU, use assembly and a floating point move:
 * this does a 64 bit move.
 *
 * For other processors, let the compiler generate the best code it can.
 */
static void move64(unsigned long long *src, unsigned long long *dest)
{
#if defined(CONFIG_MPC8260) || defined(CONFIG_MPC824X)
	asm ("lfd  0, 0(3)\n\t" /* fpr0	  =  *scr	*/
	 "stfd 0, 0(4)"		/* *dest  =  fpr0	*/
	 : : : "fr0" );		/* Clobbers fr0		*/
    return;
#else
	*dest = *src;
#endif
}

/*
 * This is 64 bit wide test patterns.  Note that they reside in ROM
 * (which presumably works) and the tests write them to RAM which may
 * not work.
 *
 * The "otherpattern" is written to drive the data bus to values other
 * than the test pattern.  This is for detecting floating bus lines.
 *
 */
const static unsigned long long pattern[] = {
	0xaaaaaaaaaaaaaaaaULL,
	0xccccccccccccccccULL,
	0xf0f0f0f0f0f0f0f0ULL,
	0xff00ff00ff00ff00ULL,
	0xffff0000ffff0000ULL,
	0xffffffff00000000ULL,
	0x00000000ffffffffULL,
	0x0000ffff0000ffffULL,
	0x00ff00ff00ff00ffULL,
	0x0f0f0f0f0f0f0f0fULL,
	0x3333333333333333ULL,
	0x5555555555555555ULL
};
const unsigned long long otherpattern = 0x0123456789abcdefULL;


static int memory_post_dataline(unsigned long long * pmem)
{
	unsigned long long temp64 = 0;
	int num_patterns = sizeof(pattern)/ sizeof(pattern[0]);
	int i;
	unsigned int hi, lo, pathi, patlo;
	int ret = 0;

	for ( i = 0; i < num_patterns; i++) {
		move64((unsigned long long *)&(pattern[i]), pmem++);
		/*
		 * Put a different pattern on the data lines: otherwise they
		 * may float long enough to read back what we wrote.
		 */
		move64((unsigned long long *)&otherpattern, pmem--);
		move64(pmem, &temp64);

#ifdef INJECT_DATA_ERRORS
		temp64 ^= 0x00008000;
#endif

		if (temp64 != pattern[i]){
			pathi = (pattern[i]>>32) & 0xffffffff;
			patlo = pattern[i] & 0xffffffff;

			hi = (temp64>>32) & 0xffffffff;
			lo = temp64 & 0xffffffff;

			post_log ("Memory (date line) error at %08x, "
				  "wrote %08x%08x, read %08x%08x !\n",
					  pmem, pathi, patlo, hi, lo);
			ret = -1;
		}
	}

	if (ret == 0)
		post_log("Memory (data line) PASS\n");
	return ret;
}

static int memory_post_addrline(ulong *testaddr, ulong *base, ulong size)
{
	ulong *target;
	ulong *end;
	ulong readback;
	ulong xor;
	int   ret = 0;

	end = (ulong *)((ulong)base + size);	/* pointer arith! */
	xor = 0;
	for(xor = sizeof(ulong); xor > 0; xor <<= 1) {
		target = (ulong *)((ulong)testaddr ^ xor);
		if((target >= base) && (target < end)) {
			*testaddr = ~*target;
			readback  = *target;

#ifdef INJECT_ADDRESS_ERRORS
			if(xor == 0x00008000) {
				readback = *testaddr;
			}
#endif
			if(readback == *testaddr) {
				post_log ("Memory (address line) error at %08x<->%08x, "
				  	"XOR value %08x !\n",
					testaddr, target, xor);
				ret = -1;
			}
		}
	}
	if (ret == 0)
		post_log ("Memory (address line) PASS\n");
	return ret;
}

static int memory_post_test1 (unsigned long start,
			      unsigned long size,
			      unsigned long val)
{
	unsigned long i, count = 0;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = val;
	}
	post_log ("write over\n");

	for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) {
		readback = mem[i];
		if (readback != val) {
			post_log ("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, val, readback);

			ret = -1;
			count++;
			if (count == 5)
				break;
		}
	}

	if (ret == 0)
		post_log ("Memory (post test1 %x) PASS\n", val);
	else
		post_log ("Memory (post test1 %x) ERROR\n", val);

	return ret;
}

static int memory_post_test2 (unsigned long start, unsigned long size)
{
	unsigned long i, count = 0;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = 1 << (i % 32);
	}
	post_log ("write over\n");

	for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) {
		readback = mem[i];
		if (readback != (1 << (i % 32))) {
			post_log ("Memory error at %08x, "
				  "wrote %08x, read %08x !\n",
					  mem + i, 1 << (i % 32), readback);

			ret = -1;
			count++;
			if (count == 5)
				break;
		}
	}

	if (ret == 0)
		post_log ("Memory (post test2) PASS\n");
	return ret;
}

static int memory_post_test3 (unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = i + start;
	}
	post_log ("Memory at: 0x%08x , size: 0x%08x(post test3) OVER\n", start, size);
	
	return 0;
}

static int memory_post_check3 (unsigned long start, unsigned long size)
{
	unsigned long i, count = 0;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) {
		readback = mem[i];
		if (readback != (i + start)) {
			post_log ("Memory error at %08x, "
				  "wrote %08x, read %08x , count %d!\n",
					  mem + i, i + start, readback, count);

			count++;
			i += 256;
			if (count == 5)
				ret = -1;
		}
	}

	if (ret == 0)
		post_log ("Memory at: 0x%08x , size: 0x%08x(post test3) PASS\n", start, size);
	else
		post_log ("Memory at: 0x%08x , size: 0x%08x(post test3) FAIL\n", start, size);
	return ret;
}

static int memory_post_test4 (unsigned long start, unsigned long size)
{
	unsigned long i;
	ulong *mem = (ulong *) start;

	for (i = 0; i < size / sizeof (ulong); i++) {
		mem[i] = ~(i + start);
	}

	post_log ("Memory at: 0x%08x , size: 0x%08x(post test4) OVER\n", start, size);

	return 0;
}

static int memory_post_check4 (unsigned long start, unsigned long size)
{
	unsigned long i, count = 0;
	ulong *mem = (ulong *) start;
	ulong readback;
	int ret = 0;

	for (i = 0; i < size / sizeof (ulong) && ret == 0; i++) {
		readback = mem[i];
		if (readback != ~(i + start)) {
			post_log ("Memory error at %08x, "
				  "wrote %08x, read %08x , count %d!\n",
					  mem + i, ~(i + start), readback, count);

			count++;
			if (count == 5)
				ret = -1;
		}
	}

	if (ret == 0)
		post_log ("Memory at: 0x%08x , size: 0x%08x(post test4) PASS\n", start, size);
	else
		post_log ("Memory at: 0x%08x , size: 0x%08x(post test4) FAIL\n", start, size);

	return ret;
}

static int memory_post_test5 (unsigned long start_va, unsigned long start_phy, unsigned long size)
{
	unsigned int testsize, i, dma_src_phy, dma_dst_phy, dma_src_va, dma_dst_va, ret = 0, readback, dmaback, expect;	
	
	dma_src_va = start_va;
	dma_src_phy = start_phy;

	while (dma_src_va < size + start_va) {
		if (dma_src_va + 0x800000 < size + start_va) 
			testsize = 0x400000;
		else
			testsize = (size + start_va - dma_src_va) / 2;
		dma_dst_phy = dma_src_phy + testsize;
		dma_dst_va = dma_src_va + testsize;
#if 0
		post_log ("dma testsize = %08x :\n", testsize);
		post_log ("dma_src_va = %08x <===> ", dma_src_va);
		post_log ("dma_dst_va = %08x\n", dma_dst_va);
		post_log ("dma_src_phy = %08x <===> ", dma_src_phy);
		post_log ("dma_dst_phy = %08x\n\n", dma_dst_phy);
#endif

		for (i = 0; i < testsize; i += 4) 	
			REG32(dma_src_va + i) = dma_src_va + i;
		flush_dcache_all();

		REG_DMAC_DMACR(3) = 0;
		REG_DMAC_DMACPR(3) = 0;
		/* Init DMA module */
		REG_DMAC_DCCSR(3) = 0;
		REG_DMAC_DRSR(3) = DMAC_DRSR_RS_AUTO;	//DRSR: DMA Request Types
		REG_DMAC_DSAR(3) = dma_src_phy;
		REG_DMAC_DTAR(3) = dma_dst_phy;
		REG_DMAC_DTCR(3) = testsize / 32;
		REG_DMAC_DCMD(3) = DMAC_DCMD_SAI | DMAC_DCMD_DAI | DMAC_DCMD_SWDH_32 | DMAC_DCMD_DWDH_32 | DMAC_DCMD_DS_32BYTE | DMAC_DCMD_TIE;
		REG_DMAC_DCCSR(3) = DMAC_DCCSR_NDES | DMAC_DCCSR_EN;
		REG_DMAC_DMACR(3) = DMAC_DMACR_DMAE; /* global DMA enable bit */

		//while(REG_DMAC_DTCR(3));
		while(!(REG_DMAC_DCCSR(3) & 0x8));

		REG_DMAC_DMACR(3) = 0;
		REG_DMAC_DCCSR(3) = 0;
		REG_DMAC_DCMD(3) = 0;
		REG_DMAC_DRSR(3) = 0;

		for (i = 0; i < testsize; i += 4) {
			readback = REG32(dma_src_va + i);
			dmaback = REG32(dma_dst_va + i);
			expect = dma_src_va + i;
			if ((readback != expect) || (dmaback != expect)) {
				post_log ("Memory error at %08x, wrote %08x, read %08x, dma read %08x !\n", dma_src_va + i, expect, readback, dmaback);
				ret = -1;
			}
		}
		dma_src_va += testsize * 2;
		dma_src_phy += testsize * 2;
	}

	if (ret == 0)
		post_log ("Memory (post test5) PASS\n");

	return ret;
}

static int memory_post_tests (unsigned long start, unsigned long size)
{
	int ret = 0;

	ret = memory_post_dataline ((unsigned long long *)start);
	ret = memory_post_addrline ((ulong *)start, (ulong *)start, size);
	ret = memory_post_addrline ((ulong *)(start + size - 8), (ulong *)start, size);
	ret = memory_post_test1 (start, size, 0x00000000);
	ret = memory_post_test1 (start, size, 0xffffffff);
	ret = memory_post_test1 (start, size, 0x55555555);
	ret = memory_post_test1 (start, size, 0xaaaaaaaa);
	ret = memory_post_test1 (start, size, 0x5aa5a55a);
	ret = memory_post_test1 (start, size, 0xa55a5aa5);
	ret = memory_post_test2 (start, size);

	return ret;
}

#define __read_32bit_c0_register(source, sel)                           \
	({ int __res;                                                           \
	 if (sel == 0)                                                   \
	 __asm__ __volatile__(                                   \
		 "mfc0\t%0, " #source "\n\t"                     \
		 "nop \n\t"					\
		 : "=r" (__res));                                \
	 else                                                            \
	 __asm__ __volatile__(                                   \
		 ".set\tmips32\n\t"                              \
		 "mfc0\t%0, " #source ", " #sel "\n\t"           \
		 "nop \n\t"					\
		 ".set\tmips0\n\t"                               \
		 : "=r" (__res));                                \
	 __res;                                                          \
	 })

#define __write_32bit_c0_register(register, sel, value)                 \
	do {                                                                    \
		if (sel == 0)                                                   \
		__asm__ __volatile__(                                   \
				"mtc0\t%z0, " #register "\n\t"                  \
				"nop \n\t"					\
				: : "Jr" ((unsigned int)(value)));              \
		else                                                            \
		__asm__ __volatile__(                                   \
				".set\tmips32\n\t"                              \
				"mtc0\t%z0, " #register ", " #sel "\n\t"        \
				"nop \n\t"					\
				".set\tmips0"                                   \
				: : "Jr" ((unsigned int)(value)));              \
	} while (0)

#define read_c0_index()         __read_32bit_c0_register($0, 0)
#define write_c0_index(val)     __write_32bit_c0_register($0, 0, val)

#define read_c0_entrylo0()      __read_32bit_c0_register($2, 0)
#define write_c0_entrylo0(val)  __write_32bit_c0_register($2, 0, val)

#define read_c0_entrylo1()      __read_32bit_c0_register($3, 0)
#define write_c0_entrylo1(val)  __write_32bit_c0_register($3, 0, val)

#define read_c0_context()       __read_32bit_c0_register($4, 0)
#define write_c0_context(val)   __write_32bit_c0_register($4, 0, val)

#define read_c0_userlocal()     __read_32bit_c0_register($4, 2)
#define write_c0_userlocal(val) __write_32bit_c0_register($4, 2, val)

#define read_c0_pagemask()      __read_32bit_c0_register($5, 0)
#define write_c0_pagemask(val)  __write_32bit_c0_register($5, 0, val)

#define read_c0_entryhi()       __read_32bit_c0_register($10, 0)
#define write_c0_entryhi(val)   __write_32bit_c0_register($10, 0, val)

void mmu_dump_tlb(void)
{
	int i;
	post_log("Dump MMU tlb entries:\n");
	post_log("index  mask    entryhi  entrylo0 entrylo1\n");
	for (i = 0; i < 32; i++) {
		write_c0_index(i);
		__asm__ __volatile__("nop;nop;nop;nop;");
		__asm__ __volatile__("tlbr");
		__asm__ __volatile__("nop;nop;nop;nop;");

		post_log(" %2d   %x %x      %x      %x\n",i,
		       read_c0_pagemask(),
		       read_c0_entryhi(),
		       read_c0_entrylo0(),
		       read_c0_entrylo1());
	}
}

static void flush_tlb_entry(void)
{
	int i;
	static int tlb_entry_num;
	unsigned long config;

	config = __read_32bit_c0_register($16, 1);
	tlb_entry_num = (config >> 25) % 64 + 1;

	/* flush TLB */
	for (i = 0; i < tlb_entry_num; i++) {
		write_c0_index(i);
		write_c0_pagemask(0);
		write_c0_entryhi(i << 13);
		write_c0_entrylo0((2) << 6 | 0x0);
		write_c0_entrylo1((3) << 6 | 0x1f);
		/* barrier */
		__asm__ __volatile__("nop;nop;nop;nop;");
		/* write indexed tlb entry */
		__asm__ __volatile__("tlbwi");
	}
}

int map_mem_tlb(unsigned long vaddr, unsigned long paddr,
		unsigned page_size, unsigned mem_size)
{       
	unsigned int page_mask;
	int i,n;

	flush_tlb_entry();

	page_mask = ~ (page_size - 1);
	n = mem_size / (page_size * 2);
	if (n > 32)
		return -1;

	for (i = 0; i < n; i++) {
		unsigned long pagemask = (page_size*2) - 1;
		unsigned long entryhi, entrylo0, entrylo1;

		entryhi = vaddr & page_mask;

		entrylo0 = (((paddr/(4*1024))<<6) | 07 | (3 << 3));	//1 - uca , 3 - cache
		paddr += page_size;
		entrylo1 = (((paddr/(4*1024))<<6) | 07 | (3 << 3));
		paddr += page_size;

		vaddr += (2*page_size);

		write_c0_index(i);
		write_c0_pagemask(pagemask);
		write_c0_entryhi(entryhi);
		write_c0_entrylo0(entrylo0);
		write_c0_entrylo1(entrylo1);
		/* barrier */
		__asm__ __volatile__("nop;nop;nop;nop;");
		/* write indexed tlb entry */
		__asm__ __volatile__("tlbwi");
	}

	//mmu_dump_tlb();
	return 0;
}

static unsigned int status = 0;

static void set_mips_regs(void)
{
	register unsigned int tmp = 0;

	__asm__ __volatile__ (
		".set\tmips32\n\t"                             
		"mfc0 %0, $12, 0\n\t"
		"nop\n\t"
		".set\tmips0\n\t"                             
		:"=r"(tmp));

	post_log("CP0 Status register = %x\n", tmp);
	status = 0;
	tmp &= ~0x4;
	tmp |= 0x2;

	__asm__ __volatile__(
		".set\tmips32\n\t"                             
		"mtc0 %0, $12, 0\n\t"
		"nop\n\t"
		".set\tmips0\n\t"                             
		::"r"(tmp));

	tmp = 0xa9000000;

	__asm__ __volatile__ (
		".set\tmips32\n\t"                             
		"mtc0 %0, $5, 4\n\t"
		"nop\n\t"
		".set\tmips0\n\t"                             
		::"r"(tmp)
		);
	
}

static void recover_mips_regs(void)
{
	__asm__ __volatile__(
		".set\tmips32\n\t"                             
		"mtc0 %0, $12, 0\n\t"
		"nop\n\t"
		".set\tmips0\n\t"                             
		::"r"(status));
}

int memory_post_test (int flags)
{
	int ret = 0, i = 0, loop = 0;
	bd_t *bd = gd->bd;
	unsigned long memsize = (bd->bi_memsize >= 256 * MB ?
			256 * MB : bd->bi_memsize) - (3 * MB);
	unsigned long memsizes[5] = {0};
	unsigned long vaddr[5] = {0}, paddr[5] = {0};
	unsigned int pagesize = 16 * MB;

	set_mips_regs();
//	while (1) {
#ifdef TEST_CS1
	memsizes[0] = 0;
#else
	memsizes[0] = memsize;
#endif
	vaddr[0] = CFG_SDRAM_BASE;
	paddr[0] = 0x00000000;
	post_log ("==>Memory test size: %08x\n", memsizes[0]);
	post_log (" Memory test at: %08x\n", vaddr[0]);
#if 1
	ret = memory_post_tests (vaddr[0], memsizes[0]);
	post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
#endif
	i++;
	memory_post_test5 (vaddr[0], paddr[0], memsizes[0]);

	memsize = initdram(0);
	if (memsize > 256 * MB) {
		unsigned long testsize = 0;
		unsigned long ext_mem = memsize - 256 * MB;	
		vaddr[1] = 0x20000000;
		paddr[1] = 0x30000000;

		post_log ("map pagesize size: %08x\n", pagesize);

		while ( (long)testsize - (long)ext_mem < 0) {
			if ((ext_mem - testsize) / (pagesize * 2) > 32) 
				memsizes[i] = pagesize * 2 * 32;
			else
				memsizes[i] = ext_mem - testsize;
			ret = map_mem_tlb(vaddr[i], paddr[i], pagesize, memsizes[i]);
			if (ret)
				post_log("map tlb mem fail\n");
			post_log ("==>Memory test size: %08x\n", memsizes[i]);
			post_log (" Memory test vaddr : %08x\n", vaddr[i]);
			post_log (" Memory test paddr : %08x\n", paddr[i]);
			ret = memory_post_tests (vaddr[i], memsizes[i]);
			post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
			memory_post_test5 (vaddr[i], paddr[i], memsizes[i]);
			vaddr[i + 1] = vaddr[i] + memsizes[i];
			paddr[i + 1] = paddr[i] + memsizes[i];
			testsize += memsizes[i];
			post_log ("has test size : %08x\n", testsize);
			i++;
		}
	}
	post_log ("i : %d\n", i);
	loop = i;

	for (i = 0; i < loop; i++) {
		if (i != 0) {
			ret = map_mem_tlb(vaddr[i], paddr[i], pagesize, memsizes[i]);
			if (ret)
				post_log("map tlb mem fail\n");
		}
		ret = memory_post_test3 (vaddr[i], memsizes[i]);
		post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
	}

	for (i = 0; i < loop; i++) {
		if (i != 0) {
			ret = map_mem_tlb(vaddr[i], paddr[i], pagesize, memsizes[i]);
			if (ret)
				post_log("map tlb mem fail\n");
		}
		ret = memory_post_check3 (vaddr[i], memsizes[i]);
		post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
	}

	for (i = loop - 1; i >= 0; i--) {
		if (i != 0) {
			ret = map_mem_tlb(vaddr[i], paddr[i], pagesize, memsizes[i]);
			if (ret)
				post_log("map tlb mem fail\n");
		}
		ret = memory_post_test4 (vaddr[i], memsizes[i]);
		post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
	}

	for (i = 0; i < loop; i++) {
		if (i != 0) {
			ret = map_mem_tlb(vaddr[i], paddr[i], pagesize, memsizes[i]);
			if (ret)
				post_log("map tlb mem fail\n");
		}
		ret = memory_post_check4 (vaddr[i], memsizes[i]);
		post_log("REG_DDRC_ST \t\t= 0x%08x\n", REG_DDRC_ST);
	}
//	}

	recover_mips_regs();
#ifndef CONFIG_GET_DDR_ARG
	WATCHDOG_DISABLE();
#endif
#ifdef CONFIG_GET_DDR_ARG
	if (ret == 0) {
		set_curr_valid();
	}
#endif
	return ret;
}

#endif /* CONFIG_POST & CFG_POST_MEMORY */
#endif /* CONFIG_POST */
