/*
 * FPGA test cmd
 */

#include <common.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && !defined(CFG_NAND_LEGACY)
#include <nand.h>

#define DATABUF_SIZE    (4096 * 4)
#define OOBBUF_SIZE     (1024 * 2)

#define DEBUG_LINE      0
#if DEBUG_LINE
#define DEBG    printk("%s: L%d\n", __FUNCTION__, __LINE__)
#else
#define DEBG 
#endif

unsigned char *databuf = NULL;
unsigned char *tempbuf = NULL;
unsigned char *oobbuf = NULL;

static void mem_dump(uint8_t *buf, int len)
{
        int line = len / 16; 
        int i, j;

        if (len & 0xF)
                line++;

        printk("\n<0x%08x> Length: %dBytes \n", (unsigned int)buf, len);

        printk("--------| ");
        for (i = 0; i < 16; i++) 
                printk("%02d ", i); 

        printk("\n");

        for (i = 0; i < line; i++) {
                printk("%08x: ", i << 4);

                if (i < (len / 16)) {
                        for (j = 0; j < 16; j++)
                                printk("%02x ", buf[(i << 4) + j]);
                } else {
                        for (j = 0; j < (len & 16); j++)
                                printk("%02x ", buf[(i << 4) + j]);
                }

                printk("\n");
        }

        printk("\n");
}

static int nand_test_read_page(struct mtd_info *mtd, int pageaddr, uint8_t *buf, int num)
{
        int pagesize = mtd->oobblock;
        loff_t from = pageaddr * pagesize;
        size_t len = num * pagesize;
        size_t retlen = 0;

        DEBG;

        return mtd->read(mtd, from, len, &retlen, buf);
}

static int nand_test_write_page(struct mtd_info *mtd, int pageaddr, uint8_t *buf, int num)
{
        int pagesize = mtd->oobblock;
        loff_t to = pageaddr * pagesize;
        size_t len = num*pagesize;
        size_t retlen = 0;

//      printk("%s: L%d, page = %d\n", __FUNCTION__, __LINE__, pageaddr);

        return mtd->write(mtd, to, len, &retlen, buf);
}

static void buf_init(uint8_t *buffer, int len)
{
        u8 *buf = buffer;
        int i;

        for (i = 0; i < len; i++)
        {
                buf[i] = i % 256;
        }
#if 0
        buf = oobbuf;
        for (i = 0; i < OOBBUF_SIZE; i++)
        {
                buf[i] = i % 256;
        }
#endif
}

static int nand_test_erase(struct mtd_info *mtd, int pageaddr, int num)
{
        struct nand_chip *chip = (struct nand_chip *)mtd->priv;
        struct erase_info erase;
        loff_t addr;
        int pagesize = mtd->oobblock;
        int ppb = chip->ppb;
        int blk = pageaddr / ppb;
        int erasesize = num * pagesize * ppb;

        memset(&erase, 0, sizeof(struct erase_info));
        erase.mtd  = mtd;
        erase.len  = erasesize;

//        blk = blk / 2;  //for two-plane erase
        addr = blk * erasesize;
        erase.addr = addr;

		printk("[erase] %s: L%d, pageaddr=%d, addr=%lu\n", __FUNCTION__, __LINE__, pageaddr, erase.addr);

        return mtd->erase(mtd, &erase);
}

static int dumpinfo(struct mtd_info *mtd)
{
        printk("\n[Nand Info]\n");
        printk("size:           %u\n", mtd->size);
        printk("erasesize:      %d\n", mtd->erasesize);
//        printk("writesize:      %d\n", mtd->data_per_page);
        printk("oobsize:        %d\n", mtd->oobsize);

        return 0;
}

extern nand_info_t nand_info[CFG_MAX_NAND_DEVICE];

int do_nand_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct mtd_info *mtd = &nand_info[0];
	int pageaddr, num;
        int i, ret;
	unsigned int regaddr, regval;

	if (argc < 3) {
		printk("Usage %s\n", cmdtp->help);
		return 1;
	}

        printk("\n[NAND Function TEST]\n");
        dumpinfo(mtd);

	databuf = (unsigned char *)simple_strtoul(argv[2], NULL, 16);
	tempbuf = databuf + 2 * mtd->oobblock;

