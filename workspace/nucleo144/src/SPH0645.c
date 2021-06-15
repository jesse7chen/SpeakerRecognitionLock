/*
 * SPH0645.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Communication with Adafruit I2S MEMS Microphone Breakout
 */
//
/* Includes ------------------------------------------------------------------*/
#include "SPH0645.h"
#include "events.h"
#include "microphone.h"
#include "stm32l4xx.h"
#include "stm32l4xx_nucleo_144.h"

/* Constants -----------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static SAI_HandleTypeDef m_MemsSaiHandler;
static DMA_HandleTypeDef m_MemsDmaHandler;

/* Private function prototypes -----------------------------------------------*/
static bool SAI1ClockInit(void);

bool SPH0645_Init(void){

    // Disable SAI peripheral

    bool success = false;

    success = SAI1ClockInit();

    // Just after enabling the clock for a peripheral,
    // software must wait for a delay before accessing the peripheral registers.
    HAL_Delay(5);

    if(success == true) {
        // Init SAI
        SPH0645_SAI_CLK_ENABLE();

        m_MemsSaiHandler.Instance = SPH0645_SAI;
        m_MemsSaiHandler.Init.AudioMode = SAI_MODEMASTER_RX;
        m_MemsSaiHandler.Init.Synchro = SAI_ASYNCHRONOUS;
        m_MemsSaiHandler.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
        m_MemsSaiHandler.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
        // Not using the MCLK for I2S
        m_MemsSaiHandler.Init.NoDivider = SAI_MASTERDIVIDER_DISABLE;
        m_MemsSaiHandler.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_HF;
        m_MemsSaiHandler.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
        // Not used
        m_MemsSaiHandler.Init.Mckdiv = 0;
        m_MemsSaiHandler.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
        m_MemsSaiHandler.Init.MonoStereoMode = SAI_MONOMODE;
        m_MemsSaiHandler.Init.CompandingMode = SAI_NOCOMPANDING;
        // Doesn't matter since we are not using as a transmitter
        m_MemsSaiHandler.Init.TriState = SAI_OUTPUT_NOTRELEASED;

        uint32_t numSlots = 2;

        // SPH0645LM4H has 24 bit, 2's complelement output with 18 bits of precision
        success = ( HAL_SAI_InitProtocol(&m_MemsSaiHandler, SAI_I2S_STANDARD,
                             SAI_PROTOCOL_DATASIZE_24BIT, numSlots) == HAL_OK );
    }

    if(success == true) {
        // Config DMA
        __HAL_RCC_DMAMUX1_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();

        m_MemsDmaHandler.Instance = SPH0645_DMA_INSTANCE;
        // Map the request to SAI1_A
        m_MemsDmaHandler.Init.Request = DMA_REQUEST_SAI1_A;
        m_MemsDmaHandler.Init.Direction = DMA_PERIPH_TO_MEMORY;
        m_MemsDmaHandler.Init.PeriphInc = DMA_PINC_DISABLE;
        m_MemsDmaHandler.Init.MemInc = DMA_MINC_ENABLE;
        m_MemsDmaHandler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        m_MemsDmaHandler.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        m_MemsDmaHandler.Init.Mode = DMA_NORMAL;
        m_MemsDmaHandler.Init.Priority = SPH0645_DMA_CHANNEL_PRIORITY;
        // De-init before init
        HAL_DMA_DeInit(&m_MemsDmaHandler);
        success = ( HAL_DMA_Init(&m_MemsDmaHandler) == HAL_OK );

        // Associate the DMA handle
        __HAL_LINKDMA(&m_MemsSaiHandler, hdmarx, m_MemsDmaHandler);
    }

    if(success == true) {
        // Config NVIC
        HAL_NVIC_SetPriority(SPH0645_SAI_IRQn, SPH0645_SAI_PRIORITY, SPH0645_SAI_SUBPRIORITY);
        HAL_NVIC_EnableIRQ(SPH0645_SAI_IRQn);

        HAL_NVIC_SetPriority(SPH0645_DMA_IRQn, SPH0645_DMA_PRIORITY, SPH0645_DMA_SUBPRIORITY);
        HAL_NVIC_EnableIRQ(SPH0645_DMA_IRQn);
    }

    return success;
}


HAL_StatusTypeDef SPH0645_StartRecord(uint8_t *buff, uint16_t size) {
    return HAL_SAI_Receive_DMA(&m_MemsSaiHandler, buff, size);
}

HAL_StatusTypeDef SPH0645_StopRecord(void) {
    return HAL_SAI_DMAStop(&m_MemsSaiHandler);
}

void SPH0645_SAIRxCpltCallback(SAI_HandleTypeDef *hsai) {
    Mic_RxCompleteCallback();
}

SAI_HandleTypeDef* SPH0645_GetSAIHandle(void) {
    return &m_MemsSaiHandler;
}

DMA_HandleTypeDef* SPH0645_GetDMAHandle(void) {
    return &m_MemsDmaHandler;
}

static bool SAI1ClockInit(void) {
    // Set clock config
    // Since we want 16 kHz sampling with 64 bit word select period (even though
    // active frame is only 32 bits), need clock of least 16kHz * 64 bits * 2 = 2.048 MHz
    
    // Due to internal resynchronization stages, PCLK APB frequency must be higher than twice
    // the bit rate clock frequency.

    // Init SAI1 PLL clock
    RCC_PeriphCLKInitTypeDef periphClkConfig = {0};
    HAL_RCCEx_GetPeriphCLKConfig(&periphClkConfig);

    periphClkConfig.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    periphClkConfig.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI1;
    periphClkConfig.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI; // MSI is at 4MHz
    periphClkConfig.PLLSAI1.PLLSAI1M = 1;
    periphClkConfig.PLLSAI1.PLLSAI1N = 64; // VCO output = 4 * (N/M) = 256 MHz
    periphClkConfig.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV5; // SAI clock input = 128 MHz / 5 = 51.2 MHz
    // 51.2 MHz is a multiple of 2.048 MHz so this should work
    periphClkConfig.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_SAI1CLK; // Enable the SAI1 clock out

    return (HAL_RCCEx_PeriphCLKConfig(&periphClkConfig) == HAL_OK);

}
