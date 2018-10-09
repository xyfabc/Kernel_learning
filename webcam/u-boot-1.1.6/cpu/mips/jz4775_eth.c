#include <config.h>

#include <common.h>
#include <malloc.h>
#include <net.h>
#include <command.h>
#include <asm/io.h>

#include <asm/jzsoc.h>

int debug_enable = 0;

#include "synopGMAC_Dev.h"

/* The amount of time between FLP bursts is 16ms +/- 8ms */
#define MAX_WAIT	40000

static synopGMACdevice	_gmacdev;
static synopGMACdevice	*gmacdev;

#define NUM_RX_DESCS	PKTBUFSRX
#define NUM_TX_DESCS	4

static DmaDesc	_tx_desc[NUM_TX_DESCS];
static DmaDesc	_rx_desc[NUM_RX_DESCS];
static DmaDesc	*tx_desc;
static DmaDesc	*rx_desc;
static int	next_tx;
static int	next_rx;

__attribute__((__unused__)) static void jzmac_dump_dma_desc2(DmaDesc *desc) {
	printf("desc: %p, status: 0x%08x buf1: 0x%08x len: %u\n",
			desc, desc->status, desc->buffer1, desc->length);
}

__attribute__((__unused__)) static void jzmac_dump_rx_desc(void) {
	int i = 0;
	printf("\n===================rx====================\n");
	for (i = 0; i < NUM_RX_DESCS; i++) {
		jzmac_dump_dma_desc2(rx_desc + i);
	}
	printf("\n=========================================\n");
}

__attribute__((__unused__)) static void jzmac_dump_tx_desc(void) {
	int i = 0;
	printf("\n===================tx====================\n");
	for (i = 0; i < NUM_TX_DESCS; i++) {
		jzmac_dump_dma_desc2(tx_desc + i);
	}
	printf("\n=========================================\n");
}

__attribute__((__unused__)) static void jzmac_dump_all_desc(void) {
	jzmac_dump_rx_desc();
	jzmac_dump_tx_desc();
}

__attribute__((__unused__)) static void jzmac_dump_pkt_data(unsigned char *data, int len) {
	int i = 0;
	printf("\t0x0000: ");
	for (i = 0; i < len; i++) {
		printf("%02x ", data[i]);

		if ((i % 8) == 7)
			printf(" ");

		if ( (i != 0) && ((i % 16) == 15) )
			printf("\n\t0x%04x: ", i+1);
	}
	printf("\n");
}

__attribute__((__unused__)) static void jzmac_dump_arp_reply(unsigned char *data, int len) {
	int i = 0;

	for (i = 0; i < 6; i++) {
		if (data[i] != 0xff)
			break;
	}

	if (i == 6)  // broadcast pkt
		return;

	if ( (data[12] == 0x08) && (data[13] == 0x06) && (data[20] == 0x00)) {
		jzmac_dump_pkt_data(data, len);
	}
}

__attribute__((__unused__)) static void jzmac_dump_icmp_reply(unsigned char *data, int len) {
	if ( (data[12] == 0x08) && (data[13] == 0x00) && (data[23] == 0x01) && (data[34] == 0x00) ) {
		jzmac_dump_pkt_data(data, len);
	}
}

static u32 full_duplex, phy_mode;


struct jzmac_reg
{
	u32    addr;
	char   * name;
};

static struct jzmac_reg mac[] =
	{
		{ 0x0000, "                  Config" },
		{ 0x0004, "            Frame Filter" },
		{ 0x0008, "             MAC HT High" },
		{ 0x000C, "              MAC HT Low" },
		{ 0x0010, "               GMII Addr" },
		{ 0x0014, "               GMII Data" },
		{ 0x0018, "            Flow Control" },
		{ 0x001C, "                VLAN Tag" },
		{ 0x0020, "            GMAC Version" },
		{ 0x0024, "            GMAC Debug  " },
		{ 0x0028, "Remote Wake-Up Frame Filter" },
		{ 0x002C, "  PMT Control and Status" },
		{ 0x0030, "  LPI Control and status" },
		{ 0x0034, "      LPI Timers Control" },
		{ 0x0038, "        Interrupt Status" },
		{ 0x003c, "        Interrupt Mask" },
		{ 0x0040, "          MAC Addr0 High" },
		{ 0x0044, "           MAC Addr0 Low" },
		{ 0x0048, "          MAC Addr1 High" },
		{ 0x004c, "           MAC Addr1 Low" },
		{ 0x0100, "           MMC Ctrl Reg " },
		{ 0x010c, "        MMC Intr Msk(rx)" },
		{ 0x0110, "        MMC Intr Msk(tx)" },
		{ 0x0200, "    MMC Intr Msk(rx ipc)" },
		{ 0x0738, "          AVMAC Ctrl Reg" },
		{ 0, 0 }
	};
