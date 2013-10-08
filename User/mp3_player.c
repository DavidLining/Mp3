/*
*********************************************************************************************************
*
*	ģ������ : MP3������������
*	�ļ����� : main.c
*	��    �� : V2.0
*	˵    �� : MP3Ӳ�����벥����������, SDIO+FatFS+VS1003B.
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-08-27 armfly  ST�̼���V3.5.0�汾��
*		v2.0    2011-10-16 armfly  �Ż����̽ṹ��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
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

#define LINE_CAP	18		/* �����м�� */

#define STR_Title	"MP3������Made By LiNing"
#define STR_Help1	"��OK����           =  ��ͣ/����"
#define STR_Help2	"��USER����         =  ����"
#define STR_Help3	"��WAKEUP����       =  ��ͷ��ʼ"
#define STR_Help4 "��TAMPER����       =  Delete" 
#define STR_Help5	"ҡ����/�¼�        =  ��������"
#define STR_Help6 "ҡ����/�Ҽ�        =  Last/Next"
#define STR_OpenSDErr	"��SD���ļ�ϵͳʧ��"
#define STR_OpenRootErr	"��SD����Ŀ¼ʧ��"
#define STR_OpenFileErr	"��ʧ��,�뽫mp3�ļ��ŵ�SD����Ŀ¼"

MP3_T g_tMP3;

/* ����Fatfs�õ���ȫ�ֱ��� */
FATFS   g_fs;
DIR     g_DirInf;
FILINFO g_FileInf;
FIL 	g_File;
FRESULT g_Result;
static TCHAR Music_Name[99][(_MAX_LFN+1)*2];
DWORD Music_Length[99];
uint8_t Music_Num=0;
int8_t  Select_Index=0;
uint8_t ucRefresh;	/* ˢ��LCD���� */
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
*	�� �� ��: Mp3Player
*	����˵��: MP3������������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void Mp3Player(void)
{
	LCD_InitHard();		/* ��ʾ����ʼ�� */
  
	LCD_SetBackLight(BRIGHT_DEFAULT);  /* ���ñ������� */
   
	DispTitle();	/* ��ʾ���� */

	DispHelp();		/* ��ʾ������Ϣ */

	vs1003_Init();
	vs1003_SoftReset();

	g_tMP3.ucVolume = 40; 			/* ȱʡ����,Խ������ԽС */
	vs1003_SetVolume(g_tMP3.ucVolume);

  g_Result=f_mount(0, &g_fs);	/* Mount a logical drive */
  if(g_Result!=FR_OK)
		{
			DispError(STR_OpenSDErr);
    }
	else
	{
		if(f_opendir(&g_DirInf,"/" ) == FR_OK)//�򿪸�Ŀ¼
	{
		while (f_readdir(&g_DirInf, &g_FileInf) == FR_OK && Music_Num<99)//��ȡĿ¼�е��ļ�����
		{
      if( !g_FileInf.fname[0] )	 /* �ļ���Ϊ�ռ�������Ŀ¼��ĩβ���˳� */
			break;  	
			if (g_FileInf.fattrib & AM_ARC) //�鵵�ļ�0x20
			{
				if(g_FileInf.lfname[0] == NULL && g_FileInf.fname !=NULL) /*�����ļ�����Ϊ�գ����ļ����ǿ�ʱת��*/
					g_FileInf.lfname =g_FileInf.fname;				
				if(strstr(g_FileInf.fname,".mp3")||strstr(g_FileInf.fname,".MP3"))
				{
					uint8_t Len;
					Len = strlen(g_FileInf.fname);
					//����������д��Music_Name������
					memcpy(Music_Name[Music_Num++],g_FileInf.fname,Len);
					Music_Length[Music_Num-1]=g_FileInf.fsize;
				}
			}
		}
	}
 }
	/* ����������ѭ���� */
  Mp3Play(Select_Index);
 	#if 0
	/* �ر��ļ�*/
	f_close(&g_File);

	/* ж���ļ�ϵͳ */
	f_mount(0, NULL);
	#endif
}

/*�����λ*/
static void SoftReset(void)
{
__set_FAULTMASK(1); // �ر������ж�
NVIC_SystemReset();// ��λ
}

