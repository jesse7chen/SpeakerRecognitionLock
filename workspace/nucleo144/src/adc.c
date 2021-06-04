/*
 * adc.c
 *
 *  Created on: March 6, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - The ADC peripheral of the STM32L4R5xx chip
 */
//

/* Includes ------------------------------------------------------------------*/
#include "adc.h"
#include "error.h"
#include "microphone.h"
#include "stm32l4xx_nucleo_144.h"


/* Defines ------------------------------------------------------------*/
#define VDDA_APPLI                     ((uint32_t) 3300)    /* Value of analog voltage supply Vdda (unit: mV) */
#define RANGE_12BITS                   ((uint32_t) 4095)    /* Max digital value with a full range of 12 bits */
/* Internal temperature sensor: constants data used for indicative values in  */
/* this example. Refer to device datasheet for min/typ/max values.            */
/* For more accurate values, device should be calibrated on offset and slope  */
/* for application temperature range.                                         */
#define INTERNAL_TEMPSENSOR_V30        ((int32_t)760)           /* Internal temperature sensor, parameter V25 (unit: mV). Refer to device datasheet for min/typ/max values. */
#define INTERNAL_TEMPSENSOR_AVGSLOPE   ((int32_t)2500)          /* Internal temperature sensor, parameter Avg_Slope (unit: uV/DegCelsius). Refer to device datasheet for min/typ/max values. */
#define TEMP30_CAL_ADDR   ((uint16_t*) ((uint32_t)0x1FFF75A8))  /* Internal temperature sensor, parameter TS_CAL1: TS ADC raw data acquired at a temperature of 30 DegC (+-5 DegC) */
#define TEMP110_CAL_ADDR  ((uint16_t*) ((uint32_t)0x1FFF75CA))  /* Internal temperature sensor, parameter TS_CAL2: TS ADC raw data acquired at a temperature of  110 DegC (+-5 DegC) */
#define VDDA_TEMP_CAL                  ((uint32_t)3000)        /* Vdda value with which temperature sensor has been calibrated in production (+-10 mV). */

#define CAL_DATA_SIZE ((uint32_t)  64)
#define CAL_TIMEOUT ((uint32_t) 10UL)

#define COMPUTE_VREF_INT(VREF_RAW_DATA)                        \
  ( (VREFINT_CAL_VREF * (*VREFINT_CAL_ADDR)) / (VREF_RAW_DATA))

#define COMPUTE_ADC_12BIT_TO_MV(ADC_DATA)                        \
  ( ((ADC_DATA) * m_VrefInt) / RANGE_12BITS)

/* Statics/Globals------------------------------------------------------------*/
static ADC_HandleTypeDef m_AdcHandler;
/* Fast channels are: PC0, PC1, PC2, PC3, PA0. */
static ADC_ChannelConfTypeDef m_AdcChanConfig;
static DMA_HandleTypeDef m_AdcDmaHandle;

static uint16_t m_CalData[CAL_DATA_SIZE];

/* Not sure if I should make this a float or not considering the computation resources that may cost */
static uint16_t m_VrefInt = 0;
static volatile uint8_t m_VrefDone = RESET;
static uint32_t m_AdcStartTicks = 0;
static uint32_t m_TimeTaken = 0;
static ADC_MODE_T m_CurrMode = ADC_MODE_CALIBRATE;

/* Private Functions----------------------------------------------------------*/
static HAL_StatusTypeDef ChangeADCMode(ADC_MODE_T mode);

