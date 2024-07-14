/*
 * Copyright (c) 2023 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef STM32LPM_DRIVER_H__
#define STM32LPM_DRIVER_H__

#define DT_DRV_COMPAT st_stm32pm

#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/state.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include <stm32_ll_utils.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_cortex.h>
#include <stm32_ll_pwr.h>
#include <stm32_ll_rcc.h>
#include <stm32_ll_system.h>
#include "stm32_ll_gpio.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stm32lpm, CONFIG_KERNEL_LOG_LEVEL);

#include "driver_stm32pm.h"

#define STM32PM_NODE DT_INST(0, st_stm32pm)

/** @brief Driver config data */
struct stm32lpm_config {
    struct gpio_dt_spec wakeup_gpios[DT_PROP_LEN(STM32PM_NODE, wakeup_gpios)];
};

/** @brief Driver instance data */
struct stm32lpm_data {
    uint32_t wakeup_pins[DT_PROP_LEN(STM32PM_NODE, wakeup_gpios)];
    int32_t active_wakeup_pin;
};


#endif
