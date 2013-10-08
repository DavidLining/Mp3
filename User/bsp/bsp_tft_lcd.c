/*
*********************************************************************************************************
*
*	ģ������ : TFTҺ����ʾ������ģ��
*	�ļ����� : LCD_tft_lcd.c
*	��    �� : V1.0
*	˵    �� : ����������������TFT��ʾ���ֱ���Ϊ240x400��3.0���������PWM������ڹ��ܡ�
*				֧�ֵ�LCD�ڲ�����оƬ�ͺ��У�SPFD5420A��OTM4001A��R61509V
*				����оƬ�ķ��ʵ�ַΪ:  0x60000000
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-08-21 armfly  ST�̼���汾 V3.5.0�汾��
*					a) ȡ�����ʼĴ����Ľṹ�壬ֱ�Ӷ���
*		V2.0    2011-10-16 armfly  ����R61509V������ʵ��ͼ����ʾ����
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

/*
	������ʾ��
	TFT��������һ���12864������ʾ���Ŀ����������������������˴��ڻ�ͼ�Ļ��ƣ�������ƶ��ڻ��ƾֲ�ͼ��
	�Ƿǳ���Ч�ġ�TFT������ָ��һ����ͼ���ڣ�Ȼ�����еĶ�д�Դ�Ĳ��������������֮�ڣ����������ҪCPU��
	�ڴ��б���������Ļ���������ݡ�
*/

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "bsp_tft_lcd.h"
#include "bsp_timer.h"
#include "fonts.h"

/* ����LCD�������ķ��ʵ�ַ
	TFT�ӿ��е�RS��������FSMC_A0���ţ�������16bitģʽ��RS��ӦA1��ַ�ߣ����
	LCD_RAM�ĵ�ַ��+2
*/
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x0C000000))
#define LCD_REG			*(__IO uint16_t *)(LCD_BASE)
#define LCD_RAM			*(__IO uint16_t *)(LCD_BASE + 2)

static __IO uint8_t s_RGBChgEn = 0;		/* RGBת��ʹ��, 4001��д�Դ������RGB��ʽ��д��Ĳ�ͬ */
static __IO uint8_t s_AddrAutoInc = 0;	/* ����һ�����غ��Դ��ַ�Ƿ��Զ���1 */

void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor);
static void LCD_WriteReg(__IO uint16_t _usAddr, uint16_t _usValue);
static uint16_t LCD_ReadReg(__IO uint16_t _usAddr);
static void LCD_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);
static void LCD_QuitWinMode(void);
static void LCD_SetCursor(uint16_t _usX, uint16_t _usY);
static void LCD_CtrlLinesConfig(void);
static void LCD_FSMCConfig(void);
static void LCD_Init_5420_4001(void);
static void LCD_Init_61509(void);
static uint16_t LCD_BGR2RGB(uint16_t _usRGB);

