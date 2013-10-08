/*
*********************************************************************************************************
*
*	模块名称 : MP3播放器主程序。
*	文件名称 : main.c
*	版    本 : V2.0
*	说    明 : MP3硬件解码播放器主程序, SDIO+FatFS+VS1003B.
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-27 armfly  ST固件库V3.5.0版本。
*		v2.0    2011-10-16 armfly  优化工程结构。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "bsp_timer.h"
#include "bsp_button.h"
#include "bsp_tft_lcd.h"
#include "bsp_led.h"
#include "bsp_VS1003B.h"
#include "bsp_sdio_sd.h"
#include "ff.h"
#include "mp3_player.h"
#include "images.h"	

#define LINE_CAP	18		/* 定义行间距 */

#define STR_Title	"MP3播放器Made By LiNing"
#define STR_Help1	"【OK】键           =  暂停/继续"
#define STR_Help2	"【USER】键         =  静音"
#define STR_Help3	"【WAKEUP】键       =  从头开始"
#define STR_Help4 "【TAMPER】键       =  Delete" 
#define STR_Help5	"摇杆上/下键        =  调节音量"
#define STR_Help6 "摇杆左/右键        =  Last/Next"
#define STR_OpenSDErr	"打开SD卡文件系统失败"
#define STR_OpenRootErr	"打开SD卡根目录失败"
#define STR_OpenFileErr	"打开失败,请将mp3文件放到SD卡根目录"

MP3_T g_tMP3;

/* 访问Fatfs用到的全局变量 */
FATFS   g_fs;
DIR     g_DirInf;
FILINFO g_FileInf;
FIL 	g_File;
FRESULT g_Result;
static TCHAR Music_Name[99][(_MAX_LFN+1)*2];
DWORD Music_Length[99];
uint8_t Music_Num=0;
int8_t  Select_Index=0;
uint8_t ucRefresh;	/* 刷新LCD请求 */
static void DispTitle(void);
static void DispHelp(void);
static void DispStatus(int8_t Select_Index);
static void DispError(char *_str);
static void Mp3Play(int8_t Select_Index);
static void Mp3Pro(void);
static void DispPicDemo(void);
static void SoftReset(void);
/*
*********************************************************************************************************
*	函 数 名: Mp3Player
*	功能说明: MP3播放器主程序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void Mp3Player(void)
{
	LCD_InitHard();		/* 显示器初始化 */
  
	LCD_SetBackLight(BRIGHT_DEFAULT);  /* 设置背光亮度 */
   
	DispTitle();	/* 显示标题 */

	DispHelp();		/* 显示帮助信息 */

	vs1003_Init();
	vs1003_SoftReset();

	g_tMP3.ucVolume = 40; 			/* 缺省音量,越大声音越小 */
	vs1003_SetVolume(g_tMP3.ucVolume);

  g_Result=f_mount(0, &g_fs);	/* Mount a logical drive */
  if(g_Result!=FR_OK)
		{
			DispError(STR_OpenSDErr);
    }
	else
	{
		if(f_opendir(&g_DirInf,"/" ) == FR_OK)//打开根目录
	{
		while (f_readdir(&g_DirInf, &g_FileInf) == FR_OK && Music_Num<99)//读取目录中的文件属性
		{
      if( !g_FileInf.fname[0] )	 /* 文件名为空即到达了目录的末尾，退出 */
			break;  	
			if (g_FileInf.fattrib & AM_ARC) //归档文件0x20
			{
				if(g_FileInf.lfname[0] == NULL && g_FileInf.fname !=NULL) /*当长文件名称为空，短文件名非空时转换*/
					g_FileInf.lfname =g_FileInf.fname;				
				if(strstr(g_FileInf.fname,".mp3")||strstr(g_FileInf.fname,".MP3"))
				{
					uint8_t Len;
					Len = strlen(g_FileInf.fname);
					//将歌曲名字写入Music_Name数组中
					memcpy(Music_Name[Music_Num++],g_FileInf.fname,Len);
					Music_Length[Music_Num-1]=g_FileInf.fsize;
				}
			}
		}
	}
 }
	/* 进入主程序循环体 */
  Mp3Play(Select_Index);
 	#if 0
	/* 关闭文件*/
	f_close(&g_File);

	/* 卸载文件系统 */
	f_mount(0, NULL);
	#endif
}

/*软件复位*/
static void SoftReset(void)
{
__set_FAULTMASK(1); // 关闭所有中端
NVIC_SystemReset();// 复位
}

