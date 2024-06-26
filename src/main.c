/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>
#include <ram_pwrdn.h>
#include <zb_nrf_platform.h>
#include <zboss_api.h>
#include <zboss_api_addons.h>
#include <zephyr/kernel.h>
#include <zigbee/zigbee_app_utils.h>
#include <zigbee/zigbee_error_handler.h>
#include "pm/flash.h"

#ifdef CONFIG_USB_DEVICE_STACK
#include <zephyr/usb/usb_device.h>
#endif /* CONFIG_USB_DEVICE_STACK */

#include "reporting.h"

/* Delay for console initialization */
#define WAIT_FOR_CONSOLE_MSEC 100
#define WAIT_FOR_CONSOLE_DEADLINE_MSEC 2000

/* Weather check period */
#define WEATHER_CHECK_PERIOD_MSEC (1000 * CONFIG_WEATHER_CHECK_PERIOD_SECONDS)

/* Delay for first weather check */
#define WEATHER_CHECK_INITIAL_DELAY_MSEC (1000 * CONFIG_FIRST_WEATHER_CHECK_DELAY_SECONDS)

/* Time of LED on state while blinking for identify mode */
#define IDENTIFY_LED_BLINK_TIME_MSEC 500

/* In Thingy53 each LED is a RGB component of a single LED */
#define LED_RED DK_LED1
#define LED_GREEN DK_LED2
#define LED_BLUE DK_LED3

/* LED indicating that device successfully joined Zigbee network */
#define ZIGBEE_NETWORK_STATE_LED LED_BLUE

/* LED used for device identification */
#define IDENTIFY_LED LED_RED

/* Button used to enter the Identify mode */
#define IDENTIFY_MODE_BUTTON DK_BTN1_MSK

/* Button to start Factory Reset */
#define FACTORY_RESET_BUTTON IDENTIFY_MODE_BUTTON

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console),
				zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");
LOG_MODULE_REGISTER(app, CONFIG_ZIGBEE_WEATHER_STATION_LOG_LEVEL);

/* Stores all cluster-related attributes */
static struct zb_device_ctx dev_ctx;

char modelid[] = {10, 'n', 'R', 'F', '5', '2', '.', 'O', 'A', 'I', 'Q'};
char manufname[] = {6, 'N', 'o', 'r', 'd', 'i', 'c'};

/* Attributes setup */
ZB_ZCL_DECLARE_BASIC_ATTRIB_LIST_WITH_MODEL_MANUF(
	basic_attr_list,
	&dev_ctx.basic_attr.zcl_version, 
	&dev_ctx.basic_attr.power_source,
	&modelid[0],
	&manufname[0]
	);

/* Declare attribute list for Identify cluster (client). */
ZB_ZCL_DECLARE_IDENTIFY_CLIENT_ATTRIB_LIST(
	identify_client_attr_list);

/* Declare attribute list for Identify cluster (server). */
ZB_ZCL_DECLARE_IDENTIFY_SERVER_ATTRIB_LIST(
	identify_server_attr_list,
	&dev_ctx.identify_attr.identify_time);

ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(
	temperature_measurement_attr_list,
	&dev_ctx.temp_attrs.measure_value,
	&dev_ctx.temp_attrs.min_measure_value,
	&dev_ctx.temp_attrs.max_measure_value,
	&dev_ctx.temp_attrs.tolerance
	);

ZB_ZCL_DECLARE_PRESSURE_MEASUREMENT_ATTRIB_LIST(
	pressure_measurement_attr_list,
	&dev_ctx.pres_attrs.measure_value,
	&dev_ctx.pres_attrs.min_measure_value,
	&dev_ctx.pres_attrs.max_measure_value,
	&dev_ctx.pres_attrs.tolerance
	);

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(
	humidity_measurement_attr_list,
	&dev_ctx.humidity_attrs.measure_value,
	&dev_ctx.humidity_attrs.min_measure_value,
	&dev_ctx.humidity_attrs.max_measure_value
	);

