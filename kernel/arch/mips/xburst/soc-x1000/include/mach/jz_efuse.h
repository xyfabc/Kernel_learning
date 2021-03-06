#ifndef __JZ_EFUSE_H__
#define __JZ_EFUSE_H__

struct jz_efuse_platform_data {
	int gpio_vddq_en_n;	/* supply 2.5V to VDDQ */
};

enum segment_id {
	CHIP_ID,
	RANDOM_ID,
	USER_ID,
	PROTECT_ID,
};

void jz_efuse_id_read(int is_chip_id, uint32_t *buf);

#endif
