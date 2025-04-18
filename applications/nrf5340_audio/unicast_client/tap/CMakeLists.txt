#
# Copyright (c) 2023 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

target_sources(app PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/tap_channel.c
			${CMAKE_CURRENT_SOURCE_DIR}/tap_uart.c
			${CMAKE_CURRENT_SOURCE_DIR}/tap_cmd_handlers.c)
