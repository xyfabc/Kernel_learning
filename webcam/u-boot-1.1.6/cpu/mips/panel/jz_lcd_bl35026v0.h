#ifndef __JZ_LCD_BL35026V0_H__
#define __JZ_LCD_BL35026V0_H__

#include <asm/jzsoc.h>

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

#if defined(CONFIG_JZ_LCD_BL35026V0)

#define SPEN		(32 * 1 + 29)	
#define SPCK		(32 * 1 + 28)		
#define SPDA		(32 * 1 + 21)		
#define LCD_RST		(32 * 0 + 1)	

#define __spi_write_reg(reg, val) 				\
	do{ 										\
		unsigned char no;						\
		unsigned int a=0;						\
		unsigned int b=0;						\
		a=reg;									\
		b=val;									\
		__gpio_set_pin(SPEN);				\
		__gpio_set_pin(SPCK);				\
		__gpio_clear_pin(SPDA);				\
		__gpio_clear_pin(SPEN);				\
		udelay(5);							\
		for(no=0;no<24;no++)					\
		{										\
			__gpio_clear_pin(SPCK);			\
			if((a&0x00800000)==0x00800000)	\
				__gpio_set_pin(SPDA);		\
			else								\
				__gpio_clear_pin(SPDA);		\
			udelay(5);						\
			__gpio_set_pin(SPCK);			\
			a=(a<<1); 							\
			udelay(5);						\
		}										\
		__gpio_set_pin(SPEN);				\
		udelay(100);	/*udelay(100);*/		\
		__gpio_clear_pin(SPEN);				\
		udelay(5);							\
		for(no=0;no<24;no++)					\
		{										\
			__gpio_clear_pin(SPCK);			\
			if((b&0x00800000)==0x00800000)	\
				__gpio_set_pin(SPDA);		\
			else								\
				__gpio_clear_pin(SPDA);		\
			udelay(5);						\
			__gpio_set_pin(SPCK);			\
			b=(b<<1); 							\
			udelay(5);						\
		}										\
		__gpio_set_pin(SPEN);				\
		udelay(100);							\
	}while (0)

	#define __lcd_special_pin_init() 		\
	do { 										\
		__gpio_as_output1(SPEN); 				\
		__gpio_as_output1(SPCK); 				\
		__gpio_as_output1(SPDA); 				\
		__gpio_as_output1(LCD_RST);			\
		udelay(50);							\
		__gpio_clear_pin(LCD_RST);			\
		mdelay(150);							\
		__gpio_set_pin(LCD_RST);				\
		udelay(50);							\
	} while (0)

#if 0
	#define __lcd_special_on() 					\
	do { 											\
		udelay(50);								\
		__gpio_clear_pin(LCD_RST);				\
		mdelay(150);								\
		__gpio_set_pin(LCD_RST);					\
		mdelay(10);								\
		__spi_write_reg(0x700001, 0x722300); 	\
		mdelay(5);									\
		__spi_write_reg(0x700002, 0x720200); 	\
		mdelay(5);									\
		__spi_write_reg(0x700003, 0x726364); 	\
		mdelay(5);									\
		__spi_write_reg(0x700004, 0x720447); 	\
		mdelay(5);									\
		__spi_write_reg(0x700005, 0x72BCC4); 	\
		mdelay(5);									\
		__spi_write_reg(0x70000A, 0x724008); 	\
		mdelay(5);									\
		__spi_write_reg(0x70000B, 0x72D400); 	\
		mdelay(5);									\
		__spi_write_reg(0x70000D, 0x723229); 	\
		mdelay(5);									\
		__spi_write_reg(0x70000E, 0x723200); 	\
		mdelay(5);									\
		__spi_write_reg(0x70000F, 0x720000); 	\
		mdelay(5);									\
		__spi_write_reg(0x700016, 0x729F80); 	\
		mdelay(5);									\
		__spi_write_reg(0x700017, 0x720000); 	\
		mdelay(5);									\
		__spi_write_reg(0x70001E, 0x7200D2); 	\
		mdelay(5);									\
		__spi_write_reg(0x700030, 0x720000); 	\
		mdelay(5);									\
		__spi_write_reg(0x700031, 0x720407); 	\
		mdelay(5);									\
		__spi_write_reg(0x700032, 0x720202); 	\
		mdelay(5);									\
		__spi_write_reg(0x700033, 0x720000); 	\
		mdelay(5);									\
		__spi_write_reg(0x700034, 0x720505); 	\
		mdelay(5);									\
		__spi_write_reg(0x700035, 0x720003); 	\
		mdelay(5);									\
		__spi_write_reg(0x700036, 0x720707); 	\
		mdelay(5);									\
		__spi_write_reg(0x700037, 0x720000); 	\
		mdelay(5);									\
		__spi_write_reg(0x70003A, 0x720904); 	\
		mdelay(5);									\
		__spi_write_reg(0x70003B, 0x720904); 	\
		mdelay(5);									\
	} while (0)	
