/*

		60B
 * JzRISC lcd controller
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

/* 
 * Fallowing macro may be used:
 *  CONFIG_LCD                        : LCD support 
 *  LCD_BPP                           : Bits per pixel, 0 = 1, 1 = 2, 2 = 4, 3 = 8
 *  CFG_WHITE_ON_BLACK
 *  CONFIG_LCD_LOGO                   : show logo
 *  CFG_LCD_LOGOONLY_NOINFO           : not display info on lcd screen, only logo
 * -----------------------------------------------------------------------
 * bugs:
 * if BMP_LOGO_HEIGHT > (lcd screen height - 2*VIDEO_FONT_HEIGHT),
 * must not print info onto screen,
 * it means should define CFG_LCD_LOGOONLY_NOINFO.
 */



#include <config.h>
#include <common.h>
#include <devices.h>
#include <lcd.h>

#include <asm/io.h>               /* virt_to_phys() */

#if defined(CONFIG_JZ4775)
#if defined(CONFIG_LCD)
#include <asm/jz4775.h>
#include "jz4775_lcd.h"

#define DEBUG 0

#if defined(CONFIG_SLCD_KGM_SPFD5420A)
	#include "panel/jz_kgm_spfd5420a.h"
#elif defined(CONFIG_LCD_TOPPOLY_TD043MGEB1)
	#include "panel/jz_toppoly_td043mgeb1.h"
#elif defined(CONFIG_SLCD_BG28021)
	#include "panel/jz_slcd_bl28021.h"
#elif defined(CONFIG_JZ_LCD_BL35026V0)
	#include "panel/jz_lcd_bl35026v0.h"
#else
	#error "Select LCD panel first!!!"
#endif


/************************************************************************/

vidinfo_t panel_info = {
#if defined(CONFIG_SLCD_KGM_SPFD5420A)
	400, 240, LCD_BPP,
#elif defined(CONFIG_LCD_TOPPOLY_TD043MGEB1)
	800, 480, LCD_BPP,
#elif defined(CONFIG_SLCD_BG28021)
	240, 320, LCD_BPP,
#elif defined(CONFIG_JZ_LCD_BL35026V0)
	320, 240, LCD_BPP,
#else
	#error "Select LCD panel first!!!"
#endif

};

/*
 * Frame buffer memory information
 */
void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;	/* Start of console buffer	*/

//lcd.c need these
short console_col;
short console_row;

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

/************************************************************************/
void lcd_ctrl_init (void *lcdbase);
void lcd_enable (void);
void lcd_disable (void);

/************************************************************************/
extern int flush_cache_all(void);

