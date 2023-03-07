# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#
# This CMake file is picked by the Zephyr build system because it is defined
# as the module CMake entry point (see zephyr/module.yml).

# Build for the M5StickC Plus board by default
set(BOARD m5stickc_plus)

zephyr_include_directories(include)