/*����Ƴ���*/
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
*	�� �� ��: DispTitle
*	����˵��: ��ʾ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispTitle(void)
{
	FONT_T tFont;		/* ����һ������ṹ���������������������� */

/*	LCD_ClrScr(CL_BLUE);  		������������ɫ */
  DispPicDemo();
	/* ������������ */
	tFont.usFontCode = FC_ST_16X16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.usTextColor = CL_WHITE;		/* ������ɫ����Ϊ��ɫ */
  tFont.usBackColor = CL_MASK;	 	/* ���ֱ�����ɫ����ɫ */
	tFont.usSpace = 0;

	LCD_DispStr(100, 0, STR_Title, &tFont);
	LCD_DrawBMP(336, 50, 64, 64, (uint16_t *)gIcon_QQ);
	LCD_DrawBMP(0, 50, 64, 64, (uint16_t *)gIcon_Bat);
}

/*��ʾ����ͼƬ*/
static void DispPicDemo(void)
{

	LCD_DrawBMP(0, 0, 240, 400, (uint16_t *)gImage_1);
}
/*
*********************************************************************************************************
*	�� �� ��: DispHelp
*	����˵��: ��ʾ������Ϣ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispHelp(void)
{
	uint16_t y;
	FONT_T tFont;		/* ����һ������ṹ���������������������� */

	/* ������������ */
	tFont.usFontCode = FC_ST_16X16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.usTextColor = CL_WHITE;		/* ������ɫ����Ϊ��ɫ */
	tFont.usBackColor = CL_MASK;	 	/* ���ֱ�����ɫ�� */
	tFont.usSpace = 0;

	y = LINE_CAP * 2; 	/* �м��(��λ������) */

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
*	�� �� ��: DispStatus
*	����˵��: ��ʾ��ǰ״̬
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/

static void DispStatus(int8_t Select_Index)
{
	uint16_t y, x1, x2;
	char buf[80];
	FONT_T tFont;		/* ����һ������ṹ���������������������� */
	/* ������������ */
	tFont.usFontCode = FC_ST_16X16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.usTextColor = CL_WHITE;		/* ������ɫ����Ϊ��ɫ */
	tFont.usBackColor = CL_BLUE;	 	/* ���ֱ�����ɫ */
	tFont.usSpace = 0;

	/* ��һ�����ο�, ��ɫ */
	LCD_DrawRect(10, LINE_CAP * 10 + 12, 48, 400 - 20, CL_YELLOW);

	x1 = 20;	/* ״̬����1��X���� */
	x2 = 200;	/* ״̬����2��X���� */
	y = LINE_CAP * 11;

	if (g_tMP3.ucPauseEn == 0)
	{
		LCD_DispStr(x1, y, "���� = ���ڲ���", &tFont);
	}
	else
	{
		LCD_DispStr(x1, y, "���� = ��ͣ����", &tFont);
	}

	sprintf((char *)buf, "���� = %d ", 100 - (g_tMP3.ucVolume * 100) / VOLUME_MAX);
	LCD_DispStr(x2, y, buf, &tFont);

	y += LINE_CAP;

	if (g_tMP3.ucMuteOn == 1)
	{
		LCD_DispStr(x1, y, "���� = �� ", &tFont);
	}
	else
	{
		LCD_DispStr(x1, y, "���� = ��", &tFont);
	}

	sprintf((char *)buf, "���� = %3d%% ", g_tMP3.uiProgress * 100 / Music_Length[Select_Index]);
	LCD_DispStr(x2, y, buf, &tFont);
	LCD_DispStr(20, 9 * LINE_CAP,"���ڲ���:", &tFont);
	LCD_DispStr(100, 9 * LINE_CAP,Music_Name[Select_Index], &tFont);
}

/*
*********************************************************************************************************
*	�� �� ��: DispError
*	����˵��: ��ʾ������Ϣ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void DispError(char *_str)
{
	FONT_T tFont;		/* ����һ������ṹ���������������������� */

	/* ������������ */
	tFont.usFontCode = FC_ST_16X16;		/* ����ѡ������16���󣬸�16x��15) */
	tFont.usTextColor = CL_WHITE;		/* ������ɫ����Ϊ��ɫ */
	tFont.usBackColor = CL_BLUE;	 	/* ���ֱ�����ɫ����ɫ */
	tFont.usSpace = 0;

	LCD_DispStr(20, 8 * LINE_CAP, _str, &tFont);
}