static struct jzmac_reg dma0[] =
	{
		{ 0x0000, "[CH0] CSR0   Bus Mode" },
		{ 0x0004, "[CH0] CSR1   TxPlDmnd" },
		{ 0x0008, "[CH0] CSR2   RxPlDmnd" },
		{ 0x000C, "[CH0] CSR3    Rx Base" },
		{ 0x0010, "[CH0] CSR4    Tx Base" },
		{ 0x0014, "[CH0] CSR5     Status" },
		{ 0x0018, "[CH0] CSR6    Control" },
		{ 0x001C, "[CH0] CSR7 Int Enable" },
		{ 0x0020, "[CH0] CSR8 Missed Fr." },
		{ 0x0024, "[CH0] Recv Intr Wd.Tm." },
		{ 0x0028, "[CH0] AXI Bus Mode   " },
		{ 0x002c, "[CH0] AHB or AXI Status" },
		{ 0x0048, "[CH0] CSR18 Tx Desc  " },
		{ 0x004C, "[CH0] CSR19 Rx Desc  " },
		{ 0x0050, "[CH0] CSR20 Tx Buffer" },
		{ 0x0054, "[CH0] CSR21 Rx Buffer" },
		{ 0x0058, "CSR22 HWCFG          " },
		{ 0, 0 }
	};