HAL_StatusTypeDef ADC_Init(void){
  // Initialize ADC handler
    m_AdcHandler.Instance = NUCLEO_ADCx;

    // Assert that we de-init HAL_ADC correctly
    assert_param(HAL_ADC_DeInit(&m_AdcHandler) == HAL_OK);

    m_AdcHandler.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; /* We choose a synchronous clock in msp_init and we choose /4*/
    m_AdcHandler.Init.Resolution = ADC_RESOLUTION_12B; // Highest possible resolution
    /* The register that holds our conversion results is 16 bits wide (ADC_DR)
     * Right justified means that MSBs are set to zero (bits 15-12)
     * Left justified means that LSBs are set to zero (bits 0-3)
     * Left justified could be useful if you just wanted to grab the most significant byte and din't need the extra two
     * bytes of precision
     * Right justified is probably better for our needs since we want all 12 bits of precision and don't want to scale,
     * which you would have to do if you wanted the entire value from a left justified register */
    m_AdcHandler.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    /* Seems like this enables/disables sequence scanning, which allows for automatic sequential scanning of multiple channels */
    m_AdcHandler.Init.ScanConvMode = DISABLE;
    /* Choose single conversion End of Conversion flag since sequencer is not enabled */
    m_AdcHandler.Init.EOCSelection =  ADC_EOC_SINGLE_CONV;
    /* Supposed to be used with polling systems. Basically waits to start new conversion until previous conversion has been retrieved.
     * This will just lead to stale data if I use it though */
    m_AdcHandler.Init.LowPowerAutoWait = DISABLE;
    /* Automatically restart conversion after each conversion */
    m_AdcHandler.Init.ContinuousConvMode = ENABLE;
    /* Only converting on one channel for now */
    m_AdcHandler.Init.NbrOfConversion = 1;
    /* Specifies (if we have a sequence) if we want to split sequencer conversion into multiple successive parts */
    m_AdcHandler.Init.DiscontinuousConvMode = DISABLE;
    /* Not even using this parameter, but it's the number of discontinuous conversions that the sequencer will be subdivided into */
    m_AdcHandler.Init.NbrOfDiscConversion = 1;
    /* We want FW to trigger conversion start */
    m_AdcHandler.Init.ExternalTrigConv =  ADC_SOFTWARE_START;
    /* We have a software trigger so the trigger edge doesn't matter */
    m_AdcHandler.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    /* Select whether DMA requests are done in a single shot or if the DMA transfer is unlimited (DMA must be in circular mode) */
    m_AdcHandler.Init.DMAContinuousRequests = ENABLE;
    /* Data should be overwritten with last conversion result in case of overrun since we don't want stale data
     * We do always want the latest data, though ideally there shouldn't be any overrun.
     * Since we are in DMA mode, an error is reported for whatever overrun setting since DMA is expected to process all data from register */
    m_AdcHandler.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    /* Oversampler can do data pre-processing to offload CPU, such as averaging, SNR improvement, and filtering. Won't use it for now, but
     * might want to enable it in the future. Can't see any reason to want to do this though */
    m_AdcHandler.Init.OversamplingMode = DISABLE;
    /* m_AdcHandler.Init.Oversampling = insert structure here if you wanted to actually do oversampling */


   return (HAL_ADC_Init(&m_AdcHandler));
}


HAL_StatusTypeDef ADC_InitChannels(void){
    HAL_StatusTypeDef retVal = HAL_OK;

  /* Configure microphone ADC channel */
    m_AdcChanConfig.Channel = NUCLEO_ADCx_CHANNEL;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_1;
    /* Sample every x ADC clock cycles - seems like fastest sampling rate will prevent SPI BLE from working */
    m_AdcChanConfig.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    //m_AdcChanConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    /* Channel is single ended */
    m_AdcChanConfig.SingleDiff = ADC_SINGLE_ENDED;
    /* We don't want a channel offset, the microphone bias should fit within the ~3.3V range of the ADC */
    m_AdcChanConfig.OffsetNumber = ADC_OFFSET_NONE;
    m_AdcChanConfig.Offset = 0;

    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);
    /* Todo: Add sequencer settings to ADC */
  /* Configure internal reference voltage channel */
    m_AdcChanConfig.Channel = ADC_CHANNEL_VREFINT;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_2;
    m_AdcChanConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;

    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    m_AdcChanConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_3;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    return retVal;
}

HAL_StatusTypeDef ADC_StartDma(uint32_t *buff, uint32_t len){
    m_AdcStartTicks = HAL_GetTick();
    return HAL_ADC_Start_DMA(&m_AdcHandler, buff, len);
}

HAL_StatusTypeDef ADC_StopDma(void){
    return HAL_ADC_Stop_DMA(&m_AdcHandler);
}

