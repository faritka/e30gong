/*
 * Copyright (c) 2023 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stm32lpm.h"

/*
 * Inits a wakeup gpio pin
 */
static int stm32lpm_init_wakeup_gpio(const struct gpio_dt_spec *wakeup_gpio)
{
    int ret = 0;

    if (!gpio_is_ready_dt(wakeup_gpio)) {
        printk("Error: button device %d is not ready\n", wakeup_gpio->pin);
        return 1;
    }

    ret = gpio_pin_configure_dt(wakeup_gpio, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure %s pin %d\n", ret, wakeup_gpio->port->name, wakeup_gpio->pin);
        return 1;
    }

    ret = gpio_pin_interrupt_configure_dt(wakeup_gpio, GPIO_INT_EDGE_RISING);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt on %s pin %d\n", ret, wakeup_gpio->port->name, wakeup_gpio->pin);
        return 1;   
    }

    return 0;
}

/**
 * Converts a wakeup pin ID to DT spec
 */
static struct gpio_dt_spec* stm32lpm_get_wakeup_gpio(const struct device *dev, int32_t wakeup_pin)
{
    const struct stm32lpm_data *data = (struct stm32lpm_data *)dev->data;
    struct stm32lpm_config *config = (struct stm32lpm_config *)dev->config;

    for (int i = 0; i < sizeof(data->wakeup_pins)/sizeof(uint32_t); i++) {
        if (data->wakeup_pins[i] == wakeup_pin) {
            return &config->wakeup_gpios[i];
        }
    }

    return NULL;
}

/*
 * Sets the Standby mode
 */
static int stm32lpm_enter_standby(const struct device *dev)
{
    const struct stm32lpm_data *data = (struct stm32lpm_data *)dev->data;

    printk("Entering Standby\n");

    // Clear all wake up Flags
    LL_PWR_ClearFlag_WU();

    for (int i = 0; i < sizeof(data->wakeup_pins)/sizeof(uint32_t); i++) {
        // Disable all used wakeup sources
        LL_PWR_DisableWakeUpPin(data->wakeup_pins[i]);

        // Enable the wakeup pin irq polarity
        LL_PWR_SetWakeUpPinPolarityHigh(data->wakeup_pins[i]);

        // Enable wakeup pin
        LL_PWR_EnableWakeUpPin(data->wakeup_pins[i]);
    }

     // Set STANDBY mode when CPU enters deepsleep
     LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);

    // Set SLEEPDEEP bit of Cortex System Control Register
    LL_LPM_EnableDeepSleep();

    // This option is used to ensure that store operations are completed
#if defined ( __CC_ARM)
  __force_stores();
#endif
  // Request Wait For Interrupt
  __WFI();

    return 0;
}

/**
  * Switches the power state
  * @param dev Pointer to device structure
 */
static int stm32lpm_state_set(const struct device *dev, enum pm_state state, uint8_t substate_id)
{
    ARG_UNUSED(dev);

    if (state == PM_STATE_STANDBY) {
        stm32lpm_enter_standby(dev);
    }

    return 0;
}

/*
 * Gets the wakeup pin that has awaken the system
 */
static int32_t stm32lpm_wakeup_pin_get(const struct device *dev)
{
    const struct stm32lpm_data *data = (struct stm32lpm_data *)dev->data;

    printk("Getting the wakeup pin that has awaken the system\n");

    return data->active_wakeup_pin;
}

/*
 * If the wakeup pin that has awaken the system is still active
 */
static int stm32lpm_wakeup_pin_active(const struct device *dev)
{
    const struct stm32lpm_data *data = (struct stm32lpm_data *)dev->data;
    int is_active = 0;

    printk("Checking if the wakeup pin that has awaken the system is still active\n");

    struct gpio_dt_spec *wakeup_gpio = stm32lpm_get_wakeup_gpio(dev, data->active_wakeup_pin);
    printk("Wakeup Pin: %u, gpio name: %s\n", wakeup_gpio->pin, wakeup_gpio->port->name); 

    is_active = gpio_pin_get_dt(wakeup_gpio);
    printk("Wakeup Pin active: %d\n", is_active); 
    
    return is_active;
}