__attribute__((__unused__)) static void jzmac_dump_dma_regs(const char *func, int line)
{
	struct jzmac_reg *reg = dma0;

	printf("======================DMA Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name, synopGMACReadReg((u32 *)gmacdev->DmaBase,reg->addr));
		reg++;
	}
	printf("======================DMA Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_mac_regs(const char *func, int line)
{
	struct jzmac_reg *reg = mac;

	printf("======================MAC Regs start===================\n");
	while(reg->name) {
		printf("%s:\t0x%08x\n", reg->name, synopGMACReadReg((u32 *)gmacdev->MacBase,reg->addr));
		reg++;
	}
	printf("======================MAC Regs end===================\n");
}

__attribute__((__unused__)) static void jzmac_dump_all_regs(const char *func, int line) {
	jzmac_dump_dma_regs(func, line);
	jzmac_dump_mac_regs(func, line);
}

static void jzmac_init(void) {
	synopGMAC_wd_enable(gmacdev);
	synopGMAC_jab_enable(gmacdev);
	synopGMAC_frame_burst_enable(gmacdev);
	synopGMAC_jumbo_frame_disable(gmacdev);
	synopGMAC_rx_own_enable(gmacdev);
	synopGMAC_loopback_off(gmacdev);
	/* default to full duplex, I think this will be the common case */
	synopGMAC_set_full_duplex(gmacdev);
	synopGMAC_retry_enable(gmacdev);
	synopGMAC_pad_crc_strip_disable(gmacdev);
	synopGMAC_back_off_limit(gmacdev,GmacBackoffLimit0);
	synopGMAC_deferral_check_disable(gmacdev);
	synopGMAC_tx_enable(gmacdev);
	synopGMAC_rx_enable(gmacdev);

	/* default to 100M, I think this will be the common case */
	synopGMAC_select_mii(gmacdev);

	/*Frame Filter Configuration*/
	synopGMAC_frame_filter_enable(gmacdev);
	synopGMAC_set_pass_control(gmacdev,GmacPassControl0);
	synopGMAC_broadcast_enable(gmacdev);
	synopGMAC_src_addr_filter_disable(gmacdev);
	synopGMAC_multicast_disable(gmacdev);
	synopGMAC_dst_addr_filter_normal(gmacdev);
	synopGMAC_multicast_hash_filter_disable(gmacdev);
	synopGMAC_promisc_disable(gmacdev);
	synopGMAC_unicast_hash_filter_disable(gmacdev);

	/*Flow Control Configuration*/
	synopGMAC_unicast_pause_frame_detect_disable(gmacdev);
	synopGMAC_rx_flow_control_disable(gmacdev);
	synopGMAC_tx_flow_control_disable(gmacdev);
}

static void jz4775_mac_configure(void) {
	synopGMAC_dma_bus_mode_init(gmacdev, DmaBurstLength32 | DmaDescriptorSkip2 | DmaDescriptor8Words | DmaFixedBurstEnable ); //pbl32 incr with rxthreshold 128 and Desc is 8 Words
	synopGMAC_dma_control_init(gmacdev,DmaStoreAndForward |DmaTxSecondFrame|DmaRxThreshCtrl128);

	/*Initialize the mac interface*/

	jzmac_init();
	//synopGMAC_pause_control(gmacdev); // This enables the pause control in Full duplex mode of operation
	synopGMAC_clear_interrupt(gmacdev);
	/*
	  Disable the interrupts generated by MMC and IPC counters.
	  If these are not disabled ISR should be modified accordingly to handle these interrupts.
	*/
	synopGMAC_disable_mmc_tx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_rx_interrupt(gmacdev, 0xFFFFFFFF);
	synopGMAC_disable_mmc_ipc_rx_interrupt(gmacdev, 0xFFFFFFFF);
}

/***************************************************************************
 * ETH interface routines
 **************************************************************************/

static void jzmac_restart_tx_dma(void) {
	u32 data;

	/* TODO: clear error status bits if any */

	data = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaControl);
	if (data & DmaTxStart) {
		synopGMAC_resume_dma_tx(gmacdev);
	} else {
		synopGMAC_enable_dma_tx(gmacdev);
	}
}

static int jz_send(struct eth_device* dev, volatile void *packet, int length)
{
	DmaDesc *desc = tx_desc + next_tx;
	unsigned int dma_status_reg, interrupt;
	int i, ret = 1;
	char c;

	//printf("====>send packet %p, length = %d\n", packet, length);

	if (!packet) {
		printf("jz_send: packet is NULL !\n");
		return -1;
	}

	synopGMAC_tx_desc_init_ring(desc, next_tx == (NUM_TX_DESCS - 1));
	desc->length |= (((length <<DescSize1Shift) & DescSize1Mask)
			 | ((0 <<DescSize2Shift) & DescSize2Mask));  // buffer2 is not used
	desc->buffer1 = virt_to_phys(packet);
	desc->buffer2 = 0;
	desc->status |=  (DescTxFirst | DescTxLast | DescTxIntEnable); //ENH_DESC
	desc->status |= DescOwnByDma;//ENH_DESC

	jz_flush_dcache();

	jzmac_restart_tx_dma();

	/* wait pkt send finish */
	do {
		dma_status_reg = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaStatus);
		//printf("=====>dma_status_reg = 0x%08x\n", dma_status_reg);

		interrupt = synopGMAC_get_interrupt_type(gmacdev);
		if (interrupt & synopGMACDmaError) {
			printf("====>GMAC dma error\n");
			/* TODO: re-init GMAC here */
			ret = -1;
			goto out;
		}

		if (interrupt & synopGMACDmaTxNormal) {
			/* a packet send successfully */
			ret = 1;
			break;
		}

		if (interrupt & synopGMACDmaTxAbnormal) {
			//printf("====>GMAC TX abnormal!\n");
			synopGMAC_resume_dma_tx(gmacdev);
			ret = -1;
			goto out;
		}

		if (interrupt & synopGMACDmaTxStopped) {
			printf("====>GMAC Dma Stopped!\n");
			ret = -1;
			goto out;
		}
		udelay(10);
		printf("loop in tx\n");		// add, yjt, 20130726
	} while(1);

	/* ok, we are successfully sent, check the status */
	if(!synopGMAC_is_desc_valid(desc->status))
		ret = -1;
 out:
	next_tx++;

	if (next_tx >= NUM_TX_DESCS)
		next_tx = 0;

	return ret;
}