/*
*********************************************************************************************************
*	�� �� ��: LCD_InitHard
*	����˵��: ��ʼ��LCD
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	uint16_t id;;

	/* ����LCD���ƿ���GPIO */
	LCD_CtrlLinesConfig();

	/* ����FSMC�ӿڣ��������� */
	LCD_FSMCConfig();

	/* FSMC���ú������ӳٲ��ܷ��������豸  */
	bsp_DelayMS(20);

	id = LCD_GetID();  	/* ��ȡLCD����оƬID */

	if (id == 0x5420)	/* 4001����5420��ͬ��4001�������Դ�RGBʱ����Ҫ����ת����5420���� */
	{
		LCD_Init_5420_4001();	/* ��ʼ��5420��4001��Ӳ�� */
	}
	else if (id == 0xB509)
	{
		LCD_Init_61509();		/* ��ʼ��61509��Ӳ�� */
	}
	else
	{
		LCD_Init_5420_4001();	/* ȱʡ��5420���� */
	}

	/* ������δ�������ʶ����4001������5420�� */
	{
		uint16_t dummy;

		LCD_WriteReg(0x0200, 0);
		LCD_WriteReg(0x0201, 0);

		LCD_REG = 0x0202;
		LCD_RAM = 0x1234;		/* дһ������ */

		LCD_WriteReg(0x0200, 0);
		LCD_WriteReg(0x0201, 0);
		LCD_REG = 0x0202;
		dummy = LCD_RAM; 		/* ������ɫֵ */
		if (dummy == 0x1234)
		{
			s_RGBChgEn = 0;
		}
		else
		{
			s_RGBChgEn = 1;		/* ������صĺ�д��Ĳ�ͬ������ҪRGBת��, ֻӰ���ȡ���صĺ��� */
		}

		if (id == 0xB509)
		{
			s_AddrAutoInc = 0;	/* 61509����ַ�������� */
		}
		else
		{
			s_AddrAutoInc = 1;	/* 5420��4001����ַ���Զ����� */
		}
	}

	/* ����Դ� */
	LCD_ClrScr(CL_BLACK);	/* ��ɫ */
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetID
*	����˵��: ��ȡLCD������ID
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint16_t LCD_GetID(void)
{
	return LCD_ReadReg(0x0000);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_Init_5420_4001
*	����˵��: ��ʼ��5420��4001��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_Init_5420_4001(void)
{
	/* ��ʼ��LCD��дLCD�Ĵ����������� */
	LCD_WriteReg(0x0000, 0x0000);
	LCD_WriteReg(0x0001, 0x0100);
	LCD_WriteReg(0x0002, 0x0100);

	/*
		R003H �Ĵ����ܹؼ��� Entry Mode ��������ɨ�跽��
		�μ���SPFD5420A.pdf ��15ҳ

		240x400��Ļ��������(px,py)����:
		    R003 = 0x1018                  R003 = 0x1008
		  -------------------          -------------------
		 |(0,0)              |        |(0,0)              |
		 |                   |        |					  |
		 |  ^           ^    |        |   ^           ^   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |  ------>  |    |        |   | <------   |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |                   |        |					  |
		 |                   |        |                   |
		 |      (x=239,y=399)|        |      (x=239,y=399)|
		 |-------------------|        |-------------------|
		 |                   |        |                   |
		  -------------------          -------------------

		���հ�����������LCD�ķ����������������������ɨ�跽�����£�(����ͼ��1���Ǻ�)
		 --------------------------------
		|  |(0,0)                        |
		|  |     --------->              |
		|  |         |                   |
		|  |         |                   |
		|  |         |                   |
		|  |         V                   |
		|  |     --------->              |
		|  |                    (399,239)|
		 --------------------------------

		��������(x,y) �����������ת����ϵ
		x = 399 - py;
		y = px;

		py = 399 - x;
		px = y;

	*/
	LCD_WriteReg(0x0003, 0x1018); /* 0x1018 1030 */

	LCD_WriteReg(0x0008, 0x0808);
	LCD_WriteReg(0x0009, 0x0001);
	LCD_WriteReg(0x000B, 0x0010);
	LCD_WriteReg(0x000C, 0x0000);
	LCD_WriteReg(0x000F, 0x0000);
	LCD_WriteReg(0x0007, 0x0001);
	LCD_WriteReg(0x0010, 0x0013);
	LCD_WriteReg(0x0011, 0x0501);
	LCD_WriteReg(0x0012, 0x0300);
	LCD_WriteReg(0x0020, 0x021E);
	LCD_WriteReg(0x0021, 0x0202);
	LCD_WriteReg(0x0090, 0x8000);
	LCD_WriteReg(0x0100, 0x17B0);
	LCD_WriteReg(0x0101, 0x0147);
	LCD_WriteReg(0x0102, 0x0135);
	LCD_WriteReg(0x0103, 0x0700);
	LCD_WriteReg(0x0107, 0x0000);
	LCD_WriteReg(0x0110, 0x0001);
	LCD_WriteReg(0x0210, 0x0000);
	LCD_WriteReg(0x0211, 0x00EF);
	LCD_WriteReg(0x0212, 0x0000);
	LCD_WriteReg(0x0213, 0x018F);
	LCD_WriteReg(0x0280, 0x0000);
	LCD_WriteReg(0x0281, 0x0004);
	LCD_WriteReg(0x0282, 0x0000);
	LCD_WriteReg(0x0300, 0x0101);
	LCD_WriteReg(0x0301, 0x0B2C);
	LCD_WriteReg(0x0302, 0x1030);
	LCD_WriteReg(0x0303, 0x3010);
	LCD_WriteReg(0x0304, 0x2C0B);
	LCD_WriteReg(0x0305, 0x0101);
	LCD_WriteReg(0x0306, 0x0807);
	LCD_WriteReg(0x0307, 0x0708);
	LCD_WriteReg(0x0308, 0x0107);
	LCD_WriteReg(0x0309, 0x0105);
	LCD_WriteReg(0x030A, 0x0F04);
	LCD_WriteReg(0x030B, 0x0F00);
	LCD_WriteReg(0x030C, 0x000F);
	LCD_WriteReg(0x030D, 0x040F);
	LCD_WriteReg(0x030E, 0x0300);
	LCD_WriteReg(0x030F, 0x0701);
	LCD_WriteReg(0x0400, 0x3500);
	LCD_WriteReg(0x0401, 0x0001);
	LCD_WriteReg(0x0404, 0x0000);
	LCD_WriteReg(0x0500, 0x0000);
	LCD_WriteReg(0x0501, 0x0000);
	LCD_WriteReg(0x0502, 0x0000);
	LCD_WriteReg(0x0503, 0x0000);
	LCD_WriteReg(0x0504, 0x0000);
	LCD_WriteReg(0x0505, 0x0000);
	LCD_WriteReg(0x0600, 0x0000);
	LCD_WriteReg(0x0606, 0x0000);
	LCD_WriteReg(0x06F0, 0x0000);
	LCD_WriteReg(0x07F0, 0x5420);
	LCD_WriteReg(0x07DE, 0x0000);
	LCD_WriteReg(0x07F2, 0x00DF);
	LCD_WriteReg(0x07F3, 0x0810);
	LCD_WriteReg(0x07F4, 0x0077);
	LCD_WriteReg(0x07F5, 0x0021);
	LCD_WriteReg(0x07F0, 0x0000);
	LCD_WriteReg(0x0007, 0x0173);

	/* ������ʾ���� WINDOWS */
	LCD_WriteReg(0x0210, 0);	/* ˮƽ��ʼ��ַ */
	LCD_WriteReg(0x0211, 239);	/* ˮƽ�������� */
	LCD_WriteReg(0x0212, 0);	/* ��ֱ��ʼ��ַ */
	LCD_WriteReg(0x0213, 399);	/* ��ֱ������ַ */
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_Init_61509
*	����˵��: ��ʼ��61509��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_Init_61509(void)
{
	LCD_WriteReg(0x000,0x0000);
	LCD_WriteReg(0x000,0x0000);
	LCD_WriteReg(0x000,0x0000);
	LCD_WriteReg(0x000,0x0000);
	bsp_DelayMS(15);

	LCD_WriteReg(0x008,0x0808);
	LCD_WriteReg(0x010,0x0010);
	LCD_WriteReg(0x400,0x6200);

	LCD_WriteReg(0x300,0x0c06);	/* GAMMA */
	LCD_WriteReg(0x301,0x9d0f);
	LCD_WriteReg(0x302,0x0b05);
	LCD_WriteReg(0x303,0x1217);
	LCD_WriteReg(0x304,0x3333);
	LCD_WriteReg(0x305,0x1712);
	LCD_WriteReg(0x306,0x950b);
	LCD_WriteReg(0x307,0x0f0d);
	LCD_WriteReg(0x308,0x060c);
	LCD_WriteReg(0x309,0x0000);

	LCD_WriteReg(0x011,0x0202);
	LCD_WriteReg(0x012,0x0101);
	LCD_WriteReg(0x013,0x0001);

	LCD_WriteReg(0x007,0x0001);
	LCD_WriteReg(0x100,0x0730);	/* BT,AP 0x0330��*/
	LCD_WriteReg(0x101,0x0237);	/* DC0,DC1,VC */
	LCD_WriteReg(0x103,0x2b00);	/* VDV	//0x0f00 */
	LCD_WriteReg(0x280,0x4000);	/* VCM */
	LCD_WriteReg(0x102,0x81b0);	/* VRH,VCMR,PSON,PON */
	bsp_DelayMS(15);

	LCD_WriteReg(0x001,0x0100);
	LCD_WriteReg(0x002,0x0100);
	/* LCD_WriteReg(0x003,0x1030); */
	LCD_WriteReg(0x003,0x1018);
	LCD_WriteReg(0x009,0x0001);

	LCD_WriteReg(0x0C,0x0000);	/* MCU interface  */
	/*
		LCD_WriteReg(0x0C,0x0110);	//RGB interface 18bit
		LCD_WriteReg(0x0C,0x0111);	//RGB interface 16bit
		LCD_WriteReg(0x0C,0x0020);	//VSYNC interface
	*/

	LCD_WriteReg(0x090,0x8000);
	LCD_WriteReg(0x00f,0x0000);

	LCD_WriteReg(0x210,0x0000);
	LCD_WriteReg(0x211,0x00ef);
	LCD_WriteReg(0x212,0x0000);
	LCD_WriteReg(0x213,0x018f);

	LCD_WriteReg(0x500,0x0000);
	LCD_WriteReg(0x501,0x0000);
	LCD_WriteReg(0x502,0x005f);
	LCD_WriteReg(0x401,0x0001);
	LCD_WriteReg(0x404,0x0000);
	bsp_DelayMS(15);
	LCD_WriteReg(0x007,0x0100);
	bsp_DelayMS(15);
	LCD_WriteReg(0x200,0x0000);
	LCD_WriteReg(0x201,0x0000);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispOn
*	����˵��: ����ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispOn(void)
{
	LCD_WriteReg(7, 0x0173); /* ����262K��ɫ���Ҵ���ʾ */
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispOff
*	����˵��: �ر���ʾ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispOff(void)
{
	LCD_WriteReg(7, 0x0);	/* �ر���ʾ*/
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ClrScr
*	����˵��: �����������ɫֵ����
*	��    �Σ�_usColor : ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
	uint32_t i;

	LCD_SetCursor(0, 0);		/* ���ù��λ�� */

	LCD_REG = LR_GRAM; 			/* ׼����д�Դ� */

	for (i = 0; i < LCD_HEIGHT * LCD_WIDTH; i++)
	{
		LCD_RAM = _usColor;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DispStr
*	����˵��: ��LCDָ�����꣨���Ͻǣ���ʾһ���ַ���
*	��    �Σ�
*		_usX : X���꣬����3.0���������ΧΪ��0 - 399��
*		_usY : Y���꣬����3.0���������ΧΪ ��0 - 239��
*		_ptr  : �ַ���ָ��
*		_tFont : ����ṹ�壬������ɫ������ɫ(֧��͸��)��������롢���ּ��Ȳ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address;
	uint8_t buf[24 * 24 / 8];	/* ���֧��24������ */
	uint8_t m, width, height;
	uint16_t x, y;

	/* ��ʱֻ֧��16�������庺�� */
	if (_tFont->usFontCode == FC_ST_16X16)
	{
		height = 16;
		while (*_ptr != 0)
		{
			code1 = *_ptr;	/* ascii���� ���ߺ��ִ���ĸ��ֽ� */
			if (code1 < 0x80)
			{
				/* ��ascii�ַ������Ƶ�buf */
				memcpy(buf, &g_Ascii16[code1 * 16], 16);
				width = 8;
			}
			else
			{
				code2 = *++_ptr;
				if (code2 == 0)
				{
					break;
				}

				/* ����16�����ֵ����ַ
					ADDRESS = [(code1-0xa1) * 94 + (code2-0xa1)] * 32
					;
				*/
				#ifdef USE_SMALL_FONT
					m = 0;
					while(1)
					{
						address = m* 34;
						m++;
						if ((code1 == g_Hz16[address + 0]) && (code2 == g_Hz16[address + 1]))
						{
							address += 2;
							memcpy(buf, &g_Hz16[address], 32);
							break;
						}
						else if ((g_Hz16[address + 0] == 0xFF) && (g_Hz16[address + 1] == 0xFF))
						{
							/* �ֿ�������ϣ�δ�ҵ��������ȫFF */
							memset(buf, 0xFF, 32);
							break;
						}
					}
					width = 16;
				#else
					address = ((code1-0xa1) * 94 + (code2-0xa1)) * 32 + HZK16_ADDR;
					memcpy(buf, (const uint8_t *)address, 32);
				#endif
					width = 16;
			}

			y = _usY;
			/* ��ʼˢLCD */
			for (m = 0; m < height; m++)	/* �ַ��߶� */
			{
				x = _usX;
				for (i = 0; i < width; i++)	/* �ַ���� */
				{
					if ((buf[m * (width / 8) + i / 8] & (0x80 >> (i % 8))) != 0x00)
					{
						LCD_PutPixel(x, y, _tFont->usTextColor);	/* ����������ɫΪ����ɫ */
					}
					else if (_tFont->usBackColor != CL_MASK)		/* �������ɫ����ֵ������͸������ */
					{
						LCD_PutPixel(x, y, _tFont->usBackColor);	/* ����������ɫΪ���ֱ���ɫ */;
					}

					x++;
				}
				y++;
			}

			if (_tFont->usBackColor != CL_MASK && _tFont->usSpace > 0)
			{
				/* ������ֵ�ɫ��_tFont->usBackColor�������ּ����ڵ���Ŀ�ȣ���ô��Ҫ������֮�����(��ʱδʵ��) */
			}
			_usX += width + _tFont->usSpace;	/* �е�ַ���� */
			_ptr++;			/* ָ����һ���ַ� */
		}
	}
	else
	{
		/* ��ʱ��֧���������� */
		return;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_PutPixel
*	����˵��: ��1������
*	��    �Σ�
*			_usX,_usY : ��������
*			_usColor  ��������ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	LCD_SetCursor(_usX, _usY);	/* ���ù��λ�� */

	/* д�Դ� */
	LCD_REG = LR_GRAM;
	/* Write 16-bit GRAM Reg */
	LCD_RAM = _usColor;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_GetPixel
*	����˵��: ��ȡ1������
*	��    �Σ�
*			_usX,_usY : ��������
*			_usColor  ��������ɫ
*	�� �� ֵ: RGB��ɫֵ
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	LCD_SetCursor(_usX, _usY);	/* ���ù��λ�� */

	/* ׼��д�Դ� */
	LCD_REG = LR_GRAM;

	usRGB = LCD_RAM;

	/* �� 16-bit GRAM Reg */
	if (s_RGBChgEn == 1)
	{
		usRGB = LCD_BGR2RGB(usRGB);
	}

	return usRGB;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawLine
*	����˵��: ���� Bresenham �㷨����2��仭һ��ֱ�ߡ�
*	��    �Σ�
*			_usX1, _usY1 ����ʼ������
*			_usX2, _usY2 ����ֹ��Y����
*			_usColor     ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* ���� Bresenham �㷨����2��仭һ��ֱ�� */

	LCD_PutPixel(_usX1 , _usY1 , _usColor);

	/* ��������غϣ���������Ķ�����*/
	if ( _usX1 == _usX2 && _usY1 == _usY2 )
	{
		return;
	}

	iTag = 0 ;
	/* dx = abs ( _usX2 - _usX1 ); */
	if (_usX2 >= _usX1)
	{
		dx = _usX2 - _usX1;
	}
	else
	{
		dx = _usX1 - _usX2;
	}

	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
	{
		dy = _usY2 - _usY1;
	}
	else
	{
		dy = _usY1 - _usY2;
	}

	if ( dx < dy )   /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* ȷ������1���Ǽ�1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* ѭ������ */
	{
		if ( d < 0 )
		{
			d += inc1 ;
		}
		else
		{
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
		{
			LCD_PutPixel ( y , x , _usColor) ;
		}
		else
		{
			LCD_PutPixel ( x , y , _usColor) ;
		}
		x += tx ;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawPoints
*	����˵��: ���� Bresenham �㷨������һ��㣬������Щ�����������������ڲ�����ʾ��
*	��    �Σ�
*			x, y     ����������
*			_usColor ����ɫ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawPoints(uint16_t *x, uint16_t *y, uint16_t _usSize, uint16_t _usColor)
{
	uint16_t i;

	for (i = 0 ; i < _usSize - 1; i++)
	{
		LCD_DrawLine(x[i], y[i], x[i + 1], y[i + 1], _usColor);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawRect
*	����˵��: ����ˮƽ���õľ��Ρ�
*	��    �Σ�
*			_usX,_usY���������Ͻǵ�����
*			_usHeight �����εĸ߶�
*			_usWidth  �����εĿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint8_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
	 ---------------->---
	|(_usX��_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	LCD_DrawLine(_usX, _usY, _usX + _usWidth - 1, _usY, _usColor);	/* �� */
	LCD_DrawLine(_usX, _usY + _usHeight - 1, _usX + _usWidth - 1, _usY + _usHeight - 1, _usColor);	/* �� */

	LCD_DrawLine(_usX, _usY, _usX, _usY + _usHeight - 1, _usColor);	/* �� */
	LCD_DrawLine(_usX + _usWidth - 1, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1, _usColor);	/* �� */
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawCircle
*	����˵��: ����һ��Բ���ʿ�Ϊ1������
*	��    �Σ�
*			_usX,_usY  ��Բ�ĵ�����
*			_usRadius  ��Բ�İ뾶
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* ��ǰ X ֵ */
	uint32_t  CurY;		/* ��ǰ Y ֵ */

	D = 3 - (_usRadius << 1);
	CurX = 0;
	CurY = _usRadius;

	while (CurX <= CurY)
	{
		LCD_PutPixel(_usX + CurX, _usY + CurY, _usColor);
		LCD_PutPixel(_usX + CurX, _usY - CurY, _usColor);
		LCD_PutPixel(_usX - CurX, _usY + CurY, _usColor);
		LCD_PutPixel(_usX - CurX, _usY - CurY, _usColor);
		LCD_PutPixel(_usX + CurY, _usY + CurX, _usColor);
		LCD_PutPixel(_usX + CurY, _usY - CurX, _usColor);
		LCD_PutPixel(_usX - CurY, _usY + CurX, _usColor);
		LCD_PutPixel(_usX - CurY, _usY - CurX, _usColor);

		if (D < 0)
		{
			D += (CurX << 2) + 6;
		}
		else
		{
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawBMP
*	����˵��: ��LCD����ʾһ��BMPλͼ��λͼ����ɨ����򣺴����ң����ϵ���
*	��    �Σ�
*			_usX, _usY : ͼƬ������
*			_usHeight  ��ͼƬ�߶�
*			_usWidth   ��ͼƬ���
*			_ptr       ��ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint32_t index = 0;
	const uint16_t *p;

	/* ����ͼƬ��λ�úʹ�С�� ��������ʾ���� */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* ׼����д�Դ� */

	p = _ptr;
	for (index = 0; index < _usHeight * _usWidth; index++)
	{
		/*
			armfly : �����Ż�, �����͵�չ��
			LCD_WriteRAM(_ptr[index]);

			�˴��ɿ�����DMA����
		*/
		LCD_RAM = *p++;
	}

	/* �˳����ڻ�ͼģʽ */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawIcon
*	����˵��: ��LCD�ϻ���һ��ͼ�꣬�Ľ��Զ���Ϊ����
*	��    �Σ�
*			_usX, _usY : ͼƬ������
*			_usHeight  ��ͼƬ�߶�
*			_usWidth   ��ͼƬ���
*			_ptr       ��ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawIcon(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* ���ڼ�¼�����ڵ�������� */

	/* ����ͼƬ��λ�úʹ�С�� ��������ʾ���� */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* ׼����д�Դ� */

	p = _ptr;
	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			usNewRGB = *p++;	/* ��ȡͼ�����ɫֵ��ָ���1 */
			#if 1	/* �����֧��ͼ���4��ֱ���и�Ϊ���ǣ��������Ǳ���ͼ�� */
				if ((y == 0 && (x < 6 || x > _usWidth - 7)) ||
					(y == 1 && (x < 4 || x > _usWidth - 5)) ||
					(y == 2 && (x < 3 || x > _usWidth - 4)) ||
					(y == 3 && (x < 2 || x > _usWidth - 3)) ||
					(y == 4 && (x < 1 || x > _usWidth - 2)) ||
					(y == 5 && (x < 1 || x > _usWidth - 2))	||

					(y == _usHeight - 1 && (x < 6 || x > _usWidth - 7)) ||
					(y == _usHeight - 2 && (x < 4 || x > _usWidth - 5)) ||
					(y == _usHeight - 3 && (x < 3 || x > _usWidth - 4)) ||
					(y == _usHeight - 4 && (x < 2 || x > _usWidth - 3)) ||
					(y == _usHeight - 5 && (x < 1 || x > _usWidth - 2)) ||
					(y == _usHeight - 6 && (x < 1 || x > _usWidth - 2))
					)
				{
					usNewRGB = LCD_RAM;	/* �ն�һ�����أ��ڲ�LCDָ���Զ����� */;
					if (s_AddrAutoInc == 0)
					{
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						LCD_RAM = usNewRGB;		/* 61509����Ҫ��дһ�β��ܵ�����ַ */
					}
				}
				else
				{
					LCD_RAM = usNewRGB;
				}
			#else 	/* ���������֧����ʹͼ���еİ�ɫ��Ϊ͸����ɫ */
				if (usNewRGB == 0xFFFF) 	/* 0xFFFF�ǰ�ɫֵ */
				{
					usNewRGB = LCD_RAM;	/* �ն�һ�����أ��ڲ�LCDָ���Զ����� */;
					if (s_AddrAutoInc == 0)
					{
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						LCD_RAM = usNewRGB;		/* 61509����Ҫ��дһ�β��ܵ�����ַ */
					}
				}
				else
				{
					LCD_RAM = usNewRGB;
				}
			#endif
		}
	}

	/* �˳����ڻ�ͼģʽ */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_DrawIconActive
*	����˵��: ��LCD�ϻ���һ������ѡ�е�ͼ��
*	��    �Σ�
*			_usX, _usY : ͼƬ������
*			_usHeight  ��ͼƬ�߶�
*			_usWidth   ��ͼƬ���
*			_ptr       ��ͼƬ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_DrawIconActive(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* ���ڼ�¼�����ڵ�������� */

	/* ����ͼƬ��λ�úʹ�С�� ��������ʾ���� */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* ׼����д�Դ� */

	p = _ptr;
	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			usNewRGB = *p++;	/* ��ȡͼ�����ɫֵ��ָ���1 */
			#if 1	/* �����֧��ͼ���4��ֱ���и�Ϊ���ǣ��������Ǳ���ͼ�� */
				if ((y == 0 && (x < 6 || x > _usWidth - 7)) ||
					(y == 1 && (x < 4 || x > _usWidth - 5)) ||
					(y == 2 && (x < 3 || x > _usWidth - 4)) ||
					(y == 3 && (x < 2 || x > _usWidth - 3)) ||
					(y == 4 && (x < 1 || x > _usWidth - 2)) ||
					(y == 5 && (x < 1 || x > _usWidth - 2))	||

					(y == _usHeight - 1 && (x < 6 || x > _usWidth - 7)) ||
					(y == _usHeight - 2 && (x < 4 || x > _usWidth - 5)) ||
					(y == _usHeight - 3 && (x < 3 || x > _usWidth - 4)) ||
					(y == _usHeight - 4 && (x < 2 || x > _usWidth - 3)) ||
					(y == _usHeight - 5 && (x < 1 || x > _usWidth - 2)) ||
					(y == _usHeight - 6 && (x < 1 || x > _usWidth - 2))
					)
				{
					usNewRGB = LCD_RAM;	/* �ն�һ�����أ��ڲ�LCDָ���Զ����� */;
					if (s_AddrAutoInc == 0)
					{
						LCD_RAM = usNewRGB;		/* 61509����Ҫ��дһ�β��ܵ�����ַ */
					}
				}
				else
				{
					/* ����ԭʼ���ص����ȣ�ʵ��ͼ�걻����ѡ�е�Ч�� */
					uint16_t R,G,B;
					uint16_t bright = 15;

					/* rrrr rggg gggb bbbb */
					R = (usNewRGB & 0xF800) >> 11;
					G = (usNewRGB & 0x07E0) >> 5;
					B =  usNewRGB & 0x001F;
					if (R > bright)
					{
						R -= bright;
					}
					else
					{
						R = 0;
					}
					if (G > 2 * bright)
					{
						G -= 2 * bright;
					}
					else
					{
						G = 0;
					}
					if (B > bright)
					{
						B -= bright;
					}
					else
					{
						B = 0;
					}
					LCD_RAM = (R << 11) + (G << 5) + B;
				}
			#else 	/* ���������֧����ʹͼ���еİ�ɫ��Ϊ͸����ɫ */
				if (usNewRGB != 0xFFFF) 	/* 0xFFFF�ǰ�ɫֵ */
				{
					LCD_RAM = usNewRGB;
				}
				else
				{
					/* ��ɫ����bit����: R5 G6 B5 */
					usNewRGB = LCD_RAM;	/* �ն�һ�����أ��ڲ�LCDָ���Զ����� */
				}
			#endif
		}
	}

	/* �˳����ڻ�ͼģʽ */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_WriteReg
*	����˵��: �޸�LCD�������ļĴ�����ֵ��
*	��    �Σ�
*			LCD_Reg ���Ĵ�����ַ;
*			LCD_RegValue : �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_WriteReg(__IO uint16_t _usAddr, uint16_t _usValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD_REG = _usAddr;
	/* Write 16-bit Reg */
	LCD_RAM = _usValue;
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_ReadReg
*	����˵��: ��ȡLCD�������ļĴ�����ֵ��
*	��    �Σ�
*			LCD_Reg ���Ĵ�����ַ;
*			LCD_RegValue : �Ĵ���ֵ
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint16_t LCD_ReadReg(__IO uint16_t _usAddr)
{
	/* Write 16-bit Index (then Read Reg) */
	LCD_REG = _usAddr;
	/* Read 16-bit Reg */
	return (LCD_RAM);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SetDispWin
*	����˵��: ������ʾ���ڣ����봰����ʾģʽ��TFT����оƬ֧�ִ�����ʾģʽ�����Ǻ�һ���12864����LCD���������
*	��    �Σ�
*		_usX : ˮƽ����
*		_usY : ��ֱ����
*		_usHeight: ���ڸ߶�
*		_usWidth : ���ڿ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	uint16_t px, py;

	/*
		240x400��Ļ��������(px,py)����:
		    R003 = 0x1018                  R003 = 0x1008
		  -------------------          -------------------
		 |(0,0)              |        |(0,0)              |
		 |                   |        |					  |
		 |  ^           ^    |        |   ^           ^   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |  ------>  |    |        |   | <------   |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |  |           |    |        |   |           |   |
		 |                   |        |					  |
		 |                   |        |                   |
		 |      (x=239,y=399)|        |      (x=239,y=399)|
		 |-------------------|        |-------------------|
		 |                   |        |                   |
		  -------------------          -------------------

		���հ�����������LCD�ķ����������������������ɨ�跽�����£�(����ͼ��1���Ǻ�)
		 --------------------------------
		|  |(0,0)                        |
		|  |     --------->              |
		|  |         |                   |
		|  |         |                   |
		|  |         |                   |
		|  |         V                   |
		|  |     --------->              |
		|  |                    (399,239)|
		 --------------------------------
	�����������������ת����ϵ��
		x = 399 - py;
		y = px;

		py = 399 - x;
		px = y;
	*/

	py = 399 - _usX;
	px = _usY;

	/* ������ʾ���� WINDOWS */
	LCD_WriteReg(0x0210, px);						/* ˮƽ��ʼ��ַ */
	LCD_WriteReg(0x0211, px + (_usHeight - 1));		/* ˮƽ�������� */
	LCD_WriteReg(0x0212, py + 1 - _usWidth);		/* ��ֱ��ʼ��ַ */
	LCD_WriteReg(0x0213, py);						/* ��ֱ������ַ */

	LCD_SetCursor(_usX, _usY);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_SetCursor
*	����˵��: ���ù��λ��
*	��    �Σ�_usX : X����; _usY: Y����
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/*
		px��py ���������꣬ x��y����������
		ת����ʽ:
		py = 399 - x;
		px = y;
	*/

	LCD_WriteReg(0x0200, _usY);  		/* px */
	LCD_WriteReg(0x0201, 399 - _usX);	/* py */
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_BGR2RGB
*	����˵��: RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
*	��    �Σ�RGB��ɫ����
*	�� �� ֵ: ת�������ɫ����
*********************************************************************************************************
*/
static uint16_t LCD_BGR2RGB(uint16_t _usRGB)
{
	uint16_t  r, g, b, rgb;

	b = (_usRGB >> 0)  & 0x1F;
	g = (_usRGB >> 5)  & 0x3F;
	r = (_usRGB >> 11) & 0x1F;

	rgb = (b<<11) + (g<<5) + (r<<0);

	return( rgb );
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_QuitWinMode
*	����˵��: �˳�������ʾģʽ����Ϊȫ����ʾģʽ
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_QuitWinMode(void)
{
	LCD_SetDispWin(0, 0, LCD_HEIGHT, LCD_WIDTH);
}

/*
*********************************************************************************************************
*	�� �� ��: LCD_CtrlLinesConfig
*	����˵��: ����LCD���ƿ��ߣ�FSMC�ܽ�����Ϊ���ù���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/* ʹ�� FSMC, GPIOD, GPIOE, GPIOF, GPIOG �� AFIO ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                     RCC_APB2Periph_AFIO, ENABLE);

	/* ���� PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* ���� PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) Ϊ����������� */
	/* PE3,PE4 ����A19, A20, STM32F103ZE-EK(REV 1.0)����ʹ�� */
	/* PE5,PE6 ����A19, A20, STM32F103ZE-EK(REV 2.0)����ʹ�� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* ���� PF.00(A0 (RS))  Ϊ����������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* ���� PG.12(NE4 (LCD/CS)) Ϊ����������� - CE3(LCD /CS) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	�� �� ��: LCD_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void LCD_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  init;
	FSMC_NORSRAMTimingInitTypeDef  timing;

	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 4 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	timing.FSMC_AddressSetupTime = 1;
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 2;
	timing.FSMC_BusTurnAroundDuration = 0;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* ע��ɿ��������Ա */
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;

	init.FSMC_ReadWriteTimingStruct = &timing;
	init.FSMC_WriteTimingStruct = &timing;

	FSMC_NORSRAMInit(&init);

	/* - BANK 3 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}


/*
*********************************************************************************************************
*	�� �� ��: LCD_SetBackLight
*	����˵��: ��ʼ������LCD�������GPIO,����ΪPWMģʽ��
*			���رձ���ʱ����CPU IO����Ϊ��������ģʽ���Ƽ�����Ϊ������������������͵�ƽ)����TIM3�ر� ʡ��
*	��    �Σ�_bright ���ȣ�0����255������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* ��1������GPIOB RCC_APB2Periph_AFIO ��ʱ��	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	if (_bright == 0)
	{
		/* ���ñ���GPIOΪ����ģʽ */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* �ر�TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}
	else if (_bright == BRIGHT_MAX)	/* ������� */
	{
		/* ���ñ���GPIOΪ�������ģʽ */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_SetBits(GPIOB, GPIO_Pin_1);

		/* �ر�TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}

	/* ���ñ���GPIOΪ�����������ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* ʹ��TIM3��ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/*
		TIM3 ����: ����1·PWM�ź�;
		TIM3CLK = 72 MHz, Prescaler = 0(����Ƶ), TIM3 counter clock = 72 MHz
		���㹫ʽ��
		PWM���Ƶ�� = TIM3 counter clock /(ARR + 1)

		������������Ϊ100Hz

		�������TIM3CLKԤ��Ƶ����ô�����ܵõ�100Hz��Ƶ��
		�������÷�Ƶ�� = 1000�� ��ô  TIM3 counter clock = 72KHz
		TIM_Period = 720 - 1;
		Ƶ���²�����
	 */
	TIM_TimeBaseStructure.TIM_Period = 720 - 1;	/* TIM_Period = TIM3 ARR Register */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	/*
		_bright = 1 ʱ, TIM_Pulse = 1
		_bright = 255 ʱ, TIM_Pulse = TIM_Period
	*/
	TIM_OCInitStructure.TIM_Pulse = (TIM_TimeBaseStructure.TIM_Period * _bright) / BRIGHT_MAX;	/* �ı�ռ�ձ� */

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);

	/* ʹ�� TIM3 ��ʱ�� */
	TIM_Cmd(TIM3, ENABLE);
}

