# BMW E30 gong software for Zephyr RTOS and STM32

This repository contains software for the BMW E30 that plays WAV files.

The main microcontroller for the gong is the STM32L452RET6. 

The microprocessor sleeps most of the time consuming only 5 μA. When the level changes on one of its interrupt lines, 
a WAKEUP interrupt awakens the microprocessor. The program detects which interrupt line awakened the system 
and plays a corresponding WAV file on its DAC. When it stops playing, the microprocessor goes back into sleeping.

WAV files are only a few seconds each, and they are stored in the microprocessor flash.

The DAC signal is amplified by the amplifier chip NJM2135M (MC34119).

The software uses the Real-Time Operating System Zephyr to organize multiple threads and simplify
writing drivers.

## Getting Started

Before getting started, make sure you have a proper Zephyr development
environment. You can follow the official
[Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html).

### Documentation

[BMW E30 gong for Zephyr RTOS and STM32](http://hobby.farit.ru/bmw-e30-gong/).

### Initialization

The first step is to initialize the workspace folder (``my-workspace``) where
the ``e30gong`` and all Zephyr modules will be cloned. You can do
that by running:

```shell
# initialize my-workspace for the e30clock application (main branch)
west init -m https://github.com/faritka/e30gong --mr main my-workspace
# update Zephyr modules
cd my-workspace
west update
```

### Configuration

The application is based on a custom board that fits inside of the existing gong module.

### Build & Run

The application can be built by running:

```shell
west build -b $BOARD -s app
```

where `$BOARD` is the target board.

Once you have built the application you can flash it by running:

```shell
west flash
```
