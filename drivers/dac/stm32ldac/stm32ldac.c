/*
 * Copyright (c) 2024 Farit N
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "stm32ldac.h"

//the flag that is set in the transfer complete interrupt
uint8_t dma_complete;
//2 buffers, fill one buffer, then send it to DAC
//after sending the first buffer, start filling the second one
//switch the buffers when the interrupt DMA_Complete fires
uint16_t dma_buffer[2][BUFFERSIZE];


/*
 * Inits the amplifier enable gpio pin
 */
static int stm32ldac_init_enable_gpio(const struct device *dev)
{
    struct stm32ldac_config *config = (struct stm32ldac_config *)dev->config;
    int ret = 0;

    printk("Configuring the amplifier enable pin\n");

    if (!gpio_is_ready_dt(&config->enable_gpio)) {
        printk("Error: enable device %d is not ready\n", config->enable_gpio.pin);
        return 1;
    }

    ret = gpio_pin_configure_dt(&config->enable_gpio, GPIO_OUTPUT_INACTIVE | GPIO_ACTIVE_HIGH);

    if (ret != 0) {
        printk("Error: failed to configure %s pin %d, error: %d\n",  
            config->enable_gpio.port->name, config->enable_gpio.pin, ret);
        return 1;
    }

    return 0;
}

/*
 * Enables the amplifier enable gpio pin
 */
static int stm32ldac_enable_enable_gpio(const struct device *dev)
{
    struct stm32ldac_config *config = (struct stm32ldac_config *)dev->config;
    int ret = 0;

    printk("Enables the amplifier enable pin\n");

    if (!gpio_is_ready_dt(&config->enable_gpio)) {
        printk("Error: enable device %d is not ready\n", config->enable_gpio.pin);
        return 1;
    }

    ret = gpio_pin_set_dt(&config->enable_gpio, 1);
    if (ret != 0) {
        printk("Error: enable device %d cannot be set, error: %d\n", config->enable_gpio.pin, ret);
        return 1;
    }

    return 0;
}

/*
 * Disables the amplifier enable gpio pin
 */
static int stm32ldac_disable_enable_gpio(const struct device *dev)
{
    struct stm32ldac_config *config = (struct stm32ldac_config *)dev->config;
    int ret = 0;

    printk("Disables the amplifier enable pin\n");

    if (!gpio_is_ready_dt(&config->enable_gpio)) {
        printk("Error: enable device %d is not ready\n", config->enable_gpio.pin);
        return 1;
    }

    ret = gpio_pin_set_dt(&config->enable_gpio, 0);
    if (ret != 0) {
        printk("Error: disable device %d cannot be set, error: %d\n", config->enable_gpio.pin, ret);
        return 1;
    }

    return 0;
}

/**
 * DMA interrupt handler 
 */
static void stm32ldac_irq_handler(const struct device *dev)
{
    ARG_UNUSED(dev);

    if (LL_DMA_IsActiveFlag_TC3(DMA1) == 1) {
        // Clear flag DMA transfer complete
        LL_DMA_ClearFlag_TC3(DMA1);
        
        dma_complete = 1;
    }
}



/**
 * Stops
  * @param dev Pointer to device structure
 */
static int stm32ldac_stop(const struct device *dev)
{
    ARG_UNUSED(dev);

    stm32ldac_disable_enable_gpio(dev);

    // Disable DAC channel DMA request
    LL_DAC_DisableDMAReq(DAC1, LL_DAC_CHANNEL_1);

    // Disable DAC channel
    LL_DAC_Disable(DAC1, LL_DAC_CHANNEL_1);

    // Disable DMA transfer interruption: transfer complete
    LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_3);

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    
    LL_TIM_DisableCounter(TIM6);

    return 0;
}

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

