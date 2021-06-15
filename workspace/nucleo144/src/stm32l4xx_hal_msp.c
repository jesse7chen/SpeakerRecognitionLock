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
#include "adc.h"
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


void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc){
    GPIO_InitTypeDef gpio_init;
    /* 2/27
     * Forgot to make this static and spent a solid 2 hours trying to debug why the ADC wasn't working
     * In the future, I should probably check if any handler goes out of scope */
    DMA_HandleTypeDef* adc_dma = ADC_GetDmaHandle();

    // Enable GPIO clock
    NUCLEO_ADCx_GPIO_CLK_ENABLE();

    // Enable core ADC clock
    NUCLEO_ADCx_CLK_ENABLE();
    // Configure the core ADC clock to be the sysclk
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);

    // Enable DMA clocks
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();

    // Initialize GPIO
    gpio_init.Pin = NUCLEO_ADCx_GPIO_PIN;
    // Not the same as GPIO_MODE_ANALOG
    gpio_init.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    gpio_init.Pull = GPIO_NOPULL;
    // Speed shouldn't matter since it's not output, just set to 0
    gpio_init.Speed = 0x00;
    // Alternate shouldn't matter since ADC is the pin's main function
    gpio_init.Alternate = 0x00;
    HAL_GPIO_Init(NUCLEO_ADCx_GPIO_PORT, &gpio_init);

    // Configure DMA
    adc_dma->Instance = NUCELO_ADC_DMA_INSTANCE;
    // Map the request to ADC1
    adc_dma->Init.Request = DMA_REQUEST_ADC1;
    adc_dma->Init.Direction = DMA_PERIPH_TO_MEMORY;
    // No need to increase peripheral memory address, just want to read the one ADC
    adc_dma->Init.PeriphInc = DMA_PINC_DISABLE;
    // Probably want to increase the memory address so we can read multiple values
    adc_dma->Init.MemInc = DMA_MINC_ENABLE;
    // Highest resolution of ADC is 12 bits, so the data width of each transfer can be a halfword (16 bits)
    adc_dma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc_dma->Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    // Circular mode is available to handle circular buffers and continuous data flows (such as ADC scan mode)
    adc_dma->Init.Mode = DMA_NORMAL;
    adc_dma->Init.Priority = NUCLEO_ADC_DMA_CHANNEL_PRIORITY;
    // De-init before init
    HAL_DMA_DeInit(adc_dma);
    HAL_DMA_Init(adc_dma);

    /* Associate the DMA handle */
    /* Documentation doesn't really say anything about this, good thing it's in the examples.
     * Pretty much just a macro which does hadc-->DMA_Handle = &(adc_dma) and adc_dma-->Parent = &(hadc)
     * Links the peripherpal and the DMA handler. */
    __HAL_LINKDMA(hadc, DMA_Handle, *adc_dma);

    /* NVIC configuration for DMA Input data interrupt */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, ADC_DMA_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

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

    if(hspi->Instance == NUCLEO_SPIx){
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
    else if(hspi->Instance == ESP8266_SPI3){
        // Config SPI pins
        ESP8266_SPI3_SCK_GPIO_CLK_ENABLE();
        gpio_init.Pin = ESP8266_SPI3_SCK_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = ESP8266_SPI3_SCK_AF;
        HAL_GPIO_Init(ESP8266_SPI3_SCK_GPIO_PORT, &gpio_init);

        // Pulldown put on MISO line to avoid floating line drawing excessive current
        ESP8266_SPI3_MISO_GPIO_CLK_ENABLE();
        gpio_init.Pin = ESP8266_SPI3_MISO_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_PULLDOWN;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = ESP8266_SPI3_MISO_AF;
        HAL_GPIO_Init(ESP8266_SPI3_MISO_GPIO_PORT, &gpio_init);

        ESP8266_SPI3_MOSI_GPIO_CLK_ENABLE();
        gpio_init.Pin = ESP8266_SPI3_MOSI_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = ESP8266_SPI3_MOSI_AF;
        HAL_GPIO_Init(ESP8266_SPI3_MOSI_GPIO_PORT, &gpio_init);

        ESP8266_SPI3_CS_GPIO_CLK_ENABLE();
        gpio_init.Pin = ESP8266_SPI3_CS_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = ESP8266_SPI3_CS_AF;
        HAL_GPIO_Init(ESP8266_SPI3_CS_GPIO_PORT, &gpio_init);

        // Config NVIC
        HAL_NVIC_SetPriority(ESP8266_SPI3_IRQn, ESP8266_SPI3_PRIORITY, ESP8266_SPI3_SUBPRIORITY);
        HAL_NVIC_EnableIRQ(ESP8266_SPI3_IRQn);
    }
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai) {
    GPIO_InitTypeDef gpio_init;

    if(hsai->Instance == SPH0645_SAI) {
        // Init SAI pins
        SPH0645_SAI_SCK_GPIO_CLK_ENABLE();
        gpio_init.Pin = SPH0645_SAI_SCK_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = SPH0645_SAI_SCK_AF;
        HAL_GPIO_Init(SPH0645_SAI_SCK_GPIO_PORT, &gpio_init);

// From SPH0645 datasheet
// When operating a single microphone on an I2S bus, a pull down resistor (100K Ohms)
// should be placed from the Data pin to ground to insure the bus capacitance is discharged.
        SPH0645_SAI_SD_GPIO_CLK_ENABLE();
        gpio_init.Pin = SPH0645_SAI_SD_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_PULLDOWN;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = SPH0645_SAI_SD_AF;
        HAL_GPIO_Init(SPH0645_SAI_SD_GPIO_PORT, &gpio_init);

        SPH0645_SAI_FS_GPIO_CLK_ENABLE();
        gpio_init.Pin = SPH0645_SAI_FS_PIN;
        gpio_init.Mode = GPIO_MODE_AF_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_HIGH;
        gpio_init.Alternate = SPH0645_SAI_FS_AF;
        HAL_GPIO_Init(SPH0645_SAI_FS_GPIO_PORT, &gpio_init);
    }

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
