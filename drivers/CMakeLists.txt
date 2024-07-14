# Copyright (c) 2021 
# SPDX-License-Identifier: Apache-2.0

add_subdirectory_ifdef(CONFIG_STM32LDAC dac)
add_subdirectory_ifdef(CONFIG_STM32LPM pm)
