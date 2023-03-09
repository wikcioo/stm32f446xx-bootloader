#include "bootloader.h"
#include "stm32f446xx_crc.h"
#include <stdlib.h>

int main()
{
    init_gpio();
    init_usart2();
    init_usart3();
    init_crc();

    if (gpio_read_pin(GPIOC, GPIO_PIN_13) == GPIO_PIN_LOW)
    {
        debug_printf("BOOTLOADER_DEBUG: Executing bootloader interactive mode.\n");
        bootloader_start_interactive_mode();
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: Executing user application.\n");
        bootloader_goto_application();
    }

    return 0;
}

void bootloader_goto_application(void)
{
    debug_printf("BOOTLOADER_DEBUG: Executing bootloader_goto_application.\n");

    /* We assume that the application firmware is stored in sector 2 of the flash memory */
    uint32_t msp = *(volatile uint32_t *)FLASH_SECTOR_2_BASE_ADDR;
    debug_printf("BOOTLOADER_DEBUG: MSP value = %#X\n", msp);

    /* Set main stack pointer */
    __asm volatile("MSR MSP, %0"::"r"(msp));

    uint32_t reset_handler_addr = *(volatile uint32_t *)(FLASH_SECTOR_2_BASE_ADDR + 0x4);
    void (*application_reset_handler)(void) = (void *)reset_handler_addr;

    debug_printf("BOOTLOADER_DEBUG: Application reset handler address = %#X\n", application_reset_handler);
    application_reset_handler();
}

void bootloader_start_interactive_mode(void)
{
    uint8_t rx_buffer[BL_RX_BUFFER_SIZE];
    uint8_t rx_length = 0;

    while (1)
    {
        memset(rx_buffer, 0, BL_RX_BUFFER_SIZE);
        bootloader_receive_data(rx_buffer, 1);
        rx_length = rx_buffer[0];
        bootloader_receive_data(&rx_buffer[1], rx_length);

        switch (rx_buffer[1])
        {
        case BL_GET_VER:
            bootloader_cmd_get_version(rx_buffer);
            break;
        case BL_GET_HELP:
            bootloader_cmd_get_help(rx_buffer);
            break;
        case BL_GET_DEV_ID:
            bootloader_cmd_get_device_id(rx_buffer);
            break;
        case BL_GET_RDP_LEVEL:
            bootloader_cmd_get_rdp_level(rx_buffer);
            break;
        case BL_JMP_ADDR:
            bootloader_cmd_jump_address(rx_buffer);
            break;
        case BL_FLASH_ERASE:
            bootloader_cmd_flash_erase(rx_buffer);
            break;
        case BL_MEM_WRITE:
            bootloader_cmd_mem_write(rx_buffer);
            break;
        case BL_MEM_READ:
            bootloader_cmd_mem_read(rx_buffer);
            break;
        default:
            debug_printf("BOOTLOADER_DEBUG: Error {Unknown command}\n");
        }
    }
}

uint8_t bootloader_verify_crc(uint8_t *data, uint32_t length, uint32_t host_crc)
{
    uint32_t crc_value;

    for (uint32_t i = 0; i < length; i++) {
        uint32_t value = data[i];
        crc_value = crc_accumulate(CRC, &value, 1);
    }
    debug_printf("BOOTLOADER_DEBUG: CRC value = %#X\n", crc_value);
    /* Reset CRC afterwards so that next time it starts accumulating with no previous value. */
    CRC->CR |= 1 << CRC_CR_RESET;

    if (crc_value == host_crc) {
        return CRC_STATUS_SUCCESS;
    }

    return CRC_STATUS_FAILURE;
}

uint8_t bootloader_verify_address(uint32_t address)
{
    if (
        (address >= FLASH_BASE_ADDR && address <= FLASH_END_ADDR) ||
        (address >= SRAM1_BASE_ADDR && address <= SRAM1_END_ADDR) ||
        (address >= SRAM2_BASE_ADDR && address <= SRAM2_END_ADDR)
    ) {
        return VALID_ADDR;
    }

    return INVALID_ADDR;
}

