/*
 * Copyright (c) 2023 Farit
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief STM32 DAC public API header file.
 */

#ifndef ZEPHYR_INCLUDE_DRIVERS_STM32DAC_H_
#define ZEPHYR_INCLUDE_DRIVERS_STM32DAC_H_

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief STM32 DAC driver APIs
 * @defgroup dac_interface DAC driver APIs
 * @ingroup io_interfaces
 * @{
 */


/*
 * Type definition of DAC API function for playing audio.
 */
typedef int (*stm32dac_api_play_audio)(const struct device *dev,
    uint8_t const* audio_data, const uint16_t play_times, const uint16_t play_delay);


/*
 * Type definition of STM32 DAC API function for stopping the DAC.
 */
typedef int (*stm32dac_api_stop)(const struct device *dev);


/*
 * STM32 DAC driver API
 *
 * This is the mandatory API any DAC driver needs to expose.
 */
__subsystem struct stm32dac_driver_api {
    stm32dac_api_play_audio play_audio;
    stm32dac_api_stop stop;
};

/**
 * @endcond
 */

/**
 * @brief Play audio data in the WAV format via DAC 
 *
 * @param dev         Pointer to the device structure for the driver instance
 * @param audio_data  Audio data in the WAV format
 * @param play_times  How many times to play the audio
 * @param play_delay  The delay before the next play 
 *
 * @retval 0        On success.
 * @retval -EINVAL  If a parameter with an invalid value has been provided.
 */
static inline int stm32dac_play_audio(const struct device *dev, uint8_t const* audio_data, 
    const uint16_t play_times, const uint16_t play_delay)
{
    const struct stm32dac_driver_api *api = (const struct stm32dac_driver_api *)dev->api;

    return api->play_audio(dev, audio_data, play_times, play_delay);
}

/**
 * @brief Stops the DAC 
 *
 * @param dev         Pointer to the device structure for the driver instance.
 *
 * @retval 0        On success.
 * @retval -EINVAL  If a parameter with an invalid value has been provided.
 */
static inline int stm32dac_stop(const struct device *dev)
{
    const struct stm32dac_driver_api *api = (const struct stm32dac_driver_api *)dev->api;

    return api->stop(dev);
}


/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  /* ZEPHYR_INCLUDE_DRIVERS_STM32DAC_H_ */
