/*
 * Copyright (c) 2023 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef STM32LDAC_DRIVER_H__
#define STM32LDAC_DRIVER_H__

#define DT_DRV_COMPAT st_stm32dac

#include <string.h>

#include "stm32_ll_bus.h"
#include "stm32_ll_rcc.h"
#include "stm32_ll_system.h"
#include "stm32_ll_utils.h"
#include "stm32_ll_exti.h"
#include "stm32_ll_dma.h"
#include "stm32_ll_tim.h"
#include "stm32_ll_dac.h"
#include "stm32_ll_gpio.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>
#include <zephyr/sys/printk.h>
#include <soc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stm32ldac, CONFIG_KERNEL_LOG_LEVEL);

#include <driver_stm32dac.h>
#include "wave.h"

#define STM32DAC_NODE DT_INST(0, st_stm32dac)

//the buffer to store audio data before sending it to DAC
#define BUFFERSIZE 512

//the IRQ number for DMA1 Channel3
#define IRQ_DMA_CHANNEL 13

/** @brief Driver config data */
struct stm32ldac_config {
    struct gpio_dt_spec enable_gpio;
};

/** @brief Driver instance data */
struct stm32ldac_data {
    
#ifdef CONFIG_PM_DEVICE
    uint32_t pm_state;
#endif
};

#endif
