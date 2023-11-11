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
static uint32_t Head_Pos, Tail_Pos;
//Bootloader Jump Command
uint8_t isDataAvailable = 0;
int32_t TIMEOUT = 0;

// Function Prototypes
void Init_RingBuffer(void);
void Reset_RingBuffer(void);
int waitForCmd(char *string, uint32_t Timeout);
void UartWaitTxComplete(UART_HandleTypeDef *huart);


// Functions

void Init_RingBuffer(void) {
	memset(DMA_Rx_Buffer, 0, RX_BUF_SIZE);
	memset(Rx_Store_Buffer, 0, STORE_BUF_SIZE);

	Head_Pos = 0;
	Tail_Pos = 0;
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

int waitForCmd (char *string, uint32_t Timeout)
{
	int so_far =0;
	int len = strlen (string);
	TIMEOUT = Timeout;
	isDataAvailable = 0;

	while ((Tail_Pos == Head_Pos) && TIMEOUT)
	{
		Common_Funcs.pBlink(250);;
	}

again:
	/* if the incoming data does not match with the string, we will simply increment the index
	 * And wait for the string to arrive in the incoming data
	 * */
	while (Rx_Store_Buffer[Tail_Pos] != string[so_far])  // peek in the rx_buffer to see if we get the string
	{
		if (TIMEOUT <= 0) return 0;


		if (Tail_Pos == Head_Pos) goto again;
		Tail_Pos++;

		if (Tail_Pos == STORE_BUF_SIZE)
			Tail_Pos = 0;
	}

	/* If the incoming data does match with the string, we will return 1 to indicate this */
	while (Rx_Store_Buffer[Tail_Pos] == string[so_far]) // if we got the first letter of the string
	{
		if (TIMEOUT <= 0) return 0;
		so_far++;

		if (Tail_Pos == Head_Pos) goto again;
			Tail_Pos++;
		if (Tail_Pos == STORE_BUF_SIZE)
			Tail_Pos = 0;
		if (so_far == len)
			return 1;
	}
	HAL_Delay (100);

	if ((so_far!=len)&&isDataAvailable)
		{
			isDataAvailable = 0;
			goto again;
		}
		else
		{
			so_far = 0;
			goto again;
		}
	return 0;
}

int __io_putchar(int ch)
{
	if (HAL_UART_Transmit_IT(&huart1, (uint8_t *)&ch, 1) == HAL_OK)
	{
		UartWaitTxComplete(&huart1);
	}
	return 1;
}
