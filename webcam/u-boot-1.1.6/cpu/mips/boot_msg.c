#include <config.h>

#include <common.h>
#include <command.h>
#include <environment.h>
#include <nand.h>

#undef DEBUG                                                                   
//#define DEBUG                                                                    
#ifdef DEBUG                                                                     
#define dputs(x...)   puts(x)                                                
#else                                                                            
#define dputs(x...)                                                            
#endif                


#if defined(CFG_JZ_LINUX_RECOVERY) && defined(CFG_SUPPORT_RECOVERY_MISC)

#include "boot_msg.h"

#ifdef CONFIG_NAND_U_BOOT
int get_bootloader_message(struct bootloader_message *out)
{
	unsigned char data[CFG_NAND_PAGE_SIZE];
	unsigned char size = CFG_NAND_PAGE_SIZE;

	memset(data, '\0', CFG_NAND_PAGE_SIZE);
	nand_read(&nand_info[0], PTN_MISC_OFFSET, &size,(u_char*) data);
	if(size == CFG_NAND_PAGE_SIZE){
		puts("get_bootloader_message:nand read error\n");
		return 1;
	}

	memcpy(out, data, sizeof(struct bootloader_message));

	return 0;
}
int set_bootloader_message(const struct bootloader_message *in)
{
	unsigned char data[CFG_NAND_PAGE_SIZE];
	unsigned char size = CFG_NAND_PAGE_SIZE;

	memset(data, '\0', CFG_NAND_PAGE_SIZE);
	memcpy(data, in, sizeof(struct bootloader_message));

	/* Clear MISC partition, and write bootloader_message. */
	if(nand_erase(&nand_info[0],PTN_MISC_OFFSET, CFG_NAND_BLOCK_SIZE)){
		puts("NAND erase failed!!!\n");
		return 1;
	}
	if(nand_write(&nand_info[0], PTN_MISC_OFFSET, &size,(u_char*) data)){
		puts("NAND program failed !!!\n");
		return 1;
	}

	dputs("set_bootloader_message finish ...\n");

	return 0;
}

#elif defined(CONFIG_MSC_U_BOOT)
#define CFG_MISC_SIZE 2048
int get_bootloader_message(struct bootloader_message *out)
{

	unsigned char data[CFG_MISC_SIZE];

	memset(data, '\0', CFG_MISC_SIZE);
	dputs("get_bootloader_message:msc read\n");
	msc_read(PTN_MISC_OFFSET, (u_char*)data, sizeof(data));
	
	memcpy(out, data, sizeof(struct bootloader_message));

	return 0;
}
int set_bootloader_message(const struct bootloader_message *in)
{
	unsigned char data[CFG_MISC_SIZE];

	memset(data, '\0', sizeof(data));
	memcpy(data, in, sizeof(struct bootloader_message));
	/* Write bootloader_message. */
	dputs("set_bootloader_message1\n");
	if(msc_write(PTN_MISC_OFFSET, (u_char*)data, sizeof(data))){
		puts("MSC program failed!!!\n");
		return 1;
	}
	dputs("set_bootloader_message finish ...\n");
	return 0;
}

#endif



#if 0
void msg_test(void)//struct bootloader_message *msg)
{
	unsigned char data[CFG_NAND_PAGE_SIZE];


	// clear data array ...
	if (nand_erase_block(64) || nand_erase_block(64 + 1))
		serial_puts_info("NAND erase failed !!!\n");


	if (nand_program_page(data, 8192))
		serial_puts_info("NAND program failed !!!\n");

	serial_puts_info("msg_test finish.\n");
}
#endif
#endif  /* CFG_JZ_LINUX_RECOVERY */

