# dSIB-Z-Si STM32 Firmware

Bare STM32 HAL firmware for STM32G474CBT6 and STM32G474CET6.

## Build

```sh
cmake -S . -B build-firmware \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build-firmware -j
```

Outputs:

- `build-firmware/dsib_z_si_firmware.elf`
- `build-firmware/dsib_z_si_firmware.bin`
- `build-firmware/dsib_z_si_firmware.hex`

Flash with ST-LinkV2 using `st-flash`:

```sh
st-flash --reset --flash=128k write build-firmware/dsib_z_si_firmware.bin 0x08000000
```

The CMake `flash` target uses OpenOCD and is also available:

```sh
cmake --build build-firmware --target flash
```

## Pin Map

ADC inputs are sampled with 12-bit resolution and 256x hardware oversampling:

- USART1 `PA9/PA10`, ECM ID `0x04`: `PB0` pot1, `PB1` pot2
- USART2 `PB3/PA15`, PSM1 ID `0x06`: `PA6` pot1, `PA7` pot2
- USART3 `PB10/PB11`, PSM2 ID `0x08`: `PA4` pot1, `PA5` pot2
- LPUART1 `PA2/PA3`, PSM3 ID `0x0a`: `PA0` pot1, `PA1` pot2

Each UART streams 12-byte packets at `115200 8N1`:

```text
'd' 'S' 'I' 'B' 'Z' id pot1_le16 pot2_le16 crc_le16
```

USB CDC is a diagnostic console and is not part of the data stream.
