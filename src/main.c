#include "main.h"
#include "board.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
  k_uart_count = BOARD_UART_PORT_COUNT,
  k_packet_size = BOARD_PACKET_SIZE,
  k_packet_crc_offset = 10,
  k_usb_diag_interval_ms = 1000,
};

typedef struct {
  UART_HandleTypeDef *huart;
  const volatile uint16_t *pot1;
  const volatile uint16_t *pot2;
  uint8_t id;
} StreamPort;

static const StreamPort s_ports[k_uart_count] = {
  {&huart1,    &g_adc1_samples[2], &g_adc1_samples[3], 0x04u},
  {&huart2,    &g_adc2_samples[2], &g_adc2_samples[3], 0x06u},
  {&huart3,    &g_adc2_samples[0], &g_adc2_samples[1], 0x08u},
  {&hlpuart1, &g_adc1_samples[0], &g_adc1_samples[1], 0x0Au},
};

static uint8_t s_uart_tx[k_uart_count][k_packet_size];
static volatile bool s_uart_busy[k_uart_count];
static volatile uint32_t s_uart_packets_started[k_uart_count];
static volatile uint32_t s_uart_packets_done[k_uart_count];
static volatile uint32_t s_uart_errors[k_uart_count];

static uint8_t s_usb_tx[256];
static volatile bool s_usb_tx_busy;
static volatile uint8_t s_boot_stage;
static uint32_t s_last_usb_diag_ms;

static uint16_t Protocol_CrcUpdate(uint16_t crc, uint8_t data);
static uint16_t Protocol_Crc16(const uint8_t *data, uint32_t len);
static void Protocol_Pack(uint8_t *out, uint8_t id, uint16_t pot1, uint16_t pot2);
static bool Protocol_SelfTest(void);
static void ServiceUartStreams(void);
static void ServiceUsbDiagnostics(uint32_t now_ms);
static int UartIndexFromHandle(const UART_HandleTypeDef *huart);
static void InitErrorLed(void);
static uint32_t LoadCounter(volatile uint32_t *counter);

