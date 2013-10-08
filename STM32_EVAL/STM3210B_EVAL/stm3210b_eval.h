/**
  ******************************************************************************
  * @file    stm3210b_eval.h
  * @author  MCD Application Team
  * @version V3.1.2
  * @date    09/28/2009
  * @brief   This file contains definitions for STM3210B_EVAL's Leds, push-buttons
  *          and COM ports hardware resources.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM3210B_EVAL_H
#define __STM3210B_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM3210B_EVAL
  * @{
  */ 


/** @defgroup STM3210B_EVAL_Exported_Types
  * @{
  */ 
/**
  * @}
  */ 



/** @defgroup STM3210B_EVAL_Exported_Constants
  * @{
  */ 
/** @addtogroup STM3210B_EVAL_LED
  * @{
  */
#define LEDn                        4
#define LED1_GPIO_PORT              GPIOC
#define LED1_GPIO_CLK               RCC_APB2Periph_GPIOC  
#define LED1_GPIO_PIN               GPIO_Pin_6
  
#define LED2_GPIO_PORT              GPIOC
#define LED2_GPIO_CLK               RCC_APB2Periph_GPIOC  
#define LED2_GPIO_PIN               GPIO_Pin_7
  
#define LED3_GPIO_PORT              GPIOC
#define LED3_GPIO_CLK               RCC_APB2Periph_GPIOC  
#define LED3_GPIO_PIN               GPIO_Pin_8
  
#define LED4_GPIO_PORT              GPIOC
#define LED4_GPIO_CLK               RCC_APB2Periph_GPIOC  
#define LED4_GPIO_PIN               GPIO_Pin_9
/**
  * @}
  */ 
  
/** @addtogroup STM3210B_EVAL_BUTTON
  * @{
  */  
#define BUTTONn                     8

/**
 * @brief Wakeup push-button
 */
#define WAKEUP_BUTTON_PORT          GPIOA
#define WAKEUP_BUTTON_CLK           RCC_APB2Periph_GPIOA
#define WAKEUP_BUTTON_PIN           GPIO_Pin_0
#define WAKEUP_BUTTON_EXTI_LINE     EXTI_Line0
#define WAKEUP_BUTTON_PORT_SOURCE   GPIO_PortSourceGPIOA
#define WAKEUP_BUTTON_PIN_SOURCE    GPIO_PinSource0
#define WAKEUP_BUTTON_IRQn          EXTI0_IRQn 
/**
 * @brief Tamper push-button
 */
#define TAMPER_BUTTON_PORT          GPIOC
#define TAMPER_BUTTON_CLK           RCC_APB2Periph_GPIOC
#define TAMPER_BUTTON_PIN           GPIO_Pin_13
#define TAMPER_BUTTON_EXTI_LINE     EXTI_Line13
#define TAMPER_BUTTON_PORT_SOURCE   GPIO_PortSourceGPIOC
#define TAMPER_BUTTON_PIN_SOURCE    GPIO_PinSource13
#define TAMPER_BUTTON_IRQn          EXTI15_10_IRQn 
/**
 * @brief Key push-button
 */
#define KEY_BUTTON_PORT             GPIOB
#define KEY_BUTTON_CLK              RCC_APB2Periph_GPIOB
#define KEY_BUTTON_PIN              GPIO_Pin_9
#define KEY_BUTTON_EXTI_LINE        EXTI_Line9
#define KEY_BUTTON_PORT_SOURCE      GPIO_PortSourceGPIOB
#define KEY_BUTTON_PIN_SOURCE       GPIO_PinSource9
#define KEY_BUTTON_IRQn             EXTI9_5_IRQn
/**
 * @brief Joystick Right push-button
 */
#define RIGHT_BUTTON_PORT           GPIOE
#define RIGHT_BUTTON_CLK            RCC_APB2Periph_GPIOE
#define RIGHT_BUTTON_PIN            GPIO_Pin_0
#define RIGHT_BUTTON_EXTI_LINE      EXTI_Line0
#define RIGHT_BUTTON_PORT_SOURCE    GPIO_PortSourceGPIOE
#define RIGHT_BUTTON_PIN_SOURCE     GPIO_PinSource0
#define RIGHT_BUTTON_IRQn           EXTI0_IRQn
/**
 * @brief Joystick Left push-button
 */
