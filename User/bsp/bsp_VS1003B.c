/*
*********************************************************************************************************
*	                                  
*	模块名称 : VS1003B mp3芯片模块
*	文件名称 : bsp_VS1003B.c
*	版    本 : V1.0
*	说    明 : VS1003B芯片底层驱动。
*	修改记录 :
*		版本号  日期       作者    说明
*		v0.1    2009-12-27 armfly  创建该文件，ST固件库版本为V3.1.2
*		v1.0    2011-09-11 armfly  实现I2S放音和录音配置，ST固件库升级到V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_VS1003B.h"	 

#define VS1003_CS_0()	GPIO_ResetBits(GPIOF, GPIO_Pin_9)
#define VS1003_CS_1()	GPIO_SetBits(GPIOF, GPIO_Pin_9)
#define VS1003_DS_0()	GPIO_ResetBits(GPIOF, GPIO_Pin_8)
#define VS1003_DS_1()	GPIO_SetBits(GPIOF, GPIO_Pin_8)
#define VS1003_IS_BUSY()	(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)

#define DUMMY_BYTE    0xFF		/* 可定义任意值 */

u8 vs1003ram[5]={0,0,0,0,250};

/*
*********************************************************************************************************
*	函 数 名: wm8978_Init
*	功能说明: 初始化VS1003B硬件设备
*	形    参：无
*	返 回 值: 1 表示初始化正常，0表示初始化不正常
*********************************************************************************************************
*/
void vs1003_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/*
		安富莱开发板和VS1003B的口线连接：
		PA6/SPI1_MISO <== SO
		PA7/SPI1_MOSI ==> SI
		PA5/SPI1_SCK  ==> SCLK
		PF9           ==> XCS\     (片选, 和LED4复用，访问VS1003B时，LED4会闪烁)
		PB5           <== DREQ
		PF8           ==> XDCS/BSYNC   (数据和命令选择)
		
		SPI1总线共有3个设备
		(1) SST25VF016B  , 片选为PB2
		(2) LCD模块的触摸IC(TS2046), 片选为PG11
		(3) CH376T (仅豪华版), 片选为PF10
		(4) VS1003B(仅豪华版)，片选为PF9
		
		CPU复位后 PB2,PG11,PF9,PF10均是输入模式，默认为低电平，会引起总线访问冲突。
		因此需要将未用的片选口线全部设置为输出高电平才能正常访问SPI1上的设备。
	*/
	/* 打开相关模块的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
		| RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOF
		| RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

	/*　配置SPI1口线：SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   /* 复用推挽输出 */
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/* 配置PB5作为VS1003B的数据请求 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    /* 输入 */
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	/* 配置PF8作为VS1003B的XDS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    /* 推挽输出 */
	GPIO_Init(GPIOF,&GPIO_InitStructure);	

	/* 将其他未用的SPI设备的片选设置为高电平 */
	#if 1	
		/* (1) SST25VF016B  , 片选为PB2 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_2);
		
		/* (2) LCD模块的触摸IC(TS2046), 片选为PG11 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
		GPIO_Init(GPIOG,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOG, GPIO_Pin_11);		
		
		/* (3) CH376T (仅豪华版), 片选为PF10 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
		GPIO_Init(GPIOF,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOF, GPIO_Pin_10);	
		
		/* (4) VS1003B(仅豪华版)，片选为PF9 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* 推挽输出 */
		GPIO_Init(GPIOF,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOF, GPIO_Pin_9);	
	#endif		

	/* SPI1 配置 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 选择2线全双工模式 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* CPU的SPI作为主设备 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 8个数据 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* CLK引脚空闲状态电平 = 1 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 数据采样在第2个边沿(上升沿) */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;  			/* 软件控制片选 */
	/* VS1003B SPI输入时钟最大2MHz，实际频率 = 72MHz / 64 = 1.125MHz */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;	
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 最高位先传输 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* SPI1 使能 */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: vs1003_WriteByte
*	功能说明: 向SPI1接口写一个字节
*	形    参：_ucByte ：数据字节.
*	返 回 值: 读到的数据.
*********************************************************************************************************
*/
uint8_t vs1003_WriteByte(uint8_t _ucByte)
{
	/* 等待发送区空 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	/* 发送1个字节 */
	SPI_I2S_SendData(SPI1, _ucByte);
	
	/* 等待接收完1个字节 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* 返回读到的数据 */
	return SPI_I2S_ReceiveData(SPI1);
}