#if 0
        printk(" [Prepare BUF]...");

        databuf = kmalloc(DATABUF_SIZE, GFP_KERNEL);
        if (databuf == NULL) {
                kfree(databuf);
                printk("%s: kmalloc failed databuf\n", __FUNCTION__);
                return 1; 
        }   
        tempbuf = kmalloc(DATABUF_SIZE, GFP_KERNEL);
        if (tempbuf == NULL) {
                kfree(databuf);
                kfree(tempbuf);
                printk("%s: kmalloc failed tempbuf\n", __FUNCTION__);
                return 1; 
        }   
        oobbuf = kmalloc(OOBBUF_SIZE, GFP_KERNEL);
        if (oobbuf == NULL) {
                kfree(databuf);
                kfree(tempbuf);
                kfree(oobbuf);
                printk("%s: kmalloc failed oobbuf\n", __FUNCTION__);
                return 1; 
        }   
        printk(" OK\n");
#endif

	if (strncmp(argv[1], "erase", 5) == 0 ) {
		pageaddr = (int)simple_strtoul(argv[2], NULL, 10);
		num = (int)simple_strtoul(argv[3], NULL, 10);
		printk("---<Erase> pageoffs: 0x%x, block: %d\n", pageaddr, num);
	
		ret = nand_test_erase(mtd, pageaddr, num);

		if (ret < 0)
			printk("---Erase Failed: %d\n", ret);
		else
			printk("---Erase OK: %d\n", ret);
	} else if (strncmp(argv[1], "read", 4) == 0) {
		pageaddr = (int)simple_strtoul(argv[3], NULL, 10);
		num = (int)simple_strtoul(argv[4], NULL, 10);
		printk("---<Read> pageoffs: 0x%x, num: %d\n", pageaddr, num);

		for (i = 0; i < num; i++) {
			ret = nand_test_read_page(mtd, pageaddr, databuf, 1);
			mem_dump(databuf, 512);

			if (ret < 0)
				printk("---Read Failed: %d\n", ret);
			else
				printk("---Read OK: %d\n", ret);
		}
	} else if (strncmp(argv[1], "write", 5) == 0) {
		pageaddr = (int)simple_strtoul(argv[3], NULL, 10);
		num = (int)simple_strtoul(argv[4], NULL, 10);
		printk("---<Write> pageoffs: 0x%x, num: %d\n", pageaddr, num);

		buf_init(tempbuf, mtd->oobblock);
		mem_dump(tempbuf, 512);

		for (i = 0; i < num; i++) {
			ret = nand_test_write_page(mtd, pageaddr, tempbuf, 1);

			if (ret < 0)
				printk("---Write Failed: %d\n", ret);
			else
				printk("---write OK: %d", ret);
		}
	} else if (strncmp(argv[1], "regset", 6) == 0) {
		regaddr = (unsigned int)simple_strtoul(argv[2], NULL, 16);
		regval = (unsigned int)simple_strtoul(argv[3], NULL, 16);

		*(volatile unsigned int *)regaddr = regval;

		printk("RegSet REG[%08x] :val[%08x]\n", regaddr, regval);
	} else if (strncmp(argv[1], "regread", 7) == 0) {
		regaddr = (unsigned int)simple_strtoul(argv[2], NULL, 16);

		regval = *(volatile unsigned int *)regaddr;

		printk("RegRead REG[%08x] :val[%08x]\n", regaddr, regval);
	}

	printk("\n[-----------Test End-------------]\n");

#if 0
        kfree(databuf);
        kfree(tempbuf);
        kfree(oobbuf);
#endif

        return 0;
}

U_BOOT_CMD(fnand, 5, 1, do_nand_test,
	"fnand    - NAND sub-system\n",
	"fnand read <databuf16> <pageaddr10> <pagenum10>\n"
	"fnand write <databuf16> <pageaddr10> <pagenum10>\n"
	"fnand erase <pageaddr10> <pagenum10>\n"
	"fnand regset <regaddr16> <value16>\n"
	"fnand regread <regaddr16>\n"
	"fnand dump[.oob] off - dump page\n"
	);

#endif
