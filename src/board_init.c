#include "board.h"
#include "main.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef hlpuart1;
DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_lpuart1_tx;

volatile uint16_t g_adc1_samples[4] __attribute__((aligned(4)));
volatile uint16_t g_adc2_samples[4] __attribute__((aligned(4)));

#define STATUS_LED_GPIO_PORT GPIOB
#define STATUS_LED_PIN       GPIO_PIN_9

typedef struct
{
  __IO uint32_t MODER;
  __IO uint32_t OTYPER;
  __IO uint32_t OSPEEDR;
  __IO uint32_t PUPDR;
  __IO uint32_t IDR;
  __IO uint32_t ODR;
  __IO uint32_t BSRR;
  __IO uint32_t LCKR;
  __IO uint32_t AFR[2];
  __IO uint32_t BRR;
  __IO uint32_t ASCR;
} GPIO_WithASCR_TypeDef;

static void CloseAnalogSwitch(GPIO_TypeDef *port, uint32_t pin_mask)
{
  GPIO_WithASCR_TypeDef *gpio = (GPIO_WithASCR_TypeDef *)port;
  gpio->ASCR |= pin_mask;
}

void SystemClock_Config(void)
{
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK) {
    Error_Handler();
  }

  RCC_OscInitTypeDef osc = {0};
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.HSI48State = RCC_HSI48_ON;
  osc.PLL.PLLState = RCC_PLL_ON;
  osc.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  osc.PLL.PLLM = RCC_PLLM_DIV4;  /* 16 MHz / 4 = 4 MHz */
  osc.PLL.PLLN = 85;
  osc.PLL.PLLP = RCC_PLLP_DIV2;  /* 170 MHz */
  osc.PLL.PLLQ = RCC_PLLQ_DIV2;
  osc.PLL.PLLR = RCC_PLLR_DIV2;  /* 170 MHz SYSCLK */
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitTypeDef clk = {0};
  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_8) != HAL_OK) {
    Error_Handler();
  }
}

void PeriphClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef p = {0};
  p.PeriphClockSelection = RCC_PERIPHCLK_ADC12 |
                           RCC_PERIPHCLK_USART1 |
                           RCC_PERIPHCLK_USART2 |
                           RCC_PERIPHCLK_USART3 |
                           RCC_PERIPHCLK_LPUART1 |
                           RCC_PERIPHCLK_USB;
  p.Adc12ClockSelection = RCC_ADC12CLKSOURCE_PLL;
  p.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  p.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  p.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  p.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  p.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&p) != HAL_OK) {
    Error_Handler();
  }

  __HAL_RCC_CRS_CLK_ENABLE();
  RCC_CRSInitTypeDef crs = {0};
  crs.Prescaler = RCC_CRS_SYNC_DIV1;
  crs.Source = RCC_CRS_SYNC_SOURCE_USB;
  crs.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  crs.ReloadValue = RCC_CRS_RELOADVALUE_DEFAULT;
  crs.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  crs.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;
  HAL_RCCEx_CRSConfig(&crs);
}

void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_SYSCFG_EnableIOSwitchBooster();
  HAL_SYSCFG_EnableIOSwitchVDD();

  GPIO_InitTypeDef g = {0};

  g.Mode = GPIO_MODE_ANALOG;
  g.Pull = GPIO_NOPULL;
  g.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &g);
  g.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  HAL_GPIO_Init(GPIOB, &g);

  CloseAnalogSwitch(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |
                           GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
  CloseAnalogSwitch(GPIOB, GPIO_PIN_0 | GPIO_PIN_1);

  g.Pin = STATUS_LED_PIN;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(STATUS_LED_GPIO_PORT, &g);
  Board_SetStatusLed(true);

  g.Pin = GPIO_PIN_8;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &g);
}

void MX_DMA_Init(void)
{
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
}

static void ADC_Common_Config(ADC_HandleTypeDef *h)
{
  h->Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV4;
  h->Init.Resolution = ADC_RESOLUTION_12B;
  h->Init.DataAlign = ADC_DATAALIGN_RIGHT;
  h->Init.GainCompensation = 0;
  h->Init.ScanConvMode = ADC_SCAN_ENABLE;
  h->Init.EOCSelection = ADC_EOC_SEQ_CONV;
  h->Init.LowPowerAutoWait = DISABLE;
  h->Init.ContinuousConvMode = ENABLE;
  h->Init.NbrOfConversion = 4;
  h->Init.DiscontinuousConvMode = DISABLE;
  h->Init.ExternalTrigConv = ADC_SOFTWARE_START;
  h->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  h->Init.DMAContinuousRequests = ENABLE;
  h->Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  h->Init.OversamplingMode = ENABLE;
  h->Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_256;
  h->Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_8;
  h->Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  h->Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(h) != HAL_OK) {
    Error_Handler();
  }
}

