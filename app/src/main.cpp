/*
 * Copyright (c) 2023 Farit N
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#include "app_version.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/sys/printk.h>

#include <zephyr/pm/device.h>
#include <zephyr/pm/state.h>

#include "GongAudio.h"
#include "GongPlayer.h"
#include "GongPm.h"


int main(void)
{

    printk("in main\n");

    //power management
    GongPm pm;
    int wakeupPin = pm.getWakeupPin();

    //if the microprocessor has been awaken by a wakeup pin
    if (wakeupPin >= 0) {
        for (int i = 0; i < 100; i++) {
            GongPlayer player;
            player.play(wakeupPin);

            if (pm.isWakeupPinActive() == 0) {
                break;
            }
            
            //still active, play again after a pause
            k_msleep(200);
        }
    }

    //increase when you need to reprogram often
    //you can't reprogram a sleeping device
    k_msleep(100);

    pm.standby();

    return 0;
}
