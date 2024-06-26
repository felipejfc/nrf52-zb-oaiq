/**
 * Copyright (C) 2021 Bosch Sensortec GmbH. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "bme68x.h"
#include "bme688.h"

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/logging/log.h>

/******************************************************************************/
/*!                 Macro definitions                                         */

/******************************************************************************/
/*!                Static variable definition                                 */

/*                  !!! Single instance device driver !!!                     */

static struct bme68x_dev bme_api_dev;
static struct bme68x_conf conf;
static struct bme68x_heatr_conf heatr_conf;
static struct bme68x_data data[10];
static uint8_t n_fields;
bme688_mode_t mode = bme688_mode_forced;
uint16_t temp_prof[10] = { 320, 100, 100, 100, 200, 200, 200, 320, 320, 320 };
uint16_t mul_prof[10] = { 5, 2, 10, 30, 5, 5, 5, 5, 5, 5 };
uint16_t dur_prof[10] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
uint8_t nb_steps = 10;

#define BSEC_TOTAL_HEAT_DUR             UINT16_C(140)

//default init function
void bme688_set_mode_forced();

/******************************************************************************/
/*!                User interface functions                                   */

/*!
 * I2C read function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme68x_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    const struct device *const sensor_dev = (struct device*)intf_ptr;
    const struct bme688_config *config = (struct bme688_config*)sensor_dev->config;
	const struct i2c_dt_spec *i2c_dev = &config->i2c;

	if (i2c_burst_read_dt(i2c_dev, reg_addr, reg_data, len)) {
        printf("bme68x_i2c_read timeout\n");
		return 1;
	}
    //else{
    //    printf("\nbme68x_i2c_read success @0x%0x: ",reg_addr);
    //    for(int i=0;i<len;i++){
    //        printf("%0x ",reg_data[i]);
    //    }
    //    printf("\n");
    //}
    return BME68X_INTF_RET_SUCCESS;
}

/*!
 * I2C write function map to COINES platform
 */
BME68X_INTF_RET_TYPE bme68x_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    const struct device *const sensor_dev = (struct device*)intf_ptr;
    const struct bme688_config *config = (struct bme688_config*)sensor_dev->config;
	const struct i2c_dt_spec *i2c_dev = &config->i2c;
    //printf("burst writing @ 0x%x len=%d\n",reg_addr,len);
    //WA fix timeout error for bursts > 10
    if(len<10){
        if(i2c_burst_write_dt(i2c_dev, reg_addr, reg_data, len)){
            printf("i2c_burst_write_dt timeout\n");
            return 1;
        }
    }else{
        if(i2c_burst_write_dt(i2c_dev, reg_addr, reg_data, 10)){
            printf("i2c_burst_write_dt timeout\n");
            return 1;
        }
        if(i2c_burst_write_dt(i2c_dev, reg_addr+10, reg_data+10, len-10)){
            printf("i2c_burst_write_dt timeout\n");
            return 1;
        }
    }
    return BME68X_INTF_RET_SUCCESS;
}

/*!
 * Delay function map to COINES platform
 */
void bme68x_delay_us(uint32_t period, void *intf_ptr)
{
    k_sleep(K_USEC(period));
}

