/*
*********************************************************************************************************
*
*	模块名称 : TFT液晶显示器驱动模块
*	文件名称 : LCD_tft_lcd.c
*	版    本 : V1.0
*	说    明 : 安富莱开发板标配的TFT显示器分辨率为240x400，3.0寸宽屏，带PWM背光调节功能。
*				支持的LCD内部驱动芯片型号有：SPFD5420A、OTM4001A、R61509V
*				驱动芯片的访问地址为:  0x60000000
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-08-21 armfly  ST固件库版本 V3.5.0版本。
*					a) 取消访问寄存器的结构体，直接定义
*		V2.0    2011-10-16 armfly  增加R61509V驱动，实现图标显示函数
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

/*
	友情提示：
	TFT控制器和一般的12864点阵显示器的控制器最大的区别在于引入了窗口绘图的机制，这个机制对于绘制局部图形
	是非常有效的。TFT可以先指定一个绘图窗口，然后所有的读写显存的操作均在这个窗口之内，因此它不需要CPU在
	内存中保存整个屏幕的像素数据。
*/

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "bsp_tft_lcd.h"
#include "bsp_timer.h"
#include "fonts.h"

/* 定义LCD驱动器的访问地址
	TFT接口中的RS引脚连接FSMC_A0引脚，由于是16bit模式，RS对应A1地址线，因此
	LCD_RAM的地址是+2
*/
#define LCD_BASE        ((uint32_t)(0x60000000 | 0x0C000000))
#define LCD_REG			*(__IO uint16_t *)(LCD_BASE)
#define LCD_RAM			*(__IO uint16_t *)(LCD_BASE + 2)

