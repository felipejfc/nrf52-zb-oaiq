#include <zephyr/kernel.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdio.h>

#include "flash_settings_storage.h"

#define USER_DATA_PARTITION_OFFSET	FIXED_PARTITION_OFFSET(user_data)
#define USER_DATA_PARTITION_DEVICE	FIXED_PARTITION_DEVICE(user_data)
#define FLASH_PAGE_SIZE 4096

#define SETTINGS_BSEC2_OFFSET (USER_DATA_PARTITION_OFFSET + 0x0)

static const struct device *flash_dev = USER_DATA_PARTITION_DEVICE;

void fss_write_data(uint8_t *data,uint16_t size){
	if (flash_erase(flash_dev, SETTINGS_BSEC2_OFFSET, FLASH_PAGE_SIZE) != 0) {
		printf("   Flash erase failed!\n");
	} else {
		printf(" * Succeeded Flash erase partition 'settings_storage' @0x%x 4KB Page!\n",SETTINGS_BSEC2_OFFSET);
	}
    off_t offset = SETTINGS_BSEC2_OFFSET;
    if (flash_write(flash_dev, offset, data,size) != 0) {
        printf("   Flash write failed!\n");
        return;
    }
}

void fss_read_data(uint8_t *data,uint16_t size){
	if (!device_is_ready(flash_dev)) {
		printf("Flash device not ready\n");
		return;
	}

    const off_t offset = SETTINGS_BSEC2_OFFSET;
    printf(" * reading from 'settings_storage' 0x%lx",offset);
    if (flash_read(flash_dev, offset, data,size) != 0){
        printf("   Flash read failed!\n");
        return;
    }
}
