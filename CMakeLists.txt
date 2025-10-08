cmake_minimum_required(VERSION 3.20)
project(stm32f446re_skeleton_code C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CPU & optimization flags (Cortex‑M4F)
add_compile_options(
  -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
  -Og -g3 -ffunction-sections -fdata-sections
)
add_link_options(
  -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
  -Wl,--gc-sections
)

# Paths to STM32CubeF4
set(CUBE ${CMAKE_SOURCE_DIR}/third_party/STM32CubeF4)
set(HAL  ${CUBE}/Drivers/STM32F4xx_HAL_Driver)
set(CMSIS_DEV ${CUBE}/Drivers/CMSIS/Device/ST/STM32F4xx)
set(CMSIS_CORE ${CUBE}/Drivers/CMSIS/Include)

include_directories(
  ${HAL}/Inc
  ${CMSIS_DEV}/Include
  ${CMSIS_CORE}
  ${CMAKE_SOURCE_DIR}/include
)

add_definitions(-DSTM32F446xx)

# Sources
set(SRCS
  src/main.c
  src/stm32f4xx_it.c
  system/system_stm32f4xx.c
  ${HAL}/Src/stm32f4xx_hal.c
  ${HAL}/Src/stm32f4xx_hal_rcc.c
  ${HAL}/Src/stm32f4xx_hal_gpio.c
  ${HAL}/Src/stm32f4xx_hal_cortex.c
  ${HAL}/Src/stm32f4xx_hal_flash.c
  ${HAL}/Src/stm32f4xx_hal_flash_ex.c
  ${HAL}/Src/stm32f4xx_hal_pwr.c
  ${HAL}/Src/stm32f4xx_hal_pwr_ex.c
)

add_executable(${PROJECT_NAME}.elf ${SRCS} startup/startup_stm32f446xx.s)

# Linker script (memory map)
target_link_options(${PROJECT_NAME}.elf PRIVATE -T${CMAKE_SOURCE_DIR}/linker/STM32F446RETX_FLASH.ld)

# Print the size after build
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
)