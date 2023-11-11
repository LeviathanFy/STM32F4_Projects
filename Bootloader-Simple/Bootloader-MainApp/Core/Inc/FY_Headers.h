#ifndef INC_FY_HEADERS_H_
#define INC_FY_HEADERS_H_

// Includes
#include "stm32f4xx_hal.h"
#include "stdio.h"

// Defines
#define LED_GREEN_PIN			GPIO_PIN_13
#define LED_RED_PIN				GPIO_PIN_14
#define ECHO					0
#define TickCount_Start()		(uint32_t TickCount = HAL_GetTick())

// Following struct of function pointers contains Applications to jump
typedef void (*pvFunction)(void);
typedef struct
{
	pvFunction App_1;
	pvFunction App_2;
	pvFunction App_3;
} Applications;

// Following struct of function pointers is used on both Bootloader and Application code
typedef struct
{
	void (*pBlink)(uint32_t delay);
} CommonFunctions;

// EXTERNED Variables
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;


// Functions
void Init_RingBuffer(void);
void DeInit_RingBuffer(void);
int waitForCmd(char *string, uint32_t Timeout);

void Error_Handler(void);

#endif /* INC_FY_HEADERS_H_ */