int main(void)
{
  Main_SetBootStage(1u);
  HAL_Init();
  Main_SetBootStage(2u);
  SystemClock_Config();
  Main_SetBootStage(3u);
  PeriphClock_Config();

  Main_SetBootStage(4u);
  MX_GPIO_Init();
  Main_SetBootStage(5u);
  MX_DMA_Init();
  Main_SetBootStage(6u);
  MX_ADC1_Init();
  Main_SetBootStage(7u);
  MX_ADC2_Init();
  Main_SetBootStage(8u);
  MX_USART1_UART_Init();
  Main_SetBootStage(9u);
  MX_USART2_UART_Init();
  Main_SetBootStage(10u);
  MX_USART3_UART_Init();
  Main_SetBootStage(11u);
  MX_LPUART1_UART_Init();
  Main_SetBootStage(12u);
  MX_USB_Device_Init();

  Main_SetBootStage(13u);
  if (!Protocol_SelfTest()) {
    Error_Handler();
  }

  Main_SetBootStage(14u);
  if (!Board_StartAdcDma()) {
    Error_Handler();
  }

  Main_SetBootStage(0u);
  s_last_usb_diag_ms = HAL_GetTick();

  while (1) {
    uint32_t now_ms = HAL_GetTick();
    ServiceUartStreams();
    ServiceUsbDiagnostics(now_ms);
    __WFI();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  InitErrorLed();
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

  while (1) {
    __NOP();
  }
}

void Main_SetBootStage(uint8_t stage)
{
  s_boot_stage = stage;
}

void Main_OnUsbTransmitDone(void)
{
  s_usb_tx_busy = false;
}

static uint16_t Protocol_CrcUpdate(uint16_t crc, uint8_t data)
{
#define QB(n) ((uint16_t)((crc >> (n)) & 1u))
#define DB(n) ((uint16_t)((data >> (n)) & 1u))
  uint16_t next = 0u;

  next |= (uint16_t)((QB(8) ^ QB(12) ^ QB(13) ^ QB(14) ^ DB(0) ^ DB(4) ^ DB(5) ^ DB(6)) << 0);
  next |= (uint16_t)((QB(8) ^ QB(9) ^ QB(12) ^ QB(15) ^ DB(0) ^ DB(1) ^ DB(4) ^ DB(7)) << 1);
  next |= (uint16_t)((QB(8) ^ QB(9) ^ QB(10) ^ QB(12) ^ QB(14) ^ DB(0) ^ DB(1) ^ DB(2) ^ DB(4) ^ DB(6)) << 2);
  next |= (uint16_t)((QB(9) ^ QB(10) ^ QB(11) ^ QB(13) ^ QB(15) ^ DB(1) ^ DB(2) ^ DB(3) ^ DB(5) ^ DB(7)) << 3);
  next |= (uint16_t)((QB(8) ^ QB(10) ^ QB(11) ^ QB(13) ^ DB(0) ^ DB(2) ^ DB(3) ^ DB(5)) << 4);
  next |= (uint16_t)((QB(8) ^ QB(9) ^ QB(11) ^ QB(13) ^ DB(0) ^ DB(1) ^ DB(3) ^ DB(5)) << 5);
  next |= (uint16_t)((QB(9) ^ QB(10) ^ QB(12) ^ QB(14) ^ DB(1) ^ DB(2) ^ DB(4) ^ DB(6)) << 6);
  next |= (uint16_t)((QB(8) ^ QB(10) ^ QB(11) ^ QB(12) ^ QB(14) ^ QB(15) ^ DB(0) ^ DB(2) ^ DB(3) ^ DB(4) ^ DB(6) ^ DB(7)) << 7);
  next |= (uint16_t)((QB(0) ^ QB(8) ^ QB(9) ^ QB(11) ^ QB(14) ^ QB(15) ^ DB(0) ^ DB(1) ^ DB(3) ^ DB(6) ^ DB(7)) << 8);
  next |= (uint16_t)((QB(1) ^ QB(9) ^ QB(10) ^ QB(12) ^ QB(15) ^ DB(1) ^ DB(2) ^ DB(4) ^ DB(7)) << 9);
  next |= (uint16_t)((QB(2) ^ QB(8) ^ QB(10) ^ QB(11) ^ QB(12) ^ QB(14) ^ DB(0) ^ DB(2) ^ DB(3) ^ DB(4) ^ DB(6)) << 10);
  next |= (uint16_t)((QB(3) ^ QB(8) ^ QB(9) ^ QB(11) ^ QB(14) ^ QB(15) ^ DB(0) ^ DB(1) ^ DB(3) ^ DB(6) ^ DB(7)) << 11);
  next |= (uint16_t)((QB(4) ^ QB(8) ^ QB(9) ^ QB(10) ^ QB(13) ^ QB(14) ^ QB(15) ^ DB(0) ^ DB(1) ^ DB(2) ^ DB(5) ^ DB(6) ^ DB(7)) << 12);
  next |= (uint16_t)((QB(5) ^ QB(9) ^ QB(10) ^ QB(11) ^ QB(14) ^ QB(15) ^ DB(1) ^ DB(2) ^ DB(3) ^ DB(6) ^ DB(7)) << 13);
  next |= (uint16_t)((QB(6) ^ QB(10) ^ QB(11) ^ QB(12) ^ QB(15) ^ DB(2) ^ DB(3) ^ DB(4) ^ DB(7)) << 14);
  next |= (uint16_t)((QB(7) ^ QB(11) ^ QB(12) ^ QB(13) ^ DB(3) ^ DB(4) ^ DB(5)) << 15);

#undef QB
#undef DB
  return next;
}

static uint16_t Protocol_Crc16(const uint8_t *data, uint32_t len)
{
  uint16_t crc = 0xFFFFu;
  for (uint32_t i = 0; i < len; ++i) {
    crc = Protocol_CrcUpdate(crc, data[i]);
  }
  return crc;
}

static void Protocol_Pack(uint8_t *out, uint8_t id, uint16_t pot1, uint16_t pot2)
{
  pot1 &= 0x0FFFu;
  pot2 &= 0x0FFFu;

  out[0] = (uint8_t)'d';
  out[1] = (uint8_t)'S';
  out[2] = (uint8_t)'I';
  out[3] = (uint8_t)'B';
  out[4] = (uint8_t)'Z';
  out[5] = (uint8_t)(id & 0x0Fu);
  out[6] = (uint8_t)(pot1 & 0xFFu);
  out[7] = (uint8_t)(pot1 >> 8);
  out[8] = (uint8_t)(pot2 & 0xFFu);
  out[9] = (uint8_t)(pot2 >> 8);

  uint16_t crc = Protocol_Crc16(out, k_packet_crc_offset);
  out[10] = (uint8_t)(crc & 0xFFu);
  out[11] = (uint8_t)(crc >> 8);
}

static bool Protocol_SelfTest(void)
{
  static const uint8_t test_ascii[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  if (Protocol_Crc16(NULL, 0u) != 0xFFFFu) {
    return false;
  }
  if (Protocol_Crc16(test_ascii, sizeof(test_ascii)) != 0x9BE5u) {
    return false;
  }

  uint8_t packet[k_packet_size];
  Protocol_Pack(packet, 2u, 0x0123u, 0x0ABCu);
  static const uint8_t expected[k_packet_size] = {
    0x64, 0x53, 0x49, 0x42, 0x5A, 0x02,
    0x23, 0x01, 0xBC, 0x0A, 0x2F, 0x0B
  };
  return memcmp(packet, expected, sizeof(expected)) == 0;
}

static void ServiceUartStreams(void)
{
  for (uint32_t i = 0; i < k_uart_count; ++i) {
    if (s_uart_busy[i]) {
      continue;
    }

    uint16_t pot1 = (uint16_t)(*s_ports[i].pot1 & 0x0FFFu);
    uint16_t pot2 = (uint16_t)(*s_ports[i].pot2 & 0x0FFFu);
    Protocol_Pack(s_uart_tx[i], s_ports[i].id, pot1, pot2);

    s_uart_busy[i] = true;
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(s_ports[i].huart, s_uart_tx[i], k_packet_size);
    if (status == HAL_OK) {
      s_uart_packets_started[i]++;
    } else {
      s_uart_busy[i] = false;
      if (status != HAL_BUSY) {
        s_uart_errors[i]++;
      }
    }
  }
}

static void ServiceUsbDiagnostics(uint32_t now_ms)
{
  if (s_usb_tx_busy) {
    return;
  }
  if ((uint32_t)(now_ms - s_last_usb_diag_ms) < k_usb_diag_interval_ms) {
    return;
  }
  s_last_usb_diag_ms = now_ms;
  Board_SetStatusLed(true);

  uint16_t pa0 = (uint16_t)(g_adc1_samples[0] & 0x0FFFu);
  uint16_t pa1 = (uint16_t)(g_adc1_samples[1] & 0x0FFFu);
  uint16_t pb0 = (uint16_t)(g_adc1_samples[2] & 0x0FFFu);
  uint16_t pb1 = (uint16_t)(g_adc1_samples[3] & 0x0FFFu);
  uint16_t pa4 = (uint16_t)(g_adc2_samples[0] & 0x0FFFu);
  uint16_t pa5 = (uint16_t)(g_adc2_samples[1] & 0x0FFFu);
  uint16_t pa6 = (uint16_t)(g_adc2_samples[2] & 0x0FFFu);
  uint16_t pa7 = (uint16_t)(g_adc2_samples[3] & 0x0FFFu);
  int len = snprintf((char *)s_usb_tx, sizeof(s_usb_tx),
                     "dSIBZ t=%lu tx=%lu,%lu,%lu,%lu done=%lu,%lu,%lu,%lu err=%lu,%lu,%lu,%lu adc PA0=%u PA1=%u PB0=%u PB1=%u PA4=%u PA5=%u PA6=%u PA7=%u\r\n",
                     (unsigned long)now_ms,
                     (unsigned long)LoadCounter(&s_uart_packets_started[0]),
                     (unsigned long)LoadCounter(&s_uart_packets_started[1]),
                     (unsigned long)LoadCounter(&s_uart_packets_started[2]),
                     (unsigned long)LoadCounter(&s_uart_packets_started[3]),
                     (unsigned long)LoadCounter(&s_uart_packets_done[0]),
                     (unsigned long)LoadCounter(&s_uart_packets_done[1]),
                     (unsigned long)LoadCounter(&s_uart_packets_done[2]),
                     (unsigned long)LoadCounter(&s_uart_packets_done[3]),
                     (unsigned long)LoadCounter(&s_uart_errors[0]),
                     (unsigned long)LoadCounter(&s_uart_errors[1]),
                     (unsigned long)LoadCounter(&s_uart_errors[2]),
                     (unsigned long)LoadCounter(&s_uart_errors[3]),
                     (unsigned)pa0,
                     (unsigned)pa1,
                     (unsigned)pb0,
                     (unsigned)pb1,
                     (unsigned)pa4,
                     (unsigned)pa5,
                     (unsigned)pa6,
                     (unsigned)pa7);
  if (len <= 0) {
    return;
  }
  if ((uint32_t)len > sizeof(s_usb_tx)) {
    len = (int)sizeof(s_usb_tx);
  }

  s_usb_tx_busy = true;
  if (CDC_Transmit_FS(s_usb_tx, (uint16_t)len) != USBD_OK) {
    s_usb_tx_busy = false;
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  int idx = UartIndexFromHandle(huart);
  if (idx >= 0) {
    s_uart_busy[idx] = false;
    s_uart_packets_done[idx]++;
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  int idx = UartIndexFromHandle(huart);
  if (idx >= 0) {
    s_uart_busy[idx] = false;
    s_uart_errors[idx]++;
  }
}

static int UartIndexFromHandle(const UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1) {
    return 0;
  }
  if (huart->Instance == USART2) {
    return 1;
  }
  if (huart->Instance == USART3) {
    return 2;
  }
  if (huart->Instance == LPUART1) {
    return 3;
  }
  return -1;
}

static void InitErrorLed(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef g = {0};
  g.Pin = GPIO_PIN_9;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &g);
}

static uint32_t LoadCounter(volatile uint32_t *counter)
{
  uint32_t value;
  __disable_irq();
  value = *counter;
  __enable_irq();
  return value;
}
