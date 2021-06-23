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
#include "fsm_evt_queue.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_hal_tim.h"
#include "stm32l4xx_nucleo_144.h"

/* Private typedef -----------------------------------------------------------*/

typedef struct BUTTON_INFO_T {
    GPIO_TypeDef* gpioPort;
    uint32_t gpioPin;
    FSM_EVT_ID_T event;
    uint16_t consecutiveCounts;
    bool lastPressedValue;
    bool debounceNeeded;
} BUTTON_INFO_T;

/* Constants -----------------------------------------------------------------*/
static const uint8_t REQUIRED_CONSECUTIVE_COUNTS = 8;

/* Private variables ---------------------------------------------------------*/
static TIM_HandleTypeDef m_buttonTmr;
static TIM_ClockConfigTypeDef m_buttonTmrCfg;

static BUTTON_INFO_T m_buttons[BUTTON_MAX] = {
    [BUTTON_USER] = {
        .gpioPort = USER_BUTTON_GPIO_PORT,
        .gpioPin = USER_BUTTON_PIN,
        .event = FSM_EVT_USER_BUTTON_PRESS,
        .consecutiveCounts = 0,
        .lastPressedValue = false,
        .debounceNeeded = false,
    },
    [BUTTON_LOCK] = {
        .gpioPort = LOCK_BUTTON_GPIO_PORT,
        .gpioPin = LOCK_BUTTON_PIN,
        .event = FSM_EVT_LOCK_BUTTON_PRESS,
        .consecutiveCounts = 0,
        .lastPressedValue = false,
        .debounceNeeded = false,
    }
};

/* Private function prototypes -----------------------------------------------*/
static void HandleDebounce(void);

bool Button_Init(void){
    bool success = true;

    /* Configure buttons as external interrupt generator */
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
    BSP_PB_Init(BUTTON_LOCK, BUTTON_MODE_EXTI);

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

void Button_DebounceCallback(Button_TypeDef button){
    m_buttons[button].debounceNeeded = true;
    // Doesn't seem to be any impact if this is called while timer is already started
    HAL_TIM_Base_Start_IT(&m_buttonTmr);
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
        HAL_NVIC_SetPriority(TIM2_IRQn, BUTTON_DEBOUNCE_PRIORITY, BUTTON_DEBOUNCE_SUBPRIORITY);

        // Enable TIM2 interrupt
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

static void HandleDebounce(void){
    bool allButtonsDebounced = true;

    for(uint8_t i = 0; i < BUTTON_MAX; i++) {
        if(m_buttons[i].debounceNeeded == true) {
            GPIO_PinState pinState =
                HAL_GPIO_ReadPin(m_buttons[i].gpioPort, m_buttons[i].gpioPin);

            bool pressed = (pinState == GPIO_PIN_RESET) ? true : false;

            if (pressed == m_buttons[i].lastPressedValue){
                m_buttons[i].consecutiveCounts++;
            }
            else{
                m_buttons[i].consecutiveCounts = 0;
                m_buttons[i].lastPressedValue = pressed;
            }

            if(m_buttons[i].consecutiveCounts > REQUIRED_CONSECUTIVE_COUNTS){
                m_buttons[i].consecutiveCounts = 0;
                m_buttons[i].debounceNeeded = false;
                // If button was pressed, set fsm event
                if (pressed == true) {

                    FSM_EVT_T event = {
                        .id = m_buttons[i].event,
                        .size = 0,
                        .data = NULL
                    };

                    FSM_EVT_QUEUE_Push(event);
                }
            }
            else {
                allButtonsDebounced = false;
            }
        }
    }

    if(allButtonsDebounced == true) {
        // Todo: Does timer need to be reset?
        HAL_TIM_Base_Stop_IT(&m_buttonTmr);
    }
}
