/*
*********************************************************************************************************
*	                                  
*	模块名称 : WM8978音频芯片驱动模块
*	文件名称 : bsp_WM8978.c
*	版    本 : V1.0
*	说    明 : WM8978音频芯片和STM32 I2S底层驱动。在安富莱STM32开发板上调试通过。
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-09-11 armfly  实现I2S放音和录音配置，ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include "bsp_WM8978.h"
#include "bsp_i2c_gpio.h"

/*
*********************************************************************************************************
*
*	重要提示:
*	1、wm8978_ 开头的函数是操作WM8978寄存器，操作WM8978寄存器是通过I2C模拟总线进行的
*	2、I2S_ 开头的函数是操作STM32  I2S相关寄存器
*	3、实现录音或放音应用，需要同时操作WM8978和STM32的I2S。
*	4、部分函数用到的形参的定义在ST固件库中，比如：I2S_Standard_Phillips、I2S_Standard_MSB、I2S_Standard_LSB
*			  I2S_MCLKOutput_Enable、I2S_MCLKOutput_Disable
*			  I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、I2S_AudioFreq_44K、I2S_AudioFreq_48
*			  I2S_Mode_MasterTx、I2S_Mode_MasterRx
*	5、注释中 pdf 指的是 wm8978.pdf 数据手册，wm8978de寄存器很多，用到的寄存器会注释pdf文件的页码，便于查询
*
*********************************************************************************************************
*/

/* 仅在本模块内部使用的局部函数 */
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr);
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue);
static void I2S_GPIO_Config(void);
static void I2S_Mode_Config(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq, uint16_t _usMode);
static void I2S_NVIC_Config(void);
static void wm8978_CfgInOut(uint8_t _ucDacEn, uint8_t _ucAuxEn, uint8_t _ucLineEn, uint8_t _ucSpkEn, uint8_t _ucEarEn);
static void wm8978_CtrlGPIO1(uint8_t _ucValue);
static void wm8978_Reset(void);
static void wm8978_CfgAdc(uint8_t _ucMicEn, uint8_t _ucAuxEn, uint8_t _ucLineEn);

/*
	wm8978寄存器缓存
	由于WM8978的I2C两线接口不支持读取操作，因此寄存器值缓存在内存中，当写寄存器时同步更新缓存，读寄存器时
	直接返回缓存中的值。
	寄存器MAP 在WM8978.pdf 的第67页，寄存器地址是7bit， 寄存器数据是9bit
*/
static uint16_t wm8978_RegCash[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
};

