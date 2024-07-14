/*
 * The class that definies the audio signal
 * 
 */
#ifndef __GONG_AUDIO_H
#define __GONG_AUDIO_H

#include <zephyr/sys/printk.h>

#include <zephyr/device.h>

/**
 * Contains audio data in the WAV format stored in the flash
 */
class GongAudio
{
    public:
        //the single  note sound data array
        const static uint8_t singleSoundData[178460];

        //the three notes sound data
        //repeat three times without a delay
        const static uint8_t threeSoundData[134594];

};

#endif

