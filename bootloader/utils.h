#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stm32f446xx_usart.h"

#define DEBUG_BUFFER_SIZE 256

#ifdef BL_ENABLE_DEBUG_PRINT
    #ifndef DEBUG_UART
        #error "DEBUG_UART not defined!"
    #else
        #define BL_LOG(format, ...) \
            do { \
                char temp_bl_buf[DEBUG_BUFFER_SIZE] = {0}; \
                snprintf(temp_bl_buf, DEBUG_BUFFER_SIZE, "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__); \
                usart_transmit(&DEBUG_UART, (uint8_t *)temp_bl_buf, strlen(temp_bl_buf)); \
            } while(0)
    #endif // DEBUG_UART
#else
    #define BL_LOG(format, ...)
#endif // BL_ENABLE_DEBUG_PRINT

#endif //__UTILS_H__