/*
*********************************************************************************************************
*	函 数 名: wm8978_Init
*	功能说明: 配置I2C GPIO，并检查I2C总线上的WM8978是否正常
*	形    参：无
*	返 回 值: 1 表示初始化正常，0表示初始化不正常
*********************************************************************************************************
*/
uint8_t wm8978_Init(void)
{
	uint8_t re;
	
	if (i2c_CheckDevice(WM8978_SLAVE_ADDRESS) == 0)	/* 这个函数会配置STM32的GPIO用于软件模拟I2C时序 */
	{
		re = 1;
	}
	else
	{
		re = 0;
	}
	wm8978_Reset();			/* 硬件复位WM8978所有寄存器到缺省状态 */
	wm8978_CtrlGPIO1(1);	/* WM8978的GPIO1引脚输出1，表示缺省是放音(仅针对安富莱开发板硬件需要) */
	return re;
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Dac2Ear
*	功能说明: 初始化wm8978硬件设备,DAC输出到耳机
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Dac2Ear(void)
{
	wm8978_CfgInOut(DAC_ON, AUX_OFF, LINE_OFF, SPK_OFF, EAR_ON);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Dac2Spk
*	功能说明: 初始化wm8978硬件设备,DAC输出到扬声器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Dac2Spk(void)
{
	wm8978_CfgInOut(DAC_ON, AUX_OFF, LINE_OFF, SPK_ON, EAR_OFF);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Aux2Ear
*	功能说明: 初始化wm8978硬件设备,Aux(FM收音机)输出到耳机
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Aux2Ear(void)
{
	wm8978_CfgInOut(DAC_OFF, AUX_ON, LINE_OFF, SPK_OFF, EAR_ON);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Aux2Spk
*	功能说明: 初始化wm8978硬件设备,Aux(FM收音机)输出到扬声器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Aux2Spk(void)
{
	wm8978_CfgInOut(DAC_OFF, AUX_ON, LINE_OFF, SPK_ON, EAR_OFF);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Mic2Ear
*	功能说明: 配置wm8978硬件,输入混音（AUX,LINE,MIC）直接输出到耳机
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Mic2Ear(void)
{
	wm8978_CfgInOut(DAC_OFF, AUX_OFF, LINE_ON, SPK_OFF, EAR_ON);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Mic2Spk
*	功能说明: 配置wm8978硬件,输入混音（AUX,LINE,MIC）直接输出到扬声器
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Mic2Spk(void)
{
	wm8978_CfgInOut(DAC_OFF, AUX_OFF, LINE_ON, SPK_ON, EAR_OFF);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Mic2Adc
*	功能说明: 配置wm8978硬件,MIC输入到ADC,准备录音
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_Mic2Adc(void)
{
	 wm8978_CfgAdc(MIC_ON, AUX_OFF, LINE_ON);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ChangeVolume
*	功能说明: 修改输出音量
*	形    参：_ucLeftVolume ：左声道音量值
*			  _ucLRightVolume : 右声道音量值
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_ChangeVolume(uint8_t _ucLeftVolume, uint8_t _ucRightVolume)
{
	uint16_t regL;
	uint16_t regR;

	if (_ucLeftVolume > 0x3F)
	{
		_ucLeftVolume = 0x3F;
	}

	if (_ucRightVolume > 0x3F)
	{
		_ucRightVolume = 0x3F;
	}

	regL = _ucLeftVolume;
	regR = _ucRightVolume;

	/* 先更新左声道缓存值 */
	wm8978_WriteReg(52, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(53, regR | 0x100);	/* 0x180表示 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */

	/* 先更新左声道缓存值 */
	wm8978_WriteReg(54, regL | 0x00);

	/* 再同步更新左右声道的音量 */
	wm8978_WriteReg(55, regR | 0x100);	/* 在音量为0时再更新，避免调节音量出现的“嘎哒”声 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ReadVolume
*	功能说明: 读回通道的音量.
*	形    参：无
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
uint8_t wm8978_ReadVolume(void)
{
	return (uint8_t)(wm8978_ReadReg(52) & 0x3F );
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Mute
*	功能说明: 输出静音.
*	形    参：_ucMute ：1是静音，0是不静音.
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
void wm8978_Mute(uint8_t _ucMute)
{
	uint16_t usRegValue;

	if (_ucMute == 1) /* 静音 */
	{
		usRegValue = wm8978_ReadReg(52); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Right Mixer Control */
		usRegValue |= (1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
	else	/* 取消静音 */
	{
		usRegValue = wm8978_ReadReg(52);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(52, usRegValue);

		usRegValue = wm8978_ReadReg(53); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(53, usRegValue);

		usRegValue = wm8978_ReadReg(54);
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(54, usRegValue);

		usRegValue = wm8978_ReadReg(55); /* Left Mixer Control */
		usRegValue &= ~(1u << 6);
		wm8978_WriteReg(55, usRegValue);
	}
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_SetMicGain
*	功能说明: 设置MIC增益
*	形    参：_ucGain ：音量值
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_SetMicGain(uint8_t _ucGain)
{
	if (_ucGain > GAIN_MAX)
	{
		_ucGain = GAIN_MAX;
	}
	
	/* PGA 音量控制  R45， R46   pdf 19页 
		Bit8	INPPGAUPDATE
		Bit7	INPPGAZCL		过零再更改
		Bit6	INPPGAMUTEL		PGA静音
		Bit5:0	增益值，010000是0dB	
	*/
	wm8978_WriteReg(45, _ucGain);	
	wm8978_WriteReg(46, _ucGain | (1 << 8));	
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_PowerDown
*	功能说明: 关闭wm8978，进入低功耗模式
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_PowerDown(void)
{
	/*
	Set DACMU = 1 to mute the audio DACs.
	Disable all Outputs.
	Disable VREF and VMIDSEL.
	Switch off the power supplies
	*/
	uint16_t usRegValue;

	usRegValue = wm8978_ReadReg(10);
	usRegValue |= (1u <<6);
	wm8978_WriteReg(10, usRegValue);

	/* 未完 */
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CfgAudioIF
*	功能说明: 配置WM8978的音频接口(I2S)
*	形    参：
*			  _usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
*			  _ucWordLen : 字长，16、24、32  （丢弃不常用的20bit格式）
*			  _usMode : CPU I2S的工作模式，I2S_Mode_MasterTx、I2S_Mode_MasterRx、
*						安富莱开发板硬件不支持 I2S_Mode_SlaveTx、I2S_Mode_SlaveRx 模式，这需要WM8978连接
*						外部振荡器
*	返 回 值: 无
*********************************************************************************************************
*/
void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen, uint16_t _usMode)
{
	uint16_t usReg;
	
	/* pdf 67页，寄存器列表 */

	/*	REG R4, 音频接口控制寄存器
		B8		BCP	 = X, BCLK极性，0表示正常，1表示反相
		B7		LRCP = x, LRC时钟极性，0表示正常，1表示反相
		B6:5	WL = x， 字长，00=16bit，01=20bit，10=24bit，11=32bit （右对齐模式只能操作在最大24bit)
		B4:3	FMT = x，音频数据格式，00=右对齐，01=左对齐，10=I2S格式，11=PCM
		B2		DACLRSWAP = x, 控制DAC数据出现在LRC时钟的左边还是右边
		B1 		ADCLRSWAP = x，控制ADC数据出现在LRC时钟的左边还是右边
		B0		MONO	= 0，0表示立体声，1表示单声道，仅左声道有效
	*/
	usReg = 0;
	if (_usStandard == I2S_Standard_Phillips)	/* I2S飞利浦标准 */
	{
		usReg |= (2 << 3);
	}
	else if (_usStandard == I2S_Standard_MSB)	/* MSB对齐标准(左对齐) */
	{
		usReg |= (1 << 3);
	}
	else if (_usStandard == I2S_Standard_LSB)	/* LSB对齐标准(右对齐) */
	{
		usReg |= (0 << 3);
	}	
	else	/* PCM标准(16位通道帧上带长或短帧同步或者16位数据帧扩展为32位通道帧) */
	{
		usReg |= (3 << 3);;
	}

	if (_ucWordLen == 24)
	{
		usReg |= (2 << 5);
	}	
	else if (_ucWordLen == 32)
	{
		usReg |= (3 << 5);
	}		
	else
	{
		usReg |= (0 << 5);		/* 16bit */
	}
	wm8978_WriteReg(4, usReg);

	/* R5  pdf 57页 */


	/* 
		R6，时钟产生控制寄存器		
		MS = 0,  WM8978被动时钟，由MCU提供MCLK时钟
	*/	
	wm8978_WriteReg(6, 0x000);

	/* 如果是放音则需要设置  WM_GPIO1 = 1 ,如果是录音则需要设置WM_GPIO1 = 0 */
	if (_usMode == I2S_Mode_MasterTx)
	{
		wm8978_CtrlGPIO1(1);	/* 驱动WM8978的GPIO1引脚输出1, 用于放音 */		
	}
	else
	{
		wm8978_CtrlGPIO1(0);	/* 驱动WM8978的GPIO1引脚输出0, 用于录音 */		
	}
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_ReadReg
*	功能说明: 从cash中读回读回wm8978寄存器
*	形    参：_ucRegAddr ： 寄存器地址
*	返 回 值: 无
*********************************************************************************************************
*/
static uint16_t wm8978_ReadReg(uint8_t _ucRegAddr)
{
	return wm8978_RegCash[_ucRegAddr];
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_WriteReg
*	功能说明: 写wm8978寄存器
*	形    参：_ucRegAddr ： 寄存器地址
*			  _usValue ：寄存器值
*	返 回 值: 无
*********************************************************************************************************
*/
static uint8_t wm8978_WriteReg(uint8_t _ucRegAddr, uint16_t _usValue)
{
	uint8_t ucAck;

	/* 发送起始位 */
	i2c_Start();

	/* 发送设备地址+读写控制bit（0 = w， 1 = r) bit7 先传 */
	i2c_SendByte(WM8978_SLAVE_ADDRESS | I2C_WR);

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送控制字节1 */
	i2c_SendByte(((_ucRegAddr << 1) & 0xFE) | ((_usValue >> 8) & 0x1));

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送控制字节2 */
	i2c_SendByte(_usValue & 0xFF);

	/* 检测ACK */
	ucAck = i2c_WaitAck();
	if (ucAck == 1)
	{
		return 0;
	}

	/* 发送STOP */
	i2c_Stop();

	wm8978_RegCash[_ucRegAddr] = _usValue;
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CfgInOut
*	功能说明: 配置wm8978输入和输出通道
*	形    参：
*			_ucDacEn : DAC输入通道使能(CPU通过I2S接口传送的数字音频信号)
*			_ucAuxEn : 辅助输入通道使能（FM收音机模块的音频输出信号）
*			_ucLineEn : 线路输入通道使能（V2板是空脚，V3板连接到VS1003B解码芯片的音频输出）
*			_ucSpkEn : 扬声器输出使能
*			_ucEarEn : 耳机输出使能
*	返 回 值: 无
*********************************************************************************************************
*/
static void wm8978_CfgInOut(uint8_t _ucDacEn, uint8_t _ucAuxEn, uint8_t _ucLineEn, uint8_t _ucSpkEn, uint8_t _ucEarEn)
{
	uint16_t usReg;
	
	/* pdf 67页，寄存器列表 */

	/*	REG 1
		B8		BUFDCOPEN	= x
		B7		OUT4MIXEN	= x
		B6		OUT3MIXEN	= X  耳机输出时，必须设置为1 (for 安富莱开发板)
		B5		PLLEN	= x
		b4`		MICBEN = x
		B3		BIASEN = 1		必须设置为1模拟放大器才工作
		B2		BUFIOEN = x
		B1:0	VMIDSEL = 3  必须设置为非00值模拟放大器才工作
	*/
	usReg = 0;
	if ((_ucSpkEn == 1) || (_ucEarEn == 1))
	{
		usReg = ((1 << 3) | (3 << 0));
	}
	if (_ucEarEn == 1)
	{
		usReg |= (1 << 6);
	}
	wm8978_WriteReg(1, usReg);

	/*	REG 2
		B8		ROUT1EN = 1;	耳机输出通道
		B7		LOUT1EN = 1;	耳机输出通道	
		B6		SLEEP = x;
		B5		BOOSTENR = x;
		B4		BOOSTENL = x;
		B3		INPGAENR = x;
		B2		NPPGAENL = x;
		B1		ADCENR = x;
		B0		ADCENL = x;
	*/
	usReg = 0;
	if (_ucEarEn == 1)
	{
		usReg = ((1 << 8) | (1 << 7));
	}
	wm8978_WriteReg(2, usReg);		
		
	/* REG 3
		B8	OUT4EN = 0
		B7	OUT3EN = x;		耳机输出，用于耳机的地线
		B6	LOUT2EN = 1;	扬声器输出通道
		B5	ROUT2EN = 1;	扬声器输出通道
		B4	----   = x
		B3	RMIXEN = 1;		输出MIX, 耳机和扬声器公用
		B2	LMIXEN = 1;		输出MIX, 耳机和扬声器公用
		B1	DACENR = x;		DAC用
		B0	DACENL = x;
	*/
	usReg = 0;
	if ((_ucSpkEn == 1) || (_ucEarEn == 1))
	{
		usReg |= ((1 << 3) | (1 << 2));
	}
	if (_ucEarEn == 1)
	{
		usReg |= (1 << 7);
	}
	if (_ucSpkEn == 1)
	{
		usReg |= ((1 << 6) | (1 << 5));
	}
	if (_ucDacEn == 1)
	{
		usReg |= ((1 << 1) | (1 << 0));
	}
	wm8978_WriteReg(3, usReg);

	/*
		REG 11,12
		DAC 音量
	*/
	if (_ucDacEn == 1)
	{	
		#if 0	/* 此处不要设置音量, 避免切换时音量状态被改变 */
		wm8978_WriteReg(11, 255);
		wm8978_WriteReg(12, 255 | 0x100);
		#endif
	}
	else
	{
		;
	}

	/*	REG 43
		B8:6 = 0
		B5	MUTERPGA2INV = 0;	Mute input to INVROUT2 mixer
		B4	INVROUT2 = ROUT2 反相; 用于扬声器推挽输出
		B3:1	BEEPVOL = 7;	AUXR input to ROUT2 inverter gain
		B0	BEEPEN = 1;	1 = enable AUXR beep input
			
	*/
	usReg = 0;
	if (_ucSpkEn == 1)
	{
		usReg |= (1 << 4);
	} 
	if (_ucAuxEn == 1)
	{
		usReg |= ((7 << 1) | (1 << 0));
	}
	wm8978_WriteReg(43, usReg);
		
	/* REG 49
		B8:7	0
		B6		DACL2RMIX = x
		B5		DACR2LMIX = x
		B4		OUT4BOOST = 0
		B3		OUT3BOOST = 0
		B2		SPKBOOST = x	SPK BOOST
		B1		TSDEN = 1    扬声器热保护使能（缺省1）
		B0		VROI = 0	Disabled Outputs to VREF Resistance
	*/
	usReg = 0;
	if (_ucDacEn == 1)
	{
		usReg |= ((0 << 6) | (0 << 5));
	}
	if (_ucSpkEn == 1)
	{
		usReg |=  ((0 << 2) | (1 << 1));	/* 1.5x增益,  热保护使能 */
	}
	wm8978_WriteReg(49, usReg);
	
	/*	REG 50    (50是左声道，51是右声道，配置寄存器功能一致) pdf 40页
		B8:6	AUXLMIXVOL = 111	AUX用于FM收音机信号输入
		B5		AUXL2LMIX = 1		Left Auxilliary input to left channel
		B4:2	BYPLMIXVOL			音量
		B1		BYPL2LMIX = 0;		Left bypass path (from the left channel input boost output) to left output mixer
		B0		DACL2LMIX = 1;		Left DAC output to left output mixer
	*/
	usReg = 0;
	if (_ucAuxEn == 1)
	{
		usReg |= ((7 << 6) | (1 << 5));
	}
	if (_ucLineEn == 1)
	{
		usReg |= ((7 << 2) | (1 << 1));
	}
	if (_ucDacEn == 1)
	{
		usReg |= (1 << 0);
	}
	wm8978_WriteReg(50, usReg);
	wm8978_WriteReg(51, usReg);

	/*	
		REG 52,53	控制EAR音量
		REG 54,55	控制SPK音量
		
		B8		HPVU		用于同步更新左右声道
		B7		LOUT1ZC = 1  零位切换
		B6		LOUT1MUTE    0表示正常， 1表示静音
	*/
#if 0	/* 此处不要设置音量, 避免应用程序切换输出时，音量状态被改变 */
	if (_ucEarEn == 1)
	{
		usReg = (0x3f | (1 << 7));
		wm8978_WriteReg(52, usReg);
		wm8978_WriteReg(53, usReg | (1 << 8));
	}
	else
	{
		usReg = ((1 << 7) | (1 << 6));
		wm8978_WriteReg(52, usReg);
		wm8978_WriteReg(53, usReg | (1 << 8));
	}
	
	if (_ucSpkEn == 1)
	{
		usReg = (0x3f | (1 << 7));
		wm8978_WriteReg(54, usReg);
		wm8978_WriteReg(55, usReg | (1 << 8));
	}
	else
	{
		usReg = ((1 << 7) | (1 << 6));
		wm8978_WriteReg(54, usReg);
		wm8978_WriteReg(55, usReg | (1 << 8));
	}		
#endif	
		
	/*	REG 56   OUT3 mixer ctrl
		B6		OUT3MUTE = 1;
		B5		LDAC2OUT3 = 0;
		B4		OUT4_2OUT3
		B3		BYPL2OUT3
		B2		LMIX2OUT3	
		B1		LDAC2OUT3
	*/
	wm8978_WriteReg(56, (1 <<6));		/**/
				
	/* 	Softmute enable = 0 */
	if (_ucDacEn == 1)
	{
		wm8978_WriteReg(10, 0);
	}
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CfgAdc
*	功能说明: 配置wm8978 ADC通道
*	形    参：
*			_ucMicEn : MIC输入通道使能(V2和V3板载MIC连接到左声道)
*			_ucAuxEn : 辅助输入通道使能（FM收音机模块的音频输出信号）
*			_ucLineEn : 线路输入通道使能（V2板是空脚，V3板连接到VS1003B解码芯片的音频输出）
*	返 回 值: 无
*********************************************************************************************************
*/
static void wm8978_CfgAdc(uint8_t _ucMicEn, uint8_t _ucAuxEn, uint8_t _ucLineEn)
{
	uint16_t usReg;

	/* MIC偏置电路设置, pdf 23页 */
	if (_ucMicEn == 1)
	{
		/* R1 Power management 1 寄存器bit4 MICBEN 用于控制MIC偏置电路使能 */
		/* R1寄存器还有其他控制功能，为了不影响其他功能，我们先读出这个寄存器的值，然后再改写 */
		usReg = wm8978_ReadReg(1);
		usReg |= (1 << 4);			/* 驻极体话筒需要设置MIC偏置电路使能，动圈话筒不要设置 */
		wm8978_WriteReg(1, usReg);	/* 回写R1寄存器 */
		
		/* R44 的 Bit8 控制MIC偏置电路电压，0表示0.9*AVDD, 1表示0.6*AVDD 
			Bit0 LIP2INPPGA 使能LIP连接到PGA
			Bit0 LIN2INPPGA 使能LIN连接到PGA
			R44寄存器还有其他控制功能，为了不影响其他功能，我们先读出这个寄存器的值，然后再改写 
		*/
		usReg = wm8978_ReadReg(44);
		usReg |= (0 << 4);			/* 根据驻极体话筒的固有增益进行选择偏置电压 */
		usReg |= ((1 << 1) | (1 << 0));	 /* 使能MIC的2个口线连接到PGA */
		wm8978_WriteReg(44, usReg);	/* 回写R1寄存器 */		
	}

	/* R2 寄存器 	pdf  24页
		B8 	ROUT1EN = 1	使能输出ROUT1， 用于录音监听
		B7 	LOUT1EN = 1使能输出LOUT1，用于录音监听
		B6	SLEEP = 0 休眠控制，0表示正常工作，1表示进入休眠降低功耗， 
		B5	BOOSTENR = 1 使能右声道自举电路，需要使用输入信道的放大功能时，必须打开自举电路
		B4	BOOSTENL = 1 使能左声道自举电路
		B3  INPPGAENR = 1 使能右声道PGA使能，1表示使能 
		B2  INPPGAENL = 1 使能左声道PGA使能，1表示使能 
		B1  ADCENR = 1 使能右声道ADC，1表示使能 
		B0  ADCENL = 1 使能左声道ADC，1表示使能 
	*/		
	if ((_ucMicEn == 0) && (_ucAuxEn == 0) && (_ucLineEn == 0))
	{
		usReg = 0;
	}
	else
	{
		usReg = (1 << 8) | (1 << 7) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
	}
	wm8978_WriteReg(2, usReg);	/* 写R2寄存器 */
	
	/* 
		Mic 输入信道的增益由 PGABOOSTL 和 PGABOOSTR 控制
		Aux 输入信道的输入增益由 AUXL2BOOSTVO[2:0] 和 AUXR2BOOSTVO[2:0] 控制 
		Line 输入信道的增益由 LIP2BOOSTVOL[2:0] 和 RIP2BOOSTVOL[2:0] 控制
	*/
	/*	pdf 21页，R47（左声道），R48（由声道）, MIC防盗器增益控制寄存器
		R47 (R48定义与此相同)
		B8		PGABOOSTL	= 1, 0表示MIC信号直通无增益，1表示MIC信号+20dB增益（通过自举电路）
		B7		= 0， 保留
		B6:4	L2_2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
		B3		= 0， 保留
		B2:0`	AUXL2BOOSTVOL = x，0表示禁止，1-7表示增益-12dB ~ +6dB  （可以衰减也可以放大）
	*/
	usReg = 0;
	if (_ucMicEn == 1)
	{
		usReg |= (1 << 8);	/* MIC增益取+20dB */
	}
	if (_ucAuxEn == 1)
	{
		usReg |= (3 << 4);	/* Aux增益固定取3，用户可以自行调整 */
	}
	if (_ucLineEn == 1)
	{
		usReg |= (3 << 0);	/* Line增益固定取3，用户可以自行调整 */
	}
	wm8978_WriteReg(47, usReg);	/* 写左声道输入增益控制寄存器 */
	wm8978_WriteReg(48, usReg);	/* 写右声道输入增益控制寄存器 */

	/* 设置高通滤波器（可选的） pdf 24、25页, 
		Bit8 	HPFEN = x，高通滤波器使能，0表示禁止，1表示使能
		BIt7 	HPFAPP = x，选择音频模式或应用模式，0表示音频模式，
		Bit6:4	HPFCUT = x，000-111选择应用模式的截止频率
		Bit3 	ADCOSR 控制ADC过采样率，0表示64x（功耗低） 1表示128x（高性能）
		Bit2   	保留，填0
		Bit1 	ADCPOLR 控制右声道反相使能，1表示使能
		Bit0 	ADCPOLL 控制左声道反相使能，1表示使能
	*/
	usReg = (1 << 3);			/* 选择高通滤波器禁止，不反相，高性能采样率 */
	wm8978_WriteReg(14, usReg);	/* 回写R14寄存器 */
	

	/* 设置陷波滤波器（notch filter），主要用于抑制话筒声波正反馈，避免啸叫，此处不使用
		R27，R28，R29，R30 用于控制限波滤波器，pdf 26页
		R7的 Bit7 NFEN = 0 表示禁止，1表示使能
	*/
	usReg = (0 << 7);
	wm8978_WriteReg(27, usReg);	/* 写寄存器 */
	usReg = 0;
	wm8978_WriteReg(28, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
	wm8978_WriteReg(29, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
	wm8978_WriteReg(30, usReg);	/* 写寄存器,填0，因为已经禁止，所以也可不做 */
	
	/* 数字ADC音量控制，pdf 27页 
		R15 控制左声道ADC音量，R16控制右声道ADC音量
		Bit8 	ADCVU  = 1 时才更新，用于同步更新左右声道的ADC音量
		Bit7:0 	增益选择； 0000 0000 = 静音
						   0000 0001 = -127dB
						   0000 0010 = -12.5dB  （0.5dB 步长）
						   1111 1111 = 0dB  （不衰减）
	*/
	usReg = 0xFF;
	wm8978_WriteReg(15, usReg);	/* 选择0dB，先缓存左声道 */
	usReg = 0x1FF;
	wm8978_WriteReg(16, usReg);	/* 同步更新左右声道 */

#if 0	/* 通过wm8978_SetMicGain函数设置PGA增益 */
	/* 
		PGA 音量控制  R45， R46   pdf 19页
	*/
	usReg = (16 << 0); 
	wm8978_WriteReg(45, usReg);	
	wm8978_WriteReg(46, usReg | (1 << 8));	
#endif	
	
	/* 自动增益控制 ALC, R32  pdf 19页 */
	usReg = wm8978_ReadReg(32);
	usReg &= ~((1 << 8) | (1 << 7));	/* 禁止自动增益控制 */
	wm8978_WriteReg(32, usReg);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_CtrlGPIO1
*	功能说明: 控制WM8978的GPIO1引脚输出0或1
*	形    参：_ucValue ：GPIO1输出值，0或1
*	返 回 值: 当前音量值
*********************************************************************************************************
*/
static void wm8978_CtrlGPIO1(uint8_t _ucValue)
{
	uint16_t usRegValue;

	/* R8， pdf 62页 */
	if (_ucValue == 0) /* 输出0 */
	{
		usRegValue = 6; /* B2:0 = 110 */
	}
	else
	{
		usRegValue = 7; /* B2:0 = 111 */	
	}
	wm8978_WriteReg(8, usRegValue);
}

/*
*********************************************************************************************************
*	函 数 名: wm8978_Reset
*	功能说明: 复位wm8978，所有的寄存器值恢复到缺省值
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void wm8978_Reset(void)
{
	/* wm8978寄存器缺省值 */
	const uint16_t reg_default[] = {
	0x000, 0x000, 0x000, 0x000, 0x050, 0x000, 0x140, 0x000,
	0x000, 0x000, 0x000, 0x0FF, 0x0FF, 0x000, 0x100, 0x0FF,
	0x0FF, 0x000, 0x12C, 0x02C, 0x02C, 0x02C, 0x02C, 0x000,
	0x032, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
	0x038, 0x00B, 0x032, 0x000, 0x008, 0x00C, 0x093, 0x0E9,
	0x000, 0x000, 0x000, 0x000, 0x003, 0x010, 0x010, 0x100,
	0x100, 0x002, 0x001, 0x001, 0x039, 0x039, 0x039, 0x039,
	0x001, 0x001
	};
	uint8_t i;

	wm8978_WriteReg(0x00, 0);

	for (i = 0; i < sizeof(reg_default) / 2; i++)
	{
		wm8978_RegCash[i] = reg_default[i];
	}
}

/*
*********************************************************************************************************
*	                     下面的代码是和STM32 I2S硬件相关的
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	函 数 名: I2S_CODEC_Init
*	功能说明: 配置GPIO引脚和中断通道用于codec应用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void I2S_CODEC_Init(void)
{
	/* 配置I2S中断通道 */
	I2S_NVIC_Config(); 
	
	/* 配置I2S2 GPIO口线 */
	I2S_GPIO_Config(); 

	/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
		
	/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
}

/*
*********************************************************************************************************
*	函 数 名: I2S_StartPlay
*	功能说明: 配置I2S工作模式，启动I2S发送缓冲区空中断，开始放音
*	形    参：_usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
*			  _usMCLKOutput : 主时钟输出，I2S_MCLKOutput_Enable or I2S_MCLKOutput_Disable
*			  _usAudioFreq : 采样频率，I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、
*							I2S_AudioFreq_44K、I2S_AudioFreq_48
*	返 回 值: 无
*********************************************************************************************************
*/
void I2S_StartPlay(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq)
{
	/* 配置I2S为主发送模式，即STM32提供主时钟，I2S数据口是发送方向(放音) */
	I2S_Mode_Config(_usStandard, _usWordLen, _usAudioFreq, I2S_Mode_MasterTx);

	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);		/* 使能发送中断 */
}

/*
*********************************************************************************************************
*	函 数 名: I2S_StartRecord
*	功能说明: 配置I2S工作模式，启动I2S接收缓冲区不空中断，开始录音。在调用本函数前，请先配置WM8978录音通道
*	形    参：_usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
*			  _usMCLKOutput : 主时钟输出，I2S_MCLKOutput_Enable or I2S_MCLKOutput_Disable
*			  _usAudioFreq : 采样频率，I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、
*							I2S_AudioFreq_44K、I2S_AudioFreq_48
*	返 回 值: 无
*********************************************************************************************************
*/
void I2S_StartRecord(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq)
{
	/* 配置I2S为主发送模式，即STM32提供主时钟，I2S数据口是发送方向(放音) */
	I2S_Mode_Config(_usStandard, _usWordLen, _usAudioFreq, I2S_Mode_MasterRx);
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);		/* 使能接收中断 */
}

/*
*********************************************************************************************************
*	函 数 名: I2S_Stop
*	功能说明: 停止I2S工作
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void I2S_Stop(void)
{
	/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
		
	/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
		
	/* 禁能 SPI2/I2S2 外设 */
	I2S_Cmd(SPI2, DISABLE);

	/* 关闭 I2S2 APB1 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
}


/*
*********************************************************************************************************
*	函 数 名: I2S_GPIO_Config
*	功能说明: 配置GPIO引脚用于codec应用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2S_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable GPIOB, GPIOC and AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOG | 
	                     RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);
	
	/* I2S2 SD, CK and WS pins configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* I2S2 MCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: I2S_Config
*	功能说明: 配置STM32的I2S外设工作模式
*	形    参：_usStandard : 接口标准，I2S_Standard_Phillips, I2S_Standard_MSB 或 I2S_Standard_LSB
*			  _usMCLKOutput : 主时钟输出，I2S_MCLKOutput_Enable or I2S_MCLKOutput_Disable
*			  _usAudioFreq : 采样频率，I2S_AudioFreq_8K、I2S_AudioFreq_16K、I2S_AudioFreq_22K、
*							I2S_AudioFreq_44K、I2S_AudioFreq_48
*			  _usMode : CPU I2S的工作模式，I2S_Mode_MasterTx、I2S_Mode_MasterRx、
*						安富莱开发板硬件不支持 I2S_Mode_SlaveTx、I2S_Mode_SlaveRx 模式，这需要WM8978连接
*						外部振荡器
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2S_Mode_Config(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq, uint16_t _usMode)
{
	I2S_InitTypeDef I2S_InitStructure; 

	if ((_usMode == I2S_Mode_SlaveTx) && (_usMode == I2S_Mode_SlaveRx))
	{
		/* 安富莱开发板不支持这2种模式 */
		return;
	}

	/* 打开 I2S2 APB1 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	/* 复位 SPI2 外设到缺省状态 */
	SPI_I2S_DeInit(SPI2); 
		
	/* I2S2 外设配置 */
	if (_usMode == I2S_Mode_MasterTx)
	{
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;			/* 配置I2S工作模式 */
		I2S_InitStructure.I2S_Standard = _usStandard;			/* 接口标准 */
		I2S_InitStructure.I2S_DataFormat = _usWordLen;			/* 数据格式，16bit */
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;	/* 主时钟模式 */
		I2S_InitStructure.I2S_AudioFreq = _usAudioFreq;			/* 音频采样频率 */
		I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			
		I2S_Init(SPI2, &I2S_InitStructure);
	}
	else if (_usMode == I2S_Mode_MasterRx)
	{
		I2S_InitStructure.I2S_Mode = I2S_Mode_MasterRx;			/* 配置I2S工作模式 */
		I2S_InitStructure.I2S_Standard = _usStandard;			/* 接口标准 */
		I2S_InitStructure.I2S_DataFormat = _usWordLen;			/* 数据格式，16bit */
		I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;	/* 主时钟模式 */
		I2S_InitStructure.I2S_AudioFreq = _usAudioFreq;			/* 音频采样频率 */
		I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			
		I2S_Init(SPI2, &I2S_InitStructure);
	}

	/* 禁止I2S2 TXE中断(发送缓冲区空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
		
	/* 禁止I2S2 RXNE中断(接收不空)，需要时再打开 */ 
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, DISABLE);
	
	/* 使能 SPI2/I2S2 外设 */
	I2S_Cmd(SPI2, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: I2S_NVIC_Config
*	功能说明: 配置I2S NVIC通道(中断模式)。中断服务函数void SPI2_IRQHandler(void) 在stm32f10x_it.c
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void I2S_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* SPI2 IRQ 通道配置 */
	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