ZB_ZCL_DECLARE_CARBON_DIOXIDE_ATTRIB_LIST(
	carbon_dioxide_attr_list,
	&dev_ctx.carbon_dioxide_attrs.measure_value,
	&dev_ctx.carbon_dioxide_attrs.min_measure_value,
	&dev_ctx.carbon_dioxide_attrs.max_measure_value,
	&dev_ctx.carbon_dioxide_attrs.tolerance
	);

ZB_ZCL_DECLARE_AIQ_ATTRIB_LIST(
	aiq_attr_list,
	&dev_ctx.aiq_attrs.iaq,
	&dev_ctx.aiq_attrs.voc,
	&dev_ctx.aiq_attrs.bat
	);

/* Clusters setup */
ZB_HA_DECLARE_WEATHER_STATION_CLUSTER_LIST(
	weather_station_cluster_list,
	basic_attr_list,
	identify_client_attr_list,
	identify_server_attr_list,
	temperature_measurement_attr_list,
	pressure_measurement_attr_list,
	humidity_measurement_attr_list,
	carbon_dioxide_attr_list,
	aiq_attr_list);

/* Endpoint setup (single) */
ZB_HA_DECLARE_WEATHER_STATION_EP(
	weather_station_ep,
	WEATHER_STATION_ENDPOINT_NB,
	weather_station_cluster_list);

/* Device context */
ZBOSS_DECLARE_DEVICE_CTX_1_EP(
	weather_station_ctx,
	weather_station_ep);


static void mandatory_clusters_attr_init(void)
{
	/* Basic cluster attributes */
	dev_ctx.basic_attr.zcl_version = ZB_ZCL_VERSION;
	dev_ctx.basic_attr.power_source = ZB_ZCL_BASIC_POWER_SOURCE_BATTERY;

	/* Identify cluster attributes */
	dev_ctx.identify_attr.identify_time = ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE;
}

static void measurements_clusters_attr_init(void)
{
	/* Temperature */
	dev_ctx.temp_attrs.measure_value = ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN;
	dev_ctx.temp_attrs.min_measure_value = WEATHER_STATION_ATTR_TEMP_MIN;
	dev_ctx.temp_attrs.max_measure_value = WEATHER_STATION_ATTR_TEMP_MAX;
	dev_ctx.temp_attrs.tolerance = WEATHER_STATION_ATTR_TEMP_TOLERANCE;

	/* Pressure */
	dev_ctx.pres_attrs.measure_value = ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_UNKNOWN;
	dev_ctx.pres_attrs.min_measure_value = WEATHER_STATION_ATTR_PRESSURE_MIN;
	dev_ctx.pres_attrs.max_measure_value = WEATHER_STATION_ATTR_PRESSURE_MAX;
	dev_ctx.pres_attrs.tolerance = WEATHER_STATION_ATTR_PRESSURE_TOLERANCE;

	/* Humidity */
	dev_ctx.humidity_attrs.measure_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN;
	dev_ctx.humidity_attrs.min_measure_value = WEATHER_STATION_ATTR_HUMIDITY_MIN;
	dev_ctx.humidity_attrs.max_measure_value = WEATHER_STATION_ATTR_HUMIDITY_MAX;

	/* CO2 */
	dev_ctx.carbon_dioxide_attrs.measure_value = ZB_ZCL_ATTR_CARBON_DIOXIDE_VALUE_UNKNOWN;
	dev_ctx.carbon_dioxide_attrs.min_measure_value = 10 * ZCL_CARBON_DIOXIDE_MEASURED_VALUE_MULTIPLIER;
	dev_ctx.carbon_dioxide_attrs.max_measure_value = 10000 * ZCL_CARBON_DIOXIDE_MEASURED_VALUE_MULTIPLIER;
	dev_ctx.carbon_dioxide_attrs.tolerance = 90 * ZCL_CARBON_DIOXIDE_MEASURED_VALUE_MULTIPLIER;

	/* AIQ */
	dev_ctx.aiq_attrs.iaq = ZB_ZCL_ATTR_AIQ_VALUE_UNKNOWN;
	dev_ctx.aiq_attrs.voc = ZB_ZCL_ATTR_AIQ_VALUE_UNKNOWN;
	dev_ctx.aiq_attrs.bat = ZB_ZCL_ATTR_AIQ_VALUE_UNKNOWN;
}

