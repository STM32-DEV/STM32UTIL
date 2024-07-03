#include <cmsis_os2.h>
#include <string.h>
#include "main.h"
#include "spi.h"
#include "stm32util-debug.h"
#include "stm32util-hcms.h"
#include "stm32util-font5x7.h"

#define WIDTH			5
#define HEIGHT			7
#define HCMS_BUFFER_SIZE	(STM32UTIL_HCMS_CHAR_PER_DEVICE * WIDTH)

#if defined(STM32UTIL_HCMS_SPI_ENABLE)

#if STM32UTIL_HCMS_SPI_USE_HAL
extern SPI_HandleTypeDef STM32UTIL_HCMS_HSPI;
#define HCMS_SPI		(&STM32UTIL_HCMS_HSPI)
#elif STM32UTIL_HCMS_SPI_USE_LL
#define HCMS_SPI		(STM32UTIL_HCMS_SPI)
#endif

#if STM32UTIL_HCMS_SPI_USE_HAL
void stm32util_hcms_transmit(uint8_t* ptr, int len)
{
	HAL_SPI_Transmit(HCMS_SPI, ptr, len, HAL_MAX_DELAY);
}

void stm32util_hcms_transmit_dma(uint8_t* ptr, int len)
{
	HAL_SPI_Transmit_DMA(HCMS_SPI, ptr, len);
}

#elif STM32UTIL_HCMS_SPI_USE_LL
void stm32util_hcms_transmit(uint8_t* ptr, int len)
{
	LL_SPI_ClearFlag_EOT(HCMS_SPI);
	LL_SPI_SetTransferSize(HCMS_SPI, len);
	LL_SPI_Enable(HCMS_SPI);
	LL_SPI_StartMasterTransfer(HCMS_SPI);

	while (len--) {
		while (!LL_SPI_IsActiveFlag_TXP(HCMS_SPI)) {
		}
		LL_SPI_TransmitData8(HCMS_SPI, *ptr++);
	}

	while (!LL_SPI_IsActiveFlag_EOT(HCMS_SPI)) {
	}
	LL_SPI_ClearFlag_EOT(HCMS_SPI);
	LL_SPI_ClearFlag_TXTF(HCMS_SPI);

	LL_SPI_Disable(HCMS_SPI);
}

void stm32util_hcms_transmit_dma(uint8_t* ptr, int len)
{
	LL_DMA_DisableRunnel(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL);
	LL_DMA_SetMemoryAddress(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL, (uint32_t)ptr);
	LL_DMA_SetDataLength(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL, len);
	LL_DMA_EnableRunnel(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL);
}

void stm32util_hcms_setup_tx_dma()
{
	LL_SPI_EnableDMAReq_TX(HCMS_SPI);
	LL_DMA_EnableIT_TC(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL);
	LL_DMA_SetPeriphAddress(STM32UTIL_HCMS_SPI_DMA, STM32UTIL_HCMS_SPI_RUNNEL, LL_SPI_DMA_GetTxRegAddr(HCMS_SPI));
}

#endif

static bool hcms_inited_ = false;
_AT_HCMS_DMA_BUFFER __attribute__((aligned(32))) static uint8_t hcms_buffer_[((HCMS_BUFFER_SIZE + 31) / 32) * 32];

#if STM32UTIL_USE_OS
static volatile osSemaphoreId_t hcms_sema_;
static const osSemaphoreAttr_t hcms_sema_attr_ = {
    .name = "hcms_tx_sema",
};
#else
/*_AT_ITCMRAM*/ static volatile bool hcms_dma_tx_flag = true;
#endif

void stm32util_hcms_dma_start_tx()
{
#if !STM32UTIL_USE_OS
	hcms_dma_tx_flag = false;
#endif
}

void stm32util_hcms_dma_end_tx()
{
#if STM32UTIL_USE_OS
	osSemaphoreRelease(hcms_sema_);
#else
	hcms_dma_tx_flag = true;
#endif
}

