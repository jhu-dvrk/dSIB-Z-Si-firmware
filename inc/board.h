#pragma once

#include "stm32g4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_ADC_CHANNEL_COUNT 8u
#define BOARD_UART_PORT_COUNT  4u
#define BOARD_PACKET_SIZE      12u

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef hlpuart1;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_lpuart1_tx;

extern volatile uint16_t g_adc1_samples[4];
extern volatile uint16_t g_adc2_samples[4];

void SystemClock_Config(void);
void PeriphClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_ADC2_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);
void MX_LPUART1_UART_Init(void);

bool Board_StartAdcDma(void);
void Board_SetStatusLed(bool on);
void Board_ToggleStatusLed(void);

#ifdef __cplusplus
}
#endif