/*
*********************************************************************************************************
*	函 数 名: vs1003_ReadByte
*	功能说明: 从SPI1接口读一个字节
*	形    参：无
*	返 回 值: 读到的数据.
*********************************************************************************************************
*/
uint8_t vs1003_ReadByte(void)
{
	return vs1003_WriteByte(DUMMY_BYTE);
}

/*
*********************************************************************************************************
*	函 数 名: vs1003_WriteCmd
*	功能说明: 向VS1003写命令
*	形    参：_ucAddr ： 地址； 		_usData ：数据
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_WriteCmd(uint8_t _ucAddr, uint16_t _usData)
{  
	while(VS1003_IS_BUSY());	/* 等待空闲 */

	VS1003_DS_1();
	VS1003_CS_0();

	vs1003_WriteByte(VS_WRITE_COMMAND);	/* 发送VS1003的写命令 */
	vs1003_WriteByte(_ucAddr); 			/* 寄存器地址 */
	vs1003_WriteByte(_usData >> 8); 	/* 发送高8位 */
	vs1003_WriteByte(_usData);	 		/* 发送低8位 */
	VS1003_CS_1();
} 

/*
*********************************************************************************************************
*	函 数 名: vs1003_ReqNewData
*	功能说明: 判断VS1003是否请求新数据。 VS1003内部有0.5k缓冲区。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t vs1003_ReqNewData(void)
{
	if (VS1003_IS_BUSY())
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

/*
*********************************************************************************************************
*	函 数 名: vs1003_PreWriteData
*	功能说明: 准备向VS1003写数据，调用1次即可
*	形    参：_无
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_PreWriteData(void)
{
	VS1003_CS_1();
	VS1003_DS_0();
} 

/*
*********************************************************************************************************
*	函 数 名: vs1003_WriteData
*	功能说明: 向VS1003写数据
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_WriteData(uint8_t _ucData)
{
	//while(VS1003_IS_BUSY());			/* 等待空闲 */
	VS1003_CS_1();
	VS1003_DS_0();
	vs1003_WriteByte(_ucData);
	VS1003_DS_1();
}         

/*
*********************************************************************************************************
*	函 数 名: vs1003_ReadReg
*	功能说明: 读VS1003的寄存器
*	形    参：_ucAddr:寄存器地址
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint16_t vs1003_ReadReg(uint8_t _ucAddr)
{ 
	uint16_t usTemp;
	
	while(VS1003_IS_BUSY());	/* 等待空闲 */
	
	VS1003_DS_1();
	VS1003_CS_0();
	vs1003_WriteByte(VS_READ_COMMAND);	/* 发送VS1003读命令 */
	vs1003_WriteByte(_ucAddr);			/* 发送地址 */
	usTemp = vs1003_ReadByte() << 8;	/* 读取高字节 */
	usTemp += vs1003_ReadByte();		/* 读取低字节 */
	VS1003_CS_1();
	return usTemp;
}  

/*
*********************************************************************************************************
*	函 数 名: vs1003_TestRam
*	功能说明: 测试VS1003B的内部RAM
*	形    参：无
*	返 回 值: 1表示OK, 0表示错误.
*********************************************************************************************************
*/																				 
uint8_t vs1003_TestRam(void)
{
	uint16_t usRegValue;
		   
 	vs1003_WriteCmd(SCI_MODE, 0x0820);	/* 进入vs1003的测试模式 */
 	
	while(VS1003_IS_BUSY());			/* 等待空闲 */
	
 	VS1003_DS_0();
	vs1003_WriteByte(0x4d);
	vs1003_WriteByte(0xea);
	vs1003_WriteByte(0x6d);
	vs1003_WriteByte(0x54);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	VS1003_DS_1();

	while(VS1003_IS_BUSY());			/* 等待空闲 */
		
	usRegValue = vs1003_ReadReg(SCI_HDAT0); /* 如果得到的值为0x807F，则表明OK */
	
	if (usRegValue == 0x807F)
	{
		return 1;
	}
	else
	{
		return 0;
	}
} 

