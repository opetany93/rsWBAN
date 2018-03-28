PROJECT_NAME     := rsWBAN-host
TARGETS          := nrf52832_xxaa
OUTPUT_DIRECTORY := _build

#PLATFORM = NRF52_SENSOR
PLATFORM = NRF52_HOST

# Common C MACROS
CFLAGS += -DNRF52
CFLAGS += -DNRF52832_XXAA
CFLAGS += -DNRF52_PAN_74

SDK_ROOT:= D:/nRF52/gnu_gcc_environment/SDKs/nRF5_SDK_14.1.0
PROJ_DIR := D:/nRF52/rsWBAN_Project/host
MYLIB_DIR := D:/nRF52/rsWBAN_Project/myLib

$(OUTPUT_DIRECTORY)/nrf52832_xxaa.out: \
  LINKER_SCRIPT  := gcc_nrf52.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/components/toolchain/gcc/gcc_startup_nrf52.S \
  $(SDK_ROOT)/components/toolchain/system_nrf52.c \
  $(wildcard $(PROJ_DIR)/source/*.c) 

ifeq ($(PLATFORM), NRF52_HOST)

	CFLAGS += -DNRF52_HOST
	CFLAGS += -DBOARD_PCA10040
	CFLAGS += -DBSP_DEFINES_ONLY
	CFLAGS += -DCONFIG_GPIO_AS_PINRESET
	
	SRC_FILES += \
		$(wildcard $(MYLIB_DIR)/source/*.c) \
    	$(SDK_ROOT)/components/boards/boards.c
    	
else ifeq ($(PLATFORM), NRF52_SENSOR)

	CFLAGS += -DNRF52_SENSOR
	
	SRC_FILES += \
    	$(MYLIB_DIR)/source/clocks.c \
  		$(MYLIB_DIR)/source/hal.c \
  		$(MYLIB_DIR)/source/radio.c \
  		$(MYLIB_DIR)/source/spi.c \
  		$(MYLIB_DIR)/drivers/src/ADXL362.c
  		
endif
  
# Include folders common to all targets
INC_FOLDERS += \
  $(PROJ_DIR)/include \
  $(MYLIB_DIR)/headers\
  $(MYLIB_DIR)/drivers/inc\
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/components/toolchain \
  $(SDK_ROOT)/components/libraries/util \
  $(SDK_ROOT)/components/toolchain/gcc \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
  $(SDK_ROOT)/config \
  $(SDK_ROOT)/components/device \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/drivers_nrf/delay \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/drivers_nrf/hal \

# Libraries common to all targets
LIB_FILES += \

# Optimization flags
OPT = -O0 -g3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS +=  -Wall
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums 

# C++ flags common to all targets
CXXFLAGS += $(OPT)

# Assembler flags common to all targets
ASMFLAGS += -g3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DBOARD_PCA10040
ASMFLAGS += -DBSP_DEFINES_ONLY
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DNRF52
ASMFLAGS += -DNRF52832_XXAA
ASMFLAGS += -DNRF52_PAN_74

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L $(TEMPLATE_PATH) -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -u _printf_float
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
# use newlib in nano version
LDFLAGS += --specs=nano.specs


# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm


.PHONY: default help

# Default target - first one defined
default: nrf52832_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		nrf52832_xxaa
	@echo		sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash erase

# Flash the program
flash: $(OUTPUT_DIRECTORY)/nrf52832_xxaa.hex
	@echo Flashing: $<
	nrfjprog -f nrf52 --program $< --sectorerase
	nrfjprog -f nrf52 --reset

erase:
	nrfjprog -f nrf52 --eraseall

SDK_CONFIG_FILE := $(SDK_ROOT)/config/sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)
