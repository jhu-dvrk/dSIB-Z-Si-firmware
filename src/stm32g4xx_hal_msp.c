#include "board.h"
#include "main.h"

#ifndef GPIO_AF10_USB
#define GPIO_AF10_USB ((uint8_t)0x0AU)
#endif

extern PCD_HandleTypeDef hpcd_USB_FS;

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

static void DMA_Config(DMA_HandleTypeDef *hdma,
                       DMA_Channel_TypeDef *channel,
                       uint32_t request,
                       uint32_t direction,
                       uint32_t periph_align,
                       uint32_t mem_align,
                       uint32_t mode,
                       uint32_t priority)
{
  hdma->Instance = channel;
  hdma->Init.Request = request;
  hdma->Init.Direction = direction;
  hdma->Init.PeriphInc = DMA_PINC_DISABLE;
  hdma->Init.MemInc = DMA_MINC_ENABLE;
  hdma->Init.PeriphDataAlignment = periph_align;
  hdma->Init.MemDataAlignment = mem_align;
  hdma->Init.Mode = mode;
  hdma->Init.Priority = priority;
  if (HAL_DMA_Init(hdma) != HAL_OK) {
    Error_Handler();
  }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) {
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);
    DMA_Config(&hdma_adc1, DMA1_Channel1, DMA_REQUEST_ADC1,
               DMA_PERIPH_TO_MEMORY, DMA_PDATAALIGN_HALFWORD,
               DMA_MDATAALIGN_HALFWORD, DMA_CIRCULAR, DMA_PRIORITY_HIGH);
  } else if (hadc->Instance == ADC2) {
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc2);
    DMA_Config(&hdma_adc2, DMA1_Channel2, DMA_REQUEST_ADC2,
               DMA_PERIPH_TO_MEMORY, DMA_PDATAALIGN_HALFWORD,
               DMA_MDATAALIGN_HALFWORD, DMA_CIRCULAR, DMA_PRIORITY_HIGH);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) {
    HAL_DMA_DeInit(&hdma_adc1);
  } else if (hadc->Instance == ADC2) {
    HAL_DMA_DeInit(&hdma_adc2);
  }
}

static void UART_Pin_Config(GPIO_TypeDef *port, uint32_t pins, uint32_t alternate)
{
  GPIO_InitTypeDef g = {0};
  g.Pin = pins;
  g.Mode = GPIO_MODE_AF_PP;
  g.Pull = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  g.Alternate = alternate;
  HAL_GPIO_Init(port, &g);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    UART_Pin_Config(GPIOA, GPIO_PIN_9 | GPIO_PIN_10, GPIO_AF7_USART1);
    __HAL_LINKDMA(huart, hdmatx, hdma_usart1_tx);
    DMA_Config(&hdma_usart1_tx, DMA1_Channel3, DMA_REQUEST_USART1_TX,
               DMA_MEMORY_TO_PERIPH, DMA_PDATAALIGN_BYTE,
               DMA_MDATAALIGN_BYTE, DMA_NORMAL, DMA_PRIORITY_HIGH);
    HAL_NVIC_SetPriority(USART1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  } else if (huart->Instance == USART2) {
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    UART_Pin_Config(GPIOB, GPIO_PIN_3, GPIO_AF7_USART2);
    UART_Pin_Config(GPIOA, GPIO_PIN_15, GPIO_AF7_USART2);
    __HAL_LINKDMA(huart, hdmatx, hdma_usart2_tx);
    DMA_Config(&hdma_usart2_tx, DMA1_Channel4, DMA_REQUEST_USART2_TX,
               DMA_MEMORY_TO_PERIPH, DMA_PDATAALIGN_BYTE,
               DMA_MDATAALIGN_BYTE, DMA_NORMAL, DMA_PRIORITY_HIGH);
    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  } else if (huart->Instance == USART3) {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    UART_Pin_Config(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_AF7_USART3);
    __HAL_LINKDMA(huart, hdmatx, hdma_usart3_tx);
    DMA_Config(&hdma_usart3_tx, DMA1_Channel5, DMA_REQUEST_USART3_TX,
               DMA_MEMORY_TO_PERIPH, DMA_PDATAALIGN_BYTE,
               DMA_MDATAALIGN_BYTE, DMA_NORMAL, DMA_PRIORITY_HIGH);
    HAL_NVIC_SetPriority(USART3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  } else if (huart->Instance == LPUART1) {
    __HAL_RCC_LPUART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    UART_Pin_Config(GPIOA, GPIO_PIN_2 | GPIO_PIN_3, GPIO_AF12_LPUART1);
    __HAL_LINKDMA(huart, hdmatx, hdma_lpuart1_tx);
    DMA_Config(&hdma_lpuart1_tx, DMA1_Channel6, DMA_REQUEST_LPUART1_TX,
               DMA_MEMORY_TO_PERIPH, DMA_PDATAALIGN_BYTE,
               DMA_MDATAALIGN_BYTE, DMA_NORMAL, DMA_PRIORITY_HIGH);
    HAL_NVIC_SetPriority(LPUART1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_DMA_DeInit(&hdma_usart1_tx);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    __HAL_RCC_USART1_CLK_DISABLE();
  } else if (huart->Instance == USART2) {
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    HAL_DMA_DeInit(&hdma_usart2_tx);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
    __HAL_RCC_USART2_CLK_DISABLE();
  } else if (huart->Instance == USART3) {
    HAL_NVIC_DisableIRQ(USART3_IRQn);
    HAL_DMA_DeInit(&hdma_usart3_tx);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    __HAL_RCC_USART3_CLK_DISABLE();
  } else if (huart->Instance == LPUART1) {
    HAL_NVIC_DisableIRQ(LPUART1_IRQn);
    HAL_DMA_DeInit(&hdma_lpuart1_tx);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
    __HAL_RCC_LPUART1_CLK_DISABLE();
  }
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
  if (hpcd->Instance == USB) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USB_CLK_ENABLE();

    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    g.Alternate = GPIO_AF10_USB;
    HAL_GPIO_Init(GPIOA, &g);

    HAL_NVIC_SetPriority(USB_LP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_IRQn);
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
  if (hpcd->Instance == USB) {
    HAL_NVIC_DisableIRQ(USB_LP_IRQn);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
    __HAL_RCC_USB_CLK_DISABLE();
  }
}
