/**
 * @file    stm32f4xx_it.c
 * @brief   Minimal interrupt service routines for STM32F4 (NUCLEO-F446RE).
 *
 * This project only needs SysTick so HAL_Delay() and timeouts work.
 * As you add peripherals (EXTI, USART, TIM, DMA...), put their IRQ
 * handlers here to override the weak defaults from the startup file.
 */

#include "stm32f4xx_hal.h"

/**
 * @brief System tick interrupt handler (1 kHz).
 *
 * The vector table (startup_stm32f446xx.s) routes SysTick to this symbol.
 * HAL uses a millisecond tick for HAL_Delay() and various timeouts.
 */
void SysTick_Handler(void)
{
  HAL_IncTick();            // advance HAL's millisecond counter
  HAL_SYSTICK_IRQHandler(); // run HAL SysTick callbacks (if any)
}

/* --------------------------------------------------------------------------
 * Examples for later (uncomment when used)
 *
 * // External interrupt lines 15..10 (e.g., user button on PC13)
 * // void EXTI15_10_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13); }
 *
 * // USART2 global interrupt
 * // extern UART_HandleTypeDef huart2;
 * // void USART2_IRQHandler(void) { HAL_UART_IRQHandler(&huart2); }
 *
 * // TIM3 global interrupt
 * // extern TIM_HandleTypeDef htim3;
 * // void TIM3_IRQHandler(void) { HAL_TIM_IRQHandler(&htim3); }
 * -------------------------------------------------------------------------- */
