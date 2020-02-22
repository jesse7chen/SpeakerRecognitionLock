/**
  ******************************************************************************
  * @file    stm32l4xx_hal_msp_template.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
  *          This file template is located in the HAL folder and should be copied
  *          to the user folder.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo_144.h"

/** @addtogroup STM32L4xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP HAL MSP module driver
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi){
	// I'm guessing that we want to define this function in user code because SPI configuration will be different depending on what interrupt priorities, polarities, etc that user wants

	GPIO_InitTypeDef gpio_init;

	 /* (##) Enable the SPIx interface clock
	  (##) SPI pins configuration
		  (+++) Enable the clock for the SPI GPIOs
		  (+++) Configure these SPI pins as alternate function push-pull
	  (##) NVIC configuration if you need to use interrupt process
		  (+++) Configure the SPIx interrupt priority
		  (+++) Enable the NVIC SPI IRQ handle
	  (##) DMA Configuration if you need to use DMA process
		  (+++) Declare a DMA_HandleTypeDef handle structure for the transmit or receive Stream/Channel
		  (+++) Enable the DMAx clock
		  (+++) Configure the DMA handle parameters
		  (+++) Configure the DMA Tx or Rx Stream/Channel
		  (+++) Associate the initialized hdma_tx(or _rx)  handle to the hspi DMA Tx or Rx handle
		  (+++) Configure the priority and enable the NVIC for the transfer complete interrupt on the DMA Tx or Rx Stream/Channel */

// Configure clk for SPI pins
	// Enable clock SCLK pin
	NUCLEO_SPIx_SCK_GPIO_CLK_ENABLE();
	// Enable clock for MISO/MOSI block (same block for SPIA)
	NUCLEO_SPIx_MISO_MOSI_GPIO_CLK_ENABLE();
	// Enable clock for NSS pin
	NUCLEO_SPIx_CS_GPIO_CLK_ENABLE();

	// Enable SPI CLK
	NUCLEO_SPIx_CLK_ENABLE();

// Configure GPIO config for SPI CLK pin
	gpio_init.Pin = NUCLEO_SPIx_SCK_PIN;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	// Assume we should do pulldown as IDLE state of clock is low
	gpio_init.Pull = GPIO_PULLDOWN;
	// For speeds from 25-50 MHz, should definitely be high enough
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
	// This actually took a lot of digging to figure out - look at notebook for notes
	gpio_init.Alternate = NUCLEO_SPIx_SCK_AF;
	HAL_GPIO_Init(NUCLEO_SPIx_SCK_GPIO_PORT, &gpio_init);

// Configure GPIO config for SPI MISO pin, can reuse most of the same fields
	gpio_init.Pin = NUCLEO_SPIx_MISO_PIN;
	gpio_init.Alternate = NUCLEO_SPIx_MISO_MOSI_AF;
	gpio_init.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(NUCLEO_SPIx_MISO_MOSI_GPIO_PORT, &gpio_init);

// Configure GPIO config for SPI MOSI pin
	gpio_init.Pin = NUCLEO_SPIx_MOSI_PIN;
	gpio_init.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(NUCLEO_SPIx_MISO_MOSI_GPIO_PORT, &gpio_init);

// Configure GPIO config for SPI NSS pin. This is literally just a GPIO pin, not an actual hardware controlled NSS pin
	gpio_init.Pin = NUCLEO_SPIx_CS_PIN;
	gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
	// Pretty sure that CS is generally active low
	gpio_init.Pull = GPIO_PULLUP;
	// Doesn't check this parameter if Mode isn't set to alternate, so just set to 0x00
	gpio_init.Alternate = 0x00;
	HAL_GPIO_Init(NUCLEO_SPIx_CS_GPIO_PORT, &gpio_init);

// todo: NVIC config to be done later

}

/**
  * @brief  Initialize the Global MSP.
  * @param  None
  * @retval None
  */
void HAL_MspInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually
            modified by the user
   */
}

/**
  * @brief  DeInitialize the Global MSP.
  * @param  None
  * @retval None
  */
void HAL_MspDeInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually
            modified by the user
   */
}

/**
  * @brief  Initialize the PPP MSP.
  * @param  None
  * @retval None
  */
void HAL_PPP_MspInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually
            modified by the user
   */
}

/**
  * @brief  DeInitialize the PPP MSP.
  * @param  None
  * @retval None
  */
void HAL_PPP_MspDeInit(void)
{
  /* NOTE : This function is generated automatically by STM32CubeMX and eventually
            modified by the user
   */
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
