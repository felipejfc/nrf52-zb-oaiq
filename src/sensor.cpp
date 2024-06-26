/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>
#include "sensor.h"
#include <string>
#include <json.hpp>
#include <zephyr/sys/time_units.h>
#include <bme688_server.h>
#include "reporting.h"
#include "epaper.h"
#include "battery.h"

using namespace nlohmann;

/*
 * Sensor value is represented as having an integer and a fractional part,
 * and can be obtained using the formula val1 + val2 * 10^(-6).
 */
#define SENSOR_VAL2_DIVISOR 1000000

LOG_MODULE_DECLARE(app, CONFIG_NRF52_ZB_OAIQ_LOG_LEVEL);

static const struct device *sensor;

static int64_t last_display_update = 0;

void bme688_pre_work_handler(){
	EPD_1in9_hardWakeUp();
}

void bme688_handler(json &data){
	std::string text = data.dump(4);
	printf(" - bme688_handler> %s\n",text.c_str());

	// Update the temperature attribute
	float temperature = data["temperature"];
	int err = weather_station_update_temperature(temperature);
	if (err) {
		LOG_ERR("Failed to update temperature: %d", err);
	}

	// Update the pressure attribute
	float pressure = data["pressure"];
	err = weather_station_update_pressure(pressure);
	if (err) {
		LOG_ERR("Failed to update pressure: %d", err);
	}

	// Update the humidity attribute
	float humidity = data["humidity"];
	err = weather_station_update_humidity(humidity);
	if (err) {
		LOG_ERR("Failed to update humidity: %d", err);
	}

	// Update the co2 attribute
	float co2 = data["co2_eq"];
	err = weather_station_update_co2(co2);
	if (err) {
		LOG_ERR("Failed to update co2: %d", err);
	}

	// Update the iaq attribute
	float iaq = data["iaq"];
	float voc = data["breath_voc"];

	uint16_t battery_millivolt = 0;
	uint8_t battery_percentage = 0;

	#ifndef CONFIG_LOG
	battery_enable_pins();
	#endif

	battery_get_millivolt(&battery_millivolt);
	battery_get_percentage(&battery_percentage, battery_millivolt);

	LOG_INF("Battery at %d mV (capacity %d%%)", battery_millivolt, battery_percentage);

	err = weather_station_update_aiq(voc, iaq, battery_millivolt);
	if (err) {
		LOG_ERR("Failed to update aiq: %d", err);
	}

	int64_t time_now = k_ticks_to_ms_near64(k_uptime_ticks());
	#ifdef CONFIG_LOG
	int update_interval = 30000;
	#else
	int update_interval = 180000;
	#endif
	if (time_now - last_display_update > update_interval || last_display_update == 0) {
		last_display_update = time_now;
		EPD_1in9_Set_TempHumPow(temperature * 100, humidity * 100, battery_percentage < 20);
	}
	#ifndef CONFIG_LOG
	battery_disable_pins(); // TODO use the app PM hooks?
	#endif
	// since we always turn on display, we need to turn it off
	EPD_1in9_sleep();
}

int sensor_init(void)
{
	int err = 0;

    start_bme688(bme688_handler, bme688_pre_work_handler);

	// Init ADC

	err |= battery_init();
	#ifdef CONFIG_LOG
	err |= battery_enable_pins();
	#endif
	//err |= battery_charge_start();

	if (err)
	{
		LOG_ERR("Failed to initialize");
	}
	else
	{
		LOG_INF("ADC Initialized");
	}

	return err;
}