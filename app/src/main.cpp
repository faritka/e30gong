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

    //power management
    GongPm pm;
    int wakeupPin = pm.getWakeupPin();

    //how many times to repeat the signal if it's active
    const uint8_t repeatTimes = 5;
    //how many minutes to wait silently after a sound stops 
    //if a wakeup pin is still active
    const uint8_t waitSilentlyMinutes = 15;

    //if the microprocessor has been awaken by a wakeup pin
    if (wakeupPin >= 0) {
        for (int i = 0; i < repeatTimes; i++) {
            GongPlayer player;
            player.play(wakeupPin);

            if (pm.isWakeupPinActive() == 0) {
                break;
            }

            printk("Repeated the sound\n");
            
            //still active, play again after a pause
            k_msleep(500);
        }

        //don't play an annoying sound, just wait
        for (int i = 0; i < waitSilentlyMinutes * 60; i++) {
            if (pm.isWakeupPinActive() == 0) {
                break;
            }
            
            printk("A wakeup pin is active, wait seconds: %u\n", i);

            //sleep for 1 second
            k_msleep(1000);
        }
    }

    //increase when you need to reprogram often
    //you can't reprogram a sleeping device
    k_msleep(100);

    pm.standby();

    return 0;
}
