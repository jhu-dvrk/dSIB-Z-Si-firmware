#pragma once

#include "stm32g4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void Error_Handler(void);
void Main_OnUsbTransmitDone(void);
void Main_SetBootStage(uint8_t stage);

#ifdef __cplusplus
}
#endif
