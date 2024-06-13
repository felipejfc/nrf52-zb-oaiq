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

using namespace nlohmann;

/*
 * Sensor value is represented as having an integer and a fractional part,
 * and can be obtained using the formula val1 + val2 * 10^(-6).
 */
#define SENSOR_VAL2_DIVISOR 1000000

LOG_MODULE_DECLARE(app, CONFIG_ZIGBEE_WEATHER_STATION_LOG_LEVEL);

static const struct device *sensor;

static int64_t last_display_update = 0;

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

	int64_t time_now = k_ticks_to_ms_near64(k_uptime_ticks());
	if (time_now - last_display_update > 180000 || last_display_update == 0) {
		EPD_1in9_hardWakeUp();
		last_display_update = time_now;
		EPD_1in9_Set_TempHum(temperature * 100, humidity * 100);
		EPD_1in9_sleep();
	}
}

int sensor_init(void)
{
	int err = 0;

    start_bme688(bme688_handler);

	return err;
}