/*
*********************************************************************************************************
*	�� �� ��: Mp3Pro
*	����˵��: MP3�ļ����ţ���������whileѭ���е���. ÿ����VS100B����32�ֽڡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void Mp3Pro(void)
{
	uint32_t bw,i;
	char buf[32];
	/* ���VS1003���У���д���µ����� */
	if (vs1003_ReqNewData())
	{
		f_read(&g_File, &buf, 32, &bw);
		if (bw <= 0)
		{
			/* �ļ���ȡ��� */
			g_tMP3.ucPauseEn = 1;
			return;
		}

		/* ������� */
		g_tMP3.uiProgress += bw;

		vs1003_PreWriteData();	/* д����׼�������ú�Ƭѡ */
		for (i = 0; i < bw; i++)
		{
			vs1003_WriteData(buf[i]);
		}
	}
}
/* MP3 ���ų��� */
static void Mp3Play(int8_t Select_Index)
{
	uint8_t ucKeyCode;	/* �������� */
	FRESULT result;
  result = f_open(&g_File,Music_Name[Select_Index], FA_OPEN_EXISTING | FA_READ);
	if ( result !=  FR_OK)
	{
		DispError(STR_OpenFileErr);
	}
	ucRefresh = 1;
	g_tMP3.ucPauseEn = 0;	/* ȱʡ��ʼ���� */
	g_tMP3.uiProgress = 0;	/* ���� */
	bsp_StartTimer(1, 100);
  while (1)
	{
    
		if (g_tMP3.ucPauseEn == 0)
		{
        Mp3Pro();
		}

		/* ˢ��״̬�� */
		if ((ucRefresh == 1) || (bsp_CheckTimer(1)))
		{
			ucRefresh = 0;
			bsp_StartTimer(1, 100);
			DispStatus(Select_Index);		/* ��ʾ��ǰ״̬�������� */
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
		/* �������¼� */
		ucKeyCode = bsp_GetKey();
		if (ucKeyCode > 0)
		{
			/* �м����� */
			switch (ucKeyCode)
			{
				case KEY_DOWN_JOY_OK:		/* OK������ */
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

				case KEY_DOWN_WAKEUP:		/* WAKEUP������ */
          Select_Index=0;
				  Mp3Play(Select_Index);
					g_tMP3.uiProgress = 0;	/* ���� */
					ucRefresh = 1;
					break;

				case KEY_DOWN_USER:			/* USER������ */
					if (g_tMP3.ucMuteOn == 1)
					{
						g_tMP3.ucMuteOn = 0;
						vs1003_SetVolume(g_tMP3.ucVolume);	/* ������ */
					}
					else
					{
						g_tMP3.ucMuteOn = 1;
						vs1003_SetVolume(VS_VOL_MUTE);	/* ���� */
					}
					ucRefresh = 1;
					break;

				case KEY_DOWN_JOY_DOWN:		/* ҡ��DOWN������ */
					if (g_tMP3.ucVolume <= VOLUME_MAX - VOLUME_STEP)
					{
						g_tMP3.ucVolume += VOLUME_STEP;
						vs1003_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case KEY_DOWN_JOY_UP:		/* ҡ��UP������ */
					if (g_tMP3.ucVolume >= VOLUME_STEP)
					{
						g_tMP3.ucVolume -= VOLUME_STEP;
						vs1003_SetVolume(g_tMP3.ucVolume);
						ucRefresh = 1;
					}
					break;

				case KEY_DOWN_JOY_LEFT:						/* ҡ��LEFT������ */
					if(Select_Index>0)
					{
						Select_Index-=1;
					}
					Mp3Play(Select_Index);
         	ucRefresh = 1;
					break;

				case KEY_DOWN_JOY_RIGHT:	/* ҡ��RIGHT������ */
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

				case KEY_DOWN_TAMPER:		/* ҡ��TAMPER������ */
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
