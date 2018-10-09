#ifndef __LCD_PANEL_H__
#define __LCD_PANEL_H__

#include <asm/jzsoc.h>

#if defined(CONFIG_LCD_TOPPOLY_TD043MGEB1)
struct jzlcd_info jzfb = 
{
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 18bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP,	/* Vsync polarity: leading edge is falling edge */
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_64,	/* 16words burst, enable out FIFO underrun irq */
		800, 480, 60, 1, 1, 40, 215, 10, 34,
		.type = DISP_PANEL_TYPE_LCD,
	},
	.osd = {
		 .osd_cfg = LCD_OSDC_OSDEN | /* Use OSD mode */
		 LCD_OSDC_ALPHAEN | /* enable alpha */
		 (1 << LCD_OSDC_COEF_SLE0_BIT) | /*src over mode*/           
		 (3 << LCD_OSDC_COEF_SLE1_BIT) |
		 (1 << 16) |
	         LCD_OSDC_PREMULT1 | 		
	         LCD_OSDC_PREMULT0 | 		
//		 LCD_OSDC_F1EN | /* enable Foreground1 */
		 LCD_OSDC_F0EN,	/* enable Foreground0 */
		 .osd_ctrl = 0,		/* disable ipu,  */
		 .rgb_ctrl = 0,
		 .bgcolor0 = 0x0, /* set background color Black */
		 .bgcolor1 = 0x0, /* set background color Black */
		 .colorkey0 = 0,  /* disable colorkey */
		 .colorkey1 = 0,  /* disable colorkey */
		 .alpha0 = 0xA0,	/* alpha value */
		 .alpha1 = 0xA0,	/* alpha value */
		 .fg0 = {32, 0, 0, 800, 480}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 800, 480}, /* bpp, x, y, w, h */
	 }
};


void special_pin_init(void *ptr)
{
#if 0	
	__gpio_as_output0(GPIO_LCD_RETET);	 
	udelay(100);			  
	__gpio_as_output1(GPIO_LCD_RETET);	  
	udelay(50);
#endif	
}

void display_panel_init(void *ptr)
{
	special_pin_init(ptr);
}

#endif 

#endif //__LCD_PANEL_H__ 
