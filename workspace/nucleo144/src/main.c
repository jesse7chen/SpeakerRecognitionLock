/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx.h"
#include "stm32l4xx_nucleo_144.h"
#include "bluetooth.h"
#include <stdio.h>

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define AUDIO_DATA_BUFFER_SIZE ((uint32_t)  32)   /* Size of array aADCxConvertedData[] */
#define VREF ((double)3.2) /* Empirically tested with a voltage supply - probably should compare to internal vref */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef spi_h;
static char test_string[] = "Test string\r\n";
ADC_HandleTypeDef adc_h;
ADC_ChannelConfTypeDef adc_chan_conf;

/* Contains ADC data from microphone */
static uint16_t   audio_data[AUDIO_DATA_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/

static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while (1)
  {
  }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{

  /* STM32L4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the System clock to have a frequency of 120 MHz */
  SystemClock_Config();


  /* Add your application code here
     */
  // Configure LED1
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED3);

// Initialize ADC handler
  adc_h.Instance = NUCLEO_ADCx;

  if (HAL_ADC_DeInit(&adc_h) != HAL_OK)
  {
    /* ADC de-initialization Error */
    Error_Handler();
  }

  adc_h.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2; /* We choose a synchronous clock in msp_init and we choose /2 because
  that's the fastest we can do unless system clock is already prescaled by 2*/
  adc_h.Init.Resolution = ADC_RESOLUTION_12B; // Highest possible resolution
  /* The register that holds our conversion results is 16 bits wide (ADC_DR)
   * Right justified means that MSBs are set to zero (bits 15-12)
   * Left justified means that LSBs are set to zero (bits 0-3)
   * Left justified could be useful if you just wanted to grab the most significant byte and din't need the extra two
   * bytes of precision
   * Right justified is probably better for our needs since we want all 12 bits of precision and don't want to scale,
   * which you would have to do if you wanted the entire value from a left justified register */
  adc_h.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  /* Seems like this enables/disables sequence scanning, which allows for automatic sequential scanning of multiple channels */
  adc_h.Init.ScanConvMode = DISABLE;
  /* Choose single conversion End of Conversion flag since sequencer is not enabled */
  adc_h.Init.EOCSelection =  ADC_EOC_SINGLE_CONV;
  /* Supposed to be used with polling systems. Basically waits to start new conversion until previous conversion has been retrieved.
   * This will just lead to stale data if I use it though */
  adc_h.Init.LowPowerAutoWait = DISABLE;
  /* Automatically restart conversion after each conversion */
  adc_h.Init.ContinuousConvMode = ENABLE;
  /* Only converting on one channel for now */
  adc_h.Init.NbrOfConversion = 1;
  /* Specifies (if we have a sequence) if we want to split sequencer conversion into multiple successive parts */
  adc_h.Init.DiscontinuousConvMode = DISABLE;
  /* Not even using this parameter, but it's the number of discontinuous conversions that the sequencer will be subdivided into */
  adc_h.Init.NbrOfDiscConversion = 1;
  /* We want FW to trigger conversion start */
  adc_h.Init.ExternalTrigConv =  ADC_SOFTWARE_START;
  /* We have a software trigger so the trigger edge doesn't matter */
  adc_h.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  /* Select whether DMA requests are done in a single shot or if the DMA transfer is unlimited (DMA must be in circular mode) */
  adc_h.Init.DMAContinuousRequests = ENABLE;
  /* Data should be overwritten with last conversion result in case of overrun since we don't want stale data
   * We do always want the latest data, though ideally there shouldn't be any overrun.
   * Since we are in DMA mode, an error is reported for whatever overrun setting since DMA is expected to process all data from register */
  adc_h.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  /* Oversampler can do data pre-processing to offload CPU, such as averaging, SNR improvement, and filtering. Won't use it for now, but
   * might want to enable it in the future. Can't see any reason to want to do this though */
  adc_h.Init.OversamplingMode = DISABLE;
  /* adc_h.Init.Oversampling = insert structure here if you wanted to actually do oversampling */

// Initialize SPI handler
  spi_h.Instance = SPI1;
  // Specify if board is master or slave
  spi_h.Init.Mode = SPI_MODE_MASTER;
  // Selecting full-duplex communication
  spi_h.Init.Direction = SPI_DIRECTION_2LINES;
  // Select dataframe size to be 8 bits
  spi_h.Init.DataSize = SPI_DATASIZE_8BIT;
  // CLK polarity determines IDLE state of SCLK when no data is being transferred
  spi_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  // Capture data on first edge (default config) of SCLK instead of second edge
  spi_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  /* Choose slave select pin to be driven by software, specifically by SSI bit value in SPIx_CR1 - leaves external pin free for other uses?
   * Can also do hardware configurations to have it output enable (master only) or output disable (which allows multi-master capabilities) */
  spi_h.Init.NSS = SPI_NSS_SOFT;
  /* For writing to BLE chip, SCLK should be less than 4MHz. System CLK is configured to 120 MHz, and both APB prescalers are 1, so SPI fpclk should also be 120 MHz.
   * Set prescaler to 32 for 3.75 MHz SCLK  */
  spi_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  // Write MSB first, as BLE chip requires that
  spi_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  /* Setting this forces the config settings to conform to the Texas Instruments protocol requirements.
   * I guess by default we are in Motorola SPI communication mode, though not necessarily, as these two modes just refer to different clock polarity/phase settings */
  spi_h.Init.TIMode = SPI_TIMODE_DISABLE;
  // Don't think we need CRC calculations?
  spi_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  // CRC Polynomial value is what is used to calculate the CRC
  spi_h.Init.CRCPolynomial = 7;
  // Setting CRC length, not that it matters
  spi_h.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  /* NSS (slave select) Pulse mode - designed for applications with a single master-slave pair. Allows slave to latch data by generating an NSS pulse
  * between two consecutive data frames. Doesn't matter as we have software NSS */
  spi_h.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  if(HAL_ADC_Init(&adc_h) != HAL_OK){
	  /* Initialization error */
	  Error_Handler();
  }

  /* Calibrate ADC */
  if (HAL_ADCEx_Calibration_Start(&adc_h, ADC_SINGLE_ENDED) != HAL_OK){
	  Error_Handler();
  }

  /* Configure ADC channels */
  adc_chan_conf.Channel = NUCLEO_ADCx_CHANNEL;
  adc_chan_conf.Rank = ADC_REGULAR_RANK_1;
  /* Sample every 2.5 ADC clock cycles - this is the fastest possible time */
  adc_chan_conf.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
  /* Channel is single ended */
  adc_chan_conf.SingleDiff = ADC_SINGLE_ENDED;
  /* We don't want a channel offset, the microphone bias should fit within the ~3.6V range of the ADC */
  adc_chan_conf.OffsetNumber = ADC_OFFSET_NONE;
  adc_chan_conf.Offset = 0;

  if (HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf) != HAL_OK){
	  Error_Handler();
  }

  if(HAL_SPI_Init(&spi_h) != HAL_OK)
  {
    /* Initialization Error */
    // For some reason this still initializes correctly if spi_h.Instance is not defined...
    Error_Handler();
  }

  bleInit(&spi_h);


  /* */
  /* Start ADC reads */
	if (HAL_ADC_Start_DMA(&adc_h, (uint32_t*)audio_data, AUDIO_DATA_BUFFER_SIZE) != HAL_OK){
		Error_Handler();
	}

  /* Infinite loop */
  while (1)
  {

	// Blink LED1
	BSP_LED_On(LED1);
	bleWriteUART(test_string, sizeof(test_string));
	HAL_Delay(500);

//	if (HAL_ADC_Stop_DMA(&adc_h) != HAL_OK){
//	  Error_Handler();
//	}

	BSP_LED_Off(LED1);
	HAL_Delay(500);
  }
}
/* End of main function */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 120000000
  *            HCLK(Hz)                       = 120000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 60
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable voltage range 1 boost mode for frequency above 80 Mhz */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  __HAL_RCC_PWR_CLK_DISABLE();

  /* Enable MSI Oscillator and activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* To avoid undershoot due to maximum frequency, select PLL as system clock source */
  /* with AHB prescaler divider 2 as first step */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* AHB prescaler divider at 1 as second step */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