void bme68x_check_rslt(const char api_name[], int8_t rslt)
{
    switch (rslt)
    {
        case BME68X_OK:

            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            printf("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            break;
        case BME68X_E_COM_FAIL:
            printf("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            break;
        case BME68X_E_INVALID_LENGTH:
            printf("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            break;
        case BME68X_E_DEV_NOT_FOUND:
            printf("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            break;
        case BME68X_E_SELF_TEST:
            printf("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            break;
        case BME68X_W_NO_NEW_DATA:
            printf("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            break;
    }
}

int bme688_init(const struct device *dev)
{
    int8_t rslt = BME68X_OK;

    bme_api_dev.read = bme68x_i2c_read;
    bme_api_dev.write = bme68x_i2c_write;
    bme_api_dev.intf = BME68X_I2C_INTF;
    bme_api_dev.delay_us = bme68x_delay_us;
    bme_api_dev.intf_ptr = dev;
    bme_api_dev.amb_temp = 25; /* The ambient temperature in deg C is used for defining the heater temperature */

    rslt = bme68x_init(&bme_api_dev);
    bme68x_check_rslt("bme68x_init",rslt);

    bme688_set_mode_forced();//default mode, can be overridden by bme688_set_mode()

    return (int)rslt;
}

void bme688_set_mode(bme688_mode_t v_mode){
    int8_t rslt = BME68X_OK;
    mode = v_mode;

    switch(v_mode){
        case bme688_mode_sleep:
            rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE,&bme_api_dev);
            bme68x_check_rslt("bme68x_set_op_mode",rslt);
        break;
        case bme688_mode_forced:
            rslt = bme68x_set_op_mode(BME68X_FORCED_MODE,&bme_api_dev);
            bme68x_check_rslt("bme68x_set_op_mode",rslt);
        break;
        case bme688_mode_parallel:
            rslt = bme68x_set_op_mode(BME68X_PARALLEL_MODE, &bme_api_dev);
            bme68x_check_rslt("bme68x_set_op_mode", rslt);
        break;
        case bme688_mode_sequencial:
            rslt = bme68x_set_op_mode(BME68X_SEQUENTIAL_MODE, &bme_api_dev);
            bme68x_check_rslt("bme68x_set_op_mode", rslt);
        break;
        default:
        break;
    }
}

void bme688_set_oversampling(uint8_t osTemp, uint8_t osPres, uint8_t osHum){

    int8_t rslt = BME68X_OK;
	rslt = bme68x_get_conf(&conf, &bme_api_dev);
    bme68x_check_rslt("bme68x_get_conf",rslt);
    if(rslt == BME68X_OK){
        conf.os_temp = osTemp;
        conf.os_pres = osPres;
        conf.os_hum = osHum;
        rslt = bme68x_set_conf(&conf,&bme_api_dev);
        bme68x_check_rslt("bme68x_set_conf",rslt);
    }
}

void bme688_set_heater_config(bme688_heater_config_t *heater_config){
    int8_t rslt = BME68X_OK;

    switch(heater_config->op_mode){
        case BME68X_FORCED_MODE:
            heatr_conf.enable = BME68X_ENABLE;
            heatr_conf.heatr_temp = heater_config->heater_temperature;
            heatr_conf.heatr_dur = heater_config->heater_duration;
            rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme_api_dev);
        break;
        case BME68X_PARALLEL_MODE:
            heatr_conf.enable = BME68X_ENABLE;
            heatr_conf.heatr_temp_prof = heater_config->heater_temperature_profile;
            heatr_conf.heatr_dur_prof = heater_config->heater_duration_profile;
            uint32_t measure_duration = bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme_api_dev);
            heatr_conf.shared_heatr_dur = BSEC_TOTAL_HEAT_DUR - (measure_duration / INT64_C(1000));
            heatr_conf.profile_len = heater_config->heater_profile_len;
            rslt = bme68x_set_heatr_conf(BME68X_PARALLEL_MODE, &heatr_conf, &bme_api_dev);
        break;
        case BME68X_SEQUENTIAL_MODE:
            heatr_conf.enable = BME68X_ENABLE;
            heatr_conf.heatr_temp_prof = heater_config->heater_temperature_profile;
            heatr_conf.heatr_dur_prof = heater_config->heater_duration_profile;
            heatr_conf.profile_len = heater_config->heater_profile_len;
            rslt = bme68x_set_heatr_conf(BME68X_SEQUENTIAL_MODE, &heatr_conf, &bme_api_dev);
        break;
    }
    bme68x_check_rslt("bme68x_set_heatr_conf",rslt);

}

void bme688_set_mode_forced(){
    int8_t rslt = BME68X_OK;

    mode = bme688_mode_forced;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_conf",rslt);

    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;
    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_heatr_conf",rslt);

}

void bme688_set_mode_parallel()
{
    int8_t rslt = BME68X_OK;

    mode = bme688_mode_parallel;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;
    conf.os_hum = BME68X_OS_1X;
    conf.os_pres = BME68X_OS_16X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_conf",rslt);
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp_prof = temp_prof;
    heatr_conf.heatr_dur_prof = mul_prof;
    heatr_conf.shared_heatr_dur = 140 - (bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme_api_dev) / 1000);
    heatr_conf.profile_len = nb_steps;
    rslt = bme68x_set_heatr_conf(BME68X_PARALLEL_MODE, &heatr_conf, &bme_api_dev);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
    rslt = bme68x_set_op_mode(BME68X_PARALLEL_MODE, &bme_api_dev);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);

}

void bme688_set_mode_sequencial(){
    int8_t rslt = BME68X_OK;

    mode = bme688_mode_sequencial;

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE; /* This parameter defines the sleep duration after each profile */
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf,&bme_api_dev);
    bme68x_check_rslt("bme68x_set_conf",rslt);
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp_prof = temp_prof;
    heatr_conf.heatr_dur_prof = dur_prof;
    heatr_conf.profile_len = nb_steps;
    rslt = bme68x_set_heatr_conf(BME68X_SEQUENTIAL_MODE, &heatr_conf, &bme_api_dev);
    bme68x_check_rslt("bme68x_set_heatr_conf", rslt);
    rslt = bme68x_set_op_mode(BME68X_SEQUENTIAL_MODE, &bme_api_dev);
    bme68x_check_rslt("bme68x_set_op_mode", rslt);
}

