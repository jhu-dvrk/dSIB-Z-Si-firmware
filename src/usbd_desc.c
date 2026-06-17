#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_cdc.h"

#define USBD_VID                     0x0483
#define USBD_PID_FS                  0x5740
#define USBD_LANGID_STRING           1033
#define USBD_MANUFACTURER_STRING     "dSIB"
#define USBD_PRODUCT_STRING_FS       "dSIB-Z-Si"
#define USBD_CONFIGURATION_STRING_FS "CDC Config"
#define USBD_INTERFACE_STRING_FS     "CDC Interface"

#define USB_CDC_CONFIG_DESC_SIZ      67U
#define CDC_DATA_FS_MAX_PACKET_SIZE  64U
#define CDC_CMD_PACKET_SIZE          8U

#ifndef USB_SIZ_STRING_SERIAL
#define USB_SIZ_STRING_SERIAL        0x1AU
#endif

__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END = {
  0x12,
  USB_DESC_TYPE_DEVICE,
  0x00, 0x02,
  0x02,
  0x02,
  0x00,
  USB_MAX_EP0_SIZE,
  LOBYTE(USBD_VID), HIBYTE(USBD_VID),
  LOBYTE(USBD_PID_FS), HIBYTE(USBD_PID_FS),
  0x00, 0x02,
  USBD_IDX_MFC_STR,
  USBD_IDX_PRODUCT_STR,
  USBD_IDX_SERIAL_STR,
  0x01
};

__ALIGN_BEGIN uint8_t USBD_CDC_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END = {
  0x09,
  USB_DESC_TYPE_CONFIGURATION,
  USB_CDC_CONFIG_DESC_SIZ, 0x00,
  0x02,
  0x01,
  0x00,
  0x80,
  0x32,

  0x09, USB_DESC_TYPE_INTERFACE, 0x00, 0x00, 0x01,
  0x02, 0x02, 0x01, 0x00,

  0x05, 0x24, 0x00, 0x10, 0x01,
  0x05, 0x24, 0x01, 0x00, 0x01,
  0x04, 0x24, 0x02, 0x02,
  0x05, 0x24, 0x06, 0x00, 0x01,

  0x07, USB_DESC_TYPE_ENDPOINT,
  CDC_CMD_EP, 0x03, CDC_CMD_PACKET_SIZE, 0x00, 0x10,

  0x09, USB_DESC_TYPE_INTERFACE, 0x01, 0x00, 0x02,
  0x0A, 0x00, 0x00, 0x00,

  0x07, USB_DESC_TYPE_ENDPOINT,
  CDC_OUT_EP, 0x02, LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00,

  0x07, USB_DESC_TYPE_ENDPOINT,
  CDC_IN_EP, 0x02, LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE), 0x00
};

__ALIGN_BEGIN uint8_t USBD_FS_StringSerial[USB_SIZ_STRING_SERIAL] __ALIGN_END = {USB_SIZ_STRING_SERIAL};

static uint8_t USBD_StringDescriptor[USBD_MAX_STR_DESC_SIZ];

static void Get_SerialNum(void);
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

__ALIGN_BEGIN uint8_t USBD_FS_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END = {
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00, 0x02,
  0x02,
  0x02,
  0x00,
  USB_MAX_EP0_SIZE,
  0x01,
  0x00
};

USBD_DescriptorsTypeDef FS_Desc = {
  USBD_FS_DeviceDescriptor,
  USBD_FS_LangIDStrDescriptor,
  USBD_FS_ManufacturerStrDescriptor,
  USBD_FS_ProductStrDescriptor,
  USBD_FS_SerialStrDescriptor,
  USBD_FS_ConfigStrDescriptor,
  USBD_FS_InterfaceStrDescriptor
};

uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = sizeof(USBD_FS_DeviceDesc);
  return USBD_FS_DeviceDesc;
}

uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  USBD_StringDescriptor[0] = 0x04;
  USBD_StringDescriptor[1] = USB_DESC_TYPE_STRING;
  USBD_StringDescriptor[2] = LOBYTE(USBD_LANGID_STRING);
  USBD_StringDescriptor[3] = HIBYTE(USBD_LANGID_STRING);
  *length = 4;
  return USBD_StringDescriptor;
}

uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StringDescriptor, length);
  return USBD_StringDescriptor;
}

uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StringDescriptor, length);
  return USBD_StringDescriptor;
}

uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  Get_SerialNum();
  *length = USB_SIZ_STRING_SERIAL;
  return USBD_FS_StringSerial;
}

uint8_t *USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StringDescriptor, length);
  return USBD_StringDescriptor;
}

uint8_t *USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StringDescriptor, length);
  return USBD_StringDescriptor;
}

uint8_t *USBD_FS_ConfigDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = sizeof(USBD_CDC_CfgFSDesc);
  return USBD_CDC_CfgFSDesc;
}

uint8_t *USBD_FS_DeviceQualifierDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  (void)speed;
  *length = sizeof(USBD_FS_DeviceQualifierDesc);
  return USBD_FS_DeviceQualifierDesc;
}

static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
  for (uint8_t idx = 0; idx < len; idx++) {
    uint8_t digit = (value >> 28) & 0xFU;
    pbuf[2U * idx] = (digit < 0xAU) ? (uint8_t)(digit + '0') : (uint8_t)(digit + 'A' - 10U);
    pbuf[(2U * idx) + 1U] = 0;
    value <<= 4;
  }
}

static void Get_SerialNum(void)
{
  uint32_t serial = HAL_GetUIDw0() ^ HAL_GetUIDw1() ^ HAL_GetUIDw2();
  USBD_FS_StringSerial[0] = USB_SIZ_STRING_SERIAL;
  USBD_FS_StringSerial[1] = USB_DESC_TYPE_STRING;
  IntToUnicode(serial, &USBD_FS_StringSerial[2], 6);
}