#else
	#define __lcd_special_on() 					\
	do {                                        \
	    __spi_write_reg(0x700001, 0x726300); 	\
		udelay(5);									\
		__spi_write_reg(0x700002, 0x720200); 	\
		udelay(5);									\
		__spi_write_reg(0x700003, 0x726364); 	\
		udelay(5);									\
		__spi_write_reg(0x700004, 0x720447); 	\
		udelay(5);									\
		__spi_write_reg(0x700005, 0x72BCC4); 	\
		udelay(5);									\
		__spi_write_reg(0x70000A, 0x724008); 	\
		udelay(5);									\
		__spi_write_reg(0x70000B, 0x72D400); 	\
		udelay(5);									\
		__spi_write_reg(0x70000D, 0x723229); 	\
		udelay(5);									\
		__spi_write_reg(0x70000E, 0x723200); 	\
		udelay(5);									\
		__spi_write_reg(0x70000F, 0x720000); 	\
		udelay(5);									\
		__spi_write_reg(0x700016, 0x729F80); 	\
		udelay(5);									\
		__spi_write_reg(0x700017, 0x720000); 	\
		udelay(5);									\
		__spi_write_reg(0x70001E, 0x7200D2); 	\
		udelay(5);									\
		__spi_write_reg(0x700030, 0x720000); 	\
		udelay(5);									\
		__spi_write_reg(0x700031, 0x720407); 	\
		udelay(5);									\
		__spi_write_reg(0x700032, 0x720202); 	\
		udelay(5);									\
		__spi_write_reg(0x700033, 0x720000); 	\
		udelay(5);									\
		__spi_write_reg(0x700034, 0x720505); 	\
		udelay(5);									\
		__spi_write_reg(0x700035, 0x720003); 	\
		udelay(5);									\
		__spi_write_reg(0x700036, 0x720707); 	\
		udelay(5);									\
		__spi_write_reg(0x700037, 0x720000); 	\
		udelay(5);									\
		__spi_write_reg(0x70003A, 0x720904); 	\
		udelay(5);									\
		__spi_write_reg(0x70003B, 0x720904); 	\
		udelay(5);									\
	} while (0)
#endif
	
struct jzlcd_info jzfb = 
{
	.panel = {
		.cfg = LCD_CFG_LCDPIN_LCD | LCD_CFG_RECOVER | /* Underrun recover */
		LCD_CFG_NEWDES | /* 8words descriptor */
		LCD_CFG_MODE_GENERIC_TFT | /* General TFT panel */
		LCD_CFG_MODE_TFT_24BIT | 	/* output 24bpp */
		LCD_CFG_HSP | 	/* Hsync polarity: active low */
		LCD_CFG_VSP |	/* Vsync polarity: leading edge is falling edge */
		LCD_CFG_PCP,
		.slcd_cfg = 0,
		.ctrl = LCD_CTRL_OFUM | LCD_CTRL_BST_16,	/* 16words burst, enable out FIFO underrun irq */
		320, 240, 75, 30, 3, 38, 20, 15, 4,
		.type = DISP_PANEL_TYPE_LCD,
	},
	.osd = {
		 .osd_cfg = LCD_OSDC_OSDEN | /* Use OSD mode */
		 LCD_OSDC_ALPHAEN | /* enable alpha */
		 (1 << LCD_OSDC_COEF_SLE0_BIT) | /*src over mode*/           
		 (0 << LCD_OSDC_COEF_SLE1_BIT) |
//		 (1 << 16) |
//	      LCD_OSDC_PREMULT0 | 		
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
		 .fg0 = {32, 0, 0, 320, 240}, /* bpp, x, y, w, h */
		 .fg1 = {32, 0, 0, 240, 320}, /* bpp, x, y, w, h */
	 }
};

void display_panel_init(void *ptr)
{
	__lcd_special_pin_init();
	__lcd_special_on();
}

#endif

#endif  /* __JZ_LCD_BL35026V0_H__ */

