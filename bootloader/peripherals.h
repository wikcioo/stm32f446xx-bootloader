#ifndef __PERIPHERALS_H__
#define __PERIPHERALS_H__

#include "stm32f446xx.h"
#include "stm32f446xx_gpio.h"
#include "stm32f446xx_usart.h"
#include "stm32f4xx_systick.h"

/*
 * PA2 -> USART2_TX
 * PA3 -> USART2_RX
 */
usart_handle_t usart2;
#define BL_UART usart2

/*
 * PC10 -> USART3_TX
 * PC11 -> USART3_RX
 */
usart_handle_t usart3;
#define DEBUG_UART usart3

void init_gpio(void)
{
    gpio_handle_t usart2_gpio = {0};
    usart2_gpio.gpiox                  = GPIOA;
    usart2_gpio.config.pin_mode        = GPIO_MODE_ALT_FUNC;
    usart2_gpio.config.pin_alt_func    = GPIO_ALT_FUNC_7;
    usart2_gpio.config.pin_output_type = GPIO_OUTPUT_PUSH_PULL;
    usart2_gpio.config.pin_pupd        = GPIO_PULL_UP;
    usart2_gpio.config.pin_speed       = GPIO_SPEED_HIGH;

    usart2_gpio.config.pin_number      = GPIO_PIN_2;
    gpio_init(&usart2_gpio);

    usart2_gpio.config.pin_number      = GPIO_PIN_3;
    gpio_init(&usart2_gpio);

    gpio_handle_t usart3_gpio = {0};
    usart3_gpio.gpiox                  = GPIOC;
    usart3_gpio.config.pin_mode        = GPIO_MODE_ALT_FUNC;
    usart3_gpio.config.pin_alt_func    = GPIO_ALT_FUNC_7;
    usart3_gpio.config.pin_output_type = GPIO_OUTPUT_PUSH_PULL;
    usart3_gpio.config.pin_pupd        = GPIO_PULL_UP;
    usart3_gpio.config.pin_speed       = GPIO_SPEED_HIGH;

    usart3_gpio.config.pin_number      = GPIO_PIN_10;
    gpio_init(&usart3_gpio);

    usart3_gpio.config.pin_number      = GPIO_PIN_11;
    gpio_init(&usart3_gpio);

    gpio_handle_t user_button = {0};
    user_button.gpiox                  = GPIOC;
    user_button.config.pin_number      = GPIO_PIN_13;
    user_button.config.pin_mode        = GPIO_MODE_INPUT;
    user_button.config.pin_speed       = GPIO_SPEED_MEDIUM;
    user_button.config.pin_pupd        = GPIO_NO_PUPD;

    gpio_init(&user_button);
}

void init_usart2(void)
{
    usart2.usartx                 = USART2;
    usart2.config.mode            = USART_MODE_TX_RX;
    usart2.config.baudrate        = USART_BAUDRATE_115200;
    usart2.config.word_length     = USART_WORD_LENGTH_8BITS;
    usart2.config.parity          = USART_PARITY_NONE;
    usart2.config.stop_bits       = USART_STOP_BITS_1;
    usart2.config.hw_flow_control = USART_HW_FLOW_CONTROL_NONE;

    usart_init(&usart2);
}

void init_usart3(void)
{
    usart3.usartx                 = USART3;
    usart3.config.mode            = USART_MODE_TX_RX;
    usart3.config.baudrate        = USART_BAUDRATE_115200;
    usart3.config.word_length     = USART_WORD_LENGTH_8BITS;
    usart3.config.parity          = USART_PARITY_NONE;
    usart3.config.stop_bits       = USART_STOP_BITS_1;
    usart3.config.hw_flow_control = USART_HW_FLOW_CONTROL_NONE;

    usart_init(&usart3);
}

#endif
