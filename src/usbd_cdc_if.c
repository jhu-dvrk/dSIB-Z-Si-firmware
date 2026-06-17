#include "usbd_cdc_if.h"
#include "main.h"
#include "usb_device.h"

#define APP_RX_DATA_SIZE 64U
#define APP_TX_DATA_SIZE 256U

static uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];
static uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

static int8_t CDC_Init_FS(void)
{
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_CDC_ReceivePacket(&hUsbDeviceFS) == USBD_OK) ? (int8_t)USBD_OK : (int8_t)USBD_FAIL;
}

static int8_t CDC_DeInit_FS(void)
{
  return (int8_t)USBD_OK;
}

static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  (void)cmd;
  (void)pbuf;
  (void)length;
  return (int8_t)USBD_OK;
}

static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len)
{
  (void)Len;
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, pbuf);
  return (USBD_CDC_ReceivePacket(&hUsbDeviceFS) == USBD_OK) ? (int8_t)USBD_OK : (int8_t)USBD_FAIL;
}

uint8_t CDC_Transmit_FS(uint8_t *buf, uint16_t len)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  if (hcdc->TxState != 0U) {
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, buf, len);
  return USBD_CDC_TransmitPacket(&hUsbDeviceFS);
}

static int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  (void)Buf;
  (void)Len;
  (void)epnum;
  Main_OnUsbTransmitDone();
  return (int8_t)USBD_OK;
}