static __IO uint8_t s_RGBChgEn = 0;		/* RGB转换使能, 4001屏写显存后读会的RGB格式和写入的不同 */
static __IO uint8_t s_AddrAutoInc = 0;	/* 读回一个像素后，显存地址是否自动增1 */

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
*	函 数 名: LCD_InitHard
*	功能说明: 初始化LCD
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_InitHard(void)
{
	uint16_t id;;

	/* 配置LCD控制口线GPIO */
	LCD_CtrlLinesConfig();

	/* 配置FSMC接口，数据总线 */
	LCD_FSMCConfig();

	/* FSMC重置后必须加延迟才能访问总线设备  */
	bsp_DelayMS(20);

	id = LCD_GetID();  	/* 读取LCD驱动芯片ID */

	if (id == 0x5420)	/* 4001屏和5420相同，4001屏读回显存RGB时，需要进行转换，5420无需 */
	{
		LCD_Init_5420_4001();	/* 初始化5420和4001屏硬件 */
	}
	else if (id == 0xB509)
	{
		LCD_Init_61509();		/* 初始化61509屏硬件 */
	}
	else
	{
		LCD_Init_5420_4001();	/* 缺省按5420处理 */
	}

	/* 下面这段代码用于识别是4001屏还是5420屏 */
	{
		uint16_t dummy;

		LCD_WriteReg(0x0200, 0);
		LCD_WriteReg(0x0201, 0);

		LCD_REG = 0x0202;
		LCD_RAM = 0x1234;		/* 写一个像素 */

		LCD_WriteReg(0x0200, 0);
		LCD_WriteReg(0x0201, 0);
		LCD_REG = 0x0202;
		dummy = LCD_RAM; 		/* 读回颜色值 */
		if (dummy == 0x1234)
		{
			s_RGBChgEn = 0;
		}
		else
		{
			s_RGBChgEn = 1;		/* 如果读回的和写入的不同，则需要RGB转换, 只影响读取像素的函数 */
		}

		if (id == 0xB509)
		{
			s_AddrAutoInc = 0;	/* 61509屏地址不会自增 */
		}
		else
		{
			s_AddrAutoInc = 1;	/* 5420和4001屏地址会自动增加 */
		}
	}

	/* 清除显存 */
	LCD_ClrScr(CL_BLACK);	/* 黑色 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetID
*	功能说明: 读取LCD的器件ID
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint16_t LCD_GetID(void)
{
	return LCD_ReadReg(0x0000);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_Init_5420_4001
*	功能说明: 初始化5420和4001屏
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_Init_5420_4001(void)
{
	/* 初始化LCD，写LCD寄存器进行配置 */
	LCD_WriteReg(0x0000, 0x0000);
	LCD_WriteReg(0x0001, 0x0100);
	LCD_WriteReg(0x0002, 0x0100);

	/*
		R003H 寄存器很关键， Entry Mode ，决定了扫描方向
		参见：SPFD5420A.pdf 第15页

		240x400屏幕物理坐标(px,py)如下:
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

		按照安富莱开发板LCD的方向，我们期望的虚拟坐标和扫描方向如下：(和上图第1个吻合)
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

		虚拟坐标(x,y) 和物理坐标的转换关系
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

	/* 设置显示窗口 WINDOWS */
	LCD_WriteReg(0x0210, 0);	/* 水平起始地址 */
	LCD_WriteReg(0x0211, 239);	/* 水平结束坐标 */
	LCD_WriteReg(0x0212, 0);	/* 垂直起始地址 */
	LCD_WriteReg(0x0213, 399);	/* 垂直结束地址 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_Init_61509
*	功能说明: 初始化61509屏
*	形    参：无
*	返 回 值: 无
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
	LCD_WriteReg(0x100,0x0730);	/* BT,AP 0x0330　*/
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
*	函 数 名: LCD_DispOn
*	功能说明: 打开显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOn(void)
{
	LCD_WriteReg(7, 0x0173); /* 设置262K颜色并且打开显示 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispOff
*	功能说明: 关闭显示
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispOff(void)
{
	LCD_WriteReg(7, 0x0);	/* 关闭显示*/
}

/*
*********************************************************************************************************
*	函 数 名: LCD_ClrScr
*	功能说明: 根据输入的颜色值清屏
*	形    参：_usColor : 背景色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_ClrScr(uint16_t _usColor)
{
	uint32_t i;

	LCD_SetCursor(0, 0);		/* 设置光标位置 */

	LCD_REG = LR_GRAM; 			/* 准备读写显存 */

	for (i = 0; i < LCD_HEIGHT * LCD_WIDTH; i++)
	{
		LCD_RAM = _usColor;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DispStr
*	功能说明: 在LCD指定坐标（左上角）显示一个字符串
*	形    参：
*		_usX : X坐标，对于3.0寸宽屏，范围为【0 - 399】
*		_usY : Y坐标，对于3.0寸宽屏，范围为 【0 - 239】
*		_ptr  : 字符串指针
*		_tFont : 字体结构体，包含颜色、背景色(支持透明)、字体代码、文字间距等参数
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DispStr(uint16_t _usX, uint16_t _usY, char *_ptr, FONT_T *_tFont)
{
	uint32_t i;
	uint8_t code1;
	uint8_t code2;
	uint32_t address;
	uint8_t buf[24 * 24 / 8];	/* 最大支持24点阵汉字 */
	uint8_t m, width, height;
	uint16_t x, y;

	/* 暂时只支持16点阵宋体汉字 */
	if (_tFont->usFontCode == FC_ST_16X16)
	{
		height = 16;
		while (*_ptr != 0)
		{
			code1 = *_ptr;	/* ascii代码 或者汉字代码的高字节 */
			if (code1 < 0x80)
			{
				/* 将ascii字符点阵复制到buf */
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

				/* 计算16点阵汉字点阵地址
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
							/* 字库搜索完毕，未找到，则填充全FF */
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
			/* 开始刷LCD */
			for (m = 0; m < height; m++)	/* 字符高度 */
			{
				x = _usX;
				for (i = 0; i < width; i++)	/* 字符宽度 */
				{
					if ((buf[m * (width / 8) + i / 8] & (0x80 >> (i % 8))) != 0x00)
					{
						LCD_PutPixel(x, y, _tFont->usTextColor);	/* 设置像素颜色为文字色 */
					}
					else if (_tFont->usBackColor != CL_MASK)		/* 如果是颜色掩码值，则做透明处理 */
					{
						LCD_PutPixel(x, y, _tFont->usBackColor);	/* 设置像素颜色为文字背景色 */;
					}

					x++;
				}
				y++;
			}

			if (_tFont->usBackColor != CL_MASK && _tFont->usSpace > 0)
			{
				/* 如果文字底色按_tFont->usBackColor，并且字间距大于点阵的宽度，那么需要在文字之间填充(暂时未实现) */
			}
			_usX += width + _tFont->usSpace;	/* 列地址递增 */
			_ptr++;			/* 指向下一个字符 */
		}
	}
	else
	{
		/* 暂时不支持其他字体 */
		return;
	}
}

/*
*********************************************************************************************************
*	函 数 名: LCD_PutPixel
*	功能说明: 画1个像素
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_PutPixel(uint16_t _usX, uint16_t _usY, uint16_t _usColor)
{
	LCD_SetCursor(_usX, _usY);	/* 设置光标位置 */

	/* 写显存 */
	LCD_REG = LR_GRAM;
	/* Write 16-bit GRAM Reg */
	LCD_RAM = _usColor;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_GetPixel
*	功能说明: 读取1个像素
*	形    参：
*			_usX,_usY : 像素坐标
*			_usColor  ：像素颜色
*	返 回 值: RGB颜色值
*********************************************************************************************************
*/
uint16_t LCD_GetPixel(uint16_t _usX, uint16_t _usY)
{
	uint16_t usRGB;

	LCD_SetCursor(_usX, _usY);	/* 设置光标位置 */

	/* 准备写显存 */
	LCD_REG = LR_GRAM;

	usRGB = LCD_RAM;

	/* 读 16-bit GRAM Reg */
	if (s_RGBChgEn == 1)
	{
		usRGB = LCD_BGR2RGB(usRGB);
	}

	return usRGB;
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawLine
*	功能说明: 采用 Bresenham 算法，在2点间画一条直线。
*	形    参：
*			_usX1, _usY1 ：起始点坐标
*			_usX2, _usY2 ：终止点Y坐标
*			_usColor     ：颜色
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawLine(uint16_t _usX1 , uint16_t _usY1 , uint16_t _usX2 , uint16_t _usY2 , uint16_t _usColor)
{
	int32_t dx , dy ;
	int32_t tx , ty ;
	int32_t inc1 , inc2 ;
	int32_t d , iTag ;
	int32_t x , y ;

	/* 采用 Bresenham 算法，在2点间画一条直线 */

	LCD_PutPixel(_usX1 , _usY1 , _usColor);

	/* 如果两点重合，结束后面的动作。*/
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

	if ( dx < dy )   /*如果dy为计长方向，则交换纵横坐标。*/
	{
		uint16_t temp;

		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	while ( x != _usX2 )     /* 循环画点 */
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
*	函 数 名: LCD_DrawPoints
*	功能说明: 采用 Bresenham 算法，绘制一组点，并将这些点连接起来。可用于波形显示。
*	形    参：
*			x, y     ：坐标数组
*			_usColor ：颜色
*	返 回 值: 无
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
*	函 数 名: LCD_DrawRect
*	功能说明: 绘制水平放置的矩形。
*	形    参：
*			_usX,_usY：矩形左上角的坐标
*			_usHeight ：矩形的高度
*			_usWidth  ：矩形的宽度
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawRect(uint16_t _usX, uint16_t _usY, uint8_t _usHeight, uint16_t _usWidth, uint16_t _usColor)
{
	/*
	 ---------------->---
	|(_usX，_usY)        |
	V                    V  _usHeight
	|                    |
	 ---------------->---
		  _usWidth
	*/

	LCD_DrawLine(_usX, _usY, _usX + _usWidth - 1, _usY, _usColor);	/* 顶 */
	LCD_DrawLine(_usX, _usY + _usHeight - 1, _usX + _usWidth - 1, _usY + _usHeight - 1, _usColor);	/* 底 */

	LCD_DrawLine(_usX, _usY, _usX, _usY + _usHeight - 1, _usColor);	/* 左 */
	LCD_DrawLine(_usX + _usWidth - 1, _usY, _usX + _usWidth - 1, _usY + _usHeight - 1, _usColor);	/* 右 */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawCircle
*	功能说明: 绘制一个圆，笔宽为1个像素
*	形    参：
*			_usX,_usY  ：圆心的坐标
*			_usRadius  ：圆的半径
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawCircle(uint16_t _usX, uint16_t _usY, uint16_t _usRadius, uint16_t _usColor)
{
	int32_t  D;			/* Decision Variable */
	uint32_t  CurX;		/* 当前 X 值 */
	uint32_t  CurY;		/* 当前 Y 值 */

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
*	函 数 名: LCD_DrawBMP
*	功能说明: 在LCD上显示一个BMP位图，位图点阵扫描次序：从左到右，从上到下
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	uint32_t index = 0;
	const uint16_t *p;

	/* 设置图片的位置和大小， 即设置显示窗口 */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* 准备读写显存 */

	p = _ptr;
	for (index = 0; index < _usHeight * _usWidth; index++)
	{
		/*
			armfly : 进行优化, 函数就地展开
			LCD_WriteRAM(_ptr[index]);

			此处可考虑用DMA操作
		*/
		LCD_RAM = *p++;
	}

	/* 退出窗口绘图模式 */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIcon
*	功能说明: 在LCD上绘制一个图标，四角自动切为弧脚
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIcon(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	/* 设置图片的位置和大小， 即设置显示窗口 */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* 准备读写显存 */

	p = _ptr;
	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			#if 1	/* 这个分支将图标的4个直角切割为弧角，弧角外是背景图标 */
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
					usNewRGB = LCD_RAM;	/* 空读一个像素，内部LCD指针自动增加 */;
					if (s_AddrAutoInc == 0)
					{
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						LCD_RAM = usNewRGB;		/* 61509屏需要再写一次才能递增地址 */
					}
				}
				else
				{
					LCD_RAM = usNewRGB;
				}
			#else 	/* 下面这个分支可以使图标中的白色成为透明颜色 */
				if (usNewRGB == 0xFFFF) 	/* 0xFFFF是白色值 */
				{
					usNewRGB = LCD_RAM;	/* 空读一个像素，内部LCD指针自动增加 */;
					if (s_AddrAutoInc == 0)
					{
						usNewRGB = LCD_RAM;
						usNewRGB = LCD_RAM;
						LCD_RAM = usNewRGB;		/* 61509屏需要再写一次才能递增地址 */
					}
				}
				else
				{
					LCD_RAM = usNewRGB;
				}
			#endif
		}
	}

	/* 退出窗口绘图模式 */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	函 数 名: LCD_DrawIconActive
*	功能说明: 在LCD上绘制一个激活选中的图标
*	形    参：
*			_usX, _usY : 图片的坐标
*			_usHeight  ：图片高度
*			_usWidth   ：图片宽度
*			_ptr       ：图片点阵指针
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_DrawIconActive(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr)
{
	const uint16_t *p;
	uint16_t usNewRGB;
	uint16_t x, y;		/* 用于记录窗口内的相对坐标 */

	/* 设置图片的位置和大小， 即设置显示窗口 */
	LCD_SetDispWin(_usX, _usY, _usHeight, _usWidth);

	LCD_REG = LR_GRAM; 			/* 准备读写显存 */

	p = _ptr;
	for (x = 0; x < _usWidth; x++)
	{
		for (y = 0; y < _usHeight; y++)
		{
			usNewRGB = *p++;	/* 读取图标的颜色值后指针加1 */
			#if 1	/* 这个分支将图标的4个直角切割为弧角，弧角外是背景图标 */
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
					usNewRGB = LCD_RAM;	/* 空读一个像素，内部LCD指针自动增加 */;
					if (s_AddrAutoInc == 0)
					{
						LCD_RAM = usNewRGB;		/* 61509屏需要再写一次才能递增地址 */
					}
				}
				else
				{
					/* 降低原始像素的亮度，实现图标被激活选中的效果 */
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
			#else 	/* 下面这个分支可以使图标中的白色成为透明颜色 */
				if (usNewRGB != 0xFFFF) 	/* 0xFFFF是白色值 */
				{
					LCD_RAM = usNewRGB;
				}
				else
				{
					/* 颜色代码bit分配: R5 G6 B5 */
					usNewRGB = LCD_RAM;	/* 空读一个像素，内部LCD指针自动增加 */
				}
			#endif
		}
	}

	/* 退出窗口绘图模式 */
	LCD_QuitWinMode();
}

/*
*********************************************************************************************************
*	函 数 名: LCD_WriteReg
*	功能说明: 修改LCD控制器的寄存器的值。
*	形    参：
*			LCD_Reg ：寄存器地址;
*			LCD_RegValue : 寄存器值
*	返 回 值: 无
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
*	函 数 名: LCD_ReadReg
*	功能说明: 读取LCD控制器的寄存器的值。
*	形    参：
*			LCD_Reg ：寄存器地址;
*			LCD_RegValue : 寄存器值
*	返 回 值: 无
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
*	函 数 名: LCD_SetDispWin
*	功能说明: 设置显示窗口，进入窗口显示模式。TFT驱动芯片支持窗口显示模式，这是和一般的12864点阵LCD的最大区别。
*	形    参：
*		_usX : 水平坐标
*		_usY : 垂直坐标
*		_usHeight: 窗口高度
*		_usWidth : 窗口宽度
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth)
{
	uint16_t px, py;

	/*
		240x400屏幕物理坐标(px,py)如下:
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

		按照安富莱开发板LCD的方向，我们期望的虚拟坐标和扫描方向如下：(和上图第1个吻合)
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
	虚拟坐标和物理坐标转换关系：
		x = 399 - py;
		y = px;

		py = 399 - x;
		px = y;
	*/

	py = 399 - _usX;
	px = _usY;

	/* 设置显示窗口 WINDOWS */
	LCD_WriteReg(0x0210, px);						/* 水平起始地址 */
	LCD_WriteReg(0x0211, px + (_usHeight - 1));		/* 水平结束坐标 */
	LCD_WriteReg(0x0212, py + 1 - _usWidth);		/* 垂直起始地址 */
	LCD_WriteReg(0x0213, py);						/* 垂直结束地址 */

	LCD_SetCursor(_usX, _usY);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_SetCursor
*	功能说明: 设置光标位置
*	形    参：_usX : X坐标; _usY: Y坐标
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_SetCursor(uint16_t _usX, uint16_t _usY)
{
	/*
		px，py 是物理坐标， x，y是虚拟坐标
		转换公式:
		py = 399 - x;
		px = y;
	*/

	LCD_WriteReg(0x0200, _usY);  		/* px */
	LCD_WriteReg(0x0201, 399 - _usX);	/* py */
}

/*
*********************************************************************************************************
*	函 数 名: LCD_BGR2RGB
*	功能说明: RRRRRGGGGGGBBBBB 改为 BBBBBGGGGGGRRRRR 格式
*	形    参：RGB颜色代码
*	返 回 值: 转化后的颜色代码
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
*	函 数 名: LCD_QuitWinMode
*	功能说明: 退出窗口显示模式，变为全屏显示模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_QuitWinMode(void)
{
	LCD_SetDispWin(0, 0, LCD_HEIGHT, LCD_WIDTH);
}

/*
*********************************************************************************************************
*	函 数 名: LCD_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void LCD_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/* 使能 FSMC, GPIOD, GPIOE, GPIOF, GPIOG 和 AFIO 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
	                     RCC_APB2Periph_AFIO, ENABLE);

	/* 设置 PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) 为复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14 |
	                            GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* 设置 PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) 为复用推挽输出 */
	/* PE3,PE4 用于A19, A20, STM32F103ZE-EK(REV 1.0)必须使能 */
	/* PE5,PE6 用于A19, A20, STM32F103ZE-EK(REV 2.0)必须使能 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
	                            GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
	                            GPIO_Pin_15 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* 设置 PF.00(A0 (RS))  为复用推挽输出 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* 设置 PG.12(NE4 (LCD/CS)) 为复用推挽输出 - CE3(LCD /CS) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

}

/*
*********************************************************************************************************
*	函 数 名: LCD_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参：无
*	返 回 值: 无
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
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;	/* 注意旧库无这个成员 */
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
*	函 数 名: LCD_SetBackLight
*	功能说明: 初始化控制LCD背景光的GPIO,配置为PWM模式。
*			当关闭背光时，将CPU IO设置为浮动输入模式（推荐设置为推挽输出，并驱动到低电平)；将TIM3关闭 省电
*	形    参：_bright 亮度，0是灭，255是最亮
*	返 回 值: 无
*********************************************************************************************************
*/
void LCD_SetBackLight(uint8_t _bright)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	/* 第1步：打开GPIOB RCC_APB2Periph_AFIO 的时钟	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

	if (_bright == 0)
	{
		/* 配置背光GPIO为输入模式 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* 关闭TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}
	else if (_bright == BRIGHT_MAX)	/* 最大亮度 */
	{
		/* 配置背光GPIO为推挽输出模式 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_SetBits(GPIOB, GPIO_Pin_1);

		/* 关闭TIM3 */
		TIM_Cmd(TIM3, DISABLE);
		return;
	}

	/* 配置背光GPIO为复用推挽输出模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 使能TIM3的时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	/*
		TIM3 配置: 产生1路PWM信号;
		TIM3CLK = 72 MHz, Prescaler = 0(不分频), TIM3 counter clock = 72 MHz
		计算公式：
		PWM输出频率 = TIM3 counter clock /(ARR + 1)

		我们期望设置为100Hz

		如果不对TIM3CLK预分频，那么不可能得到100Hz低频。
		我们设置分频比 = 1000， 那么  TIM3 counter clock = 72KHz
		TIM_Period = 720 - 1;
		频率下不来。
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
		_bright = 1 时, TIM_Pulse = 1
		_bright = 255 时, TIM_Pulse = TIM_Period
	*/
	TIM_OCInitStructure.TIM_Pulse = (TIM_TimeBaseStructure.TIM_Period * _bright) / BRIGHT_MAX;	/* 改变占空比 */

	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);

	/* 使能 TIM3 定时器 */
	TIM_Cmd(TIM3, ENABLE);
}