/*
*********************************************************************************************************
*	函 数 名: vs1003_TestSine
*	功能说明: 正弦测试
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_TestSine(void)
{						
	/*
		正弦测试通过有序的8字节初始化，0x53 0xEF 0x6E n 0 0 0 0
		想要退出正弦测试模式的话，发送如下序列 0x45 0x78 0x69 0x74 0 0 0 0 .
		
		这里的n被定义为正弦测试使用，定义
		如下：
		n bits
		名称位 描述
		FsIdx 7：5 采样率索引
		S 4：0 正弦跳过速度
		正弦输出频率可通过这个公式计算：F=Fs×(S/128).
		例如：正弦测试值为126 时被激活，二进制为
		0b01111110。则FsIdx=0b011=3,所以Fs=22050Hz。
		S=0b11110=30, 所以最终的正弦输出频率为
		F=22050Hz×30/128=5168Hz。
		
		
		正弦输出频率可通过这个公式计算：F = Fs×(S/128).
	*/
			
	vs1003_WriteCmd(0x0b,0x2020);	  	/* 设置音量	*/
 	vs1003_WriteCmd(SCI_MODE, 0x0820);	/* 进入vs1003的测试模式	*/
 	
 	while(VS1003_IS_BUSY());			/* 等待空闲 */
 	
 	/* 
 		进入正弦测试状态
 		命令序列：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 		其中n = 0x24, 设定vs1003所产生的正弦波的频率值
 	*/
    VS1003_DS_0();
	vs1003_WriteByte(0x53);
	vs1003_WriteByte(0xef);
	vs1003_WriteByte(0x6e);
	vs1003_WriteByte(0x24);	/* 0x24 or 0x44 */
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	VS1003_DS_1();
	
	/* 退出正弦测试 */
    VS1003_DS_0();
	vs1003_WriteByte(0x45);
	vs1003_WriteByte(0x78);
	vs1003_WriteByte(0x69);
	vs1003_WriteByte(0x74);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	vs1003_WriteByte(0x00);
	VS1003_DS_1();		 
}	 

/*
*********************************************************************************************************
*	函 数 名: vs1003_SoftReset
*	功能说明: 软复位VS1003
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_SoftReset(void)
{	 
	uint8_t retry; 	
		
	while(VS1003_IS_BUSY());			/* 等待空闲 */	   
	
	vs1003_WriteByte(0X00);//启动传输
	retry = 0;
	while(vs1003_ReadReg(SCI_MODE) != 0x0804) // 软件复位,新模式  
	{
		/* 等待至少1.35ms  */
		vs1003_WriteCmd(SCI_MODE, 0x0804);// 软件复位,新模式
		
		while(VS1003_IS_BUSY());			/* 等待空闲 */	   
		
		if (retry++>5)
		{
			break; 
		}
	}	 				  

	vs1003_WriteCmd(SCI_CLOCKF,0x9800); 	    
	vs1003_WriteCmd(SCI_AUDATA,0xBB81); /* 采样率48k，立体声 */
	
	vs1003_WriteCmd(SCI_BASS, 0x0000);	/* */
    vs1003_WriteCmd(SCI_VOL, 0x2020); 	/* 设置为最大音量,0是最大  */
		 
	ResetDecodeTime();	/* 复位解码时间	*/
	
    /* 向vs1003发送4个字节无效数据，用以启动SPI发送 */
    VS1003_DS_0();//选中数据传输
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	VS1003_DS_1();//取消数据传输
} 

