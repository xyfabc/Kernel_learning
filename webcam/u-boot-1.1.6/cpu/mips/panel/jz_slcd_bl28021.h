#ifndef __JZ_SLCD_BL28021_H__
#define __JZ_SLCD_BL28021_H__

#include <asm/jzsoc.h>

#if defined(CONFIG_SLCD_BG28021)

#if 1
#define mdelay(m)	\
do {			\
	int n = m;	\
	while(n--)	\
		udelay(1000);	\
}while(0)
#else
#define mdelay		udelay
#endif

#define WR_GRAM_CMD	0x0022

#if defined(CONFIG_JZ4775)
	#define PIN_CS_N 	(32 * 2 + 11)	// Chip select 	//GPC11;
	#define PIN_RESET_N 	(32 * 2 + 10)	/* LCD_REV GPF6 nor data0 */
#elif defined(CONFIG_JZ4775_ORION)
	#define PIN_CS_N 	(32 * 2 + 21)	// Chip select 	//GPC21;
	#define PIN_RESET_N 	(32 * 3 + 22)	/* LCD_RESET GPD22 nor data0 */
#elif defined(CONFIG_JZ4775_MENSA)
	#define PIN_CS_N 	(32 * 2 + 11)	// Chip select 	//GPC21;
	#define PIN_RESET_N 	(32 * 2 + 10)	/* LCD_REV GPB28  */
#else
	#error "Define special lcd pins for your platform."
#endif

#define __slcd_dmamode_oneshot()      ( REG_SLCD_CTRL |= SLCD_CTRL_DMA_MODE)
#define __slcd_dmamode_continue()    ( REG_SLCD_CTRL &= ~SLCD_CTRL_DMA_MODE)

struct jzlcd_info jzfb = 
{
	.panel = {
//		 .cfg = LCD_CFG_LCDPIN_SLCD | LCD_CFG_RECOVER | /* Underrun recover*/
		 .cfg = LCD_CFG_LCDPIN_SLCD | /* Underrun recover*/
		 LCD_CFG_NEWDES | /* 8words descriptor */
		 LCD_CFG_MODE_SLCD, /* TFT Smart LCD panel */
		 .slcd_cfg = SLCD_CFG_DWIDTH_18BIT | SLCD_CFG_CWIDTH_18BIT | SLCD_CFG_CS_ACTIVE_LOW | SLCD_CFG_RS_CMD_LOW | SLCD_CFG_CLK_ACTIVE_FALLING | SLCD_CFG_TYPE_PARALLEL,
		 .ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		 240, 320, 60, 0, 0, 0, 0, 0, 0,
		 .type = DISP_PANEL_TYPE_SLCD,
	 },
	.osd = {
		 .osd_cfg = LCD_OSDC_OSDEN | /* Use OSD mode */
		 LCD_OSDC_ALPHAEN | /* enable alpha */
		 (1 << LCD_OSDC_COEF_SLE0_BIT) | /*src over mode*/           
		 (3 << LCD_OSDC_COEF_SLE1_BIT) |
		 (1 << 16) |
	      LCD_OSDC_PREMULT0 | 		
//		 LCD_OSDC_F1EN | /* enable Foreground1 */
		 LCD_OSDC_F0EN,	/* enable Foreground0 */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor0 = 0x0, /* set background color Black */
		 .bgcolor1 = 0x0, /* set background color Black */
		 .colorkey0 = 0, /* disable colorkey */
		 .colorkey1 = 0, /* disable colorkey */
		 .alpha0 = 0xFF,	/* alpha value */		// modify, yjt, 20130521
		 .alpha1 = 0xFF,	/* alpha value */		// modify, yjt, 20130521
		 .ipu_restart = 0x80001000, /* ipu restart */
		 .fg0 = {32, 0, 0, 240, 320}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 240, 320}, /* bpp, x, y, w, h */
	 }
};

