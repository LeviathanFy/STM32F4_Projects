#ifndef INC_FY_HEADERS_H_
#define INC_FY_HEADERS_H_

// Includes
#include "stm32f4xx_hal.h"

// Defines
#define ECHO					0
#define TickCount_Start()		(uint32_t TickCount = HAL_GetTick())


// EXTERNED Variables
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;


// Functions
void Init_RingBuffer(void);
void DeInit_RingBuffer(void);

void Error_Handler(void);

#endif /* INC_FY_HEADERS_H_ */
