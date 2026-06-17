#pragma once

#include "usbd_cdc.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t CDC_Transmit_FS(uint8_t *buf, uint16_t len);
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

#ifdef __cplusplus
}
#endif
