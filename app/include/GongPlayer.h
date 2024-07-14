/*
 * The class that plays gong signals
 * 
 */
#ifndef __GONG_PLAYER_H
#define __GONG_PLAYER_H

#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <stm32_ll_pwr.h>

#include "GongAudio.h"
#include <driver_stm32dac.h>


class GongPlayer
{
    public:

        GongPlayer();

        /**
         * Plays a signal depending on the wakeup pin
         *
         * @param int32_t wakeupPin The wakeup pin
         */
        void play(int32_t wakeupPin);

    private:

        //the DAC device
        const struct device *dac;

        //play the Gong T1 sound
        void playT1();

        //play the Gong T2 sound
        void playT2();

        //play the Gong T3 sound
        void playT3();

        //play the Gong T4 sound
        void playT4();

};

#endif

