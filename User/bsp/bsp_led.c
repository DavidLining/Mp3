/*
*********************************************************************************************************
*	                                  
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V2.0
*	说    明 : 驱动LED指示灯
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-01-11 armfly  ST固件库升级到V3.4.0版本。
*       v2.0    2011-10-16 armfly  ST固件库升级到V3.5.0版本。
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_led.h"

/*
	安富莱STM32F103ZE-EK开发板LED指示灯口线分配：
	LED1 : PF6  低电平点亮
	LED2 : PF7  低电平点亮	
	LED3 : PF8  低电平点亮
	LED4 : PF9  低电平点亮		
*/
#define GPIO_PORT_LED1	GPIOF
#define GPIO_PORT_LED2	GPIOF
#define GPIO_PORT_LED3	GPIOF
#define GPIO_PORT_LED4	GPIOF

#define GPIO_PIN_LED1	GPIO_Pin_6
#define GPIO_PIN_LED2	GPIO_Pin_7
#define GPIO_PIN_LED3	GPIO_Pin_8
#define GPIO_PIN_LED4	GPIO_Pin_9

#define GPIO_CLK_LED1	RCC_APB2Periph_GPIOF
#define GPIO_CLK_LED2	RCC_APB2Periph_GPIOF
#define GPIO_CLK_LED3	RCC_APB2Periph_GPIOF
#define GPIO_CLK_LED4	RCC_APB2Periph_GPIOF

/*
*********************************************************************************************************
*	函 数 名: bsp_InitLed
*	功能说明: 初始化LED指示灯
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#if 1	/* 采用宏定义的方式初始化GPIO，以便于修改GPIO口线 */
	/* 打开GPIOF的时钟 */
	RCC_APB2PeriphClockCmd(GPIO_CLK_LED1 | GPIO_CLK_LED2 | GPIO_CLK_LED3 | GPIO_CLK_LED4, ENABLE);

	/* 配置所有的LED指示灯GPIO为推挽输出模式 */
	/* 由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动LED点亮
		这是我不希望的，因此在改变GPIO为输出前，先修改输出寄存器的值为1 */
	GPIO_SetBits(GPIO_PORT_LED1,  GPIO_PIN_LED1);
	GPIO_SetBits(GPIO_PORT_LED2,  GPIO_PIN_LED2);
	GPIO_SetBits(GPIO_PORT_LED3,  GPIO_PIN_LED3);
	GPIO_SetBits(GPIO_PORT_LED4,  GPIO_PIN_LED4);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1;
	GPIO_Init(GPIO_PORT_LED1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED2;
	GPIO_Init(GPIO_PORT_LED2, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED3;
	GPIO_Init(GPIO_PORT_LED3, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED4;
	GPIO_Init(GPIO_PORT_LED4, &GPIO_InitStructure);
#else
	/* 打开GPIOF的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);

	/* 配置所有的LED指示灯GPIO为推挽输出模式 */
	/* 由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动LED点亮
		这是我不希望的，因此在改变GPIO为输出前，先修改输出寄存器的值为1 */
	GPIO_SetBits(GPIOF,  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
#endif
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 点亮指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->BRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BRR = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BRR = GPIO_PIN_LED3;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED4->BRR = GPIO_PIN_LED4;
	}		
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOff
*	功能说明: 熄灭指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->BSRR = GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->BSRR = GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->BSRR = GPIO_PIN_LED3;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED4->BSRR = GPIO_PIN_LED4;
	}		
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedToggle
*	功能说明: 翻转指定的LED指示灯。
*	形    参：_no : 指示灯序号，范围 1 - 4
*	返 回 值: 按键代码
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
	_no--;
	
	if (_no == 0)
	{
		GPIO_PORT_LED1->ODR ^= GPIO_PIN_LED1;
	}
	else if (_no == 1)
	{
		GPIO_PORT_LED2->ODR ^= GPIO_PIN_LED2;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED3->ODR ^= GPIO_PIN_LED3;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED4->ODR ^= GPIO_PIN_LED4;
	}		
}
