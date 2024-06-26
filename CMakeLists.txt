#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

################################################################################

# The application uses the configuration/<board> scheme for configuration files.
set(APPLICATION_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/configuration/\${BOARD}")

macro(app_set_runner_args)
    board_runner_args(openocd "-f interface/cmsis-dap.cfg -c \"transport select swd\" -f target/nrf52.cfg -s tcl")
endmacro()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(zigbee_weather_station)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/bme688)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/subsys/bme688_server)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/subsys/app_flash)

#include dirs
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/bme688)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/xiao_sense_nrf52840_battery_lib/src/battery)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/epaper)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/subsys/bme688_server)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/subsys/app_flash)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

################################################################################

# NORDIC SDK APP START
target_sources(app PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sensor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/reporting.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pm/flash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/zigbee/clusters.c
  ${CMAKE_CURRENT_SOURCE_DIR}/subsys/bme688_server/bme688_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/subsys/bme688_server/bsec2/bsec2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/subsys/app_flash/flash_settings_storage.cpp
  #epaper
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/epaper/epaper.c
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/epaper/epaper_comm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/bme688/bme688.c
  ${CMAKE_CURRENT_SOURCE_DIR}/modules/xiao_sense_nrf52840_battery_lib/src/battery/battery.c
)

# NORDIC SDK APP END
