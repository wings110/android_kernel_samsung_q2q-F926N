#
# Makefile for the Ilitek common touchscreen driver.
#

TARGET = ilitek_ts_common

$(TARGET)-objs	:= ilitek_common_spi.o ilitek_spi.o ilitek_init.o ilitek_ic.o ilitek_touch.o ilitek_mp.o ilitek_hostdl.o ilitek_sec_fn.o ilitek_sec_fn_test.o
ifeq ($(filter y m, $(CONFIG_SAMSUNG_PRODUCT_SHIP)),)
ifneq ($(filter y m, $(CONFIG_QGKI)),)
$(TARGET)-objs += ilitek_node.o
endif
endif

ccflags-$(CONFIG_SAMSUNG_TUI)	+= -Idrivers/misc/tui

obj-$(CONFIG_TOUCHSCREEN_ILITEK_COMMON_SPI) += $(TARGET).o

ccflags-y += -Wformat
