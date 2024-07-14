# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources(stm32ldac.c wave.c)

zephyr_include_directories(
  ${ZEPHYR_E30GONG_MODULE_DIR}/app/include
)

