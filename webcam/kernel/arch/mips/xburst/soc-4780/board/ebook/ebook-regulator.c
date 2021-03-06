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
 * Core voltage Regulator.
 * Couldn't be modified.
 * Voltage was inited at bootloader.
 */
CORE_REGULATOR_DEF(
	ebook,	1000000,	1400000);

/**
 * I/O Regulator.
 * It's the parent regulator of most of devices regulator on board.
 * Voltage was inited at bootloader.
 */
IO_REGULATOR_DEF(
	ebook_vccio,
	"Vcc-IO",	3000000,	1);

/**
 * USB VBUS Regulators.
 * Switch of USB VBUS. It may be a actual or virtual regulator.
 */
#ifdef CONFIG_BOARD_EBOOK_V_1_1
VBUS_REGULATOR_DEF(
	ebook,	"Vcc-5V");
#else
VBUS_REGULATOR_DEF(
	ebook,	"OTG-Vbus");
#endif
/**
 * Exclusive Regulators.
 * They are only used by one device each other.
 */
EXCLUSIVE_REGULATOR_DEF(
	ebook_vwifi,
	"Wi-Fi",	"vwifi",	NULL,
	NULL,		3000000,	1);

EXCLUSIVE_REGULATOR_DEF(
	ebook_vtsc,
	"Touch Screen",	"vtsc",		NULL,
	NULL,		3000000,	1);

EXCLUSIVE_REGULATOR_DEF(
	ebook_vgsensor,
	"G-sensor",	"vgsensor",	NULL,
	NULL,		3300000,	0);

#ifdef CONFIG_BOARD_EBOOK_V_1_1
EXCLUSIVE_REGULATOR_DEF(
	ebook_vcc5,
	"Vcc-5V",	"vhdmi",	NULL,
	"jz-hdmi",	5000000,	0);
#endif
/**
 * Fixed voltage Regulators.
 * GPIO silulator regulators. Everyone is an independent device.
 */
#ifndef CONFIG_BOARD_EBOOK_V_1_1
FIXED_REGULATOR_DEF(
	ebook_vcc5,
	"Vcc-5V",	5000000,	GPIO_PA(17),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vhdmi",	"jz-hdmi");

FIXED_REGULATOR_DEF(
	ebook_vbus,
	"OTG-Vbus",	5000000,	GPIO_PE(10),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	"Vcc-5V",	"vdrvvbus",	NULL);
#endif
#if 0
FIXED_REGULATOR_DEF(
	ebook_vcim,
	"Camera",	2800000,	GPIO_PB(27),
	HIGH_ENABLE,	UN_AT_BOOT,	0,
	NULL,		"vcim",		"jz-cim");
#endif
FIXED_REGULATOR_DEF(
	ebook_vlcd,
	"LCD",		3300000,	GPIO_PB(23),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vlcd",		NULL);

FIXED_REGULATOR_DEF(
	ebook_vmmc,
	"TF",		3300000,	GPIO_PF(20),
	HIGH_ENABLE,	EN_AT_BOOT,	0,
	NULL,		"vmmc.0",	NULL);



static struct platform_device *fixed_regulator_devices[] __initdata = {
#ifndef CONFIG_BOARD_EBOOK_V_1_1
	&ebook_vcc5_regulator_device,
	&ebook_vbus_regulator_device,
#endif
	//&ebook_vcim_regulator_device,
	&ebook_vlcd_regulator_device,
	&ebook_vmmc_regulator_device,
};

/*
 * Regulators definitions used in PMU.
 *
 * If +5V is supplied by PMU, please define "VBUS" here with init_data NULL,
 * otherwise it should be supplied by a exclusive DC-DC, and you should define
 * it as a fixed regulator.
 */
static struct regulator_info ebook_pmu_regulators[] = {
	{"OUT1", &ebook_vcore_init_data},
	{"OUT3", &ebook_vccio_init_data},
#ifdef CONFIG_BOARD_EBOOK_V_1_1
	{"OUT4", &ebook_vcc5_init_data},
#endif
	{"OUT6", &ebook_vwifi_init_data},
	{"OUT7", &ebook_vtsc_init_data},
	{"OUT8", &ebook_vgsensor_init_data},
	{"VBUS", &ebook_vbus_init_data},
};

static struct charger_board_info charger_board_info = {
	.gpio	= GPIO_PB(2),
	.enable_level	= LOW_ENABLE,
};

static struct pmu_platform_data ebook_pmu_pdata = {
	.gpio = GPIO_PA(28),
	.num_regulators = ARRAY_SIZE(ebook_pmu_regulators),
	.regulators = ebook_pmu_regulators,
	.charger_board_info = &charger_board_info,
};

#define PMU_I2C_BUSNUM 0

struct i2c_board_info ebook_pmu_board_info = {
	I2C_BOARD_INFO("act8600", 0x5a),
	.platform_data = &ebook_pmu_pdata,
};

static int __init ebook_pmu_dev_init(void)
{
	struct i2c_adapter *adap;
	struct i2c_client *client;
	int busnum = PMU_I2C_BUSNUM;
	int i;

	adap = i2c_get_adapter(busnum);
	if (!adap) {
		pr_err("failed to get adapter i2c%d\n", busnum);
		return -1;
	}

	client = i2c_new_device(adap, &ebook_pmu_board_info);
	if (!client) {
		pr_err("failed to register pmu to i2c%d\n", busnum);
		return -1;
	}

	i2c_put_adapter(adap);

	for (i = 0; i < ARRAY_SIZE(fixed_regulator_devices); i++)
		fixed_regulator_devices[i]->id = i;

	return platform_add_devices(fixed_regulator_devices,
				    ARRAY_SIZE(fixed_regulator_devices));
}

subsys_initcall_sync(ebook_pmu_dev_init);
