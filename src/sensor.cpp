/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>
#include "sensor.h"
#include <string>
#include <json.hpp>
#include <bme688_server.h>
#include "reporting.h"

using namespace nlohmann;

/*
 * Sensor value is represented as having an integer and a fractional part,
 * and can be obtained using the formula val1 + val2 * 10^(-6).
 */
#define SENSOR_VAL2_DIVISOR 1000000

LOG_MODULE_DECLARE(app, CONFIG_ZIGBEE_WEATHER_STATION_LOG_LEVEL);

static const struct device *sensor;

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
}

int sensor_init(void)
{
	int err = 0;

    start_bme688(bme688_handler);

	return err;
}