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
  ( ((ADC_DATA) * vref_int) / RANGE_12BITS)

/* Statics/Globals------------------------------------------------------------*/
ADC_HandleTypeDef adc_h;
/* Fast channels are: PC0, PC1, PC2, PC3, PA0. */
static ADC_ChannelConfTypeDef adc_chan_conf;
static uint16_t   cal_data[CAL_DATA_SIZE];

/* Not sure if I should make this a float or not considering the computation resources that may cost */
static uint16_t vref_int = 0;
static volatile uint8_t  vref_done = RESET;

/* This is expected to be called after ADC init */
HAL_StatusTypeDef changeADCMode(uint8_t mode){
  HAL_StatusTypeDef retVal = HAL_OK;


  /* Assert that ADC is disabled */
  assert_param((ADC_IS_ENABLE(&adc_h)) == RESET);
  // Assert that we de-init HAL_ADC correctly
  assert_param(HAL_ADC_DeInit(&adc_h) == HAL_OK);

  if (mode == RECORD_MODE){
    /* Change relevant ADC parameters */
    adc_h.Init.ScanConvMode = DISABLE;
    adc_h.Init.NbrOfConversion = 1;

    assert_param(HAL_ADC_Init(&adc_h) == HAL_OK);

    /* Change relevant ADC channel parameters */
    adc_chan_conf.Channel = NUCLEO_ADCx_CHANNEL;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_1;
    adc_chan_conf.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    adc_chan_conf.Channel = ADC_CHANNEL_VREFINT;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_2;
    adc_chan_conf.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    adc_chan_conf.Channel = ADC_CHANNEL_TEMPSENSOR;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_3;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    retVal |= HAL_ADCEx_Calibration_Start(&adc_h, ADC_SINGLE_ENDED);

  }
  else if (mode == CALIBRATE_MODE){
    /* Change relevant ADC parameters */
    adc_h.Init.ScanConvMode = ENABLE;
    adc_h.Init.NbrOfConversion = 2;

    assert_param(HAL_ADC_Init(&adc_h) == HAL_OK);

    /* Change relevant ADC channel parameters */
    adc_chan_conf.Channel = NUCLEO_ADCx_CHANNEL;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_3;
    adc_chan_conf.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    adc_chan_conf.Channel = ADC_CHANNEL_VREFINT;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_1;
    adc_chan_conf.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    adc_chan_conf.Channel = ADC_CHANNEL_TEMPSENSOR;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_2;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    retVal |= HAL_ADCEx_Calibration_Start(&adc_h, ADC_SINGLE_ENDED);


  }
  else{
    /* Didn't put in a supported mode, return error */
    return HAL_ERROR;
  }

  return retVal;

}

HAL_StatusTypeDef calibrateVRefInt(void){
  uint32_t tickstart;
  uint8_t i = 0;
  uint32_t vref_sum = 0;
  uint32_t temp_sum = 0;

  /* Should never try to calibrate while user is actively recording audio - these asserts are probably extreme
   * Would do a softer failure for production code of course */
  // assert_param((ADC_IS_ENABLE(&adc_h)) == RESET);

  // adc_chan_conf.Channel = ADC_CHANNEL_VREFINT;
  // adc_chan_conf.Rank = ADC_REGULAR_RANK_1;
  // /* Choose slowest sampling time so we can catch variation across Vref_int if any */
  // adc_chan_conf.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  //
  // assert_param(HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf) == HAL_OK);
  //
  // adc_chan_conf.Rank = ADC_REGULAR_RANK_2;
  // assert_param(HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf) == HAL_OK);

  assert_param(changeADCMode(CALIBRATE_MODE) == HAL_OK);

  vref_done = RESET;

  assert_param(HAL_ADC_Start_DMA(&adc_h, (uint32_t*)cal_data, CAL_DATA_SIZE) == HAL_OK);

  // if (HAL_ADC_PollForEvent(&adc_h, ADC_EOSMP_EVENT, 10) != HAL_OK){
  //   /* Must have timed out for some reason */
  //   HAL_ADC_Stop_DMA(&adc_h);
  //   return HAL_TIMEOUT;
  // }

 /* If calibration doesn't finish by timeout period, return error */
  tickstart = HAL_GetTick();
  while(vref_done != SET){
    if (((HAL_GetTick() - tickstart) > CAL_TIMEOUT) || (CAL_TIMEOUT == 0UL)) {
      return HAL_TIMEOUT;
    }
  }

  HAL_ADC_Stop_DMA(&adc_h);
  vref_done = RESET;

  /* Average out values in buffer to get Vref_int (oversampler can do this for us but seems unnecessary
   * if this doesn't occur often and CPU isn't doing anything else */
  for(; i < CAL_DATA_SIZE; i++){
    if(i%2){
      temp_sum += cal_data[i];
    }
    else{
      vref_sum += cal_data[i];
    }
  }

  vref_int = COMPUTE_VREF_INT(vref_sum/CAL_DATA_SIZE);

  /* Change back to record mode when done, to microphone module it should seem like nothing has changed */
  assert_param(changeADCMode(RECORD_MODE) == HAL_OK);
  return HAL_OK;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
  /* Report to main program that DMA has filled array */
  vref_done = SET;
}


HAL_StatusTypeDef adcChannelsInit(void){
    HAL_StatusTypeDef retVal = HAL_OK;

  /* Configure microphone ADC channel */
    adc_chan_conf.Channel = NUCLEO_ADCx_CHANNEL;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_1;
    /* Sample every x ADC clock cycles - seems like fastest sampling rate will prevent SPI BLE from working */
    adc_chan_conf.SamplingTime = NUCLEO_ADCx_SAMPLETIME;
    //adc_chan_conf.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
    /* Channel is single ended */
    adc_chan_conf.SingleDiff = ADC_SINGLE_ENDED;
    /* We don't want a channel offset, the microphone bias should fit within the ~3.3V range of the ADC */
    adc_chan_conf.OffsetNumber = ADC_OFFSET_NONE;
    adc_chan_conf.Offset = 0;

    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);
    /* Todo: Add sequencer settings to ADC */
  /* Configure internal reference voltage channel */
    adc_chan_conf.Channel = ADC_CHANNEL_VREFINT;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_2;
    adc_chan_conf.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;

    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    adc_chan_conf.Channel = ADC_CHANNEL_TEMPSENSOR;
    adc_chan_conf.Rank = ADC_REGULAR_RANK_3;
    retVal |= HAL_ADC_ConfigChannel(&adc_h, &adc_chan_conf);

    return retVal;
}

HAL_StatusTypeDef adcInit(void){
  // Initialize ADC handler
    adc_h.Instance = NUCLEO_ADCx;

    // Assert that we de-init HAL_ADC correctly
    assert_param(HAL_ADC_DeInit(&adc_h) == HAL_OK);

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


   return (HAL_ADC_Init(&adc_h));
}