/* Sent a command with data (18-bit bus, 16-bit index, 16-bit register value) */
static void Mcupanel_RegSet(unsigned int cmd, unsigned int data)
{
        unsigned int timecnt1 = 0;
        unsigned int timecnt2 = 0;
        //the value is tested,and don't change it 
        const unsigned int timeout = 0xFFFFF;
	//printf("Mcupanel RegSet\n");

        cmd = ((cmd & 0xff) << 1) | ((cmd & 0xff00) << 2);
        data = ((data & 0xff) << 1) | ((data & 0xff00) << 2);
        data = ((data<<6)&0xfc0000)|((data<<4)&0xfc00) | ((data<<2)&0xfc);

        while((REG_SLCD_STATE & SLCD_STATE_BUSY) && ++ timecnt1 < timeout);
        REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | cmd;

        while((REG_SLCD_STATE & SLCD_STATE_BUSY) && ++ timecnt2 < timeout);
        REG_SLCD_DATA = SLCD_DATA_RS_DATA | data;

        if(timecnt1 >= timeout || timecnt1 >= timeout)
                printf("SLCD Mcupanel_RegSet timeout!!!\n");
}

/* Sent a command without data  (18-bit bus, 16-bit index) */
static void Mcupanel_Command(unsigned int cmd) 
{
	unsigned int timecnt = 0;
        //the value is tested,and don't change it 
        const unsigned int timeout = 0xFFFFF;

        while((REG_SLCD_STATE & SLCD_STATE_BUSY) && ++ timecnt < timeout);
        REG_SLCD_DATA = SLCD_DATA_RS_COMMAND | ((cmd&0xff00) << 2) | ((cmd&0xff) << 1);

        if(timecnt >= timeout)
                printf("SLCD Mcupanel_Command timeout!!!\n");
}

static void Mcupanel_Data(unsigned int data) {
	while (REG_SLCD_STATE & SLCD_STATE_BUSY);
	REG_SLCD_DATA = ((data&0xff00) << 2) | ((data&0xff) << 1);
}

/* Set the start address of screen, for example (0, 0) */
void Mcupanel_SetAddr(u32 x, u32 y) //u32
{
	Mcupanel_RegSet(0x200,x) ;
	udelay(1);
	Mcupanel_RegSet(0x201,y) ;
	udelay(1);
	Mcupanel_Command(0x202);
}

/* Set the start address of screen to (0,0)*/
void Mcupanel_ResetStartAddr(void) //u32
{
        __slcd_dmamode_oneshot();
        REG_SLCD_STATE = 0;

        Mcupanel_RegSet(0x0002, 0x0000);
        Mcupanel_RegSet(0x0003, 0x0000);/*Column Start*/
        Mcupanel_RegSet(0x0004, 0x0000);
        Mcupanel_RegSet(0x0005, 0x00EF);/*Column End*/

        Mcupanel_RegSet(0x0006, 0x0000);
        Mcupanel_RegSet(0x0007, 0x0000);/*Row Start*/
        Mcupanel_RegSet(0x0008, 0x0001);
        Mcupanel_RegSet(0x0009, 0x003f);/*Row End*/

        Mcupanel_Command(0x0022);               /*Start GRAM write*/
        udelay(50);

        __slcd_dmamode_continue();
}

static void special_pin_init(void)
{ 
	__gpio_as_output0(PIN_CS_N); /* Clear CS */	
	mdelay(100);	
	__gpio_as_output1(PIN_RESET_N);	
	mdelay(10);	
	__gpio_as_output0(PIN_RESET_N);	
	mdelay(10);
	__gpio_as_output1(PIN_RESET_N);	
	mdelay(100);	
}

