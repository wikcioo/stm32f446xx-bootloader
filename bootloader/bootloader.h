#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include <string.h>

#define BL_ENABLE_DEBUG_PRINT
#include "peripherals.h"
#include "utils.h"

/* Version 1.0 */
#define BL_VERSION 0x10

#define BL_RX_BUFFER_SIZE       1024
#define FLASH_SECTOR2_BASE_ADDR 0x08008000U

#define BL_ACK  0xBB
#define BL_NACK 0xEE

#define CRC_STATUS_SUCCESS 0
#define CRC_STATUS_FAILURE 1

#define BL_GET_VER          0xA1
#define BL_GET_HELP         0xA2
#define BL_GET_DEV_ID       0xA3
#define BL_GET_RDP_LEVEL    0xA4

uint8_t supported_commands[] = {
    BL_GET_VER, BL_GET_HELP, BL_GET_DEV_ID, BL_GET_RDP_LEVEL
};

void bootloader_cmd_get_version(uint8_t *buffer);
void bootloader_cmd_get_help(uint8_t *buffer);
void bootloader_cmd_get_device_id(uint8_t *buffer);
void bootloader_cmd_get_rdp_level(uint8_t *buffer);

void bootloader_goto_application(void);
void bootloader_start_interactive_mode(void);
void bootloader_send_data(uint8_t *tx_data, uint32_t length);
void bootloader_receive_data(uint8_t *rx_data, uint32_t length);
void bootloader_send_ack(uint8_t length_to_follow);
void bootloader_send_nack();

uint8_t bootloader_verify_crc(uint8_t *data, uint32_t length, uint32_t host_crc);
uint8_t bootloader_get_version();
uint8_t bootloader_get_rdp_level();
uint16_t bootloader_get_device_id();

#endif
