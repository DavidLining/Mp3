/*
*********************************************************************************************************
*
*	模块名称 : 主程序模块。
*	文件名称 : main.c
*	版    本 : V2.0
*	说    明 : MP3硬件解码例程(SDIO+FatFS+VS1003B)。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-27 armfly  ST固件库V3.5.0版本。
*		v2.0    2011-10-16 armfly  优化工程结构。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*	   
	!!!注意：该例程可以在CPU Flash和CPU RAM中调试，暂时不支持在外部RAM调试。

	这个例程中的函数用到了较多的局部变量，因此缺省的堆栈不够用。
	需要调整大一些。
	
	修改 startup_stm32f10x_hd.s 文件
	以前为 ：Stack_Size      EQU     0x00000400
	现在为 ：Stack_Size      EQU     0x00001000
*/

/*
	操作说明：
	(1) 请接上串口线，打开windows的串口工具，比如超级终端。
	(2) 输入字符1，打印SD根目录下的文件和文件夹列表。
	(3) 输入字符2，在SD卡根目录下创建一个文件，文件名为 armfly.txt
		并且向该文件写入一个字符串。
	(4) 输入字符3，打开SD卡根目录下的armfly.txt文件，读出其内容，打印到串口。
	(5) 输入字符4，创建打开SD卡根目录下的armfly.txt文件，读出其内容，打印到串口。
	(6) 输入字符5，写文件和读文件速度测试
*/

#include "stm32f10x.h"		/* 如果要用ST的固件库，必须包含这个文件 */
#include <stdio.h>			/* 因为用到了printf函数，所以必须包含这个文件 */
#include "bsp_usart.h"		/* printf函数定向输出到串口，所以必须包含这个文件 */
#include "bsp_led.h"		/* LED指示灯驱动模块 */
#include "bsp_button.h"		/* 按键驱动模块 */
#include "bsp_timer.h"		/* systick定时器模块 */
#include "bsp_sdio_sd.h"  	/* SD卡驱动模块 */
#include "bsp_tft_lcd.h"	/* TFT液晶显示器驱动模块 */
#include "ff.h"				/* FatFS文件系统模块*/	
#include "mp3_player.h"	/* MP3播放器模块*/
#include "images.h"	
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"MP3硬件解码例程(SDIO+FatFS+VS1003B)"
#define EXAMPLE_DATE	"2011-10-16"
#define DEMO_VER		"2.0"
	

/* 仅允许本文件内调用的函数声明 */
static void InitBoard(void);
static void PrintfLogo(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参：无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	/*
		由于ST固件库的启动文件已经执行了CPU系统时钟的初始化，所以不必再次重复配置系统时钟。
		启动文件配置了CPU主时钟频率、内部Flash访问速度和可选的外部SRAM FSMC初始化。

		系统时钟缺省配置为72MHz，如果需要更改，可以修改：
		\Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\system_stm32f10x.c
		中配置系统时钟的宏。
	*/

	InitBoard();	/* 为了是main函数看起来更简洁些，我们将硬件初始化的代码封装到这个函数 */
	PrintfLogo();	/* 打印例程Logo到串口1 */
	SDIO_Interrupts_Config();	/* 配置SDIO中断， 此函数在bsp_sdio_sd.c */
	Mp3Player();	/* MP3主程序 */
}

/*
*********************************************************************************************************
*	函 数 名: InitBoard
*	功能说明: 初始化硬件设备
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitBoard(void)
{
	/* 配置串口，用于printf输出 */
	bsp_InitUart();

	/* 配置LED指示灯GPIO */
	bsp_InitLed();

	/* 配置按键GPIO, 必须在bsp_InitTimer之前调用 */
	bsp_InitButton();
 
	/* 初始化systick定时器，并启动定时中断 */
	bsp_InitTimer();
}





/*
*********************************************************************************************************
*	函 数 名: PrintfLogo
*	功能说明: 打印例程名称和例程发布日期, 接上串口线后，打开PC机的超级终端软件可以观察结果
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void PrintfLogo(void)
{
	printf("*************************************************************\n\r");
	printf("* 例程名称   : %s\r\n", EXAMPLE_NAME);	/* 打印例程名称 */
	printf("* 例程版本   : %s\r\n", DEMO_VER);		/* 打印例程版本 */
	printf("* 发布日期   : %s\r\n", EXAMPLE_DATE);	/* 打印例程日期 */

	/* 打印ST固件库版本，这3个定义宏在stm32f10x.h文件中 */
	printf("* 固件库版本 : %d.%d.%d\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("* \n\r");	/* 打印一行空格 */
	printf("* QQ    : 1295744630 \r\n");
	printf("* Email : armfly@qq.com \r\n");
	printf("* Copyright www.armfly.com 安富莱电子\r\n");
	printf("*************************************************************\n\r");
}