static void toggle_identify_led(zb_bufid_t bufid)
{
	static bool led_on;

	led_on = !led_on;
	dk_set_led(IDENTIFY_LED, led_on);
	zb_ret_t err = ZB_SCHEDULE_APP_ALARM(toggle_identify_led,
					     bufid,
					     ZB_MILLISECONDS_TO_BEACON_INTERVAL(
						     IDENTIFY_LED_BLINK_TIME_MSEC));
	if (err) {
		LOG_ERR("Failed to schedule app alarm: %d", err);
	}
}

static void start_identifying(zb_bufid_t bufid)
{
	ZVUNUSED(bufid);

	if (ZB_JOINED()) {
		/*
		 * Check if endpoint is in identifying mode,
		 * if not put desired endpoint in identifying mode.
		 */
		if (dev_ctx.identify_attr.identify_time ==
		    ZB_ZCL_IDENTIFY_IDENTIFY_TIME_DEFAULT_VALUE) {

			zb_ret_t zb_err_code = zb_bdb_finding_binding_target(
				WEATHER_STATION_ENDPOINT_NB);

			if (zb_err_code == RET_OK) {
				LOG_INF("Manually enter identify mode");
			} else if (zb_err_code == RET_INVALID_STATE) {
				LOG_WRN("RET_INVALID_STATE - Cannot enter identify mode");
			} else {
				ZB_ERROR_CHECK(zb_err_code);
			}
		} else {
			LOG_INF("Manually cancel identify mode");
			zb_bdb_finding_binding_target_cancel();
		}
	} else {
		LOG_WRN("Device not in a network - cannot identify itself");
	}
}

static void identify_callback(zb_bufid_t bufid)
{
	zb_ret_t err = RET_OK;

	if (bufid) {
		/* Schedule a self-scheduling function that will toggle the LED */
		err = ZB_SCHEDULE_APP_CALLBACK(toggle_identify_led, bufid);
		if (err) {
			LOG_ERR("Failed to schedule app callback: %d", err);
		} else {
			LOG_INF("Enter identify mode");
		}
	} else {
		/* Cancel the toggling function alarm and turn off LED */
		err = ZB_SCHEDULE_APP_ALARM_CANCEL(toggle_identify_led,
						   ZB_ALARM_ANY_PARAM);
		if (err) {
			LOG_ERR("Failed to schedule app alarm cancel: %d", err);
		} else {
			dk_set_led_off(IDENTIFY_LED);
			LOG_INF("Cancel identify mode");
		}
	}
}

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (IDENTIFY_MODE_BUTTON & has_changed) {
		if (IDENTIFY_MODE_BUTTON & button_state) {
			/* Button changed its state to pressed */
		} else {
			/* Button changed its state to released */
			if (was_factory_reset_done()) {
				/* The long press was for Factory Reset */
				LOG_DBG("After Factory Reset - ignore button release");
			} else   {
				/* Button released before Factory Reset */

				/* Start identification mode */
				zb_ret_t err = ZB_SCHEDULE_APP_CALLBACK(start_identifying, 0);

				if (err) {
					LOG_ERR("Failed to schedule app callback: %d", err);
				}

				/* Inform default signal handler about user input at the device */
				user_input_indicate();
			}
		}
	}

	check_factory_reset_button(button_state, has_changed);
}

static void gpio_init(void)
{
	int err = dk_buttons_init(button_changed);

	if (err) {
		LOG_ERR("Cannot init buttons (err: %d)", err);
	}

	err = dk_leds_init();
	if (err) {
		LOG_ERR("Cannot init LEDs (err: %d)", err);
	}
}

