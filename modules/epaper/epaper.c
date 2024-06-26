#include <stdlib.h>
#include "epaper.h"
#include "math.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

//////////////////////////////////////full screen update LUT////////////////////////////////////////////

unsigned char DSPNUM_1in9_on[]   = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,       };  // all black
unsigned char DSPNUM_1in9_off[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       };  // all white
unsigned char DSPNUM_1in9_WB[]   = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,       };  // All black font
unsigned char DSPNUM_1in9_W0[]   = {0x00, 0xbf, 0x1f, 0xbf, 0x1f, 0xbf, 0x1f, 0xbf, 0x1f, 0xbf, 0x1f, 0xbf, 0x1f, 0x00, 0x00,       };  // 0
unsigned char DSPNUM_1in9_W1[]   = {0xff, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00, 0x00,       };  // 1
unsigned char DSPNUM_1in9_W2[]   = {0x00, 0xfd, 0x17, 0xfd, 0x17, 0xfd, 0x17, 0xfd, 0x17, 0xfd, 0x17, 0xfd, 0x37, 0x00, 0x00,       };  // 2
unsigned char DSPNUM_1in9_W3[]   = {0x00, 0xf5, 0x1f, 0xf5, 0x1f, 0xf5, 0x1f, 0xf5, 0x1f, 0xf5, 0x1f, 0xf5, 0x1f, 0x00, 0x00,       };  // 3
unsigned char DSPNUM_1in9_W4[]   = {0x00, 0x47, 0x1f, 0x47, 0x1f, 0x47, 0x1f, 0x47, 0x1f, 0x47, 0x1f, 0x47, 0x3f, 0x00, 0x00,       };  // 4
unsigned char DSPNUM_1in9_W5[]   = {0x00, 0xf7, 0x1d, 0xf7, 0x1d, 0xf7, 0x1d, 0xf7, 0x1d, 0xf7, 0x1d, 0xf7, 0x1d, 0x00, 0x00,       };  // 5
unsigned char DSPNUM_1in9_W6[]   = {0x00, 0xff, 0x1d, 0xff, 0x1d, 0xff, 0x1d, 0xff, 0x1d, 0xff, 0x1d, 0xff, 0x3d, 0x00, 0x00,       };  // 6
unsigned char DSPNUM_1in9_W7[]   = {0x00, 0x21, 0x1f, 0x21, 0x1f, 0x21, 0x1f, 0x21, 0x1f, 0x21, 0x1f, 0x21, 0x1f, 0x00, 0x00,       };  // 7
unsigned char DSPNUM_1in9_W8[]   = {0x00, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x1f, 0xff, 0x3f, 0x00, 0x00,       };  // 8
unsigned char DSPNUM_1in9_W9[]   = {0x00, 0xf7, 0x1f, 0xf7, 0x1f, 0xf7, 0x1f, 0xf7, 0x1f, 0xf7, 0x1f, 0xf7, 0x1f, 0x00, 0x00,       };  // 9

unsigned char VAR_Temperature=20; 
#define I2C_FREQ_HZ 100000 // 100kHz
struct i2c_dt_spec epaper_dev = { 0 };

LOG_MODULE_REGISTER(epaper, LOG_LEVEL_DBG);

/******************************************************************************
function :	GPIO Init
parameter:
******************************************************************************/
void EPaper_GPIOInit(void)
{

	const struct gpio_dt_spec rst =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, rst_gpios);

	const struct gpio_dt_spec busy =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, busy_gpios);

	const struct gpio_dt_spec pw =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, epaper_gpios);

	gpio_pin_configure_dt(&busy, GPIO_INPUT);
	gpio_pin_configure_dt(&rst, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&pw, GPIO_OUTPUT_ACTIVE);
}


/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void EPD_1in9_Reset(void)
{
	const struct gpio_dt_spec rst =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, rst_gpios);

	gpio_pin_set_dt(&rst, 1);
	k_usleep(200 * 1000); //200ms
	gpio_pin_set_dt(&rst, 0);
	k_usleep(20 * 1000); //20ms
	gpio_pin_set_dt(&rst, 1);
	k_usleep(200 * 1000); //200ms
}