void stm32util_hcms_dma_wait_tx()
{
#if STM32UTIL_USE_OS
	// Wait for the transmission to complete
	while (osSemaphoreAcquire(hcms_sema_, osWaitForever) != osOK) {
	}
#else
	while (!hcms_dma_tx_flag) {
	}
#endif
}

/**
 * @brief Callback function for SPI transmission complete.
 *
 * @param hdma SPI handle.
 */
#if STM32UTIL_HCMS_SPI_USE_HAL
/*_AT_ITCMRAM*/ void hcms_tx_completed(SPI_HandleTypeDef *hdma)
{
	stm32util_hcms_dma_end_tx();
}

#elif STM32UTIL_HCMS_SPI_USE_LL
/*_AT_ITCMRAM*/ void STM32UTIL_HCMS_SPI_DMA_TC_isr()
{
	//osSemaphoreRelease(hcms_sema_);
}

/*_AT_ITCMRAM*/ void STM32UTIL_HCMS_SPI_TC_isr()
{
	stm32util_hcms_dma_end_tx();
}

#endif

bool stm32util_hcms_init()
{
	if (!hcms_inited_) { // first check
#if STM32UTIL_USE_OS
		osKernelLock();
#endif

		if (!hcms_inited_) { // double check
			stm32util_hcms_reset();

			HAL_GPIO_WritePin(STM32UTIL_HCMS_BL_PORT, STM32UTIL_HCMS_BL_PIN, RESET);

			HAL_GPIO_WritePin(STM32UTIL_HCMS_RS_PORT, STM32UTIL_HCMS_RS_PIN, SET);

			uint8_t cw0 = 0x40 | 0x0C | 0x20;
			stm32util_hcms_transmit(&cw0, 1);

			HAL_GPIO_WritePin(LED_RS_GPIO_Port, LED_RS_Pin, RESET);

#if STM32UTIL_USE_OS
			hcms_sema_ = osSemaphoreNew(1, 1, &hcms_sema_attr_);
			if (!hcms_sema_) {
				CRITICAL("osSemaphoreNew hcms");
			}
#endif

#if STM32UTIL_HCMS_SPI_USE_HAL
			if (HAL_OK != HAL_SPI_RegisterCallback(HCMS_SPI, HAL_SPI_TX_COMPLETE_CB_ID, hcms_tx_completed)) {
				CRITICAL("HAL_SPI_RegisterCallback hcms");
			}
#elif STM32UTIL_HCMS_SPI_USE_LL
			stm32util_hcms_setup_tx_dma();
#endif

			hcms_inited_ = true;
		}

#if STM32UTIL_USE_OS
		osKernelUnlock();
#endif
	}

	return true;
}

void stm32util_hcms_reset()
{
	HAL_GPIO_WritePin(STM32UTIL_HCMS_RESET_PORT, STM32UTIL_HCMS_RESET_PIN, SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(STM32UTIL_HCMS_RESET_PORT, STM32UTIL_HCMS_RESET_PIN, RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(STM32UTIL_HCMS_RESET_PORT, STM32UTIL_HCMS_RESET_PIN, SET);
	HAL_Delay(10);
}

void stm32util_hcms_puts(const char* str)
{
	ASSERT(hcms_inited_, "");

	uint8_t* buffer = hcms_buffer_;

	for (int i = 0; i < STM32UTIL_HCMS_CHAR_PER_DEVICE; ++i, ++str) {
		memcpy(buffer, font5x7 + *str * WIDTH, WIDTH);
		buffer += WIDTH;
	}

#if STM32UTIL_HCMS_SPI_USE_DMA
	stm32util_hcms_dma_wait_tx();
	SCB_CleanDCache_by_Addr((uint32_t*)hcms_buffer_, HCMS_BUFFER_SIZE);
	stm32util_hcms_transmit_dma(hcms_buffer_, HCMS_BUFFER_SIZE);
	stm32util_hcms_dma_start_tx();
#else
	stm32util_hcms_transmit(hcms_buffer_, HCMS_BUFFER_SIZE);
#endif
}

#endif // STM32UTIL_HCMS_SPI_ENABLE
