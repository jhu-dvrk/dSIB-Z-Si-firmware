#pragma once

#include <stdint.h>
#include <string.h>
#include "stm32g4xx_hal.h"

#define USBD_MAX_NUM_INTERFACES     1
#define USBD_MAX_NUM_CONFIGURATION  1
#define USBD_MAX_STR_DESC_SIZ       512
#define USBD_SUPPORT_USER_STRING    0
#define USBD_SELF_POWERED           0
#define USBD_DEBUG_LEVEL            0

#define DEVICE_FS                   0

#define LPM_ENABLED                 0
#define USBD_CDC_INTERVAL           1

void *USBD_static_malloc(uint32_t size);
void USBD_static_free(void *p);
void USBD_LL_Delay(uint32_t Delay);

#define USBD_malloc         (void *)USBD_static_malloc
#define USBD_free           USBD_static_free
#define USBD_memset         memset
#define USBD_memcpy         memcpy
#define USBD_Delay          HAL_Delay

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...) printf(__VA_ARGS__)
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)
#define USBD_ErrLog(...) printf(__VA_ARGS__)
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...) printf(__VA_ARGS__)
#else
#define USBD_DbgLog(...)
#endif
