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
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef hspi;

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

// Initialize SPI handler
  hspi.Instance = SPI1;
  // Specify if board is master or slave
  hspi.Init.Mode = SPI_MODE_MASTER;
  // Selecting full-duplex communication
  hspi.Init.Direction = SPI_DIRECTION_2LINES;
  // Select dataframe size to be 8 bits
  hspi.Init.DataSize = SPI_DATASIZE_8BIT;
  // CLK polarity determines IDLE state of SCLK when no data is being transferred
  hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
  // Capture data on first edge (default config) of SCLK instead of second edge
  hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
  /* Choose slave select pin to be driven by software, specifically by SSI bit value in SPIx_CR1 - leaves external pin free for other uses?
   * Can also do hardware configurations to have it output enable (master only) or output disable (which allows multi-master capabilities) */
  hspi.Init.NSS = SPI_NSS_SOFT;
  /* For writing to BLE chip, SCLK should be less than 4MHz. System CLK is configured to 120 MHz, and both APB prescalers are 1, so SPI fpclk should also be 120 MHz.
   * Set prescaler to 32 for 3.75 MHz SCLK  */
  hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  // Write MSB first, as BLE chip requires that
  hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  /* Setting this forces the config settings to conform to the Texas Instruments protocol requirements.
   * I guess by default we are in Motorola SPI communication mode, though not necessarily, as these two modes just refer to different clock polarity/phase settings */
  hspi.Init.TIMode = SPI_TIMODE_DISABLE;
  // Don't think we need CRC calculations?
  hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  // CRC Polynomial value is what is used to calculate the CRC
  hspi.Init.CRCPolynomial = 7;
  // Setting CRC length, not that it matters
  hspi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  /* NSS (slave select) Pulse mode - designed for applications with a single master-slave pair. Allows slave to latch data by generating an NSS pulse
  * between two consecutive data frames. Doesn't matter as we have software NSS */
  hspi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

  if(HAL_SPI_Init(&hspi) != HAL_OK)
  {
    /* Initialization Error */
    // For some reason this still initializes correctly if hspi.Instance is not defined...
    Error_Handler();
  }

  bleInit(&hspi);

  /* Infinite loop */
  while (1)
  {
	  // Blink LED1
	  BSP_LED_On(LED1);
	  bleWriteUART("Tests\r\n", 7);
	  HAL_Delay(500);
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