static int send_cmd(struct i2c_dt_spec* dev, uint8_t cmd, uint16_t addr)
{
    if (i2c_write(dev->bus, &cmd, 1, addr)) // can send many
    {
        LOG_ERR("i2c_write_dt failed\n");
        return 1;
    }
    return 0;
}

void EPaper_Init(bool fullUpdate){
	LOG_INF("Initializing E-Paper module");
    EPD_1in9_Reset();
	k_usleep(100 * 1000); //100ms

    send_cmd(&epaper_dev, 0x2B, adds_com);
    send_cmd(&epaper_dev, 0xA7, adds_com);
    send_cmd(&epaper_dev, 0xE0, adds_com);
    
    EPD_1in9_Temperature(&epaper_dev);
    EPD_1in9_lut_5S();

    if (fullUpdate)
    {
        EPD_1in9_Write_Screen(DSPNUM_1in9_off);

        EPD_1in9_lut_GC();

        EPD_1in9_Write_Screen(DSPNUM_1in9_on);
        EPD_1in9_Write_Screen(DSPNUM_1in9_off);
    }else {
        EPD_1in9_Write_Screen(DSPNUM_1in9_off);
    }

    EPD_1in9_lut_DU_WB();
    EPD_1in9_sleep();
}

int epaper_comm_init(const struct device *dev)
{
    epaper_dev = ((struct epaper_config*)dev->config)->bus;
    EPaper_GPIOInit();
    EPaper_Init(true);
    return (int)0;
}


/*
# Note that the size and frame rate of V0 need to be set during initialization, 
# otherwise the local brush will not be displayed
*/

/* 
# 5 waveform  better ghosting
# Boot waveform
*/
void EPD_1in9_lut_5S()
{ 	
    send_cmd(&epaper_dev, 0x82, adds_com);
    send_cmd(&epaper_dev, 0x28, adds_com);
    send_cmd(&epaper_dev, 0x20, adds_com);
    send_cmd(&epaper_dev, 0xA8, adds_com);
    send_cmd(&epaper_dev, 0xA0, adds_com);
    send_cmd(&epaper_dev, 0x50, adds_com);
    send_cmd(&epaper_dev, 0x65, adds_com);
}

void EPD_1in9_Write_Screen(unsigned char *image)
{
    send_cmd(&epaper_dev, 0xAC, adds_com);
    send_cmd(&epaper_dev, 0x2B, adds_com);
    send_cmd(&epaper_dev, 0x40, adds_com);
    send_cmd(&epaper_dev, 0xA9, adds_com);
    send_cmd(&epaper_dev, 0xA8, adds_com);

	for(uint8_t j = 0 ; j<15 ; j++ )
        send_cmd(&epaper_dev, image[j], adds_data);

    send_cmd(&epaper_dev, 0x00, adds_data);

    send_cmd(&epaper_dev, 0xAB, adds_com);
    send_cmd(&epaper_dev, 0xAA, adds_com);
    send_cmd(&epaper_dev, 0xAF, adds_com);

	EPD_1in9_ReadBusy();
	
    send_cmd(&epaper_dev, 0xAE, adds_com);
    send_cmd(&epaper_dev, 0x28, adds_com);
    send_cmd(&epaper_dev, 0xAD, adds_com);
}

/*   
# GC waveform
# The brush waveform
*/
void EPD_1in9_lut_GC()
{
    send_cmd(&epaper_dev, 0x82, adds_com);
    send_cmd(&epaper_dev, 0x20, adds_com);
    send_cmd(&epaper_dev, 0x00, adds_com);
    send_cmd(&epaper_dev, 0xA0, adds_com);
    send_cmd(&epaper_dev, 0x80, adds_com);
    send_cmd(&epaper_dev, 0x40, adds_com);
    send_cmd(&epaper_dev, 0x63, adds_com);
}


/*
# DU waveform white extinction diagram + black out diagram
# Bureau of brush waveform
*/
void EPD_1in9_lut_DU_WB()
{
    send_cmd(&epaper_dev, 0x82, adds_com);
    send_cmd(&epaper_dev, 0x80, adds_com);
    send_cmd(&epaper_dev, 0x00, adds_com);
    send_cmd(&epaper_dev, 0xC0, adds_com);
    send_cmd(&epaper_dev, 0x80, adds_com);
    send_cmd(&epaper_dev, 0x80, adds_com);
    send_cmd(&epaper_dev, 0x62, adds_com);
}