#define LEFT_BUTTON_PORT            GPIOE
#define LEFT_BUTTON_CLK             RCC_APB2Periph_GPIOE
#define LEFT_BUTTON_PIN             GPIO_Pin_1
#define LEFT_BUTTON_EXTI_LINE       EXTI_Line1
#define LEFT_BUTTON_PORT_SOURCE     GPIO_PortSourceGPIOE
#define LEFT_BUTTON_PIN_SOURCE      GPIO_PinSource1
#define LEFT_BUTTON_IRQn            EXTI1_IRQn  
/**
 * @brief Joystick Up push-button
 */
#define UP_BUTTON_PORT              GPIOD
#define UP_BUTTON_CLK               RCC_APB2Periph_GPIOD
#define UP_BUTTON_PIN               GPIO_Pin_8
#define UP_BUTTON_EXTI_LINE         EXTI_Line8
#define UP_BUTTON_PORT_SOURCE       GPIO_PortSourceGPIOD
#define UP_BUTTON_PIN_SOURCE        GPIO_PinSource8
#define UP_BUTTON_IRQn              EXTI9_5_IRQn  
/**
 * @brief Joystick Down push-button
 */  
#define DOWN_BUTTON_PORT            GPIOD
#define DOWN_BUTTON_CLK             RCC_APB2Periph_GPIOD
#define DOWN_BUTTON_PIN             GPIO_Pin_14
#define DOWN_BUTTON_EXTI_LINE       EXTI_Line14
#define DOWN_BUTTON_PORT_SOURCE     GPIO_PortSourceGPIOD
#define DOWN_BUTTON_PIN_SOURCE      GPIO_PinSource14
#define DOWN_BUTTON_IRQn            EXTI15_10_IRQn  
/**
 * @brief Joystick Sel push-button
 */
#define SEL_BUTTON_PORT             GPIOD
#define SEL_BUTTON_CLK              RCC_APB2Periph_GPIOD
#define SEL_BUTTON_PIN              GPIO_Pin_12
#define SEL_BUTTON_EXTI_LINE        EXTI_Line12
#define SEL_BUTTON_PORT_SOURCE      GPIO_PortSourceGPIOD
#define SEL_BUTTON_PIN_SOURCE       GPIO_PinSource12
#define SEL_BUTTON_IRQn             EXTI15_10_IRQn   
/**
  * @}
  */ 

/** @addtogroup STM3210B_EVAL_COM
  * @{
  */
#define COMn                        2

/**
 * @brief Definition for COM port1, connected to USART1
 */ 
#define EVAL_COM1                   USART1
#define EVAL_COM1_GPIO              GPIOA
#define EVAL_COM1_CLK               RCC_APB2Periph_USART1
#define EVAL_COM1_GPIO_CLK          RCC_APB2Periph_GPIOA
#define EVAL_COM1_RxPin             GPIO_Pin_10
#define EVAL_COM1_TxPin             GPIO_Pin_9

/**
 * @brief Definition for COM port2, connected to USART2 (USART2 pins remapped on GPIOD)
 */ 
#define EVAL_COM2                   USART2
#define EVAL_COM2_GPIO              GPIOD
#define EVAL_COM2_CLK               RCC_APB1Periph_USART2
#define EVAL_COM2_GPIO_CLK          RCC_APB2Periph_GPIOD
#define EVAL_COM2_RxPin             GPIO_Pin_6
#define EVAL_COM2_TxPin             GPIO_Pin_5

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/** @defgroup STM3210B_EVAL_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup STM3210B_EVAL_Exported_Functions
  * @{
  */ 

/**
  * @}
  */ 
    
#ifdef __cplusplus
}
#endif
  
#endif /* __STM3210B_EVAL_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
