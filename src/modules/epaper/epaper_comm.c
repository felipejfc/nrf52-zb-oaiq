#include "epaper.h"

#define EPAPER_COMM_I2C(inst)  \
	{					       \
		.bus = I2C_DT_SPEC_INST_GET(inst), \
	}

#define DT_DRV_COMPAT waveshare_epaper_comm

// epaper_data unused
#define EPAPER_COMM_DEFINE(inst)						                                \
	static const struct epaper_data epaper_comm_data##inst;   \
	static const struct epaper_config epaper_comm_config_##inst = EPAPER_COMM_I2C(inst);   \
	I2C_DEVICE_DT_INST_DEFINE(inst,				                                    \
			 epaper_comm_init,					                                    \
			 NULL,						                                                \
             &epaper_comm_data##inst,                                                    \
			 &epaper_comm_config_##inst,				                                 \
			 POST_KERNEL,					                                            \
			 CONFIG_SENSOR_INIT_PRIORITY,			                                    \
			 NULL);

/* Create the struct device for every status "okay" node in the devicetree. */
DT_INST_FOREACH_STATUS_OKAY(EPAPER_COMM_DEFINE)