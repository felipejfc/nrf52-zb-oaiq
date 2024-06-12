/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>


/* Measurements ranges for Bosch BME688 sensor */
#define SENSOR_TEMP_CELSIUS_MIN (-40)
#define SENSOR_TEMP_CELSIUS_MAX (85)
#define SENSOR_TEMP_CELSIUS_TOLERANCE (1)
#define SENSOR_PRESSURE_KPA_MIN (30)
#define SENSOR_PRESSURE_KPA_MAX (110)
#define SENSOR_PRESSURE_KPA_TOLERANCE (0)
#define SENSOR_HUMIDITY_PERCENT_MIN (10)
#define SENSOR_HUMIDITY_PERCENT_MAX (90)
#define SENSOR_HUMIDITY_PERCENT_TOLERANCE (3)

/**
 * @brief Initializes Bosch BME688 sensor.
 *
 * @note Has to be called before other functions are used.
 *
 * @return 0 if success, error code if failure.
 */
#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC int sensor_init(void);

#undef EXTERNC
#endif /* SENSOR_H */