void bme688_set_mode_default_conf(bme688_mode_t v_mode)
{
    int8_t rslt = BME68X_OK;
    mode = v_mode;

    switch(v_mode){
        case bme688_mode_sleep:
            rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE,&bme_api_dev);
            bme68x_check_rslt("bme68x_set_op_mode",rslt);
        break;
        case bme688_mode_forced:
            bme688_set_mode_forced();
        break;
        case bme688_mode_parallel:
            bme688_set_mode_parallel();
        break;
        case bme688_mode_sequencial:
            bme688_set_mode_sequencial();
        break;
        default:
        break;
    }
}

int wait_for_forced(){

    //https://github.com/boschsensortec/BME68x-Sensor-API/blob/80ea120a8b8ac987d7d79eb68a9ed796736be845/examples/forced_mode/forced_mode.c#L71
    uint32_t del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme_api_dev) + (heatr_conf.heatr_dur * 1000);
    //printf("del_period = %u\n",del_period);
    k_sleep(K_USEC(del_period));

    return 0;
}

int wait_for_parallel(){

    //https://github.com/boschsensortec/BME68x-Sensor-API/blob/80ea120a8b8ac987d7d79eb68a9ed796736be845/examples/parallel_mode/parallel_mode.c#L97
    uint32_t del_period = bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme_api_dev) + (heatr_conf.shared_heatr_dur * 1000);
    //printf("del_period = %u\n",del_period);
    k_sleep(K_USEC(del_period));

    return 0;
}

int wait_for_sequencial(){

    //https://github.com/boschsensortec/BME68x-Sensor-API/blob/80ea120a8b8ac987d7d79eb68a9ed796736be845/examples/sequential_mode/sequential_mode.c#L84
    uint32_t del_period = bme68x_get_meas_dur(BME68X_SEQUENTIAL_MODE, &conf, &bme_api_dev) + (heatr_conf.heatr_dur_prof[0] * 1000);
    //printf("del_period = %u\n",del_period);
    k_sleep(K_USEC(del_period));

    return 0;
}

void bme688_wait_for_measure()
{
    switch(mode){
        case bme688_mode_forced:
            wait_for_forced();
        break;
        case bme688_mode_parallel:
            wait_for_parallel();
        break;
        case bme688_mode_sequencial:
            wait_for_sequencial();
        break;
        default:
        break;
    }

}

