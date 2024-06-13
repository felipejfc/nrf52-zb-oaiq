#ifndef _EPD_1in9_H_
#define _EPD_1in9_H_

#include <stdlib.h>
#include <zephyr/drivers/i2c.h>

// address
#define adds_com  	0x3C
#define adds_data	0x3D

#define EPD_BUSY_PIN 13
#define EPD_RST_PIN 22
#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

struct epaper_config {
	struct i2c_dt_spec bus;
};

// unused
struct epaper_data {
	uint8_t status_mask;
	struct k_sem int_sem;
};

extern unsigned char DSPNUM_1in9_on[];
extern unsigned char DSPNUM_1in9_off[];
extern unsigned char DSPNUM_1in9_WB[];
extern unsigned char DSPNUM_1in9_W0[];
extern unsigned char DSPNUM_1in9_W1[];
extern unsigned char DSPNUM_1in9_W2[];
extern unsigned char DSPNUM_1in9_W3[];
extern unsigned char DSPNUM_1in9_W4[];
extern unsigned char DSPNUM_1in9_W5[];
extern unsigned char DSPNUM_1in9_W6[];
extern unsigned char DSPNUM_1in9_W7[];
extern unsigned char DSPNUM_1in9_W8[];
extern unsigned char DSPNUM_1in9_W9[];

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void EPaper_GPIOInit(void);
EXTERNC void EPaper_Init(bool fullUpdate);
EXTERNC void EPD_1in9_Reset(void);
EXTERNC void EPD_1in9_ReadBusy();
EXTERNC void EPD_1in9_lut_DU_WB();
EXTERNC void EPD_1in9_lut_GC();
EXTERNC void EPD_1in9_lut_5S();
EXTERNC void EPD_1in9_Temperature();
EXTERNC int  EPD_1in9_init();
EXTERNC void EPD_1in9_Write_Screen(unsigned char *image);
EXTERNC void EPD_1in9_Set_TempHum(int16_t temp, uint16_t hum);
EXTERNC void EPD_1in9_sleep();
EXTERNC int epaper_comm_init(const struct device *dev);
EXTERNC void EPD_1in9_hardWakeUp();
#undef EXTERNC

#endif