/**
 * @brief Inits the driver
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static int stm32lpm_init(const struct device *dev)
{
    printk("Configuring STM32 PM\n");

    struct stm32lpm_config *config = (struct stm32lpm_config *)dev->config;
    struct stm32lpm_data *data = (struct stm32lpm_data *)dev->data;

    //assign wakeup_pins from GPIOs for STM32L452
    for (int i = 0; i < sizeof(config->wakeup_gpios)/sizeof(struct gpio_dt_spec); i++) {
        printk("Wakeup Pin: %u, gpio name: %s\n", config->wakeup_gpios[i].pin, config->wakeup_gpios[i].port->name); 

        switch(config->wakeup_gpios[i].pin) {
            //gpioa 0
            case 0:
                data->wakeup_pins[i] = LL_PWR_WAKEUP_PIN1;
                break;
            //gpioc 13
            case 13:
                data->wakeup_pins[i] = LL_PWR_WAKEUP_PIN2;
                break;
            //gpioe 6
            case 6:
                data->wakeup_pins[i] = LL_PWR_WAKEUP_PIN3;
                break;
            //gpioa 2
            case 2:
                data->wakeup_pins[i] = LL_PWR_WAKEUP_PIN4;
                break;
            //gpioc 5
            case 5:
                data->wakeup_pins[i] = LL_PWR_WAKEUP_PIN5;
                break;
        }

        printk("Wakeup Pin number: %d\n", data->wakeup_pins[i]); 
    }

    printk("In init after\n");

    // Check if the system was resumed from StandBy mode
    if (LL_PWR_IsActiveFlag_SB() != 0) { 
        // Clear Standby flag
        LL_PWR_ClearFlag_SB(); 

        printk("Resumed from the Standby mode\n");
    }

    printk("In init after 2\n");

    // Check and Clear the Wakeup pins
    if (LL_PWR_IsActiveFlag_WU1() != 0) {
        printk("Wakeup pin 1 was active\n");
        data->active_wakeup_pin = LL_PWR_WAKEUP_PIN1;
    }
    else if (LL_PWR_IsActiveFlag_WU2() != 0) {
        printk("Wakeup pin 2 was active\n");
        data->active_wakeup_pin = LL_PWR_WAKEUP_PIN2;
    }
    else if (LL_PWR_IsActiveFlag_WU3() != 0) {
        printk("Wakeup pin 3 was active\n");
        data->active_wakeup_pin = LL_PWR_WAKEUP_PIN3;
    }
    else if (LL_PWR_IsActiveFlag_WU4() != 0) {
        printk("Wakeup pin 4 was active\n");
        data->active_wakeup_pin = LL_PWR_WAKEUP_PIN4;
    }
    else if (LL_PWR_IsActiveFlag_WU5() != 0) {
        printk("Wakeup pin 5 was active\n");
        data->active_wakeup_pin = LL_PWR_WAKEUP_PIN5;
    }

    printk("In init after 3\n");

    //clear all Wakeup pins
    LL_PWR_ClearFlag_WU();

    printk("In init after 4\n");


    for (int i = 0; i < sizeof(config->wakeup_gpios)/sizeof(struct gpio_dt_spec); i++) {
        struct gpio_dt_spec *wakeup_gpio = &config->wakeup_gpios[i];

        stm32lpm_init_wakeup_gpio(wakeup_gpio);
    }

    return 0;
}

static struct stm32lpm_data stm32lpm_data = {
    .active_wakeup_pin = -1,
};

static struct stm32lpm_config stm32lpm_config = {
    .wakeup_gpios = { DT_FOREACH_PROP_ELEM_SEP(STM32PM_NODE, wakeup_gpios, GPIO_DT_SPEC_GET_BY_IDX, (,)) },
};

static const struct stm32pm_driver_api stm32lpm_driver_api = {
    .state_set = stm32lpm_state_set,
    .wakeup_pin_get = stm32lpm_wakeup_pin_get,
    .wakeup_pin_active = stm32lpm_wakeup_pin_active,
};

DEVICE_DT_INST_DEFINE(0, &stm32lpm_init,
    NULL, &stm32lpm_data,
    &stm32lpm_config, PRE_KERNEL_1,
    CONFIG_KERNEL_INIT_PRIORITY_DEVICE, &stm32lpm_driver_api);
