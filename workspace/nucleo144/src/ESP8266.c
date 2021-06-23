/*
 * ESP8266.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - Communication with Feather ESP8266 over SPI
 */
//
/* Includes ------------------------------------------------------------------*/
#include "ESP8266.h"
#include "events.h"
#include "microphone.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#include "stm32l4xx_nucleo_144.h"
#include <string.h>

/* Constants -----------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef m_ESP8266SpiHandler;

/* Private function prototypes -----------------------------------------------*/
static void AssertInterruptPin(void);
static void ClearInterruptPin(void);

// NodeMCU Feather ESP8266 only supports SPI master functionality, so STM32
// will have to act as a SPI slave in this case

bool ESP8266_Init(void){
    // Init SPI pins
    GPIO_InitTypeDef gpio_init;

    // Init interrupt pin
    ESP8266_INT_GPIO_CLK_ENABLE();
    ClearInterruptPin();
    gpio_init.Pin = ESP8266_INT_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(ESP8266_INT_GPIO_PORT, &gpio_init);

    // Init SPI
    ESP8266_SPI3_CLK_ENABLE();

    // Initialize SPI handler
    m_ESP8266SpiHandler.Instance = ESP8266_SPI3;
    m_ESP8266SpiHandler.Init.Mode = SPI_MODE_SLAVE;
    m_ESP8266SpiHandler.Init.Direction = SPI_DIRECTION_2LINES;
    m_ESP8266SpiHandler.Init.DataSize = SPI_DATASIZE_8BIT;
    m_ESP8266SpiHandler.Init.CLKPolarity = SPI_POLARITY_LOW;
    m_ESP8266SpiHandler.Init.CLKPhase = SPI_PHASE_1EDGE;
    m_ESP8266SpiHandler.Init.NSS = SPI_NSS_HARD_INPUT;
    m_ESP8266SpiHandler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    m_ESP8266SpiHandler.Init.FirstBit = SPI_FIRSTBIT_MSB;
    m_ESP8266SpiHandler.Init.TIMode = SPI_TIMODE_DISABLE;
    m_ESP8266SpiHandler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    m_ESP8266SpiHandler.Init.CRCPolynomial = 7;
    m_ESP8266SpiHandler.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    m_ESP8266SpiHandler.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    return (HAL_SPI_Init(&m_ESP8266SpiHandler) == HAL_OK);
}

// HAL_SPI_Transmit_IT treats third argument as length or number of units of data
// to send, where a uint of data is the DataSize set during init
bool ESP8266_TransmitData(uint8_t* txData, uint16_t length){
    // Enable interrupt line
    AssertInterruptPin();
    BSP_LED_On(LED1);
    return (HAL_SPI_Transmit_IT(&m_ESP8266SpiHandler, (uint8_t*)txData, length) == HAL_OK);
}

bool ESP8266_ReceiveData(uint8_t* rxData, uint16_t length){
    return (HAL_SPI_Receive_IT(&m_ESP8266SpiHandler, (uint8_t*)rxData, length) == HAL_OK);
}

void ESP8266_SpiRxCpltCallback(SPI_HandleTypeDef *hspi){
    Event_Set(EVENT_SERVER_DATA_RX);
}

void ESP8266_SpiTxCpltCallback(SPI_HandleTypeDef *hspi){
    // Disable interrupt line
    BSP_LED_Off(LED1);
    ClearInterruptPin();
    Event_Set(EVENT_AUDIO_PKT_SENT);
}

SPI_HandleTypeDef* ESP8266_GetSpiHandle(void){
    return &m_ESP8266SpiHandler;
}

static void AssertInterruptPin(void){
    HAL_GPIO_WritePin(ESP8266_INT_GPIO_PORT, ESP8266_INT_PIN, GPIO_PIN_RESET);
}

static void ClearInterruptPin(void){
    HAL_GPIO_WritePin(ESP8266_INT_GPIO_PORT, ESP8266_INT_PIN, GPIO_PIN_SET);
}
