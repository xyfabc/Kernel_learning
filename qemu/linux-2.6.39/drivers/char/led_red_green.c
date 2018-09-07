// #include <linux/config.h>
#include <linux/utsname.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mm.h>





#include <asm/page.h>

#include <linux/device.h>
MODULE_AUTHOR("shecun feng <fsc0@163.com>");
MODULE_DESCRIPTION("led red_green Driver"); 
MODULE_LICENSE("GPL");
#define JZ_led_pwm_DEBUG 1


#ifdef JZ_led_pwm_DEBUG
#define dbg(format, arg...) printk( ": " format "\n" , ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif
#define err(format, arg...) printk(": " format "\n" , ## arg)
#define info(format, arg...) printk(": " format "\n" , ## arg)
#define warn(format, arg...) printk(": " format "\n" , ## arg)

int led_red_green_major = 56;


#define GPIO_LED_RED	(32*4 + 2) // PE02    
#define GPIO_LED_GREEN	(32*6 + 10) // PG13

#define ledopen     1
#define ledclose     2

#define redcmd      1
#define greencmd   2
#define yellowcmd  3



static int LED_red_green_Close(struct inode * inode, struct file * file)
{
	return 0;
}
  
static int LED_red_green_Open(struct inode * inode, struct file * file)
{

	return 0;
}


static int LED_red_green_Read(struct file *fp, unsigned int *buf, size_t count)
{
	return 0;

}
 
int LED_red_green_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int LED_red_green_Ioctl(struct inode *inode,struct file *file,unsigned int cmd, unsigned long arg)
{ 
   
	return 0; 

}

 
struct file_operations LED_red_green_fops = 
{
	open:	 LED_red_green_Open, 
	//ioctl:	  LED_red_green_Ioctl, 
	release:    LED_red_green_Close, 
	read:	  LED_red_green_Read, 
	release:    LED_red_green_release,
};


struct class *my_led_red_green_class;

static int MY_create_device(int major_num, char *dev_name, char *class_name)
{
   	my_led_red_green_class = class_create(THIS_MODULE, class_name);
//	device_create(my_led_class, NULL, MKDEV(major_num, 0), "led_pwm%d", 0);

	device_create(my_led_red_green_class, NULL, MKDEV(major_num, 0), dev_name, "led_red_green", 0);

	return 0;

	return 0;
}
struct page *m_page = NULL;

static int hanvon_power_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int l = 0;
	l = simple_strtoul(buffer, 0, 10);
	HANVON_DBG("----%d------\n",l);

	unsigned char  *bmp_bufff = NULL;
	HANVON_DBG("----%x------\n",bmp_bufff);
	bmp_bufff=(unsigned char *)kmalloc(1024,GFP_KERNEL);
	HANVON_DBG("----%x------\n",bmp_bufff);

	m_page = alloc_pages(GFP_KERNEL,l);
	page_count(m_page);
	unsigned long pfn = page_to_pfn(m_page);
	//HANVON_DBG("----%x------\n",pfn_to_phys(pfn));
	//;
	HANVON_DBG("----page=%x-%d--pfn=%x---%x-%x-%d\n",m_page,page_count(m_page),page_to_pfn(m_page),pfn_to_page(page_to_pfn(m_page)),*m_page,mem_map);

	HANVON_DBG("---mem_map = %x::%x\n",mem_map,ARCH_PFN_OFFSET);
	return count;
}

int __init LED_red_green_Init(void)
{
	int result;

	result = register_chrdev(led_red_green_major, "led_red_green", &LED_red_green_fops);
	
	if (result<0)
	{
		printk(KERN_INFO"[FALLED: Cannot register led_red_green_driver!]\n");
		return result;
	}
	unsigned char  *bmp_bufff = NULL;
	bmp_bufff=(unsigned char *)kmalloc(1024,GFP_KERNEL);
	struct page *m_page = NULL;
	m_page = alloc_page(GFP_KERNEL);
HANVON_DBG("--------------------------");
	MY_create_device(led_red_green_major, "led_red_green", "class_led_red_green");

	struct proc_dir_entry *res_ir_led;
	res_ir_led = create_proc_entry("kernel_test", 0644, NULL);
	if (res_ir_led) {
		res_ir_led->read_proc = NULL;
		res_ir_led->write_proc = hanvon_power_proc_write;
		res_ir_led->data = NULL;
	}



	return 0;

}

void __exit LED_red_green_Exit(void)
{
	unregister_chrdev(led_red_green_major, "led_red_green");
	device_destroy(my_led_red_green_class, MKDEV(led_red_green_major, 0));
	class_destroy(my_led_red_green_class);
	//	free(my_led_class);
}

//EXPORT_SYMBOL(LED_OPENED);
module_init(LED_red_green_Init);
module_exit(LED_red_green_Exit);