HAL_StatusTypeDef ADC_CalibrateVRefInt(void){
  uint32_t tickstart;
  uint8_t i = 0;
  uint32_t vref_sum = 0;
  uint32_t temp_sum = 0;

  /* Should never try to calibrate while user is actively recording audio - these asserts are probably extreme
   * Would do a softer failure for production code of course */
  // assert_param((ADC_IS_ENABLE(&m_AdcHandler)) == RESET);

  // m_AdcChanConfig.Channel = ADC_CHANNEL_VREFINT;
  // m_AdcChanConfig.Rank = ADC_REGULAR_RANK_1;
  // /* Choose slowest sampling time so we can catch variation across m_VrefInt if any */
  // m_AdcChanConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  //
  // assert_param(HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig) == HAL_OK);
  //
  // m_AdcChanConfig.Rank = ADC_REGULAR_RANK_2;
  // assert_param(HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig) == HAL_OK);

  assert_param(ChangeADCMode(ADC_MODE_CALIBRATE) == HAL_OK);

  m_VrefDone = RESET;

  assert_param(HAL_ADC_Start_DMA(&m_AdcHandler, (uint32_t*)m_CalData, CAL_DATA_SIZE) == HAL_OK);

  // if (HAL_ADC_PollForEvent(&m_AdcHandler, ADC_EOSMP_EVENT, 10) != HAL_OK){
  //   /* Must have timed out for some reason */
  //   HAL_ADC_Stop_DMA(&m_AdcHandler);
  //   return HAL_TIMEOUT;
  // }

 /* If calibration doesn't finish by timeout period, return error */
  tickstart = HAL_GetTick();
  while(m_VrefDone != SET){
    if (((HAL_GetTick() - tickstart) > CAL_TIMEOUT) || (CAL_TIMEOUT == 0UL)) {
      return HAL_TIMEOUT;
    }
  }

  HAL_ADC_Stop_DMA(&m_AdcHandler);
  m_VrefDone = RESET;

  /* Average out values in buffer to get m_VrefInt (oversampler can do this for us but seems unnecessary
   * if this doesn't occur often and CPU isn't doing anything else */
  for(; i < CAL_DATA_SIZE; i++){
    if(i%2){
      temp_sum += m_CalData[i];
    }
    else{
      vref_sum += m_CalData[i];
    }
  }

  m_VrefInt = COMPUTE_VREF_INT(vref_sum/CAL_DATA_SIZE);

  /* Change back to record mode when done, to microphone module it should seem like nothing has changed */
  assert_param(ChangeADCMode(ADC_MODE_RECORD) == HAL_OK);
  return HAL_OK;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    m_TimeTaken = HAL_GetTick() - m_AdcStartTicks;

    if(m_CurrMode == ADC_MODE_CALIBRATE){
        /* Report to main program that DMA has filled array */
        m_VrefDone = SET;
    }
    else if(m_CurrMode == ADC_MODE_RECORD){
        Mic_ConvCompleteCallback(hadc);
    }
}

// Todo: Implement more specialized error handling here
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc){
    Error_Handler();
}

/* This is expected to be called after ADC init */
static HAL_StatusTypeDef ChangeADCMode(ADC_MODE_T mode){
  HAL_StatusTypeDef retVal = HAL_OK;

  if(mode < ADC_MODE_MIN || mode >= ADC_MODE_MAX){
      Error_Handler();
  }

  m_CurrMode = mode;

  /* Assert that ADC is disabled */
  assert_param((ADC_IS_ENABLE(&m_AdcHandler)) == RESET);
  // Assert that we de-init HAL_ADC correctly
  assert_param(HAL_ADC_DeInit(&m_AdcHandler) == HAL_OK);

  if (mode == ADC_MODE_RECORD){
    /* Change relevant ADC parameters */
    m_AdcHandler.Init.ScanConvMode = DISABLE;
    m_AdcHandler.Init.NbrOfConversion = 1;

    assert_param(HAL_ADC_Init(&m_AdcHandler) == HAL_OK);

    /* Change relevant ADC channel parameters */
    m_AdcChanConfig.Channel = NUCLEO_ADCx_CHANNEL;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_1;
    m_AdcChanConfig.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    // VrefInt has min sampling time of 4 us
    m_AdcChanConfig.Channel = ADC_CHANNEL_VREFINT;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_2;
    m_AdcChanConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    m_AdcChanConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_3;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    retVal |= HAL_ADCEx_Calibration_Start(&m_AdcHandler, ADC_SINGLE_ENDED);

  }
  else if (mode == ADC_MODE_CALIBRATE){
    /* Change relevant ADC parameters */
    m_AdcHandler.Init.ScanConvMode = ENABLE;
    m_AdcHandler.Init.NbrOfConversion = 2;

    assert_param(HAL_ADC_Init(&m_AdcHandler) == HAL_OK);

    /* Change relevant ADC channel parameters */
    m_AdcChanConfig.Channel = NUCLEO_ADCx_CHANNEL;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_3;
    m_AdcChanConfig.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    m_AdcChanConfig.Channel = ADC_CHANNEL_VREFINT;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_1;
    m_AdcChanConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    m_AdcChanConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    m_AdcChanConfig.Rank = ADC_REGULAR_RANK_2;
    retVal |= HAL_ADC_ConfigChannel(&m_AdcHandler, &m_AdcChanConfig);

    retVal |= HAL_ADCEx_Calibration_Start(&m_AdcHandler, ADC_SINGLE_ENDED);
  }
  else{
    /* Didn't put in a supported mode, return error */
    return HAL_ERROR;
  }

  return retVal;
}

ADC_HandleTypeDef* ADC_GetHandle(void){
    return &m_AdcHandler;
}

DMA_HandleTypeDef* ADC_GetDmaHandle(void){
    return &m_AdcDmaHandle;
}
