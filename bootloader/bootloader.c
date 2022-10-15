#include "bootloader.h"

int main()
{
    init_gpio();
    init_usart2();
    init_usart3();

    if (gpio_read_pin(GPIOC, GPIO_PIN_13) == GPIO_PIN_LOW)
    {
        debug_printf("BOOTLOADER_DEBUG: Executing bootloader subroutines.\n");
        bootloader_read_uart_data();
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

void bootloader_read_uart_data(void)
{
    while (1);
}