static void slcd_panel_init(void)	
{      
	__gpio_as_output1(PIN_RESET_N);	
	mdelay(100);	
	__gpio_as_output0(PIN_RESET_N);	
	mdelay(200);	
	__gpio_as_output1(PIN_RESET_N);	
	mdelay(100);
	Mcupanel_RegSet(0x00EA, 0x0000);	
	mdelay(50); 		
	__gpio_as_output0(PIN_RESET_N);	
	mdelay(200);	
	__gpio_as_output1(PIN_RESET_N);	
	mdelay(100);
#if 0			// modify slcd config data, yjt, 20130717, rotate display
	Mcupanel_RegSet(0x002E, 0x0089);
        Mcupanel_RegSet(0x0029, 0x008F);
        Mcupanel_RegSet(0x002B, 0x0002);
        Mcupanel_RegSet(0x00E4, 0x0001);
        Mcupanel_RegSet(0x00E5, 0x0010);
        Mcupanel_RegSet(0x00E6, 0x0001);
        Mcupanel_RegSet(0x00F7, 0x0010);
        Mcupanel_RegSet(0x00EA, 0x0000);
        Mcupanel_RegSet(0x00EB, 0x0020);
        Mcupanel_RegSet(0x00EC, 0x003C);
        Mcupanel_RegSet(0x00ED, 0x00C8);
        Mcupanel_RegSet(0x00E8, 0x0078);
        Mcupanel_RegSet(0x00E9, 0x0038);
        Mcupanel_RegSet(0x00F1, 0x0001);
        Mcupanel_RegSet(0x00F2, 0x0000);
        Mcupanel_RegSet(0x0040, 0x0000);
        Mcupanel_RegSet(0x0041, 0x0000);
        Mcupanel_RegSet(0x0042, 0x0000);
        Mcupanel_RegSet(0x0043, 0x0019);
        Mcupanel_RegSet(0x0044, 0x001B);
        Mcupanel_RegSet(0x0045, 0x003F);
        Mcupanel_RegSet(0x0046, 0x0011);
        Mcupanel_RegSet(0x0047, 0x0060);
        Mcupanel_RegSet(0x0048, 0x0000);
        Mcupanel_RegSet(0x0049, 0x0016);
        Mcupanel_RegSet(0x004A, 0x0019);
        Mcupanel_RegSet(0x004B, 0x0019);
        Mcupanel_RegSet(0x004C, 0x0018);
        Mcupanel_RegSet(0x0050, 0x0000);
        Mcupanel_RegSet(0x0051, 0x0024);
	Mcupanel_RegSet(0x0052, 0x0026);
        Mcupanel_RegSet(0x0053, 0x003F);
        Mcupanel_RegSet(0x0054, 0x003F);
        Mcupanel_RegSet(0x0055, 0x003F);
        Mcupanel_RegSet(0x0056, 0x001F);
        Mcupanel_RegSet(0x0057, 0x006E);
        Mcupanel_RegSet(0x0058, 0x0007);
        Mcupanel_RegSet(0x0059, 0x0006);
        Mcupanel_RegSet(0x005A, 0x0006);
        Mcupanel_RegSet(0x005B, 0x0009);
        Mcupanel_RegSet(0x005C, 0x001F);
        Mcupanel_RegSet(0x005D, 0x00CC);
        Mcupanel_RegSet(0x001B, 0x001B);
        Mcupanel_RegSet(0x001A, 0x0002);
        Mcupanel_RegSet(0x0024, 0x0062);
        Mcupanel_RegSet(0x0025, 0x006A);
        Mcupanel_RegSet(0x0023, 0x0071);
        Mcupanel_RegSet(0x0018, 0x0034);
        Mcupanel_RegSet(0x0019, 0x0001);
        Mcupanel_RegSet(0x0001, 0x0000);
        Mcupanel_RegSet(0x001F, 0x0088);
	mdelay(5);
        Mcupanel_RegSet(0x001F, 0x0080);
        mdelay(5);
        Mcupanel_RegSet(0x001F, 0x0090);
        mdelay(5);
        Mcupanel_RegSet(0x001F, 0x00D4);
        mdelay(5);
        Mcupanel_RegSet(0x0017, 0x0005);
        Mcupanel_RegSet(0x0036, 0x0009);
        Mcupanel_RegSet(0x0028, 0x0038);
        mdelay(50);
        Mcupanel_RegSet(0x0028, 0x003f);
        Mcupanel_RegSet(0x0002, 0x0000);
        Mcupanel_RegSet(0x0003, 0x0000);
        Mcupanel_RegSet(0x0004, 0x0000);
        Mcupanel_RegSet(0x0005, 0x00EF);
        Mcupanel_RegSet(0x0006, 0x0000);
        Mcupanel_RegSet(0x0007, 0x0000);
        Mcupanel_RegSet(0x0008, 0x0001);
        Mcupanel_RegSet(0x0009, 0x003f);
        Mcupanel_Command(0x0022);
#else
	Mcupanel_RegSet(0x0016, 0x00c0);/*rotate image, zyl*/    
	Mcupanel_RegSet(0x002E, 0x0089);	
	Mcupanel_RegSet(0x0029, 0x008F);	
	Mcupanel_RegSet(0x002B, 0x0002);	
	Mcupanel_RegSet(0x00E4, 0x0001);	
	Mcupanel_RegSet(0x00E5, 0x0010);	
	Mcupanel_RegSet(0x00E6, 0x0001);	
	Mcupanel_RegSet(0x00F7, 0x0010);	
	Mcupanel_RegSet(0x00EA, 0x0000);	
	Mcupanel_RegSet(0x00EB, 0x0020);	
	Mcupanel_RegSet(0x00EC, 0x003C);	
	Mcupanel_RegSet(0x00ED, 0x00C8);	
	Mcupanel_RegSet(0x00E8, 0x0078);	
	Mcupanel_RegSet(0x00E9, 0x0038);	
	Mcupanel_RegSet(0x00F1, 0x0001);	
	Mcupanel_RegSet(0x00F2, 0x0000);	
	Mcupanel_RegSet(0x0040, 0x0000);	
	Mcupanel_RegSet(0x0041, 0x0000);	
	Mcupanel_RegSet(0x0042, 0x0000);	
	Mcupanel_RegSet(0x0043, 0x0019);	
	Mcupanel_RegSet(0x0044, 0x001B);	
	Mcupanel_RegSet(0x0045, 0x003F);	
	Mcupanel_RegSet(0x0046, 0x0011);	
	Mcupanel_RegSet(0x0047, 0x0060);	
	Mcupanel_RegSet(0x0048, 0x0000);	
	Mcupanel_RegSet(0x0049, 0x0016);	
	Mcupanel_RegSet(0x004A, 0x0019);	
	Mcupanel_RegSet(0x004B, 0x0019);	
	Mcupanel_RegSet(0x004C, 0x0018);	
	Mcupanel_RegSet(0x0050, 0x0000);	
	Mcupanel_RegSet(0x0051, 0x0024);	
	Mcupanel_RegSet(0x0052, 0x0026);	
	Mcupanel_RegSet(0x0053, 0x003F);	
	Mcupanel_RegSet(0x0054, 0x003F);	
	Mcupanel_RegSet(0x0055, 0x003F);	
	Mcupanel_RegSet(0x0056, 0x001F);	
	Mcupanel_RegSet(0x0057, 0x006E);	
	Mcupanel_RegSet(0x0058, 0x0007);	
	Mcupanel_RegSet(0x0059, 0x0006);	
	Mcupanel_RegSet(0x005A, 0x0006);	
	Mcupanel_RegSet(0x005B, 0x0009);	
	Mcupanel_RegSet(0x005C, 0x001F);	
	Mcupanel_RegSet(0x005D, 0x00CC);	
	Mcupanel_RegSet(0x001B, 0x001B);	
	Mcupanel_RegSet(0x001A, 0x0002);	
	Mcupanel_RegSet(0x0024, 0x0062);	
	Mcupanel_RegSet(0x0025, 0x006A);	
	Mcupanel_RegSet(0x0023, 0x0071);	
	Mcupanel_RegSet(0x0018, 0x0034);	
	Mcupanel_RegSet(0x0019, 0x0001);	
	Mcupanel_RegSet(0x0001, 0x0000);	
	Mcupanel_RegSet(0x001F, 0x0088);	
	mdelay(5);	
	Mcupanel_RegSet(0x001F, 0x0080);	
	mdelay(5);	
	Mcupanel_RegSet(0x001F, 0x0090);	
	mdelay(5);	
	Mcupanel_RegSet(0x001F, 0x00D4);	
	mdelay(5);	
	Mcupanel_RegSet(0x0017, 0x0005);	
	Mcupanel_RegSet(0x0036, 0x0009);	
	Mcupanel_RegSet(0x0028, 0x0038);	
	mdelay(50);	
	Mcupanel_RegSet(0x0028, 0x003f);	
	Mcupanel_RegSet(0x0002, 0x0000);	
	Mcupanel_RegSet(0x0003, 0x0000);	
	Mcupanel_RegSet(0x0004, 0x0000);	
	Mcupanel_RegSet(0x0005, 0x00EF);	
	Mcupanel_RegSet(0x0006, 0x0000);	
	Mcupanel_RegSet(0x0007, 0x0000);	
	Mcupanel_RegSet(0x0008, 0x0001);	
	Mcupanel_RegSet(0x0009, 0x003f);	
	Mcupanel_Command(0x0022);  
#endif
        mdelay(50);
} 

static void slcd_bus_init(void)
{	
	__slcd_set_data_18bit();
	__slcd_set_cmd_18bit();
	__slcd_set_cs_low();
	__slcd_set_rs_low();
	__slcd_set_clk_falling();
	__slcd_set_parallel_type();
}


void display_panel_init(void *ptr)
{
	slcd_bus_init();
	special_pin_init();
	slcd_panel_init();
}

#endif

#endif  /* __JZ_SLCD_BL28021_H__ */
