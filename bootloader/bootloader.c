#include "bootloader.h"
#include "stm32f446xx_crc.h"

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
    void (*application_reset_handler)(void);

    debug_printf("BOOTLOADER_DEBUG: Executing bootloader_goto_application.\n");

    /* We assume that the application firmware is stored in sector 2 of the flash memory */
    uint32_t msp = *(volatile uint32_t *)FLASH_SECTOR2_BASE_ADDR;
    debug_printf("BOOTLOADER_DEBUG: MSP value = %#X\n", msp);

    /* Set main stack pointer */
    __asm volatile("MSR MSP, %0"::"r"(msp));

    uint32_t reset_handler_addr = *(volatile uint32_t *)(FLASH_SECTOR2_BASE_ADDR + 0x4);
    application_reset_handler = (void *)reset_handler_addr;
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

void bootloader_send_nack()
{
    uint8_t nack = BL_NACK;
    bootloader_send_data(&nack, 1);
}

uint8_t bootloader_get_version()
{
    return (uint8_t)BL_VERSION;
}
