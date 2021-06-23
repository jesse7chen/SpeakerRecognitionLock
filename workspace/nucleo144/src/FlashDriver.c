/*
 * FlashDriver.c
 *
 *      Author: jessechen
 *      Brief:
 *          - This file provides set of firmware functions to manage:
 *          - On-chip flash interface
 */
//
/* Includes ------------------------------------------------------------------*/
#include "error.h"
#include "FlashDriver.h"
#include "stm32l4xx_nucleo_144.h"

/* Constants -----------------------------------------------------------------*/
#define UNPROGRAMMED_FLASH ( 0xFFFFFFFF )
#define PROGRAMMED_FLASH ( 0x00000000 )

/* Private variables ---------------------------------------------------------*/
extern uint32_t __USER_SECTION_START;
extern FLASH_ProcessTypeDef pFlash;
static uint32_t* m_TrainedFlagPtr = &__USER_SECTION_START;

/* Private function prototypes -----------------------------------------------*/


bool FlashDriver_GetTrainedFlag(void) {
    return (*m_TrainedFlagPtr != UNPROGRAMMED_FLASH);
}

bool FlashDriver_SetTrainedFlag(void) {
    bool success = true;

    success = (HAL_FLASH_Unlock() == HAL_OK);

    if (success == true) {
        success = (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t )m_TrainedFlagPtr,
                                    PROGRAMMED_FLASH) == HAL_OK);
    }

    if (success == true) {
        success = (HAL_FLASH_Lock() == HAL_OK);
    }

    return success;
}

uint32_t FlashDriver_GetErrorCode(void) {
    return pFlash.ErrorCode;
}