/*
*********************************************************************************************************
*	函 数 名: vs1003_SetVolume
*	功能说明: 设置VS1003音量。最大的音量是0，而静音为0xFEFE   （VS_VOL_MUTE）
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void vs1003_SetVolume(uint8_t _ucVol)
{
	vs1003_WriteCmd(SCI_VOL, (_ucVol << 8) | _ucVol);
}

/*
*********************************************************************************************************
*	函 数 名: ResetDecodeTime
*	功能说明: 重设解码时间
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void ResetDecodeTime(void)
{
	vs1003_WriteCmd(SCI_DECODE_TIME, 0x0000);
}

/*
*********************************************************************************************************
*	下面的代码还未调试
*********************************************************************************************************
*/

#if 0

//ram 测试 																				 
void VsRamTest(void)
{
	uint16_t u16 regvalue ;
		   
	Mp3Reset();     
 	vs1003_CMD_Write(SPI_MODE,0x0820);// 进入vs1003的测试模式
	while ((GPIOC->IDR&MP3_DREQ)==0); // 等待DREQ为高
 	MP3_DCS_SET(0);	       			  // xDCS = 1，选择vs1003的数据接口
	SPI1_ReadWriteByte(0x4d);
	SPI1_ReadWriteByte(0xea);
	SPI1_ReadWriteByte(0x6d);
	SPI1_ReadWriteByte(0x54);
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	SPI1_ReadWriteByte(0x00);
	delay_ms(50);  
	MP3_DCS_SET(1);
	regvalue=vs1003_REG_Read(SPI_HDAT0); // 如果得到的值为0x807F，则表明完好。
	printf("regvalueH:%x\n",regvalue>>8);//输出结果 
	printf("regvalueL:%x\n",regvalue&0xff);//输出结果 
}     

//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//比特率预定值
const uint16_t bitrate[2][16]=
{ 
	{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
	{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//返回Kbps的大小
//得到mp3&wma的波特率
uint16_t GetHeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;    
	        
    HEAD0=vs1003_REG_Read(SPI_HDAT0); 
    HEAD1=vs1003_REG_Read(SPI_HDAT1);
    switch(HEAD1)
    {        
        case 0x7665:return 0;//WAV格式
        case 0X4D54:return 1;//MIDI格式 
        case 0X574D://WMA格式
        {
            HEAD1=HEAD0*2/25;
            if((HEAD1%10)>5)return HEAD1/10+1;
            else return HEAD1/10;
        }
        default://MP3格式
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    } 
}  

//得到mp3的播放时间n sec
uint16_t GetDecodeTime(void)
{ 
    return vs1003_REG_Read(SPI_DECODE_TIME);   
} 
//加载频谱分析的代码到VS1003
void LoadPatch(void)
{
	uint16_t i;

	for (i=0;i<943;i++)vs1003_CMD_Write(atab[i],dtab[i]); 
	delay_ms(10);
}
//得到频谱数据
void GetSpec(u8 *p)
{
	u8 byteIndex=0;
	u8 temp;
	vs1003_CMD_Write(SPI_WRAMADDR,0x1804);                                                                                             
	for (byteIndex=0;byteIndex<14;byteIndex++) 
	{                                                                               
		temp=vs1003_REG_Read(SPI_WRAM)&0x63;//取小于100的数    
		*p++=temp;
	} 
}
 
//设定vs1003播放的音量和高低音 
void set1003(void)
{
    uint8 t;
    uint16_t bass=0; //暂存音调寄存器值
    uint16_t volt=0; //暂存音量值
    uint8_t vset=0;  //暂存音量值 	 

    vset=255-vs1003ram[4];//取反一下,得到最大值,表示最大的表示 
    volt=vset;
    volt<<=8;
    volt+=vset;//得到音量设置后大小
     //0,henh.1,hfreq.2,lenh.3,lfreq        
    for(t=0;t<4;t++)
    {
        bass<<=4;
        bass+=vs1003ram[t]; 
    }     
	vs1003_CMD_Write(SPI_BASS, 0x0000);//BASS   
    vs1003_CMD_Write(SPI_VOL, 0x0000); //设音量 
}     

#endif
