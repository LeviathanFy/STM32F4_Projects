# Simple Bootloader

This project implements a simple bootloader to facilitate learning about the fundamental workings of bootloaders.

## Features:
- 32Kb FLASH is used for bootloader. Rest of the FLASH belongs to the main application. (see FLASH.ld file)
- Utilizes the UART peripheral along with a RingBuffer to capture user-defined KEYWORDS, enabling a jump to the main application.
- The green LED blinks when the system is in bootloader mode.
- The red LED indicates a successful jump to the main application.

## Usage:
Users can directly jump to the main application code by pressing "x" on the serial terminal.