void bootloader_cmd_get_version(uint8_t *buffer)
{
    uint8_t bl_version;
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_get_version.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        bl_version = bootloader_get_version();
        debug_printf("BOOTLOADER_DEBUG: BL_VERSION = %d (%#X)\n", bl_version, bl_version);
        bootloader_send_ack(1);
        bootloader_send_data(&bl_version, 1);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_help(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_get_help.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        uint8_t length = sizeof(supported_commands);
        bootloader_send_ack(length);
        bootloader_send_data(supported_commands, length);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_device_id(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_get_device_id.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        uint16_t dev_id = bootloader_get_device_id();
        debug_printf("BOOTLOADER_DEBUG: DEVICE_ID = %#X\n", dev_id);
        bootloader_send_ack(2);
        bootloader_send_data((uint8_t *)&dev_id, 2);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_rdp_level(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_get_rdp_level.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        uint8_t rdp_level = bootloader_get_rdp_level();
        debug_printf("BOOTLOADER_DEBUG: RDP LEVEL = %#X\n", rdp_level);
        bootloader_send_ack(1);
        bootloader_send_data(&rdp_level, 1);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_jump_address(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_jump_address.\n");

    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        bootloader_send_ack(1);

        uint32_t jump_addr = *(uint32_t *)(buffer + 2);
        debug_printf("BOOTLOADER_DEBUG: Jump address = %#X\n", jump_addr);
        if (bootloader_verify_address(jump_addr) == VALID_ADDR)
        {
            uint8_t valid_addr = VALID_ADDR;
            bootloader_send_data(&valid_addr, 1);

            debug_printf("BOOTLOADER_DEBUG: Valid. Jumping to %#X.\n", jump_addr);

            /* Ensure that the last bit in the address is set for it to be a THUMB instruction */
            jump_addr |= 1; 

            void (*jump_address)(void) = (void *)jump_addr;
            jump_address();
        }
        else
        {
            debug_printf("BOOTLOADER_DEBUG: Invalid address!\n");
            uint8_t invalid_addr = INVALID_ADDR;
            bootloader_send_data(&invalid_addr, 1);
        }
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_flash_erase(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_flash_erase.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");
        bootloader_send_ack(1);

        uint8_t status = bootloader_flash_erase(buffer[2], buffer[3]);
        bootloader_send_data(&status, 1);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_mem_write(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_mem_write.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");

        uint32_t base_address = *(uint32_t *)(buffer + 2);
        uint8_t payload_size = buffer[6];

        bootloader_send_ack(1);

        flash_init();
        flash_write(base_address, &buffer[7], payload_size);

        uint8_t status = FLASH_SUCCESS;
        bootloader_send_data(&status, 1);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_mem_read(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    debug_printf("BOOTLOADER_DEBUG: Called bootloader_cmd_mem_read.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum approved!\n");

        uint32_t base_address = *(uint32_t *)(buffer + 2);
        uint32_t length = buffer[6];

        bootloader_send_ack(length + 1);

        debug_printf("BOOTLOADER_DEBUG: Address: %#x, Length: %d.\n", base_address, length);

        uint8_t *response_buffer = (uint8_t *)malloc(length + 1);

        flash_init();
        uint8_t status = flash_read(base_address, &response_buffer[1], length);
        response_buffer[0] = status;

        debug_printf("BOOTLOADER_DEBUG: Flash read status: %s.\n", status == FLASH_SUCCESS ? "success" : "fail");
        bootloader_send_data(response_buffer, length + 1);
    }
    else
    {
        debug_printf("BOOTLOADER_DEBUG: CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_send_data(uint8_t *tx_data, uint32_t length)
{
    usart_transmit(&BL_UART, tx_data, length);
}

void bootloader_receive_data(uint8_t *rx_data, uint32_t length)
{
    usart_receive(&BL_UART, rx_data, length);
}

void bootloader_send_ack(uint8_t length_to_follow)
{
    uint8_t buffer[2];
    buffer[0] = BL_ACK;
    buffer[1] = length_to_follow;
    bootloader_send_data(buffer, 2);
}

void bootloader_send_nack(void)
{
    uint8_t nack = BL_NACK;
    bootloader_send_data(&nack, 1);
}

uint8_t bootloader_get_version(void)
{
    return (uint8_t)BL_VERSION;
}

uint8_t bootloader_get_rdp_level(void)
{
    volatile uint32_t *option_bytes = (uint32_t *)0x1FFFC000U;
    return (uint8_t)((*option_bytes >> 8) & 0xFF);
}

uint16_t bootloader_get_device_id(void)
{
    return (uint16_t)(DBGMCU->IDCODE & 0x0FFF);
}

uint8_t bootloader_flash_erase(uint8_t base_sector_number, uint8_t num_of_sectors)
{
    if (base_sector_number == 0xFF)
    {
        /* Perform mass erase */
        debug_printf("BOOTLOADER_DEBUG: Performing mass erase of flash memory.\n");
        flash_init();
        flash_mass_erase();
        return ERASE_SUCCESS;
    }

    if (
        base_sector_number < FLASH_SECTOR_0_NUMBER ||
        base_sector_number > FLASH_SECTOR_7_NUMBER ||
        num_of_sectors > FLASH_SECTOR_7_NUMBER - base_sector_number)
    {
        debug_printf("BOOTLOADER_DEBUG: Incorrect parameters for flash erase.\n");
        return ERASE_FAILURE;
    }

    debug_printf("BOOTLOADER_DEBUG: Erasing %d sectors starting from %d.\n", num_of_sectors, base_sector_number);
    flash_init();
    for (uint8_t i = base_sector_number; i < base_sector_number + num_of_sectors; i++)
    {
        flash_sector_erase(i);
        debug_printf("BOOTLOADER_DEBUG: Erased %d sector.\n", i);
    }

    return ERASE_SUCCESS;
}
