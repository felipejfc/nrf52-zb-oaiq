/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>

#include "reporting.h"

LOG_MODULE_DECLARE(app, CONFIG_ZIGBEE_WEATHER_STATION_LOG_LEVEL);

int weather_station_init(void)
{
	int err = sensor_init();

	if (err) {
		LOG_ERR("Failed to initialize sensor: %d", err);
	}

	return err;
}

int weather_station_update_temperature(float measured_temperature)
{
	int err = 0;

	int16_t temperature_attribute = 0;

	/* Convert measured value to attribute value, as specified in ZCL */
	temperature_attribute = (int16_t)(measured_temperature *
									  ZCL_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute T:%10d", temperature_attribute);

	/* Set ZCL attribute */
	zb_zcl_status_t status = zb_zcl_set_attr_val(WEATHER_STATION_ENDPOINT_NB,
												 ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
												 ZB_ZCL_CLUSTER_SERVER_ROLE,
												 ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
												 (zb_uint8_t *)&temperature_attribute,
												 ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

	return err;
}

int weather_station_update_pressure(float measured_pressure)
{
	int err = 0;

	int16_t pressure_attribute = 0;

	/* Convert measured value to attribute value, as specified in ZCL */
	pressure_attribute = (int16_t)(measured_pressure *
								   ZCL_PRESSURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute P:%10d", pressure_attribute);

	/* Set ZCL attribute */
	zb_zcl_status_t status = zb_zcl_set_attr_val(
		WEATHER_STATION_ENDPOINT_NB,
		ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&pressure_attribute,
		ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

	return err;
}

int weather_station_update_humidity(float measured_humidity)
{
	int err = 0;

	int16_t humidity_attribute = 0;

	/* Convert measured value to attribute value, as specified in ZCL */
	humidity_attribute = (int16_t)(measured_humidity *
								   ZCL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute H:%10d", humidity_attribute);

	zb_zcl_status_t status = zb_zcl_set_attr_val(
		WEATHER_STATION_ENDPOINT_NB,
		ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
		(zb_uint8_t *)&humidity_attribute,
		ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

	return err;
}