static int stm32ldac_play_audio(const struct device *dev, uint8_t const* audio_data, 
    const uint16_t play_times, const uint16_t play_delay)
{
    ARG_UNUSED(dev);

    printk("In DAC play audio\n");

    stm32ldac_enable_enable_gpio(dev);
    
    //parse the audio data in the WAV format
    WAVFile wav_data = WAV_ParseFileData(audio_data);

    //we use 16-bit WAV files, so each sample consist of 2 bytes, low and high, in a WAV file
    //the data_length is in bytes, find the number of 16-bit samples
    uint32_t samples = wav_data.data_length / 2;

    printk("Data length: %lu\n", (unsigned long)samples);

    if ((strcmp(wav_data.header.file_id, "RIFF") != 0) || (strcmp(wav_data.header.format, "WAVE") != 0)) {
        printk("Incorrect audio format. Only WAV is supported. File id: %s, format: %s\n", wav_data.header.file_id, wav_data.header.format);
        return -EINVAL;
    }

    if ((wav_data.header.number_of_channels != 1) || (wav_data.header.bits_per_sample != 16)) {
        printk("Only 16-bit mono audio is supported. Number of channels: %u, bits per sample: %u\n", 
            wav_data.header.number_of_channels, wav_data.header.bits_per_sample);
        return -EINVAL;
    }
    
    printk("Clock: %lu\n", (unsigned long)sys_clock_hw_cycles_per_sec());
    //calculate the timer autoreload value to play the wav data according to its sample rate.
    //get the processor speed and divide it to the sample rate minus 1 (the timer starts from 0)
    uint16_t timer_autoreload = (sys_clock_hw_cycles_per_sec() / wav_data.header.sample_rate) - 1;

    printk("Timer autoreload: %lu\n", (unsigned long)timer_autoreload);


    // DMA controller clock enable
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // DMA interrupt init
    // DMA1_Channel3_IRQn interrupt configuration
    IRQ_CONNECT(IRQ_DMA_CHANNEL, 6, stm32ldac_irq_handler, 0, 0);
    irq_enable(IRQ_DMA_CHANNEL); 

    //Enable DAC
    LL_DAC_InitTypeDef DAC_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Peripheral clock enable
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_DAC1);
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    // DAC1 GPIO Configuration PA4   ------> DAC1_OUT1
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // DAC_CH1 Init
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMA_REQUEST_6);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_HALFWORD);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_HALFWORD);

    // DAC channel OUT1 config
    DAC_InitStruct.TriggerSource = LL_DAC_TRIG_EXT_TIM6_TRGO;
    DAC_InitStruct.WaveAutoGeneration = LL_DAC_WAVE_AUTO_GENERATION_NONE;
    DAC_InitStruct.OutputBuffer = LL_DAC_OUTPUT_BUFFER_DISABLE;
    DAC_InitStruct.OutputConnection = LL_DAC_OUTPUT_CONNECT_GPIO;
    DAC_InitStruct.OutputMode = LL_DAC_OUTPUT_MODE_NORMAL;
    LL_DAC_Init(DAC1, LL_DAC_CHANNEL_1, &DAC_InitStruct);
    LL_DAC_EnableTrigger(DAC1, LL_DAC_CHANNEL_1);

    //enable the transfer complete interrupt
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);

    //Enable the timer 6. The DAC sends the next sample on the next tick.
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    // Peripheral clock enable
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM6);

    TIM_InitStruct.Prescaler = 0;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = timer_autoreload;
    LL_TIM_Init(TIM6, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM6);
    LL_TIM_SetTriggerOutput(TIM6, LL_TIM_TRGO_UPDATE);
    LL_TIM_DisableMasterSlaveMode(TIM6);
    LL_TIM_EnableCounter(TIM6);
    

    //play the audio several times
    for (uint16_t k = 0; k < play_times; k++) {
        //where audio data starts
        const uint8_t* p = wav_data.data;

        uint8_t bank = 0;
        dma_complete = 1;

        uint32_t count = samples;
        while (0 < count) {
            //the last audio chunk can be smaller than the buffer size
            uint32_t block_size = (count <= BUFFERSIZE) ? count : BUFFERSIZE;

            for (int i = 0; i < block_size; i++) {
                //the low order byte of a 16-bit wav file
                uint8_t low_byte = *p;
                p++;
                //the high order byte of a 16-bit wav file
                uint8_t high_byte = *p;
                p++;

                //combine the low and high bytes to create a 16-bit DAC value
                //values in a wav file are stored signed in the range -32767 to 32768
                //convert the DAC value to unsigned in the range 0-65535 by adding 32767
                uint16_t dac_value = ((high_byte << 8) | low_byte) + 32767;

                //the STM32 DAC is 12 bit, it can represent the values in the range 0-4095
                // 65535/4095 = 16
                dac_value /= 16;

                dma_buffer[bank][i] = dac_value;
            }

            // wait for DMA complete
            while(!dma_complete) {
                //__NOP();
                k_busy_wait(10);
            }

            dma_complete = 0;

            //Disable the DMA channel to configure it
            LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

            // Set DMA transfer addresses of source and destination
            LL_DMA_ConfigAddresses(DMA1,
                LL_DMA_CHANNEL_3,
                (uint32_t)dma_buffer[bank],
                LL_DAC_DMA_GetRegAddr(DAC1, LL_DAC_CHANNEL_1, LL_DAC_DMA_REG_DATA_12BITS_RIGHT_ALIGNED),
                LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

            // Set DMA transfer size
            LL_DMA_SetDataLength(DMA1,
                LL_DMA_CHANNEL_3,
                block_size);

            // Activation of DMA
            // Enable the DMA transfer
            LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

            // Enable DAC channel DMA request
            LL_DAC_EnableDMAReq(DAC1, LL_DAC_CHANNEL_1);

            // Enable DAC channel
            LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);

            bank = !bank;
            count -= block_size;
        }

        k_msleep(play_delay);
    }

    //stm32ldac_stop(dev);

    return 0;
}