static void ADC_Channel_Config(ADC_HandleTypeDef *h, uint32_t channel, uint32_t rank)
{
  ADC_ChannelConfTypeDef s = {0};
  s.Channel = channel;
  s.Rank = rank;
  s.SamplingTime = ADC_SAMPLETIME_12CYCLES_5;
  s.SingleDiff = ADC_SINGLE_ENDED;
  s.OffsetNumber = ADC_OFFSET_NONE;
  s.Offset = 0;
  if (HAL_ADC_ConfigChannel(h, &s) != HAL_OK) {
    Error_Handler();
  }
}

void MX_ADC1_Init(void)
{
  __HAL_RCC_ADC12_CLK_ENABLE();
  hadc1.Instance = ADC1;
  ADC_Common_Config(&hadc1);
  ADC_Channel_Config(&hadc1, ADC_CHANNEL_1, ADC_REGULAR_RANK_1);   /* PA0 */
  ADC_Channel_Config(&hadc1, ADC_CHANNEL_2, ADC_REGULAR_RANK_2);   /* PA1 */
  ADC_Channel_Config(&hadc1, ADC_CHANNEL_15, ADC_REGULAR_RANK_3);  /* PB0 */
  ADC_Channel_Config(&hadc1, ADC_CHANNEL_12, ADC_REGULAR_RANK_4);  /* PB1 */
}

void MX_ADC2_Init(void)
{
  __HAL_RCC_ADC12_CLK_ENABLE();
  hadc2.Instance = ADC2;
  ADC_Common_Config(&hadc2);
  ADC_Channel_Config(&hadc2, ADC_CHANNEL_17, ADC_REGULAR_RANK_1);  /* PA4 */
  ADC_Channel_Config(&hadc2, ADC_CHANNEL_13, ADC_REGULAR_RANK_2);  /* PA5 */
  ADC_Channel_Config(&hadc2, ADC_CHANNEL_3, ADC_REGULAR_RANK_3);   /* PA6 */
  ADC_Channel_Config(&hadc2, ADC_CHANNEL_4, ADC_REGULAR_RANK_4);   /* PA7 */
}

static void UART_Common_Config(UART_HandleTypeDef *h)
{
  h->Init.BaudRate = 115200;
  h->Init.WordLength = UART_WORDLENGTH_8B;
  h->Init.StopBits = UART_STOPBITS_1;
  h->Init.Parity = UART_PARITY_NONE;
  h->Init.Mode = UART_MODE_TX_RX;
  h->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  h->Init.OverSampling = UART_OVERSAMPLING_16;
  h->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  h->Init.ClockPrescaler = UART_PRESCALER_DIV1;
  h->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
}

void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  UART_Common_Config(&huart1);
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  UART_Common_Config(&huart2);
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();
  }
}

void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  UART_Common_Config(&huart3);
  if (HAL_UART_Init(&huart3) != HAL_OK) {
    Error_Handler();
  }
}

void MX_LPUART1_UART_Init(void)
{
  hlpuart1.Instance = LPUART1;
  UART_Common_Config(&hlpuart1);
  if (HAL_UART_Init(&hlpuart1) != HAL_OK) {
    Error_Handler();
  }
}

bool Board_StartAdcDma(void)
{
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
    return false;
  }
  if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK) {
    return false;
  }
  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)g_adc1_samples, 4) != HAL_OK) {
    return false;
  }
  if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)g_adc2_samples, 4) != HAL_OK) {
    return false;
  }

  __HAL_DMA_DISABLE_IT(&hdma_adc1, DMA_IT_HT);
  __HAL_DMA_DISABLE_IT(&hdma_adc2, DMA_IT_HT);
  return true;
}

void Board_SetStatusLed(bool on)
{
  HAL_GPIO_WritePin(STATUS_LED_GPIO_PORT, STATUS_LED_PIN, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void Board_ToggleStatusLed(void)
{
  HAL_GPIO_TogglePin(STATUS_LED_GPIO_PORT, STATUS_LED_PIN);
}
