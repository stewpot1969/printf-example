# Selecting Core
CORTEX_M=3

# Use newlib-nano. To disable it, specify USE_NANO=
USE_NANO=--specs=nano.specs

# Use seimhosting or not
USE_SEMIHOST=--specs=rdimon.specs
USE_NOHOST=--specs=nosys.specs

CORE=CM$(CORTEX_M)


# Compiler & Linker
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++

# Options for specific architecture
ARCH_FLAGS=-mthumb -mcpu=cortex-m$(CORTEX_M)

# Startup code
STARTUP=startup_ARM$(CORE).S

#OPENCM3_PATH       = ../libopencm3
OPENCM3_PATH    = /usr/local/arm-none-eabi

# include/ sub-directory
OPENCM3_INC_PATH   = $(OPENCM3_PATH)/include
# lib/ sub-directory
OPENCM3_LIB_PATH   = $(OPENCM3_PATH)/lib

CURDIR := $(shell pwd)

#Processor family for libopencm3
CM3_FAM=STM32F1

# -Os -flto -ffunction-sections -fdata-sections to compile for code size
CFLAGS=$(ARCH_FLAGS) $(STARTUP_DEFS) -Os -flto -ffunction-sections -fdata-sections \
-I$(OPENCM3_INC_PATH)\
-L$(OPENCM3_LIB_PATH)\
-lopencm3_stm32f1\
-D$(CM3_FAM) -g

CXXFLAGS=$(CFLAGS)

# Link for code size
GC=-Wl,--gc-sections

# Create map file
MAP=-Wl,-Map=$(NAME).map
NAME=interrupt
STARTUP_DEFS=-D__STARTUP_CLEAR_BSS -D__START=main

LDSCRIPTS=-L. -T nokeep.ld
LFLAGS=$(USE_NANO) $(USE_NOHOST) $(LDSCRIPTS) $(GC) $(MAP)

$(NAME)-$(CORE).axf: $(NAME).c $(STARTUP)
	$(CC) $^ $(CFLAGS) $(LFLAGS) -o $@
	arm-none-eabi-objcopy -O ihex $(NAME)-$(CORE).axf $(NAME)-$(CORE).hex
	arm-none-eabi-objcopy -O binary $(NAME)-$(CORE).axf $(NAME)-$(CORE).bin

clean: 
	rm -f $(NAME)*.axf *.map *.o *.hex *.bin

test:
	echo $(CURDIR)
	echo $(@D)
	echo $(@F)

prog:  clean $(NAME)-$(CORE).axf
	arm-none-eabi-gdb \
	--eval-command="target remote localhost:3333" \
	--eval-command="monitor reset halt" \
	--eval-command="monitor stm32f1x mass_erase 0" \
	--eval-command="monitor flash write_bank 0 $(CURDIR)/$(NAME)-$(CORE).bin 0" \
	--eval-command="monitor reset" \
	--batch

