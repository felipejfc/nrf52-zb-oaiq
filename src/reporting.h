/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef REPORTING_H
#define REPORTING_H

#include <zcl/zb_zcl_temp_measurement_addons.h>

#include "sensor.h"
#include "zigbee/co2.h"
#include "zigbee/aiq.h"

/* Zigbee Cluster Library 4.4.2.2.1.1: MeasuredValue = 100x temperature in degrees Celsius */
#define ZCL_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER 100
#define ZCL_BATTERY_VOLTAGE_MEASUREMENT_MULTIPLIER 0.01
#define ZCL_BATTERY_PERCENTAGE_REMAINING_MULTIPLIER 2
#define ZCL_PRESSURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER 1
/* Zigbee Cluster Library 4.7.2.1.1: MeasuredValue = 100x water content in % */
#define ZCL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_MULTIPLIER 100

/* Measurements ranges scaled for attribute values */
#define WEATHER_STATION_ATTR_TEMP_MIN (	  \
		SENSOR_TEMP_CELSIUS_MIN * \
		ZCL_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_TEMP_MAX (	  \
		SENSOR_TEMP_CELSIUS_MAX * \
		ZCL_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_TEMP_TOLERANCE (	\
		SENSOR_TEMP_CELSIUS_TOLERANCE *	\
		ZCL_TEMPERATURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_PRESSURE_MIN ( \
		SENSOR_PRESSURE_KPA_MIN *   \
		ZCL_PRESSURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_PRESSURE_MAX ( \
		SENSOR_PRESSURE_KPA_MAX *   \
		ZCL_PRESSURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_PRESSURE_TOLERANCE ( \
		SENSOR_PRESSURE_KPA_TOLERANCE *	  \
		ZCL_PRESSURE_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_HUMIDITY_MIN (   \
		SENSOR_HUMIDITY_PERCENT_MIN * \
		ZCL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_HUMIDITY_MAX (   \
		SENSOR_HUMIDITY_PERCENT_MAX * \
		ZCL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)
#define WEATHER_STATION_ATTR_HUMIDITY_TOLERANCE (   \
		SENSOR_HUMIDITY_PERCENT_TOLERANCE * \
		ZCL_HUMIDITY_MEASUREMENT_MEASURED_VALUE_MULTIPLIER)

/* Number chosen for the single endpoint provided by weather station */
#define WEATHER_STATION_ENDPOINT_NB 42

/* Temperature sensor device version */
#define ZB_HA_DEVICE_VER_TEMPERATURE_SENSOR     0
/* Basic, identify, temperature, pressure, humidity, co2, aiq, power config */
#define ZB_HA_WEATHER_STATION_IN_CLUSTER_NUM    8
/* Identify */
#define ZB_HA_WEATHER_STATION_OUT_CLUSTER_NUM   1

/* Temperature, pressure, humidity, co2, aiq, voc, bat% */
#define ZB_HA_WEATHER_STATION_REPORT_ATTR_COUNT 7

#define ZB_HA_DECLARE_WEATHER_STATION_CLUSTER_LIST(						\
		cluster_list_name,								\
		basic_attr_list,								\
		identify_client_attr_list,							\
		identify_server_attr_list,							\
		temperature_measurement_attr_list,						\
		pressure_measurement_attr_list,							\
		humidity_measurement_attr_list,							\
		carbon_dioxide_attr_list,							\
		aiq_attr_list,									\
		power_config_battery_attr_list							\
		)										\
	zb_zcl_cluster_desc_t cluster_list_name[] =						\
	{											\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_BASIC,						\
			ZB_ZCL_ARRAY_SIZE(basic_attr_list, zb_zcl_attr_t),			\
			(basic_attr_list),							\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_IDENTIFY,						\
			ZB_ZCL_ARRAY_SIZE(identify_server_attr_list, zb_zcl_attr_t),		\
			(identify_server_attr_list),						\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,					\
			ZB_ZCL_ARRAY_SIZE(temperature_measurement_attr_list, zb_zcl_attr_t),	\
			(temperature_measurement_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT,					\
			ZB_ZCL_ARRAY_SIZE(pressure_measurement_attr_list, zb_zcl_attr_t),	\
			(pressure_measurement_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,				\
			ZB_ZCL_ARRAY_SIZE(humidity_measurement_attr_list, zb_zcl_attr_t),	\
			(humidity_measurement_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE,				\
			ZB_ZCL_ARRAY_SIZE(carbon_dioxide_attr_list, zb_zcl_attr_t),	\
			(carbon_dioxide_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_AIQ,				\
			ZB_ZCL_ARRAY_SIZE(aiq_attr_list, zb_zcl_attr_t),	\
			(aiq_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_IDENTIFY,						\
			ZB_ZCL_ARRAY_SIZE(identify_client_attr_list, zb_zcl_attr_t),		\
			(identify_client_attr_list),						\
			ZB_ZCL_CLUSTER_CLIENT_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
		ZB_ZCL_CLUSTER_DESC(								\
			ZB_ZCL_CLUSTER_ID_POWER_CONFIG,				\
			ZB_ZCL_ARRAY_SIZE(power_config_battery_attr_list, zb_zcl_attr_t),	\
			(power_config_battery_attr_list),					\
			ZB_ZCL_CLUSTER_SERVER_ROLE,						\
			ZB_ZCL_MANUF_CODE_INVALID						\
			),									\
	}

#define ZB_ZCL_DECLARE_WEATHER_STATION_DESC(						\
		ep_name,								\
		ep_id,									\
		in_clust_num,								\
		out_clust_num)								\
	ZB_DECLARE_SIMPLE_DESC(in_clust_num, out_clust_num);				\
	ZB_AF_SIMPLE_DESC_TYPE(in_clust_num, out_clust_num) simple_desc_##ep_name =	\
	{										\
		ep_id,									\
		ZB_AF_HA_PROFILE_ID,							\
		ZB_HA_TEMPERATURE_SENSOR_DEVICE_ID,					\
		ZB_HA_DEVICE_VER_TEMPERATURE_SENSOR,					\
		0,									\
		in_clust_num,								\
		out_clust_num,								\
		{									\
			ZB_ZCL_CLUSTER_ID_BASIC,					\
			ZB_ZCL_CLUSTER_ID_IDENTIFY,					\
			ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,				\
			ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT,				\
			ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,			\
			ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE,			\
			ZB_ZCL_CLUSTER_ID_AIQ,			\
			ZB_ZCL_CLUSTER_ID_POWER_CONFIG,			\
			ZB_ZCL_CLUSTER_ID_IDENTIFY,					\
		}									\
	}

#define ZB_HA_DECLARE_WEATHER_STATION_EP(ep_name, ep_id, cluster_list)				\
	ZB_ZCL_DECLARE_WEATHER_STATION_DESC(							\
		ep_name,									\
		ep_id,										\
		ZB_HA_WEATHER_STATION_IN_CLUSTER_NUM,						\
		ZB_HA_WEATHER_STATION_OUT_CLUSTER_NUM);						\
	ZBOSS_DEVICE_DECLARE_REPORTING_CTX(							\
		reporting_info##ep_name,							\
		ZB_HA_WEATHER_STATION_REPORT_ATTR_COUNT);					\
	ZB_AF_DECLARE_ENDPOINT_DESC(								\
		ep_name,									\
		ep_id,										\
		ZB_AF_HA_PROFILE_ID,								\
		0,										\
		NULL,										\
		ZB_ZCL_ARRAY_SIZE(cluster_list, zb_zcl_cluster_desc_t),				\
		cluster_list,									\
		(zb_af_simple_desc_1_1_t *)&simple_desc_##ep_name,				\
		ZB_HA_WEATHER_STATION_REPORT_ATTR_COUNT, reporting_info##ep_name, 0, NULL)

#define ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_WITH_MODEL_MANUF(attr_list, zcl_version, power_source, model_id, manufacturer_name) \
    ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_BASIC) \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, (zcl_version))      \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, (power_source))    \
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, (manufacturer_name))\
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, (model_id))         \
    ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

#define ZB_ZCL_DECLARE_POWER_CONFIG_BATTERY_ATTRIB_LIST(attr_list, voltage, rated_voltage, alarm_mask, voltage_min_threshold, percentage_remaining) \
 ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_POWER_CONFIG) \
 ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID(voltage, ), \
 ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID(rated_voltage, ), \
 ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID(alarm_mask, ), \
 ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_MIN_THRESHOLD_ID(voltage_min_threshold, ), \
 ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID(percentage_remaining, ), \
 ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

struct zb_zcl_pressure_measurement_attrs_t {
	zb_int16_t measure_value;
	zb_int16_t min_measure_value;
	zb_int16_t max_measure_value;
	zb_uint16_t tolerance;
};

struct zb_zcl_humidity_measurement_attrs_t {
	zb_int16_t measure_value;
	zb_int16_t min_measure_value;
	zb_int16_t max_measure_value;
};

struct zb_zcl_analog_output_attrs_t {
	zb_int16_t present_value;
	zb_int16_t min_present_value;
	zb_int16_t max_present_value;
};

struct zb_zcl_carbon_dioxide_attrs_t {
	zb_uint32_t measure_value;
	zb_uint32_t min_measure_value;
	zb_uint32_t max_measure_value;
	zb_uint32_t tolerance;
};

struct zb_zcl_aiq_attrs_t {
	zb_uint16_t iaq;
	zb_uint16_t voc;
};

struct zb_zcl_power_config_battery_attrs_t {
	zb_uint16_t voltage;
	zb_uint16_t rated_voltage;
	zb_uint16_t alarm_mask;
	zb_uint16_t voltage_min_threshold;
	zb_uint16_t percentage_remaining;
};

struct zb_device_ctx {
	zb_zcl_basic_attrs_t basic_attr;
	zb_zcl_identify_attrs_t identify_attr;
	zb_zcl_temp_measurement_attrs_t temp_attrs;
	struct zb_zcl_pressure_measurement_attrs_t pres_attrs;
	struct zb_zcl_humidity_measurement_attrs_t humidity_attrs;
	struct zb_zcl_carbon_dioxide_attrs_t carbon_dioxide_attrs;
	struct zb_zcl_aiq_attrs_t aiq_attrs;
	struct zb_zcl_power_config_battery_attrs_t power_config_battery_attrs;
};

/**
 * @brief Initializes HW sensor used for performing measurements.
 *
 * @note Has to be called before other functions can be used.
 *
 * @return 0 if success, error code if failure.
 */
int weather_station_init(void);

/**
 * @brief Updates internal measurements performed by sensor.
 *
 * @note It has to be called each time a fresh measurements are required.
 *	 It does not change any ZCL attributes.
 *
 * @return 0 if success, error code if failure.
 */
int weather_station_check_weather(void);

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

/**
 * @brief Updates ZCL carbon dioxide attribute using value obtained during last weather check.
 *
 * @return 0 if success, error code if failure.
 */
EXTERNC int weather_station_update_co2(float measured_co2);

/**
 * @brief Updates ZCL temperature attribute using value obtained during last weather check.
 *
 * @return 0 if success, error code if failure.
 */
EXTERNC int weather_station_update_temperature(float measured_temperature);

/**
 * @brief Updates ZCL pressure attribute using value obtained during last weather check.
 *
 * @return 0 if success, error code if failure.
 */
EXTERNC int weather_station_update_pressure(float measured_pressure);

/**
 * @brief Updates ZCL relative humidity attribute using value obtained during last weather check.
 *
 * @return 0 if success, error code if failure.
 */
EXTERNC int weather_station_update_humidity(float measured_humidity);

/**
 * @brief Updates ZCL air quality attributes using values obtained during last weather check.
 *
 * @return 0 if success, error code if failure.
 */
EXTERNC int weather_station_update_aiq(float measured_voc, float measured_iaq);

EXTERNC int weather_station_update_battery(uint16_t battery_percentage);

#undef EXTERNC

#endif /* REPORTING_H */