/*
# temperature measurement
# You are advised to periodically measure the temperature and modify the driver parameters
# If an external temperature sensor is available, use an external temperature sensor
*/
void EPD_1in9_Temperature()
{
	if( VAR_Temperature < 10 )
	{
        send_cmd(&epaper_dev, 0x7E, adds_com);
        send_cmd(&epaper_dev, 0x81, adds_com);
        send_cmd(&epaper_dev, 0xB4, adds_com);
	}
	else
	{
        send_cmd(&epaper_dev, 0x7E, adds_com);
        send_cmd(&epaper_dev, 0x81, adds_com);
        send_cmd(&epaper_dev, 0xB4, adds_com);
	}
         
	k_usleep(10 * 1000); //10ms
    send_cmd(&epaper_dev, 0xe7, adds_com);
        
	// Set default frame time
	if(VAR_Temperature<5)
        send_cmd(&epaper_dev, 0x31, adds_com);
	else if(VAR_Temperature<10)
        send_cmd(&epaper_dev, 0x22, adds_com);
	else if(VAR_Temperature<15)
        send_cmd(&epaper_dev, 0x18, adds_com);
	else if(VAR_Temperature<20)
        send_cmd(&epaper_dev, 0x13, adds_com);
	else
        send_cmd(&epaper_dev, 0x0e, adds_com);
}

void EPD_1in9_sleep()
{
    send_cmd(&epaper_dev, 0x28, adds_com);
	EPD_1in9_ReadBusy();
    send_cmd(&epaper_dev, 0xAD, adds_com);

	const struct gpio_dt_spec pw =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, epaper_gpios);

	gpio_pin_set_dt(&pw, 0);

    LOG_DBG("e-Paper sleep");
}

void EPD_1in9_hardWakeUp()
{
	const struct gpio_dt_spec pw =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, epaper_gpios);

	gpio_pin_set_dt(&pw, 1);

	k_usleep(100 * 1000); //10ms
}

void EPD_1in9_ReadBusy(void)
{
	const struct gpio_dt_spec busy =
		GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, busy_gpios);
        
    LOG_DBG("e-Paper busy");
	while(1)
	{
		if(gpio_pin_get_dt(&busy) == 1) {
			break;
        }
	    k_usleep(10 * 1000); //10ms
	}
	k_usleep(10 * 1000); //10ms
    LOG_DBG("e-Paper busy release");
}

void EPD_1in9_Set_TempHumPow(int16_t temp, uint16_t hum, bool lowPower){
    unsigned char image[15] = {0};
    uint8_t numbers[10][2] = {
        {0xbf, 0x1f}, // 0
        {0x00, 0x1f}, // 1
        {0xfd, 0x17}, // 2
        {0xf5, 0x1f}, // 3
        {0x47, 0x1f}, // 4
        {0xf7, 0x1d}, // 5
        {0xff, 0x1d}, // 6
        {0x21, 0x1f}, // 7
        {0xff, 0x1f}, // 8
        {0xf7, 0x1f}, // 9
    }; 
    int num0temp = floor(temp / 1000.0f);
    image[1] = numbers[num0temp][0];
    image[2] = numbers[num0temp][1];
    int num1temp = floor((temp % 1000) / 100.0f);
    image[3] = numbers[num1temp][0];
    image[4] = numbers[num1temp][1] | 1 << 5;
    int num2temp = floor((temp % 100) / 10.0f);
    image[11] = numbers[num2temp][0];
    image[12] = numbers[num2temp][1];
    int num0hum = floor(hum / 1000.0f);
    image[5] = numbers[num0hum][0];
    image[6] = numbers[num0hum][1];
    int num1hum = floor((hum % 1000) / 100.0f);
    image[7] = numbers[num1hum][0];
    image[8] = numbers[num1hum][1] | 1 << 5;
    int num2hum = floor((hum % 100) / 10.0f);
    image[9] = numbers[num2hum][0];
    image[10] = numbers[num2hum][1] | 1 << 5;
    image[13] = 0x05 | 1 << 3 | lowPower << 4;
    EPD_1in9_Write_Screen(&image);
}