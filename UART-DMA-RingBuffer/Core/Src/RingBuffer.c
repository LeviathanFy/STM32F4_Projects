#include "FY_Headers.h"
#include "stdio.h"
#include "string.h"

// Defines and Variables
#define RX_BUF_SIZE									512
#define STORE_BUF_SIZE								2048


// Variables
static uint8_t DMA_Rx_Buffer[RX_BUF_SIZE];
static uint8_t Rx_Store_Buffer[STORE_BUF_SIZE];
//Ring buffer indexes
static uint32_t Idx_Previous;
static uint32_t Idx_New;
static uint32_t Head_Pos;


// Function Prototypes
void Init_RingBuffer(void);
void Reset_RingBuffer(void);
void UartWaitTxComplete(UART_HandleTypeDef *huart);


// Functions

void Init_RingBuffer(void) {
	memset(DMA_Rx_Buffer, 0, RX_BUF_SIZE);
	memset(Rx_Store_Buffer, 0, STORE_BUF_SIZE);

	Head_Pos = 0;
	Idx_New = 0;
	Idx_Previous = 0;

	// DMA Receive is enabled
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, &DMA_Rx_Buffer[0], RX_BUF_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);	// Half transfer interrupt is disabled
}

void Reset_RingBuffer(void) {
	memset(DMA_Rx_Buffer, 0, RX_BUF_SIZE);
	memset(Rx_Store_Buffer, 0, STORE_BUF_SIZE);

	Head_Pos = 0;
	Idx_New = 0;
	Idx_Previous = 0;
}

// Wait for UART transmission to complete
void UartWaitTxComplete(UART_HandleTypeDef *huart)
{
	// Wait until the UART is no longer busy
	while (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET) {}
	__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	Idx_Previous = Idx_New;  // Last index is updated

	// If last index + incoming data size exceeds buffer size, copy remaining data and update the indexes
	if (Idx_Previous + Size > STORE_BUF_SIZE)
	{
		uint16_t copy_len = STORE_BUF_SIZE - Idx_Previous;
		memcpy((uint8_t*) (Rx_Store_Buffer + Idx_Previous), (uint8_t*) DMA_Rx_Buffer, copy_len);

		Idx_Previous = 0;  // point to the start of the buffer
		memcpy((uint8_t*) Rx_Store_Buffer, (uint8_t*) (DMA_Rx_Buffer + copy_len), (Size - copy_len));  // copy the remaining data
		Idx_New = (Size - copy_len);  // update the position
	}
	else
	{
		memcpy((uint8_t*) (Rx_Store_Buffer + Idx_Previous), (uint8_t*) DMA_Rx_Buffer, Size);
		Idx_New = Size + Idx_Previous;
	}

	// ECHO is defined in FY_Header.h	 	1 -> ECHO ON : 0 -> ECHO OFF
#if ECHO
	uint16_t echo_idx;
	for (int i = 0; i < Size; i++)
	{
		if (Head_Pos + i < STORE_BUF_SIZE)
		{
			echo_idx = Head_Pos + i;
		}
		else
			echo_idx = Head_Pos + i - STORE_BUF_SIZE;

		HAL_UART_Transmit(&huart1, (uint8_t*)&Rx_Store_Buffer[echo_idx], 1, HAL_MAX_DELAY);
		UartWaitTxComplete(&huart1);
	}
#endif

	if (Head_Pos + Size < STORE_BUF_SIZE)
		Head_Pos += Size;
	else
		Head_Pos = Head_Pos + Size - STORE_BUF_SIZE;

	// DMA Receive is enabled
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)DMA_Rx_Buffer, RX_BUF_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);	// Half transfer interrupt is disabled
}
