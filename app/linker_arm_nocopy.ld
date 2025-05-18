/*
 * Copyright (c) 2022 Carlo Caione <ccaione@baylibre.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Linker command/script file
 *
 * Linker script for the Cortex-M platforms.
 */

#include <zephyr/linker/sections.h>
#include <zephyr/devicetree.h>

#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

#include <zephyr/arch/arm/cortex_m/scripts/linker.ld>

SECTIONS
{
  .audioData 0x8010000 : 
  {
    . = ALIGN(4);
    KEEP(*(.audioData))
    . = ALIGN(4);  	
  } > FLASH

}
