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
        BL_LOG("Executing bootloader interactive mode.\n");
        bootloader_start_interactive_mode();
    }
    else
    {
        BL_LOG("Executing user application.\n");
        bootloader_goto_application();
    }

    return 0;
}

void bootloader_goto_application(void)
{
    BL_LOG("Executing bootloader_goto_application.\n");

    uint32_t reset_handler_addr = *(volatile uint32_t *)(FLASH_SECTOR_2_BASE_ADDR + 0x4);
    void (*application_reset_handler)(void) = (void (*)(void))reset_handler_addr;

    BL_LOG("Application reset handler address = 0x%08lX\n", (unsigned long)application_reset_handler);

    /* We assume that the application firmware is stored in sector 2 of the flash memory */
    uint32_t msp = *(volatile uint32_t *)FLASH_SECTOR_2_BASE_ADDR;
    BL_LOG("MSP value = 0x%08lX\n", msp);

    /* Set main stack pointer */
    __asm volatile("MSR MSP, %0"::"r"(msp));

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
        case BL_SET_RW_PROTECT:
            bootloader_cmd_set_rw_protect(rx_buffer);
            break;
        case BL_GET_RW_PROTECT:
            bootloader_cmd_get_rw_protect(rx_buffer);
            break;
        default:
            BL_LOG("Error {Unknown command}\n");
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
    BL_LOG("CRC value = 0x%08lX\n", crc_value);
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

    BL_LOG("Called bootloader_cmd_get_version.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        BL_LOG("CRC checksum approved!\n");
        bl_version = bootloader_get_version();
        BL_LOG("BL_VERSION = %d (%#02X)\n", bl_version, bl_version);
        bootloader_send_ack(1);
        bootloader_send_data(&bl_version, 1);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_help(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_get_help.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        BL_LOG("CRC checksum approved!\n");
        uint8_t length = sizeof(supported_commands);
        bootloader_send_ack(length);
        bootloader_send_data(supported_commands, length);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_device_id(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_get_device_id.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        BL_LOG("CRC checksum approved!\n");
        uint16_t dev_id = bootloader_get_device_id();
        BL_LOG("DEVICE_ID = %#04X\n", dev_id);
        bootloader_send_ack(2);
        bootloader_send_data((uint8_t *)&dev_id, 2);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_rdp_level(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_get_rdp_level.\n");
    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        BL_LOG("CRC checksum approved!\n");
        uint8_t rdp_level = bootloader_get_rdp_level();
        BL_LOG("RDP LEVEL = %#02X\n", rdp_level);
        bootloader_send_ack(1);
        bootloader_send_data(&rdp_level, 1);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_jump_address(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_jump_address.\n");

    if (!bootloader_verify_crc(buffer, packet_length - 4, host_crc))
    {
        BL_LOG("CRC checksum approved!\n");
        bootloader_send_ack(1);

        uint32_t jump_addr = *(uint32_t *)(buffer + 2);
        BL_LOG("Jump address = 0x%08lX\n", jump_addr);
        if (bootloader_verify_address(jump_addr) == VALID_ADDR)
        {
            uint8_t valid_addr = VALID_ADDR;
            bootloader_send_data(&valid_addr, 1);

            BL_LOG("Valid. Jumping to 0x%08lX.\n", jump_addr);

            /* Ensure that the last bit in the address is set for it to be a THUMB instruction */
            jump_addr |= 1; 

            void (*jump_address)(void) = (void (*)(void))jump_addr;
            jump_address();
        }
        else
        {
            BL_LOG("Invalid address!\n");
            uint8_t invalid_addr = INVALID_ADDR;
            bootloader_send_data(&invalid_addr, 1);
        }
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_flash_erase(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_flash_erase.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        BL_LOG("CRC checksum approved!\n");
        bootloader_send_ack(1);

        uint8_t status = bootloader_flash_erase(buffer[2], buffer[3]);
        bootloader_send_data(&status, 1);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_mem_write(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_mem_write.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        BL_LOG("CRC checksum approved!\n");

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
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_mem_read(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_mem_read.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        BL_LOG("CRC checksum approved!\n");

        uint32_t base_address = *(uint32_t *)(buffer + 2);
        uint32_t length = buffer[6];

        bootloader_send_ack(length + 1);

        BL_LOG("Address: 0x%08lX, Length: %lu.\n", base_address, length);

        uint8_t *response_buffer = (uint8_t *)malloc(length + 1);

        flash_init();
        uint8_t status = flash_read(base_address, &response_buffer[1], length);
        response_buffer[0] = status;

        BL_LOG("Flash read status: %s.\n", status == FLASH_SUCCESS ? "success" : "fail");
        bootloader_send_data(response_buffer, length + 1);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_set_rw_protect(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_set_rw_protect.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        BL_LOG("CRC checksum approved!\n");

        bootloader_send_ack(1);

        uint8_t sectors = buffer[2];
        uint8_t prot_level = buffer[3];

        char bin_sectors[sizeof(uint8_t) * 8 + 1];

        for (uint8_t i = 0; i < sizeof(uint8_t) * 8; i++)
        {
            bin_sectors[i] = sectors >> ((sizeof(uint8_t) * 8) - i - 1) & 1 ? '1' : '0';
        }

        bin_sectors[sizeof(uint8_t) * 8] = '\0';

        BL_LOG("Sectors: 0b%s, Protection Level: %02u\n", bin_sectors, prot_level);

        flash_init();
        flash_set_protection_level(prot_level, sectors);

        uint8_t status = FLASH_SUCCESS;
        bootloader_send_data(&status, 1);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
        bootloader_send_nack();
    }
}

void bootloader_cmd_get_rw_protect(uint8_t *buffer)
{
    uint32_t packet_length = buffer[0] + 1;
    uint32_t host_crc = *(uint32_t *)(buffer + packet_length - 4);

    BL_LOG("Called bootloader_cmd_get_rw_protect.\n");

    if (bootloader_verify_crc(buffer, packet_length - 4, host_crc) == CRC_STATUS_SUCCESS)
    {
        BL_LOG("CRC checksum approved!\n");

        bootloader_send_ack(8);

        uint8_t prot_level[8] = {0};

        flash_init();
        flash_get_protection_level(prot_level);

        bootloader_send_data(prot_level, 8);
    }
    else
    {
        BL_LOG("CRC checksum failed!\n");
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
        BL_LOG("Performing mass erase of flash memory.\n");
        flash_init();
        flash_mass_erase();
        return ERASE_SUCCESS;
    }

    if (
        base_sector_number < FLASH_SECTOR_0_NUMBER ||
        base_sector_number > FLASH_SECTOR_7_NUMBER ||
        num_of_sectors > FLASH_SECTOR_7_NUMBER - base_sector_number)
    {
        BL_LOG("Incorrect parameters for flash erase.\n");
        return ERASE_FAILURE;
    }

    BL_LOG("Erasing %d sectors starting from %d.\n", num_of_sectors, base_sector_number);
    flash_init();
    for (uint8_t i = base_sector_number; i < base_sector_number + num_of_sectors; i++)
    {
        flash_sector_erase(i);
        BL_LOG("Erased %d sector.\n", i);
    }

    return ERASE_SUCCESS;
}
