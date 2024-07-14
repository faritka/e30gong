/*
 * Copyright (c) 2023 Farit
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief STM32 Power Management public API header file.
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_STM32PM_H_
#define ZEPHYR_INCLUDE_DRIVERS_STM32PM_H_

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief STM32 PM driver APIs
 * @defgroup pm_interface PM driver APIs
 * @ingroup io_interfaces
 * @{
 */

/**
 * @brief Get the wakeup pin that caused the processor to exit from the Standby mode
 *
 * @return int The ID of the wakeup pin
 */
typedef int32_t (*stm32pm_api_wakeup_pin_get)(const struct device *dev);

/**
 * @brief Check if the wakeup pin is still active
 *
 * @return int Active or not
 */
typedef int (*stm32pm_api_wakeup_pin_active)(const struct device *dev);


/**
 * @brief Put processor into a power state.
 *
 * This function implements the SoC specific details necessary
 * to put the processor into available power states.
 *
 * @param state Power state.
 * @param substate_id Power substate id.
 */

typedef int (*stm32pm_api_state_set)(const struct device *dev, enum pm_state state, uint8_t substate_id);


/*
 * STM32 PM driver API
 *
 * This is the mandatory API any STM32PM driver needs to expose.
 */
__subsystem struct stm32pm_driver_api {
    stm32pm_api_wakeup_pin_get wakeup_pin_get;
    stm32pm_api_wakeup_pin_active wakeup_pin_active;
    stm32pm_api_state_set state_set;
};

/**
 * @endcond
 */

/**
 * @brief Put processor into a power state.
 *
 * This function implements the SoC specific details necessary
 * to put the processor into available power states.
 *
 * @param state Power state.
 * @param substate_id Power substate id.
 * @retval 0        On success.
 * @retval -EINVAL  If a parameter with an invalid value has been provided.
 */
static inline int stm32pm_state_set(const struct device *dev, enum pm_state state, uint8_t substate_id)
{
    const struct stm32pm_driver_api *api = (const struct stm32pm_driver_api *)dev->api;

    return api->state_set(dev, state, substate_id);
}

/**
 * @brief Gets the wakeup pin that awaken the system
 *
 *
 * @retval int The number of the wakeup pin
 */
static inline int stm32pm_wakeup_pin_get(const struct device *dev)
{
    const struct stm32pm_driver_api *api = (const struct stm32pm_driver_api *)dev->api;

    return api->wakeup_pin_get(dev);
}

/**
 * @brief If the wakeup pin that has awaken the system is still active
 *
 *
 * @retval int Active or not
 */
static inline int stm32pm_wakeup_pin_active(const struct device *dev)
{
    const struct stm32pm_driver_api *api = (const struct stm32pm_driver_api *)dev->api;

    return api->wakeup_pin_active(dev);
}


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  /* ZEPHYR_INCLUDE_DRIVERS_STM32PM_H_ */
