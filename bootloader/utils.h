#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32f446xx_usart.h"
#define DEBUG_BUFFER_SIZE 256

void debug_printf(char *format, ...)
{
#ifdef BL_ENABLE_DEBUG_PRINT
    #ifndef DEBUG_UART
        #error "DEBUG_UART not defined!"
    #endif
    char buffer[DEBUG_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    usart_transmit(&DEBUG_UART, (uint8_t *)buffer, strlen(buffer));
    va_end(args);
#endif
}

#endif
