# Configurations
CONFIG_PATH=~/.config/vega-tools/settings.mk
include $(CONFIG_PATH)

SDK_PATH=${VEGA_SDK}
EXECUTABLE_NAME=uart2_verify

# Add -u _printf_float to the LDFLAGS via USER_LDFLAGS
# This is required for printing PID decimals correctly
USER_LDFLAGS = -u _printf_float -lm

# Include the source files in current directory
SOURCES = main.c ./debug_uart.c

# Pull in the BSP settings
include $(SDK_PATH)/bsp/common/config.mk
