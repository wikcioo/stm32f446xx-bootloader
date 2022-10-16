# stm32f446xx bootloader

## Supported bootloader commands
| Implemented | Host Command      | Command Code | BL Reply                 | Description                                              |
| ----------- | ----------------- | ------------ | ------------------------ | -------------------------------------------------------- |
| YES         | BL_GET_VER        | 0xA1         | Version (1 byte)         | Get the bootloader version                               |
| YES         | BL_GET_HELP       | 0xA2         | Commands (x bytes)       | Get all commands supported by the bootloader             |
| NO          | BL_GET_CID        | 0xA3         | Id (2 bytes)             | Get chip identification number                           |
| NO          | BL_GET_RDP_STATUS | 0xA4         | Level (1 byte)           | Get FLASH read protection level                          |
| NO          | BL_JMP_ADDR       | 0xA5         | Error Code (1 byte)      | Jump to specified address                                |
| NO          | BL_FLASH_ERASE    | 0xA6         | Error Code (1 byte)      | Erase sector(s) of the FLASH                             |
| NO          | BL_MEM_WRITE      | 0xA7         | Error Code (1 byte)      | Write to dirrerent memories of the MCU                   |
| NO          | BL_MEM_READ       | 0xA8         | Memory Content (x bytes) | Write to dirrerent memories of the MCU                   |
| NO          | BL_SET_RW_PROTECT | 0xA9         | Error Code (1 byte)      | Enable or disable read/write protection of FLASH sectors |