/*跑马灯程序*/
static void Led_Test(void)
{
	uint8_t i=0;
	while(i<5){
		bsp_LedOn(1);
	  bsp_DelayMS(100);
	  bsp_LedOff(1);
	  bsp_DelayMS(100);
	  bsp_LedOn(2);
	  bsp_DelayMS(100);
	  bsp_LedOff(2);
	  bsp_DelayMS(100);
	  bsp_LedOn(3);
	  bsp_DelayMS(100);
	  bsp_LedOff(3);
	  bsp_DelayMS(100);
	  bsp_LedOn(4);
	  bsp_DelayMS(100);
	  bsp_LedOff(4);
    bsp_DelayMS(100);
		i++;
  }		
}
/*
*********************************************************************************************************
*	函 数 名: DispTitle
*	功能说明: 显示标题行
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispTitle(void)
{
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */

/*	LCD_ClrScr(CL_BLUE);  		清屏，背景蓝色 */
  DispPicDemo();
	/* 设置字体属性 */
	tFont.usFontCode = FC_ST_16X16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.usTextColor = CL_WHITE;		/* 字体颜色设置为白色 */
  tFont.usBackColor = CL_MASK;	 	/* 文字背景颜色，蓝色 */
	tFont.usSpace = 0;

	LCD_DispStr(100, 0, STR_Title, &tFont);
	LCD_DrawBMP(336, 50, 64, 64, (uint16_t *)gIcon_QQ);
	LCD_DrawBMP(0, 50, 64, 64, (uint16_t *)gIcon_Bat);
}

/*显示背景图片*/
static void DispPicDemo(void)
{

	LCD_DrawBMP(0, 0, 240, 400, (uint16_t *)gImage_1);
}
/*
*********************************************************************************************************
*	函 数 名: DispHelp
*	功能说明: 显示帮助信息
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispHelp(void)
{
	uint16_t y;
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */

	/* 设置字体属性 */
	tFont.usFontCode = FC_ST_16X16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.usTextColor = CL_WHITE;		/* 字体颜色设置为白色 */
	tFont.usBackColor = CL_MASK;	 	/* 文字背景颜色， */
	tFont.usSpace = 0;

	y = LINE_CAP * 2; 	/* 行间距(单位：像素) */

	LCD_DispStr(72, y, STR_Help1, &tFont);
	y += LINE_CAP;

	LCD_DispStr(72, y, STR_Help2, &tFont);
	y += LINE_CAP;

	LCD_DispStr(72, y, STR_Help3, &tFont);
	y += LINE_CAP;

	LCD_DispStr(72, y, STR_Help4, &tFont);
	y += LINE_CAP;
	
	LCD_DispStr(72, y, STR_Help5, &tFont);
	y += LINE_CAP;
	
	LCD_DispStr(72, y, STR_Help6, &tFont);
	y += LINE_CAP;

}

/*
*********************************************************************************************************
*	函 数 名: DispStatus
*	功能说明: 显示当前状态
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

static void DispStatus(int8_t Select_Index)
{
	uint16_t y, x1, x2;
	char buf[80];
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */
	/* 设置字体属性 */
	tFont.usFontCode = FC_ST_16X16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.usTextColor = CL_WHITE;		/* 字体颜色设置为红色 */
	tFont.usBackColor = CL_BLUE;	 	/* 文字背景颜色 */
	tFont.usSpace = 0;

	/* 画一个矩形框, 黄色 */
	LCD_DrawRect(10, LINE_CAP * 10 + 12, 48, 400 - 20, CL_YELLOW);

	x1 = 20;	/* 状态栏第1列X坐标 */
	x2 = 200;	/* 状态栏第2列X坐标 */
	y = LINE_CAP * 11;

	if (g_tMP3.ucPauseEn == 0)
	{
		LCD_DispStr(x1, y, "播放 = 正在播放", &tFont);
	}
	else
	{
		LCD_DispStr(x1, y, "播放 = 暂停播放", &tFont);
	}

	sprintf((char *)buf, "音量 = %d ", 100 - (g_tMP3.ucVolume * 100) / VOLUME_MAX);
	LCD_DispStr(x2, y, buf, &tFont);

	y += LINE_CAP;

	if (g_tMP3.ucMuteOn == 1)
	{
		LCD_DispStr(x1, y, "静音 = 开 ", &tFont);
	}
	else
	{
		LCD_DispStr(x1, y, "静音 = 关", &tFont);
	}

	sprintf((char *)buf, "进度 = %3d%% ", g_tMP3.uiProgress * 100 / Music_Length[Select_Index]);
	LCD_DispStr(x2, y, buf, &tFont);
	LCD_DispStr(20, 9 * LINE_CAP,"正在播放:", &tFont);
	LCD_DispStr(100, 9 * LINE_CAP,Music_Name[Select_Index], &tFont);
}

