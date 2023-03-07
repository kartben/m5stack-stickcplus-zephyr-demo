/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>

#include "app_version.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

void main(void)
{
	int ret;
	const struct device *display_dev;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));


	printk("Zephyr Example Application %s\n", APP_VERSION_STR);

	while (1) {
		printk("Hello World from %s!\n", CONFIG_BOARD);

		k_sleep(K_MSEC(1000));
	}
}

