#include <GongPlayer.h>

GongPlayer::GongPlayer()
{
    dac = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(st_stm32dac));
    
    if (!dac) {
        printk("Device STM32DAC not found.\n");
    }

}

void GongPlayer::play(int32_t wakeupPin)
{
    printk("wakeupPin in play: %d\n", wakeupPin);

    switch(wakeupPin) {
        case LL_PWR_WAKEUP_PIN1:
            playT1();
            break;
        case LL_PWR_WAKEUP_PIN4:
            playT2();
            break;
        case LL_PWR_WAKEUP_PIN2:
            playT3();
            break;
        case LL_PWR_WAKEUP_PIN5:
            playT4();
            break;

        default:
            break;
    }

    //disables the amplifier
//    stm32dac_stop(dac);
}

void GongPlayer::playT1()
{
    printk("Playing T1 signal\n");

    stm32dac_play_audio(dac, GongAudio::threeSoundData, 1, 30);
}

void GongPlayer::playT2()
{
    printk("Playing T2 signal\n");

    stm32dac_play_audio(dac, GongAudio::singleSoundData, 1, 30);
}

void GongPlayer::playT3()
{
    printk("Playing T3 signal\n");

    stm32dac_play_audio(dac, GongAudio::singleSoundData, 1, 30);
}

void GongPlayer::playT4()
{
    printk("Playing T4 signal\n");

}