/************************************************************************/
#if DEBUG
void print_lcdc_registers(void)	/* debug */
{
	/* LCD Controller Resgisters */
	printf("======== lcd =============\n");
	printf("REG_LCD_CFG:\t0x%08x\n", REG_LCD_CFG);
	printf("REG_LCD_CTRL:\t0x%08x\n", REG_LCD_CTRL);
	printf("REG_LCD_STATE:\t0x%08x\n", REG_LCD_STATE);
	printf("REG_LCD_OSDC:\t0x%08x\n", REG_LCD_OSDC);
	printf("REG_LCD_OSDCTRL:\t0x%08x\n", REG_LCD_OSDCTRL);
	printf("REG_LCD_OSDS:\t0x%08x\n", REG_LCD_OSDS);
	printf("REG_LCD_BGC0:\t0x%08x\n", REG_LCD_BGC0);
	printf("REG_LCD_BGC1:\t0x%08x\n", REG_LCD_BGC1);
	printf("REG_LCD_KEK0:\t0x%08x\n", REG_LCD_KEY0);
	printf("REG_LCD_KEY1:\t0x%08x\n", REG_LCD_KEY1);
	printf("REG_LCD_ALPHA:\t0x%08x\n", REG_LCD_ALPHA);
	printf("REG_LCD_IPUR:\t0x%08x\n", REG_LCD_IPUR);
	printf("REG_LCD_VAT:\t0x%08x\n", REG_LCD_VAT);
	printf("REG_LCD_DAH:\t0x%08x\n", REG_LCD_DAH);
	printf("REG_LCD_DAV:\t0x%08x\n", REG_LCD_DAV);
	printf("REG_LCD_XYP0:\t0x%08x\n", REG_LCD_XYP0);
	printf("REG_LCD_XYP1:\t0x%08x\n", REG_LCD_XYP1);
	printf("REG_LCD_SIZE0:\t0x%08x\n", REG_LCD_SIZE0);
	printf("REG_LCD_SIZE1:\t0x%08x\n", REG_LCD_SIZE1);
	printf("REG_LCD_RGBC\t0x%08x\n", REG_LCD_RGBC);
	printf("REG_LCD_VSYNC:\t0x%08x\n", REG_LCD_VSYNC);
	printf("REG_LCD_HSYNC:\t0x%08x\n", REG_LCD_HSYNC);
	printf("REG_LCD_PS:\t0x%08x\n", REG_LCD_PS);
	printf("REG_LCD_CLS:\t0x%08x\n", REG_LCD_CLS);
	printf("REG_LCD_SPL:\t0x%08x\n", REG_LCD_SPL);
	printf("REG_LCD_REV:\t0x%08x\n", REG_LCD_REV);
	printf("REG_LCD_IID:\t0x%08x\n", REG_LCD_IID);
	printf("REG_LCD_DA0:\t0x%08x\n", REG_LCD_DA0);
	printf("REG_LCD_SA0:\t0x%08x\n", REG_LCD_SA0);
	printf("REG_LCD_FID0:\t0x%08x\n", REG_LCD_FID0);
	printf("REG_LCD_CMD0:\t0x%08x\n", REG_LCD_CMD0);
	printf("REG_LCD_OFFS0:\t0x%08x\n", REG_LCD_OFFS0);
	printf("REG_LCD_PW0:\t0x%08x\n", REG_LCD_PW0);
	printf("REG_LCD_CNUM0:\t0x%08x\n", REG_LCD_CNUM0);
	printf("REG_LCD_DESSIZE0:\t0x%08x\n", REG_LCD_DESSIZE0);
	printf("REG_LCD_DA1:\t0x%08x\n", REG_LCD_DA1);
	printf("REG_LCD_SA1:\t0x%08x\n", REG_LCD_SA1);
	printf("REG_LCD_FID1:\t0x%08x\n", REG_LCD_FID1);
	printf("REG_LCD_CMD1:\t0x%08x\n", REG_LCD_CMD1);
	printf("REG_LCD_OFFS1:\t0x%08x\n", REG_LCD_OFFS1);
	printf("REG_LCD_PW1:\t0x%08x\n", REG_LCD_PW1);
	printf("REG_LCD_CNUM1:\t0x%08x\n", REG_LCD_CNUM1);
	printf("REG_LCD_DESSIZE1:\t0x%08x\n", REG_LCD_DESSIZE1);
	printf("==================================\n");
	printf("REG_LCD_VSYNC:\t%d:%d\n", REG_LCD_VSYNC >> 16, REG_LCD_VSYNC & 0xfff);
	printf("REG_LCD_HSYNC:\t%d:%d\n", REG_LCD_HSYNC >> 16, REG_LCD_HSYNC & 0xfff);
	printf("REG_LCD_VAT:\t%d:%d\n", REG_LCD_VAT >> 16, REG_LCD_VAT & 0xfff);
	printf("REG_LCD_DAH:\t%d:%d\n", REG_LCD_DAH >> 16, REG_LCD_DAH & 0xfff);
	printf("REG_LCD_DAV:\t%d:%d\n", REG_LCD_DAV >> 16, REG_LCD_DAV & 0xfff);
	/* Smart LCD Controller Resgisters */
	printf("======== slcd =============\n");
	printf("REG_SLCD_CFG:\t0x%08x\n", REG_SLCD_CFG);
	printf("REG_SLCD_CTRL:\t0x%08x\n", REG_SLCD_CTRL);
	printf("REG_SLCD_STATE:\t0x%08x\n", REG_SLCD_STATE);
	printf("==================================\n");
	printf("==================================\n");
}
#endif