#ifdef CONFIG_USB_DEVICE_STACK
static void wait_for_console(void)
{
	const struct device *console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;
	uint32_t time = 0;

	/* Enable the USB subsystem and associated HW */
	if (usb_enable(NULL)) {
		LOG_ERR("Failed to enable USB");
	} else {
		/* Wait for DTR flag or deadline (e.g. when USB is not connected) */
		while (!dtr && time < WAIT_FOR_CONSOLE_DEADLINE_MSEC) {
			uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
			/* Give CPU resources to low priority threads */
			k_sleep(K_MSEC(WAIT_FOR_CONSOLE_MSEC));
			time += WAIT_FOR_CONSOLE_MSEC;
		}
	}
}
#endif /* CONFIG_USB_DEVICE_STACK */

void zboss_signal_handler(zb_bufid_t bufid)
{
	zb_zdo_app_signal_hdr_t *signal_header = NULL;
	zb_zdo_app_signal_type_t signal = zb_get_app_signal(bufid, &signal_header);

	/* Update network status LED but only for debug configuration */
	#ifdef CONFIG_LOG
	zigbee_led_status_update(bufid, ZIGBEE_NETWORK_STATE_LED);
	#endif /* CONFIG_LOG */

	/* Detect ZBOSS startup */
	switch (signal) {
	case ZB_ZDO_SIGNAL_SKIP_STARTUP:
		/* ZBOSS framework has started - schedule first weather check */
		/*
		err = ZB_SCHEDULE_APP_ALARM(check_weather,
					    0,
					    ZB_MILLISECONDS_TO_BEACON_INTERVAL(
						    WEATHER_CHECK_INITIAL_DELAY_MSEC));
		*/
	case ZB_COMMON_SIGNAL_CAN_SLEEP:
		/* Zigbee stack can enter sleep mode */
		zb_sleep_now();
		break;
	default:
		break;
	}

	/* Let default signal handler process the signal*/
	ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

	/*
	 * All callbacks should either reuse or free passed buffers.
	 * If bufid == 0, the buffer is invalid (not passed).
	 */
	if (bufid) {
		zb_buf_free(bufid);
	}
}

void turn_off_flash(void)
{
/* Put the flash into deep power down mode
     * Note. The init will fail if the system is reset without power loss.
     * I believe the init fails because the chip is still in dpd mode. I also
     * think this is why Zephyr is having issues.
     * The chip seems to stay in dpd and is OK after another reset or full power
     * cycle.
     * Since the errors are ignored I removed the checks.
     */
    da_flash_init();
    da_flash_command(0xB9);
    da_flash_uninit();	
}

void configure_zb(){
	zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
	zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000));
}

int main(void)
{
	#ifdef CONFIG_USB_DEVICE_STACK
	wait_for_console();
	#endif /* CONFIG_USB_DEVICE_STACK */

	register_factory_reset_button(FACTORY_RESET_BUTTON);
	gpio_init();
	weather_station_init();

	/* Register device context (endpoint) */
	ZB_AF_REGISTER_DEVICE_CTX(&weather_station_ctx);

	/* Init Basic and Identify attributes */
	mandatory_clusters_attr_init();

	/* Init measurements-related attributes */
	measurements_clusters_attr_init();

	/* Register callback to identify notifications */
	ZB_AF_SET_IDENTIFY_NOTIFICATION_HANDLER(WEATHER_STATION_ENDPOINT_NB, identify_callback);

	if (IS_ENABLED(CONFIG_RAM_POWER_DOWN_LIBRARY)) {
		power_down_unused_ram();
	}

	configure_zb();

	// Turn off qspi flash
	turn_off_flash();

	/* Enable Sleepy End Device behavior */
	//zigbee_configure_sleepy_behavior(true);
	zb_set_rx_on_when_idle(ZB_FALSE);

	/* Start Zigbee stack */
	zigbee_enable();

	return;
}
