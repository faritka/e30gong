/*
 * The class for Power management
 * 
 */
#ifndef __GONG_PM_H
#define __GONG_PM_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/state.h>

#include <driver_stm32pm.h>


class GongPm
{
    public:
        GongPm();

        /**
         * Gets the wakeup pin that has awaken the system
         */
        int32_t getWakeupPin();

        /**
         * Checks if the wakeup pin is still active
         */
        int isWakeupPinActive();

        /**
         * Puts the system into the standby mode
         */
        void standby();

    private:
        
        //the PM device
        const struct device *pm;


};

#endif