void gpio_init(void)
{
	if(jzfb.panel.cfg & LCD_CFG_MODE_TFT_16BIT)	
		__gpio_as_lcd_16bit();
	else if(jzfb.panel.cfg & LCD_CFG_MODE_TFT_24BIT) 
		__gpio_as_lcd_24bit();
	else
		__gpio_as_lcd_18bit();
}

/*
 * Before enabled lcd controller, lcd registers should be configured correctly.
 */
void lcd_enable (void)
{
	REG_LCD_CTRL &= ~LCD_CTRL_DIS;		//clear disable bit
	REG_LCD_CTRL |= LCD_CTRL_ENA;		//set eable bit
#if DEBUG
	print_lcdc_registers();
#endif
}

/*----------------------------------------------------------------------*/
void lcd_disable (void)
{
	REG_LCD_CTRL |= LCD_CTRL_DIS;
//	REG_LCD_CTRL &= ~(1 << 3);
}

/************************************************************************/
static int jz_lcd_init_mem(void *lcdbase, vidinfo_t *vid)
{
	u_long palette_mem_size;
	struct jz_fb_info *fbi = &vid->jz_fb;
	int fb_size = vid->vl_row * (vid->vl_col * NBITS (vid->vl_bpix)) / 8;

	fbi->screen = (u_long)lcdbase;
	fbi->palette_size = 256;
	palette_mem_size = fbi->palette_size * sizeof(u16);

	debug("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);
	/* locate palette and descs at end of page following fb */
	fbi->palette = (u_long)lcdbase + 2 * fb_size + PAGE_SIZE - palette_mem_size;

	return 0;
}

static void jz_lcd_desc_init(vidinfo_t *vid)
{
	unsigned int bpp = 0;
	struct jz_fb_info * fbi;

	switch (vid->vl_bpix) {
		case 4:
			bpp |= (LCD_CPOS_BPP_15_16 | LCD_CPOS_RGB555);
			break;
		case 5:
			bpp |= LCD_CPOS_BPP_18_24;	/* target is 4bytes/pixel */
			break;
		default:
			printf("The BPP %d is not supported\n", 1 << panel_info.vl_bpix);
			bpp |= LCD_CPOS_BPP_18_24;
			break;
	}

	fbi = &vid->jz_fb;
	fbi->dma_desc0 = (struct jz_fb_dma_descriptor *)((unsigned int)fbi->palette - 1 * 32);
	fbi->dma_desc1 = (struct jz_fb_dma_descriptor *)((unsigned int)fbi->palette - 2 * 32);
	fbi->dma_desc_dummy = (struct jz_fb_dma_descriptor *)((unsigned int)fbi->palette - 3 * 32);
	fbi->dma_desc_palette = (struct jz_fb_dma_descriptor *)((unsigned int)fbi->palette - 4 * 32);


//	#define BYTES_PER_PANEL	 (vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8)
#define BYTES_PER_PANEL	 (((vid->vl_col * NBITS(vid->vl_bpix) / 8 + 3) >> 2 << 2) * vid->vl_row)

	/* populate descriptors */
#if 0
	//we don't support palette for jz4775
	fbi->dma_desc_palette->fdadr = virt_to_phys(fbi->dma_desc0);
	fbi->dma_desc_palette->fsadr = virt_to_phys((void *)fbi->palette);
	fbi->dma_desc_palette->fidr  = 0;
	fbi->dma_desc_palette->ldcmd = (fbi->palette_size * 2) / 4 | (1 << 28);
#endif	

	//set channel 0
	fbi->dma_desc0->fdadr = virt_to_phys(fbi->dma_desc0);
	fbi->dma_desc0->fsadr = virt_to_phys((void *)(fbi->screen));
	fbi->dma_desc0->fidr  = 1;
	fbi->dma_desc0->ldcmd = BYTES_PER_PANEL / 4 ;
	fbi->dma_desc0->offsize = 0;
	fbi->dma_desc0->page_width = 0;
	fbi->dma_desc0->desc_size = ((jzfb.osd.fg0.h - 1) << LCD_DESSIZE_HEIGHT_BIT) | ((jzfb.osd.fg0.w - 1) << LCD_DESSIZE_WIDTH_BIT) | (jzfb.osd.alpha0 << LCD_DESSIZE_ALPHA_BIT);
	fbi->dma_desc0->cmd_num = bpp | (jzfb.osd.fg0.x << LCD_CPOS_XPOS_BIT) | (jzfb.osd.fg0.y << LCD_CPOS_YPOS_BIT);

	if( jzfb.osd.osd_cfg & LCD_OSDC_F0EN )
		fbi->dma_desc0->ldcmd |= LCD_CMD_FRM_EN;
	else
		fbi->dma_desc0->ldcmd &= ~LCD_CMD_FRM_EN;

	/* change alpha mode */
	if(jzfb.osd.osd_cfg & LCD_OSDC_ALPHAMD0)
		fbi->dma_desc0->cmd_num |= LCD_CPOS_ALPHAMD;
	else		
		fbi->dma_desc0->cmd_num &= ~LCD_CPOS_ALPHAMD;
	
	/* change premult*/
	if(jzfb.osd.osd_cfg & LCD_OSDC_PREMULT0)
		fbi->dma_desc0->cmd_num |= LCD_CPOS_PREMULT;
	else
		fbi->dma_desc0->cmd_num &= ~LCD_CPOS_PREMULT;	
		
	fbi->dma_desc0->cmd_num &= ~(LCD_OSDC_COEF_SLE0_MASK);
	fbi->dma_desc0->cmd_num |= ((jzfb.osd.osd_cfg & LCD_OSDC_COEF_SLE0_MASK) >> LCD_OSDC_COEF_SLE0_BIT) << LCD_CPOS_COEF_SLE_BIT;	

	//set channel 1
	if ( jzfb.panel.type == DISP_PANEL_TYPE_SLCD) {	
		fbi->dma_desc1->fdadr = virt_to_phys(fbi->dma_desc0);
		fbi->dma_desc1->fsadr = virt_to_phys((void *)(fbi->screen + BYTES_PER_PANEL)); 
		fbi->dma_desc1->fidr = 2;
		fbi->dma_desc1->ldcmd =  LCD_CMD_CMD | 2 | LCD_CMD_FRM_EN; /* length in word */
		fbi->dma_desc1->offsize = 0;
		fbi->dma_desc1->page_width = 0;
		fbi->dma_desc1->desc_size = 0;
		fbi->dma_desc1->cmd_num = 2;
		
		fbi->dma_desc_dummy->fdadr = virt_to_phys(fbi->dma_desc0);
		fbi->dma_desc_dummy->fsadr = 0; 
		fbi->dma_desc_dummy->fidr = 3;
		fbi->dma_desc_dummy->ldcmd =  LCD_CMD_CMD | 0 | LCD_CMD_FRM_EN; /* length in word */
		fbi->dma_desc_dummy->offsize = 0;
		fbi->dma_desc_dummy->page_width = 0;
		fbi->dma_desc_dummy->desc_size = 0;
		fbi->dma_desc_dummy->cmd_num = 0;
		
		fbi->dma_desc0->fdadr = virt_to_phys(fbi->dma_desc_dummy);
		fbi->fdadr0 = virt_to_phys(fbi->dma_desc1);		
	} else {
		fbi->dma_desc1->fdadr = virt_to_phys(fbi->dma_desc1);
		fbi->dma_desc1->fsadr = virt_to_phys((void *)(fbi->screen + BYTES_PER_PANEL)); 
		fbi->dma_desc1->fidr = 2;
		fbi->dma_desc1->ldcmd =  BYTES_PER_PANEL / 4; /* length in word */
		fbi->dma_desc1->offsize = 0;
		fbi->dma_desc1->page_width = 0;
		fbi->dma_desc1->desc_size = (jzfb.osd.fg1.h << 16) | jzfb.osd.fg1.w | (jzfb.osd.alpha1 << LCD_DESSIZE_ALPHA_BIT);	
		fbi->dma_desc1->cmd_num = bpp | (jzfb.osd.fg1.x << LCD_CPOS_XPOS_BIT) | (jzfb.osd.fg1.y << LCD_CPOS_YPOS_BIT);
	
		if( jzfb.osd.osd_cfg & LCD_OSDC_F1EN )
			fbi->dma_desc1->ldcmd |= LCD_CMD_FRM_EN;
		else
			fbi->dma_desc1->ldcmd &= ~LCD_CMD_FRM_EN;

		if(jzfb.osd.osd_cfg & LCD_OSDC_ALPHAMD1)
			fbi->dma_desc1->cmd_num |= LCD_CPOS_ALPHAMD;
		else		
			fbi->dma_desc1->cmd_num &= ~LCD_CPOS_ALPHAMD;
			
		/* change premult*/
		if(jzfb.osd.osd_cfg & LCD_OSDC_PREMULT0)
			fbi->dma_desc1->cmd_num |= LCD_CPOS_PREMULT;
		else
			fbi->dma_desc1->cmd_num &= ~LCD_CPOS_PREMULT;		
		
		fbi->dma_desc1->cmd_num &= ~(LCD_OSDC_COEF_SLE1_MASK);
		fbi->dma_desc1->cmd_num |= ((jzfb.osd.osd_cfg & LCD_OSDC_COEF_SLE1_MASK) >> LCD_OSDC_COEF_SLE1_BIT) << LCD_CPOS_COEF_SLE_BIT;	
		
		fbi->fdadr0 = virt_to_phys(fbi->dma_desc0);
		fbi->fdadr1 = virt_to_phys(fbi->dma_desc1);			
	}

	if (jzfb.panel.type == DISP_PANEL_TYPE_SLCD) {
		int data, i, *ptr;
		ptr = (unsigned int *)(fbi->screen + BYTES_PER_PANEL);
		data = 0x0022;	//WR_GRAM_CMD;
		data = ((data & 0xff) << 1) | ((data & 0xff00) << 2);
		for(i = 0; i < 3; i++) {
			ptr[i] = data;
		}
	}
	
	flush_cache_all();
}

static int  jz_lcd_hw_init(vidinfo_t *vid)
{
	struct jz_fb_info *fbi = &vid->jz_fb;
	unsigned int pclk, val;
    
	__cpm_start_lcd();
	lcd_disable(); 
   	
	/* Configure the LCD panel */
	REG_LCD_CTRL = jzfb.panel.ctrl; 	/* LCDC Controll Register */
	REG_LCD_CFG  = jzfb.panel.cfg;	 	/* LCDC Configure Register */
	REG_LCD_OSDC = jzfb.osd.osd_cfg | (1 << 16); 
	REG_LCD_RGBC = jzfb.osd.rgb_ctrl;
	REG_LCD_BGC0 = jzfb.osd.bgcolor0;
	REG_LCD_BGC1 = jzfb.osd.bgcolor1;
	REG_LCD_KEY0 = jzfb.osd.colorkey0;
	REG_LCD_KEY1 = jzfb.osd.colorkey1;
	REG_LCD_IPUR = jzfb.osd.ipu_restart;
	
	REG_SLCD_CFG = jzfb.panel.slcd_cfg; 	/* Smart LCD Configure Register */
	
	switch ( jzfb.panel.cfg & LCD_CFG_MODE_MASK ) {
	case LCD_CFG_MODE_GENERIC_TFT:
	case LCD_CFG_MODE_INTER_CCIR656:
	case LCD_CFG_MODE_NONINTER_CCIR656:
	case LCD_CFG_MODE_SLCD:
	default:		/* only support TFT16 TFT32, not support STN and Special TFT by now(10-06-2008)*/
		REG_LCD_VAT = (((jzfb.panel.blw + jzfb.panel.w + jzfb.panel.elw + jzfb.panel.hsw)) << 16) | (jzfb.panel.vsw + jzfb.panel.bfw + jzfb.panel.h + jzfb.panel.efw);
		REG_LCD_DAH = ((jzfb.panel.hsw + jzfb.panel.blw) << 16) | (jzfb.panel.hsw + jzfb.panel.blw + jzfb.panel.w);
		REG_LCD_DAV = ((jzfb.panel.vsw + jzfb.panel.bfw) << 16) | (jzfb.panel.vsw + jzfb.panel.bfw + jzfb.panel.h);
		REG_LCD_HSYNC = (0 << 16) | jzfb.panel.hsw;
		REG_LCD_VSYNC = (0 << 16) | jzfb.panel.vsw;
		break;
	}
	
	REG_LCD_DA0 = fbi->fdadr0; /* foreground 0 descripter*/
	REG_LCD_DA1 = fbi->fdadr1; /* foreground 1 descripter*/

	/* Timing setting */
	val = jzfb.panel.fclk; /* frame clk */
	if ( (jzfb.panel.cfg & LCD_CFG_MODE_MASK) != LCD_CFG_MODE_SERIAL_TFT) {
		pclk = val * (jzfb.panel.w + jzfb.panel.hsw + jzfb.panel.elw + jzfb.panel.blw) * (jzfb.panel.h + jzfb.panel.vsw + jzfb.panel.efw + jzfb.panel.bfw); /* Pixclk */
	}
	else {
		/* serial mode: Hsync period = 3*Width_Pixel */
		pclk = val * (jzfb.panel.w * 3 + jzfb.panel.hsw + jzfb.panel.elw + jzfb.panel.blw) * (jzfb.panel.h + jzfb.panel.vsw + jzfb.panel.efw + jzfb.panel.bfw); /* Pixclk */
	}

	val = __cpm_get_xpllout(SCLK_APLL);

	//set clock
	REG32(0xB0000064) = (1 << 28) | (val / pclk - 1);
	
	while(REG32(0xB0000064) & (1 << 27));
	if ( jzfb.panel.type == DISP_PANEL_TYPE_SLCD) { /* enable Smart LCD DMA */
		REG_LCD_STATE = 0;
		REG_LCD_OSDS = 0;
		__lcd_clr_dis();

		REG_SLCD_CTRL |= ((1 << 0) | (1 << 2));	
		REG_SLCD_CTRL &= ~(1 << 3);
		
		//first enable lcd controller, then write slcd init 
		lcd_enable(); 	/* enable lcdc */
		display_panel_init(0);
		REG_SLCD_CTRL &= ~((1 << 2)); 
	} else {
		display_panel_init(0);
		lcd_enable(); 	/* enable lcdc */
	}

	return 0;
}

void lcd_display_on(void)
{
	// open power
	// __gpio_as_output0(GPIO_LCD_VCC_EN_N);
}

void lcd_display_off(void)
{
	// close backlight
	// __gpio_as_output0(GPIO_LCD_VCC_EN_N);	
} 

void lcd_ctrl_init (void *lcdbase)
{
	__gpio_as_output0(CONFIG_GPIO_BACKLIGHT_PIN);	// yjt, defined in include/configs/mensa.h
	//open power
	lcd_display_on();
	//gpio
	gpio_init();
	jz_lcd_init_mem(lcdbase, &panel_info);
	jz_lcd_desc_init(&panel_info);
	jz_lcd_hw_init(&panel_info);
	
	//open backlight
	mdelay(50);
	__gpio_as_output1(CONFIG_GPIO_BACKLIGHT_PIN);
	
	printf("lcd bl light on\n");

//	REG_SLCD_CTRL |= SLCD_CTRL_DMA_MODE;
}


#endif /* CONFIG_LCD */
#endif //CONFIG_JZ4775
