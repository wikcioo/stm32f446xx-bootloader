FW_NAME = bootloader
FIRMWARE = $(FW_NAME).elf
CC = arm-none-eabi-gcc
MACH = cortex-m4
CORE_DRIVERS_DIR = drivers/stm32f446xx/drivers
BUILD_DIR = build
BOOTLOADER_DIR = bootloader
SOURCES  = $(wildcard $(CORE_DRIVERS_DIR)/src/*.c)
SOURCES += $(wildcard ./*.c)
SOURCES += $(wildcard $(BOOTLOADER_DIR)/*.c)
OBJECTS  = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
CFLAGS = -c -mcpu=$(MACH) -mthumb -mfloat-abi=soft -std=gnu11 -g -Wall -Wformat -Wpedantic -Wshadow -O0 -I$(CORE_DRIVERS_DIR)/inc
LDFLAGS = -mcpu=$(MACH) -mthumb -mfloat-abi=soft --specs=nano.specs -Tstm32f446xx_flash_ram.ld -Wl,-Map=$(BUILD_DIR)/$(FW_NAME).map

.PHONY = all clean

all:
	@mkdir -p $(BUILD_DIR)
	@make --no-print-directory $(BUILD_DIR)/$(FIRMWARE)
	@arm-none-eabi-objcopy -O binary $(BUILD_DIR)/$(FIRMWARE) $(BUILD_DIR)/$(FW_NAME).bin

$(BUILD_DIR)/$(FIRMWARE): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(CORE_DRIVERS_DIR)/src/%.c
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(BOOTLOADER_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)
