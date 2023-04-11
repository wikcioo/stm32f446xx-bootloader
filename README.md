# STM32F446xx bootloader

## How it works?
The bootloader code is stored in the first 2 sectors of the FLASH memory. It assumes that the user application is located starting from the 3rd sector. If the user button is pressed when the board undergoes reset, it will activate the interactive mode which allows it to communicate over UART peripheral with the host application running on your desktop PC such as [this](https://github.com/wikcioo/stm32-flash-programmer-cli) one. If the user button is NOT pressed when the board undergoes reset, the bootloader will simply transfer execution to the user program located at the beginning of the 3rd sector of the FLASH memory. The bootloader outputs debug messages over UART peripheral which you can read by connecting a USB to serial TTL level converter cable to pins PC10 and PC11. To read data from a serial port, you can use a program such as minicom. Remember to set the baud rate to 115200.

## How to flash the bootloader onto the target board?
**Note**: This project has only been tested on Linux

### Install arm-none-eabi-gcc cross-compiler and st-link software
On Debian based distributions:
```sh
sudo apt install gcc-arm-none-eabi libusb-1.0-0-dev
```
On Arch based distributions:
```sh
sudo pacman -S arm-none-eabi-gcc stlink
```

### Clone the repository
```sh
git clone --recursive https://github.com/wikcioo/stm32f446xx-bootloader.git
cd stm32f446xx-bootloader
```

### Cross-compile using Make
```sh
make
```

### Flash the firmware using stlink
```sh
st-flash --reset write build/bootloader.bin 0x08000000
```

## Supported bootloader commands:
| Command           | Code | Reply                      | Description                                   |
| ----------------- | ---- | -------------------------- | --------------------------------------------- |
| BL_GET_VER        | 0xA1 | Version (1 byte)           | Get the bootloader version                    |
| BL_GET_HELP       | 0xA2 | Commands (x bytes)         | Get all commands supported by the bootloader  |
| BL_GET_DEV_ID     | 0xA3 | Id (2 bytes)               | Get device identification number              |
| BL_GET_RDP_LEVEL  | 0xA4 | Level (1 byte)             | Get FLASH read protection level               |
| BL_JMP_ADDR       | 0xA5 | Error Code (1 byte)        | Jump to specified address                     |
| BL_FLASH_ERASE    | 0xA6 | Error Code (1 byte)        | Erase sector(s) of the FLASH                  |
| BL_MEM_WRITE      | 0xA7 | Error Code (1 byte)        | Write to FLASH memory of the MCU              |
| BL_MEM_READ       | 0xA8 | Memory Content (x bytes)   | Read from FLASH memory of the MCU             |
| BL_SET_RW_PROTECT | 0xA9 | Error Code (1 byte)        | Enable read/write protection of FLASH sectors |
| BL_GET_RW_PROTECT | 0xAA | Protection Codes (8 bytes) | Get read/write protection of FLASH sectors    |
