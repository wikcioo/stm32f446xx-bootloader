#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include <string.h>
#include "stm32f446xx_flash.h"

#define BL_ENABLE_DEBUG_PRINT
#include "peripherals.h"
#include "utils.h"

/* Version 1.0 */
#define BL_VERSION          0x10
#define BL_RX_BUFFER_SIZE   1024

/* STM32F466xx memory addresses */
#define SRAM1_SIZE      (112 * 1024)                      // 112KB of SRAM1
#define SRAM1_END_ADDR  (SRAM1_BASE_ADDR + SRAM1_SIZE)
#define SRAM2_SIZE      (16 * 1024)                       // 16KB of SRAM2
#define SRAM2_END_ADDR  (SRAM2_BASE_ADDR + SRAM2_SIZE)

#define BL_ACK              0xBB
#define BL_NACK             0xEE
#define VALID_ADDR          0
#define INVALID_ADDR        1
#define ERASE_SUCCESS       0
#define ERASE_FAILURE       1
#define CRC_STATUS_SUCCESS  0
#define CRC_STATUS_FAILURE  1

#define BL_GET_VER          0xA1
#define BL_GET_HELP         0xA2
#define BL_GET_DEV_ID       0xA3
#define BL_GET_RDP_LEVEL    0xA4
#define BL_JMP_ADDR         0xA5
#define BL_FLASH_ERASE      0xA6
#define BL_MEM_WRITE        0xA7
#define BL_MEM_READ         0xA8

uint8_t supported_commands[] = {
    BL_GET_VER, BL_GET_HELP, BL_GET_DEV_ID, BL_GET_RDP_LEVEL, BL_JMP_ADDR, BL_FLASH_ERASE, BL_MEM_WRITE, BL_MEM_READ
};

void bootloader_cmd_get_version(uint8_t *buffer);
void bootloader_cmd_get_help(uint8_t *buffer);
void bootloader_cmd_get_device_id(uint8_t *buffer);
void bootloader_cmd_get_rdp_level(uint8_t *buffer);
void bootloader_cmd_jump_address(uint8_t *buffer);
void bootloader_cmd_flash_erase(uint8_t *buffer);
void bootloader_cmd_mem_write(uint8_t *buffer);
void bootloader_cmd_mem_read(uint8_t *buffer);

void bootloader_goto_application(void);
void bootloader_start_interactive_mode(void);
void bootloader_send_data(uint8_t *tx_data, uint32_t length);
void bootloader_receive_data(uint8_t *rx_data, uint32_t length);
void bootloader_send_ack(uint8_t length_to_follow);
void bootloader_send_nack(void);

uint8_t bootloader_verify_crc(uint8_t *data, uint32_t length, uint32_t host_crc);
uint8_t bootloader_verify_address(uint32_t address);
uint8_t bootloader_get_version(void);
uint8_t bootloader_get_rdp_level(void);
uint16_t bootloader_get_device_id(void);
uint8_t bootloader_flash_erase(uint8_t base_sector_number, uint8_t num_of_sectors);

#endif
