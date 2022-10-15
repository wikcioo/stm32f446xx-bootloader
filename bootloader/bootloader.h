#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#define BL_ENABLE_DEBUG_PRINT
#include "peripherals.h"
#include "utils.h"
#include <string.h>

#define FLASH_SECTOR2_BASE_ADDR 0x08008000U

void bootloader_goto_application(void);
void bootloader_read_uart_data(void);

#endif
