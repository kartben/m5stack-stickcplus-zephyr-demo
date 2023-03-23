/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>

#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app);

static lv_obj_t * chart1;
static lv_chart_series_t * ser_x;
static lv_chart_series_t * ser_y;
static lv_chart_series_t * ser_z;

const struct device *display_dev;
const struct device *sensor;

static void fetch_and_display(lv_timer_t * timer)
{
	static unsigned int count;
	struct sensor_value accel[3];
	const char *overrun = "";
	int rc = sensor_sample_fetch(sensor);

	++count;
	if (rc == -EBADMSG) {
		/* Sample overrun.  Ignore in polled mode. */
		if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
			overrun = "[OVERRUN] ";
		}
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		LOG_ERR("ERROR: Update failed: %d\n", rc);
	} else {
	    lv_chart_set_next_value(chart1, ser_x, sensor_value_to_double(&accel[0]));
	    lv_chart_set_next_value(chart1, ser_y, sensor_value_to_double(&accel[1]));
	    lv_chart_set_next_value(chart1, ser_z, sensor_value_to_double(&accel[2]));
	}
}

void accelerometer_chart(void)
{
    chart1 = lv_chart_create(lv_scr_act());
    lv_obj_set_size(chart1, 320, 240);
    lv_obj_center(chart1);
    lv_chart_set_type(chart1, LV_CHART_TYPE_LINE);

    lv_chart_set_div_line_count(chart1, 5, 8);

	lv_chart_set_range(chart1, LV_CHART_AXIS_PRIMARY_Y, -20, 20);

    lv_chart_set_update_mode(chart1, LV_CHART_UPDATE_MODE_SHIFT);

    ser_x = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    ser_y = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    ser_z = lv_chart_add_series(chart1, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);

	lv_chart_set_point_count(chart1, 50);
    /*Do not display points on the data*/
    lv_obj_set_style_size(chart1, 0, LV_PART_INDICATOR);

    lv_timer_create(fetch_and_display, 10, NULL);
}  

void main(void)
{
	int err;

	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	if (!device_is_ready(display_dev)) {
		LOG_ERR("Device not ready, aborting test");
		return;
	}

	sensor = DEVICE_DT_GET_ANY(st_lis2dh);
	if (sensor == NULL) {
		LOG_ERR("No lis2dh/lis3dh device found\n");
		return;
	}
	if (!device_is_ready(sensor)) {
		LOG_ERR("Device %s is not ready\n", sensor->name);
		return;
	}

	accelerometer_chart();

	// add buffer overflow bug
	char *buf = k_malloc(18);
	sprintf(buf, "Overflow much? %d", 123456789);

	display_blanking_off(display_dev);

	while (1) {
		lv_task_handler();
		k_sleep(K_MSEC(5));
	}
}