int bme688_sample_fetch(const struct device *dev,enum sensor_channel chan)
{
    int8_t rslt = BME68X_OK;

    //this should be performed in a loop before each wait
    if(mode == bme688_mode_forced){
        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE,&bme_api_dev);
        bme68x_check_rslt("bme68x_set_op_mode",rslt);
    }

    bme688_wait_for_measure();

	return rslt;
}

uint8_t bme688_data_get(const struct device *dev, struct bme68x_data *p_data){
    int8_t rslt = BME68X_OK;
    switch(mode){
        case bme688_mode_forced:
            rslt = bme68x_get_data(BME68X_FORCED_MODE, data, &n_fields, &bme_api_dev);
            bme68x_check_rslt("bme68x_get_data",rslt);
        break;
        case bme688_mode_parallel:
            rslt = bme68x_get_data(BME68X_PARALLEL_MODE, data, &n_fields, &bme_api_dev);
            bme68x_check_rslt("bme68x_get_data",rslt);
        break;
        case bme688_mode_sequencial:
            rslt = bme68x_get_data(BME68X_SEQUENTIAL_MODE, data, &n_fields, &bme_api_dev);
            bme68x_check_rslt("bme68x_get_data",rslt);
        break;
        default:
        break;
    }
    if(mode == bme688_mode_forced){
        if(n_fields == 1){
            *p_data = data[0];
        }
    }
    else{
        for(uint8_t i = 0; i < n_fields; i++){
            p_data[i] = data[i];
        }
    }
    return n_fields;
}

static int bme688_channel_get(const struct device *dev,enum sensor_channel chan,struct sensor_value *val)
{
    if (n_fields == 0)
    {
        return EINPROGRESS;
    }

	switch (chan) {
	case SENSOR_CHAN_AMBIENT_TEMP:
		/*
		 * data[0].temperature has a resolution of 0.01 degC.
		 * So 5123 equals 51.23 degC.
		 */
		val->val1 = (int32_t)(data[0].temperature / 100);
		val->val2 = (data[0].temperature - val->val1 * 100) * 10000;
		break;
	case SENSOR_CHAN_PRESS:
		/*
		 * data[0].pressure has a resolution of 1 Pa.
		 * So 96321 equals 96.321 kPa.
		 */
		val->val1 = (int32_t)(data[0].pressure / 1000);
		val->val2 = (data[0].pressure - val->val1 * 1000) * 1000;
		break;
	case SENSOR_CHAN_HUMIDITY:
		/*
		 * data[0].humidity has a resolution of 0.001 %RH.
		 * So 46333 equals 46.333 %RH.
		 */
		val->val1 = (int32_t)(data[0].humidity / 1000);
		val->val2 = (data[0].humidity - val->val1 * 1000) * 1000;
		break;
	case SENSOR_CHAN_GAS_RES:
		/*
		 * data[0].gas_resistance has a resolution of 1 ohm.
		 * So 100000 equals 100000 ohms.
		 */
		val->val1 = data[0].gas_resistance;
		val->val2 = 0;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}


static const struct sensor_driver_api bme688_api_funcs = {
	.sample_fetch = bme688_sample_fetch,
	.channel_get = bme688_channel_get,
};

#define DT_DRV_COMPAT bosch_bme688

#define BME688_CONFIG_I2C(inst)			       \
	{					       \
		.i2c = I2C_DT_SPEC_INST_GET(inst), \
	}

//bme68x_data unused

#define BME688_DEFINE(inst)						                                        \
	static struct bme68x_data bme68x_data_##inst;			                            \
	static const struct bme688_config bme688_config_##inst = BME688_CONFIG_I2C(inst);   \
	SENSOR_DEVICE_DT_INST_DEFINE(inst,				                                    \
			 bme688_init,					                                    \
			 NULL,						                                                \
			 &bme68x_data_##inst,				                                        \
			 &bme688_config_##inst,				                                        \
			 POST_KERNEL,					                                            \
			 CONFIG_SENSOR_INIT_PRIORITY,			                                    \
			 &bme688_api_funcs);

/* Create the struct device for every status "okay" node in the devicetree. */
DT_INST_FOREACH_STATUS_OKAY(BME688_DEFINE)
