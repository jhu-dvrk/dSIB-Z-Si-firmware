cmake_minimum_required(VERSION 3.16)

project(dsib_z_si_firmware LANGUAGES C ASM)

set(STM32_CHIP "STM32G474CBTx" CACHE STRING "")

include(FetchContent)

FetchContent_Declare(STM32CubeG4
  GIT_REPOSITORY https://github.com/STMicroelectronics/STM32CubeG4.git
  GIT_TAG master
)
FetchContent_MakeAvailable(STM32CubeG4)

set(CUBE_G4        ${stm32cubeg4_SOURCE_DIR})
set(HAL_DIR        ${CUBE_G4}/Drivers/STM32G4xx_HAL_Driver)
set(CMSIS_DEV_DIR  ${CUBE_G4}/Drivers/CMSIS/Device/ST/STM32G4xx)
set(CMSIS_CORE_DIR ${CUBE_G4}/Drivers/CMSIS/Include)

add_executable(${PROJECT_NAME}.elf
  src/main.c
  src/board_init.c
  src/stm32g4xx_it.c
  src/stm32g4xx_hal_msp.c
  src/syscalls.c
  src/usb_device.c
  src/usbd_conf.c
  src/usbd_desc.c
  src/usbd_cdc_if.c

  ${CMSIS_DEV_DIR}/Source/Templates/system_stm32g4xx.c
  ${CMSIS_DEV_DIR}/Source/Templates/gcc/startup_stm32g474xx.s

  ${HAL_DIR}/Src/stm32g4xx_hal.c
  ${HAL_DIR}/Src/stm32g4xx_hal_adc.c
  ${HAL_DIR}/Src/stm32g4xx_hal_adc_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_cortex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_dma.c
  ${HAL_DIR}/Src/stm32g4xx_hal_dma_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_exti.c
  ${HAL_DIR}/Src/stm32g4xx_hal_flash.c
  ${HAL_DIR}/Src/stm32g4xx_hal_flash_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_gpio.c
  ${HAL_DIR}/Src/stm32g4xx_hal_pcd.c
  ${HAL_DIR}/Src/stm32g4xx_hal_pcd_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_pwr.c
  ${HAL_DIR}/Src/stm32g4xx_hal_pwr_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_rcc.c
  ${HAL_DIR}/Src/stm32g4xx_hal_rcc_ex.c
  ${HAL_DIR}/Src/stm32g4xx_hal_uart.c
  ${HAL_DIR}/Src/stm32g4xx_hal_uart_ex.c
  ${HAL_DIR}/Src/stm32g4xx_ll_usb.c

  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
  inc
  config
  ${HAL_DIR}/Inc
  ${CMSIS_DEV_DIR}/Include
  ${CMSIS_CORE_DIR}
  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
  ${CUBE_G4}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
)

target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
  USE_HAL_DRIVER
  STM32G474xx
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
  -ffunction-sections -fdata-sections
  -fno-builtin
  -O2 -g3
  -Wall -Wextra -Wno-unused-parameter
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=hard
  -nostartfiles
  -T${CMAKE_SOURCE_DIR}/ld/STM32G474CB_FLASH.ld
  -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
  -Wl,--gc-sections
  -static
  --specs=nano.specs
  --specs=nosys.specs
)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
  COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
  BYPRODUCTS ${PROJECT_NAME}.bin ${PROJECT_NAME}.hex
)

add_custom_target(flash
  COMMAND openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf verify reset exit"
  DEPENDS ${PROJECT_NAME}.elf
  USES_TERMINAL
)

set(DFU_BINARY_PATH "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin" CACHE STRING "Path to binary used for DFU flashing")
add_custom_target(dfu
  COMMAND dfu-util -a 0 -D ${DFU_BINARY_PATH} -s 0x08000000
  DEPENDS ${PROJECT_NAME}.elf
  USES_TERMINAL
)

message(STATUS "Using STM32CubeG4 from: ${CUBE_G4}")
