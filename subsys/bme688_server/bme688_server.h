
#ifndef __BME688_SERVER_H__
#define __BME688_SERVER_H__

#include <stdint.h>
#include <string>

#ifdef __cplusplus

#include <json.hpp>
using json = nlohmann::json;

extern "C" {
#endif

namespace hsm
{

}

typedef void (*bme688_handler_t)(json &data);

typedef void (*bme688_pre_work_handler_t)();

/*  Setting the handler and starts the BME688 thread loop*/
void start_bme688(bme688_handler_t handler, bme688_pre_work_handler_t pre_work_handler = NULL);

/* (Optional) The config currently should be set before setting the handler */
void set_bme688_config(json &config);

#ifdef __cplusplus
}
#endif

#endif /*__BME688_SERVER_H__*/