#ifdef CONFIG_PM_DEVICE
static int stm32ldac_pm_action(const struct device *dev,
        enum pm_device_action action)
{
    int ret = 0;
    struct stm32ldac_data *data = (struct stm32ldac_data *)dev->data;

    switch (action) {
    case PM_DEVICE_ACTION_RESUME:
        break;
    case PM_DEVICE_ACTION_SUSPEND:
        ret = stm32ldac_stop(dev);
        break;
    default:
        ret = -ENOTSUP;
        break;
    }

    return ret;
}
#endif /* CONFIG_PM_DEVICE */


/**
 * @brief Inits the driver
 *
 * @param dev Pointer to device structure
 *
 * @retval 0 on success else negative errno code.
 */
static int stm32ldac_init(const struct device *dev)
{
    printk("Configuring STM32L4 DAC Wave signal\n");

    stm32ldac_init_enable_gpio(dev);

#ifdef CONFIG_PM_DEVICE
    data->pm_state = PM_DEVICE_STATE_ACTIVE;
#endif
    
    return 0;
}

static const struct stm32dac_driver_api stm32ldac_api = {
    .play_audio = stm32ldac_play_audio,
    .stop = stm32ldac_stop,
};

#define STM32LDAC_INIT(inst)                                       \
static struct stm32ldac_data stm32ldac_data_ ## inst = {};         \
                                                                   \
static struct stm32ldac_config stm32ldac_config_ ## inst = {       \
    .enable_gpio = GPIO_DT_SPEC_GET(DT_INST(inst, st_stm32dac),    \
    enable_gpios)                                                  \
};                                                                 \
                                                                   \
PM_DEVICE_DT_INST_DEFINE(inst, stm32ldac_pm_action);               \
                                                                   \
DEVICE_DT_INST_DEFINE(inst, &stm32ldac_init,                       \
    PM_DEVICE_DT_INST_REF(inst), &stm32ldac_data_ ## inst,         \
    &stm32ldac_config_ ## inst, APPLICATION,                       \
    CONFIG_APPLICATION_INIT_PRIORITY, &stm32ldac_api);

DT_INST_FOREACH_STATUS_OKAY(STM32LDAC_INIT)