static int jz_recv(struct eth_device* dev)
{
	volatile DmaDesc *desc;
	unsigned int dma_status_reg, interrupt;
	int ret = 1;

	u32	length;
	int	i;
	char	c;

	for (;;) {
	retry:
		desc = rx_desc + next_rx;
		// printf("=====>%s:%d: desc = %p\n", __func__, __LINE__, desc);

		do {
			dma_status_reg = synopGMACReadReg((u32 *)gmacdev->DmaBase, DmaStatus);
			interrupt = synopGMAC_get_interrupt_type(gmacdev);

			if (interrupt & synopGMACDmaError) {
				printf("====>GMAC dma error\n");
				/* TODO: re-init GMAC here */
				ret = -1;
				break;
			}

			if (interrupt & synopGMACDmaRxNormal) {
				ret = 0;
				break;
			}

			if (interrupt & synopGMACDmaRxStopped) {
				printf("===>GMAC rx dma stopped!\n");
				synopGMAC_enable_dma_rx(gmacdev);
				goto retry;
			}

			if (interrupt & synopGMACDmaRxAbnormal) {
				// printf("====>GMaC rx abnormal\n");
				synopGMAC_resume_dma_rx(gmacdev);
				ret = -1;
				break;
			}
			udelay(10);
			printf("loop in eth recv\n");	// add, yjt, 20130726
		} while(1);

		if (ret >= 0) {
			length = synopGMAC_get_rx_desc_frame_length(desc->status);
			/* Pass the packet up to the protocol layers */
			jzmac_dump_arp_reply(NetRxPackets[next_rx], length - 4);
			jzmac_dump_icmp_reply(NetRxPackets[next_rx], length - 4);
			NetReceive(NetRxPackets[next_rx], length - 4);
		} else {
			// jzmac_dump_all_desc();
			// jzmac_dump_all_regs(__func__, __LINE__);
			return ret;
		}

		/*  re-init descriptor */
		synopGMAC_rx_desc_init_ring(desc, next_rx == (NUM_RX_DESCS - 1));

		desc->length |= ((PKTSIZE_ALIGN <<DescSize1Shift) & DescSize1Mask) |
			((0 << DescSize2Shift) & DescSize2Mask);
		desc->buffer1 = virt_to_phys(NetRxPackets[next_rx]);

		desc->extstatus = 0;
		desc->reserved1 = 0;
		desc->timestamplow = 0;
		desc->timestamphigh = 0;

		jz_flush_dcache();

		/* start transfer */
		desc->status = DescOwnByDma;
		synopGMAC_resume_dma_rx(gmacdev);

		next_rx++;
		if (next_rx >= NUM_RX_DESCS)
			next_rx = 0;
	}

}

#define __gpio_as_eth4775()                             \
	do {                                            \
		REG_GPIO_PXINTC(1) =  0x00000010;       \
		REG_GPIO_PXMASKC(1) = 0x00000010;       \
		REG_GPIO_PXPAT1C(1) = 0x00000010;       \
		REG_GPIO_PXPAT0S(1) = 0x00000010;       \
		REG_GPIO_PXINTC(3) =  0x3c000000;       \
		REG_GPIO_PXMASKC(3) = 0x3c000000;       \
		REG_GPIO_PXPAT1C(3) = 0x3c000000;       \
		REG_GPIO_PXPAT0S(3) = 0x3c000000;       \
		REG_GPIO_PXINTC(5) =  0x0000fff0;       \
		REG_GPIO_PXMASKC(5) = 0x0000fff0;       \
		REG_GPIO_PXPAT1C(5) = 0x0000fff0;       \
		REG_GPIO_PXPAT0C(5) = 0x0000fff0;       \
	} while (0)


