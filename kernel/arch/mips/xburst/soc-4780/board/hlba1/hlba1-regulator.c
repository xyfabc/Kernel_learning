/*
 * [board]-pmu.c - This file defines PMU board information.
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Author: Large Dipper <ykli@ingenic.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/platform_device.h>

#include <linux/regulator/machine.h>
#include <linux/mfd/pmu-common.h>
#include <linux/i2c.h>
#include <gpio.h>

/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */
FIXED_REGULATOR_DEF(
	hlba1_vcc5,
	"Vcc-5V",	5000000,	GPIO_PE(15),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vhdmi",	"jz-hdmi");

FIXED_REGULATOR_DEF(
	hlba1_vbus,
	"OTG-Vbus",	5000000,	GPIO_PE(10),
	LOW_ENABLE,	UN_AT_BOOT,	0,
	NULL,	"vbus",	NULL);

FIXED_REGULATOR_DEF(
	hlba1_vwifi,
	"vwifi",	3000000,	GPIO_PF(4),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vwifi",		NULL);
/*
FIXED_REGULATOR_DEF(
	hlba1_ucharger,
	"ucharger",	3000000,	GPIO_PD(21),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"ucharger",		NULL);
	

FIXED_REGULATOR_DEF(
	hlba1_vlcd,
	"LCD",		3300000,	GPIO_PB(23),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);
*/
static struct platform_device *fixed_regulator_devices[] __initdata = {
	&hlba1_vcc5_regulator_device,
	&hlba1_vbus_regulator_device,
	&hlba1_vwifi_regulator_device,
//	&hlba1_ucharger_regulator_device,
//	&hlba1_vlcd_regulator_device,
};


static int __init hlba1_pmu_dev_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
		fixed_regulator_devices[i]->id = i;

	return platform_add_devices(fixed_regulator_devices,
				    ARRAY_SIZE(fixed_regulator_devices));
}

subsys_initcall_sync(hlba1_pmu_dev_init);
