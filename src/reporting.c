/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/logging/log.h>

#include "reporting.h"

LOG_MODULE_DECLARE(app, CONFIG_NRF52_ZB_OAIQ_LOG_LEVEL);

int weather_station_init(void)
{
	int err = sensor_init();

	if (err) {
		LOG_ERR("Failed to initialize sensor: %d", err);
	}

	return err;
}

int weather_station_update_aiq(float voc, float iaq, uint16_t bat_millivolt)
{
	int err = 0;

	uint16_t voc_attribute = 0;
	uint16_t iaq_attribute = 0;

	/* Convert measured value to attribute value, as specified in ZCL */
	voc_attribute = (uint16_t)(voc *
									  ZCL_AIQ_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute V:%10d", voc_attribute);

	/* Convert measured value to attribute value, as specified in ZCL */
	iaq_attribute = (uint16_t)(iaq *
									  ZCL_AIQ_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute I:%10d", iaq_attribute);
	LOG_INF("Attribute B:%10d", bat_millivolt);

	/* Set ZCL attribute */
	zb_zcl_status_t status = zb_zcl_set_attr_val(WEATHER_STATION_ENDPOINT_NB,
												 ZB_ZCL_CLUSTER_ID_AIQ,
												 ZB_ZCL_CLUSTER_SERVER_ROLE,
												 ZB_ZCL_ATTR_AIQ_VOC_ID,
												 (zb_uint8_t *)&voc_attribute,
												 ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

	/* Set ZCL attribute */
	status = zb_zcl_set_attr_val(WEATHER_STATION_ENDPOINT_NB,
												 ZB_ZCL_CLUSTER_ID_AIQ,
												 ZB_ZCL_CLUSTER_SERVER_ROLE,
												 ZB_ZCL_ATTR_AIQ_IAQ_ID,
												 (zb_uint8_t *)&iaq_attribute,
												 ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

/* Set ZCL attribute */
	status = zb_zcl_set_attr_val(WEATHER_STATION_ENDPOINT_NB,
												 ZB_ZCL_CLUSTER_ID_AIQ,
												 ZB_ZCL_CLUSTER_SERVER_ROLE,
												 ZB_ZCL_ATTR_AIQ_BAT_ID,
												 (zb_uint8_t *)&bat_millivolt,
												 ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
	}

	return err;
}

int weather_station_update_co2(float measured_co2)
{
	int err = 0;

	float32_t co2_attribute = 0;

	/* Convert measured value to attribute value, as specified in ZCL */
	co2_attribute = (float32_t)(measured_co2 *
									  ZCL_CARBON_DIOXIDE_MEASURED_VALUE_MULTIPLIER);
	LOG_INF("Attribute C:%10.7f", co2_attribute);

	/* Set ZCL attribute */
	zb_zcl_status_t status = zb_zcl_set_attr_val(WEATHER_STATION_ENDPOINT_NB,
												 ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE,
												 ZB_ZCL_CLUSTER_SERVER_ROLE,
												 ZB_ZCL_ATTR_CARBON_DIOXIDE_VALUE_ID,
												 (zb_uint8_t *)&co2_attribute,
												 ZB_FALSE);
	if (status)
	{
		LOG_ERR("Failed to set ZCL attribute: %d", status);
		err = status;
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
