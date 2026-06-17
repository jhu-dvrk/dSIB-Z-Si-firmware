#include "board.h"
#include "stm32g4xx_it.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void DMA1_Channel3_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_usart1_tx); }
void DMA1_Channel4_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_usart2_tx); }
void DMA1_Channel5_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_usart3_tx); }
void DMA1_Channel6_IRQHandler(void) { HAL_DMA_IRQHandler(&hdma_lpuart1_tx); }

void USART1_IRQHandler(void) { HAL_UART_IRQHandler(&huart1); }
void USART2_IRQHandler(void) { HAL_UART_IRQHandler(&huart2); }
void USART3_IRQHandler(void) { HAL_UART_IRQHandler(&huart3); }
void LPUART1_IRQHandler(void) { HAL_UART_IRQHandler(&hlpuart1); }
void USB_LP_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_USB_FS); }
