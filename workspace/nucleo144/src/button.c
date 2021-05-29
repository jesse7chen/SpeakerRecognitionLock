/*
 * button.c
 *
 *  Created on: March 7, 2020
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Button interfaces
 */
//
/* Includes ------------------------------------------------------------------*/
#include "button.h"
#include "events.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_nucleo_144.h"

/* Constants -----------------------------------------------------------------*/
const static uint8_t REQUIRED_CONSECUTIVE_COUNTS = 8;

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef m_buttonTmr;
static TIM_ClockConfigTypeDef m_buttonTmrCfg;
static bool m_debounceFlag = false;
static uint16_t m_consecutiveCounts = 0;
static bool m_lastPressedValue = false;

/* Private function prototypes -----------------------------------------------*/
static void HandleDebounce(void);

bool Button_Init(void){
    bool success = true;

    /* Configure button as external interrupt generator */
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

    // Configure debounce timer
    m_buttonTmr.Instance = TIM2;
    m_buttonTmr.Init.Prescaler = ((uint32_t)SystemCoreClock/ (uint32_t)10000) - (uint32_t)1;
    m_buttonTmr.Init.CounterMode = TIM_COUNTERMODE_UP;
    m_buttonTmr.Init.Period = 10;
    m_buttonTmr.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    m_buttonTmrCfg.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_Base_Init(&m_buttonTmr) != HAL_OK){
        success = false;
    }
    else{
        if(HAL_TIM_ConfigClockSource(&m_buttonTmr, &m_buttonTmrCfg) != HAL_OK){
            success = false;
        }
    }

    return success;
}

void Button_DebounceCallback(void){
    if(m_debounceFlag == false)
    {
        m_debounceFlag = true;
        HAL_TIM_Base_Start_IT(&m_buttonTmr);
    }
}

TIM_HandleTypeDef* Button_GetDebounceTmrHandle(void){
    return &m_buttonTmr;
}

/**
  * @brief  Period elapsed callback in non-blocking mode
  * @param  htim TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim == &m_buttonTmr){
      HandleDebounce();
  }
}

/**
  * @brief  Initializes the TIM Base MSP.
  * @param  htim TIM Base handle
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim == &m_buttonTmr)
    {
        // Enable TIM2 periperhal clock
        __HAL_RCC_TIM2_CLK_ENABLE();

        // Set TIM2 interrupt priority
        HAL_NVIC_SetPriority(TIM2_IRQn, BUTTON_DEBOUNCE_PRIORITY, BUTTON_DEBOUNCE_SUB_PRIORITY);

        // Enable TIM2 interrupt
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

static void HandleDebounce(void){
    GPIO_PinState pinState =
        HAL_GPIO_ReadPin(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN);

    bool pressed = (pinState == GPIO_PIN_RESET) ? true : false;

    if (pressed == m_lastPressedValue){
        m_consecutiveCounts++;
    }
    else{
        m_consecutiveCounts = 0;
        m_lastPressedValue = pressed;
    }

    if(m_consecutiveCounts > REQUIRED_CONSECUTIVE_COUNTS){
        m_consecutiveCounts = 0;
        m_debounceFlag = false;
        // If button was pressed, set event
        if (pressed == true){
            Event_Set(EVENT_USER_BUTTON_PRESS);
        }
        HAL_TIM_Base_Stop_IT(&m_buttonTmr);
    }
}