/*
*********************************************************************************************************
*	函 数 名: DispError
*	功能说明: 显示错误信息
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DispError(char *_str)
{
	FONT_T tFont;		/* 定义一个字体结构体变量，用于设置字体参数 */

	/* 设置字体属性 */
	tFont.usFontCode = FC_ST_16X16;		/* 字体选择宋体16点阵，高16x宽15) */
	tFont.usTextColor = CL_WHITE;		/* 字体颜色设置为白色 */
	tFont.usBackColor = CL_BLUE;	 	/* 文字背景颜色，蓝色 */
	tFont.usSpace = 0;

	LCD_DispStr(20, 8 * LINE_CAP, _str, &tFont);
}


/*
*********************************************************************************************************
*	函 数 名: Mp3Pro
*	功能说明: MP3文件播放，在主程序while循环中调用. 每次向VS100B发送32字节。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Mp3Pro(void)
{
	uint32_t bw,i;
	char buf[32];
	/* 如果VS1003空闲，则写入新的数据 */
	if (vs1003_ReqNewData())
	{
		f_read(&g_File, &buf, 32, &bw);
		if (bw <= 0)
		{
			/* 文件读取完毕 */
			g_tMP3.ucPauseEn = 1;
			return;
		}

		/* 计算进度 */
		g_tMP3.uiProgress += bw;

		vs1003_PreWriteData();	/* 写数据准备，设置好片选 */
		for (i = 0; i < bw; i++)
		{
			vs1003_WriteData(buf[i]);
		}
	}
}
/* MP3 播放程序 */
static void Mp3Play(int8_t Select_Index)
{
	uint8_t ucKeyCode;	/* 按键代码 */
	FRESULT result;
  result = f_open(&g_File,Music_Name[Select_Index], FA_OPEN_EXISTING | FA_READ);
	if ( result !=  FR_OK)
	{
		DispError(STR_OpenFileErr);
	}
	ucRefresh = 1;
	g_tMP3.ucPauseEn = 0;	/* 缺省开始播放 */
	g_tMP3.uiProgress = 0;	/* 进度 */
	bsp_StartTimer(1, 100);
  while (1)
	{
    
		if (g_tMP3.ucPauseEn == 0)
		{
        Mp3Pro();
		}

		/* 刷新状态栏 */
		if ((ucRefresh == 1) || (bsp_CheckTimer(1)))
		{
			ucRefresh = 0;
			bsp_StartTimer(1, 100);
			DispStatus(Select_Index);		/* 显示当前状态，音量等 */
		}
    if (g_tMP3.uiProgress / Music_Length[Select_Index]==1)
		{
				if(Select_Index<Music_Num-1)
				{
					 Select_Index+=1;
				}
				else
				{ 
					Select_Index=0;
         }
				Mp3Play(Select_Index);
				ucRefresh = 1;
    }
		/* 处理按键事件 */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* 有键按下 */
			switch (ucKeyCode)
			{
				case KEY_DOWN_JOY_OK:		/* OK键按下 */
					if (g_tMP3.ucPauseEn == 0)
					{
						g_tMP3.ucPauseEn = 1;
					}
					else
					{
						g_tMP3.ucPauseEn = 0;
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_WAKEUP:		/* WAKEUP键按下 */
          Select_Index=0;
				  Mp3Play(Select_Index);
					g_tMP3.uiProgress = 0;	/* 进度 */
					ucRefresh = 1;
					break;

				case KEY_DOWN_USER:			/* USER键按下 */
					if (g_tMP3.ucMuteOn == 1)
					{
						g_tMP3.ucMuteOn = 0;
						vs1003_SetVolume(g_tMP3.ucVolume);	/* 不静音 */
					}
					else
					{
						g_tMP3.ucMuteOn = 1;
						vs1003_SetVolume(VS_VOL_MUTE);	/* 静音 */
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_JOY_DOWN:		/* 摇杆DOWN键按下 */
					if (g_tMP3.ucVolume <= VOLUME_MAX - VOLUME_STEP)
					{
						g_tMP3.ucVolume += VOLUME_STEP;
						vs1003_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case KEY_DOWN_JOY_UP:		/* 摇杆UP键按下 */
					if (g_tMP3.ucVolume >= VOLUME_STEP)
					{
						g_tMP3.ucVolume -= VOLUME_STEP;
						vs1003_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case KEY_DOWN_JOY_LEFT:						/* 摇杆LEFT键按下 */
					if(Select_Index>0)
					{
						Select_Index-=1;
					}
					Mp3Play(Select_Index);
         	ucRefresh = 1;
					break;

				case KEY_DOWN_JOY_RIGHT:	/* 摇杆RIGHT键按下 */
					if(Select_Index<Music_Num-1)
					{
					  Select_Index+=1;
					}
					else
					{ 
						Select_Index=0;
          }
					Mp3Play(Select_Index);
					ucRefresh = 1;
					break;

				case KEY_DOWN_TAMPER:		/* 摇杆TAMPER键按下 */
					f_unlink(Music_Name[Select_Index]);
		      Led_Test();
				  SoftReset();
					ucRefresh = 1;
					break;

				default:
					break;
			}
		}
	}
}