static int jz_init(struct eth_device* dev, bd_t * bd)
{
	int i;
	int phy_id;

	
	/* init global pointers */
	tx_desc = (DmaDesc *)((unsigned long)_tx_desc | 0xa0000000);
	rx_desc = (DmaDesc *)((unsigned long)_rx_desc | 0xa0000000);

	//printf("===>init GMAC......\n");

	/* reset GMAC, prepare to search phy */
	synopGMAC_reset(gmacdev);

	/* we do not process interrupts */
	synopGMAC_disable_interrupt_all(gmacdev);

	// Set MAC address
	synopGMAC_set_mac_addr(gmacdev,
			       GmacAddr0High,GmacAddr0Low,
			       eth_get_dev()->enetaddr);

	synopGMAC_set_mdc_clk_div(gmacdev,GmiiCsrClk2);
	gmacdev->ClockDivMdc = synopGMAC_get_mdc_clk_div(gmacdev);

	/* search phy */
	phy_id = synopGMAC_search_phy(gmacdev);
	if (phy_id >= 0) {
		printf("====>found PHY %d\n", phy_id);
		gmacdev->PhyBase = phy_id;
	} else {
		printf("====>PHY not found!\n");
	}

	/* setup tx_desc */
	for (i = 0; i <  NUM_TX_DESCS; i++) {
		synopGMAC_tx_desc_init_ring(tx_desc + i, i == (NUM_TX_DESCS - 1));
	}
	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaTxBaseAddr, virt_to_phys(_tx_desc));

	/* setup rx_desc */
	for (i = 0; i < NUM_RX_DESCS; i++) {
		DmaDesc *curr_desc = rx_desc + i;
		synopGMAC_rx_desc_init_ring(curr_desc, i == (NUM_RX_DESCS - 1));

		curr_desc->length |= ((PKTSIZE_ALIGN <<DescSize1Shift) & DescSize1Mask) |
			((0 << DescSize2Shift) & DescSize2Mask);
		curr_desc->buffer1 = virt_to_phys(NetRxPackets[i]);

		curr_desc->extstatus = 0;
		curr_desc->reserved1 = 0;
		curr_desc->timestamplow = 0;
		curr_desc->timestamphigh = 0;

		/* start transfer */
		curr_desc->status = DescOwnByDma;
	}
	synopGMACWriteReg((u32 *)gmacdev->DmaBase,DmaRxBaseAddr, virt_to_phys(_rx_desc));

	jz_flush_dcache();

	jz4775_mac_configure();

	jzmac_dump_all_regs(__func__, __LINE__);
	/* we only enable rx here */
	synopGMAC_enable_dma_rx(gmacdev);

	printf("GMAC init finish\n");

	return 1;
}

static void jz_halt(struct eth_device *dev)
{
	synopGMAC_disable_dma_rx(gmacdev);
	synopGMAC_disable_dma_tx(gmacdev);
}

int jz_enet_initialize(bd_t *bis)
{
	struct eth_device	*dev;

	dev = (struct eth_device *) malloc(sizeof *dev);
	memset(dev, 0, sizeof *dev);

#if 0
	sprintf(dev->name, "JZ ETHERNET");
#else
	sprintf(dev->name, "JZ ETH4775");
#endif

	dev->iobase	= 0;
	dev->priv	= 0;
	dev->init	= jz_init;
	dev->halt	= jz_halt;
	dev->send	= jz_send;
	dev->recv	= jz_recv;

	gmacdev = &_gmacdev;
#define JZ_GMAC_BASE 0xb34b0000
	gmacdev->DmaBase =  JZ_GMAC_BASE + DMABASE;
	gmacdev->MacBase =  JZ_GMAC_BASE + MACBASE;

	eth_register(dev);

	__cpm_start_mac();

	__gpio_clear_pin(32 * 1 + 4);
	__gpio_clear_pin(32 * 3 + 29);
	__gpio_clear_pin(32 * 3 + 28);
	__gpio_clear_pin(32 * 3 + 27);
	__gpio_clear_pin(32 * 3 + 26);
	REG_GPIO_PXINTC(5)  = 0x0000fff0;
	REG_GPIO_PXMASKS(5) = 0x0000fff0;
	REG_GPIO_PXPAT1C(5) = 0x0000fff0;
	REG_GPIO_PXPAT0C(5) = 0x0000ff70;
	REG_GPIO_PXPAT0S(5) = 0x80;

#if 0	// del, yjt, 20130726

	__gpio_as_output0(32 * 5 + 1);
	udelay(100000);
	__gpio_as_output1(32 * 5 + 1);
	
#else

#define PIN_PHY_RESET	(32 * 3 + 21)
    __gpio_as_output1(PIN_PHY_RESET);
  	udelay(100000);
	__gpio_as_output0(PIN_PHY_RESET);
	udelay(100000);
	__gpio_as_output1(PIN_PHY_RESET);

#endif

	__gpio_as_eth4775();

	return 1;
}

