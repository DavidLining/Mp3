/*
*********************************************************************************************************
*	                                  
*	ģ������ : VS1003B mp3оƬģ��
*	�ļ����� : bsp_VS1003B.c
*	��    �� : V1.0
*	˵    �� : VS1003BоƬ�ײ�������
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v0.1    2009-12-27 armfly  �������ļ���ST�̼���汾ΪV3.1.2
*		v1.0    2011-09-11 armfly  ʵ��I2S������¼�����ã�ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp_VS1003B.h"	 

#define VS1003_CS_0()	GPIO_ResetBits(GPIOF, GPIO_Pin_9)
#define VS1003_CS_1()	GPIO_SetBits(GPIOF, GPIO_Pin_9)
#define VS1003_DS_0()	GPIO_ResetBits(GPIOF, GPIO_Pin_8)
#define VS1003_DS_1()	GPIO_SetBits(GPIOF, GPIO_Pin_8)
#define VS1003_IS_BUSY()	(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == 0)

#define DUMMY_BYTE    0xFF		/* �ɶ�������ֵ */

u8 vs1003ram[5]={0,0,0,0,250};

/*
*********************************************************************************************************
*	�� �� ��: wm8978_Init
*	����˵��: ��ʼ��VS1003BӲ���豸
*	��    �Σ���
*	�� �� ֵ: 1 ��ʾ��ʼ��������0��ʾ��ʼ��������
*********************************************************************************************************
*/
void vs1003_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/*
		�������������VS1003B�Ŀ������ӣ�
		PA6/SPI1_MISO <== SO
		PA7/SPI1_MOSI ==> SI
		PA5/SPI1_SCK  ==> SCLK
		PF9           ==> XCS\     (Ƭѡ, ��LED4���ã�����VS1003Bʱ��LED4����˸)
		PB5           <== DREQ
		PF8           ==> XDCS/BSYNC   (���ݺ�����ѡ��)
		
		SPI1���߹���3���豸
		(1) SST25VF016B  , ƬѡΪPB2
		(2) LCDģ��Ĵ���IC(TS2046), ƬѡΪPG11
		(3) CH376T (��������), ƬѡΪPF10
		(4) VS1003B(��������)��ƬѡΪPF9
		
		CPU��λ�� PB2,PG11,PF9,PF10��������ģʽ��Ĭ��Ϊ�͵�ƽ�����������߷��ʳ�ͻ��
		�����Ҫ��δ�õ�Ƭѡ����ȫ������Ϊ����ߵ�ƽ������������SPI1�ϵ��豸��
	*/
	/* �����ģ���ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB
		| RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOF
		| RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

	/*������SPI1���ߣ�SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   /* ����������� */
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/* ����PB5��ΪVS1003B���������� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    /* ���� */
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	/* ����PF8��ΪVS1003B��XDS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    /* ������� */
	GPIO_Init(GPIOF,&GPIO_InitStructure);	

	/* ������δ�õ�SPI�豸��Ƭѡ����Ϊ�ߵ�ƽ */
	#if 1	
		/* (1) SST25VF016B  , ƬѡΪPB2 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
		GPIO_Init(GPIOB,&GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_2);
		
		/* (2) LCDģ��Ĵ���IC(TS2046), ƬѡΪPG11 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
		GPIO_Init(GPIOG,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOG, GPIO_Pin_11);		
		
		/* (3) CH376T (��������), ƬѡΪPF10 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
		GPIO_Init(GPIOF,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOF, GPIO_Pin_10);	
		
		/* (4) VS1003B(��������)��ƬѡΪPF9 */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; /* ������� */
		GPIO_Init(GPIOF,&GPIO_InitStructure);		
		GPIO_SetBits(GPIOF, GPIO_Pin_9);	
	#endif		

	/* SPI1 ���� */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ѡ��2��ȫ˫��ģʽ */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* CPU��SPI��Ϊ���豸 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 8������ */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* CLK���ſ���״̬��ƽ = 1 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* ���ݲ����ڵ�2������(������) */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;  			/* �������Ƭѡ */
	/* VS1003B SPI����ʱ�����2MHz��ʵ��Ƶ�� = 72MHz / 64 = 1.125MHz */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;	
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* ���λ�ȴ��� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);

	/* SPI1 ʹ�� */
	SPI_Cmd(SPI1,ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: vs1003_WriteByte
*	����˵��: ��SPI1�ӿ�дһ���ֽ�
*	��    �Σ�_ucByte �������ֽ�.
*	�� �� ֵ: ����������.
*********************************************************************************************************
*/
uint8_t vs1003_WriteByte(uint8_t _ucByte)
{
	/* �ȴ��������� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	/* ����1���ֽ� */
	SPI_I2S_SendData(SPI1, _ucByte);
	
	/* �ȴ�������1���ֽ� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* ���ض��������� */
	return SPI_I2S_ReceiveData(SPI1);
}

/*
*********************************************************************************************************
*	�� �� ��: vs1003_ReadByte
*	����˵��: ��SPI1�ӿڶ�һ���ֽ�
*	��    �Σ���
*	�� �� ֵ: ����������.
*********************************************************************************************************
*/
uint8_t vs1003_ReadByte(void)
{
	return vs1003_WriteByte(DUMMY_BYTE);
}

/*
*********************************************************************************************************
*	�� �� ��: vs1003_WriteCmd
*	����˵��: ��VS1003д����
*	��    �Σ�_ucAddr �� ��ַ�� 		_usData ������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_WriteCmd(uint8_t _ucAddr, uint16_t _usData)
{  
	while(VS1003_IS_BUSY());	/* �ȴ����� */

	VS1003_DS_1();
	VS1003_CS_0();

	vs1003_WriteByte(VS_WRITE_COMMAND);	/* ����VS1003��д���� */
	vs1003_WriteByte(_ucAddr); 			/* �Ĵ�����ַ */
	vs1003_WriteByte(_usData >> 8); 	/* ���͸�8λ */
	vs1003_WriteByte(_usData);	 		/* ���͵�8λ */
	VS1003_CS_1();
} 

/*
*********************************************************************************************************
*	�� �� ��: vs1003_ReqNewData
*	����˵��: �ж�VS1003�Ƿ����������ݡ� VS1003�ڲ���0.5k��������
*	��    �Σ���
*	�� �� ֵ: ��
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
*	�� �� ��: vs1003_PreWriteData
*	����˵��: ׼����VS1003д���ݣ�����1�μ���
*	��    �Σ�_��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_PreWriteData(void)
{
	VS1003_CS_1();
	VS1003_DS_0();
} 

/*
*********************************************************************************************************
*	�� �� ��: vs1003_WriteData
*	����˵��: ��VS1003д����
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_WriteData(uint8_t _ucData)
{
	//while(VS1003_IS_BUSY());			/* �ȴ����� */
	VS1003_CS_1();
	VS1003_DS_0();
	vs1003_WriteByte(_ucData);
	VS1003_DS_1();
}         

/*
*********************************************************************************************************
*	�� �� ��: vs1003_ReadReg
*	����˵��: ��VS1003�ļĴ���
*	��    �Σ�_ucAddr:�Ĵ�����ַ
*	�� �� ֵ: �Ĵ���ֵ
*********************************************************************************************************
*/
uint16_t vs1003_ReadReg(uint8_t _ucAddr)
{ 
	uint16_t usTemp;
	
	while(VS1003_IS_BUSY());	/* �ȴ����� */
	
	VS1003_DS_1();
	VS1003_CS_0();
	vs1003_WriteByte(VS_READ_COMMAND);	/* ����VS1003������ */
	vs1003_WriteByte(_ucAddr);			/* ���͵�ַ */
	usTemp = vs1003_ReadByte() << 8;	/* ��ȡ���ֽ� */
	usTemp += vs1003_ReadByte();		/* ��ȡ���ֽ� */
	VS1003_CS_1();
	return usTemp;
}  

/*
*********************************************************************************************************
*	�� �� ��: vs1003_TestRam
*	����˵��: ����VS1003B���ڲ�RAM
*	��    �Σ���
*	�� �� ֵ: 1��ʾOK, 0��ʾ����.
*********************************************************************************************************
*/																				 
uint8_t vs1003_TestRam(void)
{
	uint16_t usRegValue;
		   
 	vs1003_WriteCmd(SCI_MODE, 0x0820);	/* ����vs1003�Ĳ���ģʽ */
 	
	while(VS1003_IS_BUSY());			/* �ȴ����� */
	
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

	while(VS1003_IS_BUSY());			/* �ȴ����� */
		
	usRegValue = vs1003_ReadReg(SCI_HDAT0); /* ����õ���ֵΪ0x807F�������OK */
	
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
*	�� �� ��: vs1003_TestSine
*	����˵��: ���Ҳ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_TestSine(void)
{						
	/*
		���Ҳ���ͨ�������8�ֽڳ�ʼ����0x53 0xEF 0x6E n 0 0 0 0
		��Ҫ�˳����Ҳ���ģʽ�Ļ��������������� 0x45 0x78 0x69 0x74 0 0 0 0 .
		
		�����n������Ϊ���Ҳ���ʹ�ã�����
		���£�
		n bits
		����λ ����
		FsIdx 7��5 ����������
		S 4��0 ���������ٶ�
		�������Ƶ�ʿ�ͨ�������ʽ���㣺F=Fs��(S/128).
		���磺���Ҳ���ֵΪ126 ʱ�����������Ϊ
		0b01111110����FsIdx=0b011=3,����Fs=22050Hz��
		S=0b11110=30, �������յ��������Ƶ��Ϊ
		F=22050Hz��30/128=5168Hz��
		
		
		�������Ƶ�ʿ�ͨ�������ʽ���㣺F = Fs��(S/128).
	*/
			
	vs1003_WriteCmd(0x0b,0x2020);	  	/* ��������	*/
 	vs1003_WriteCmd(SCI_MODE, 0x0820);	/* ����vs1003�Ĳ���ģʽ	*/
 	
 	while(VS1003_IS_BUSY());			/* �ȴ����� */
 	
 	/* 
 		�������Ҳ���״̬
 		�������У�0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 		����n = 0x24, �趨vs1003�����������Ҳ���Ƶ��ֵ
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
	
	/* �˳����Ҳ��� */
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
*	�� �� ��: vs1003_SoftReset
*	����˵��: ��λVS1003
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_SoftReset(void)
{	 
	uint8_t retry; 	
		
	while(VS1003_IS_BUSY());			/* �ȴ����� */	   
	
	vs1003_WriteByte(0X00);//��������
	retry = 0;
	while(vs1003_ReadReg(SCI_MODE) != 0x0804) // �����λ,��ģʽ  
	{
		/* �ȴ�����1.35ms  */
		vs1003_WriteCmd(SCI_MODE, 0x0804);// �����λ,��ģʽ
		
		while(VS1003_IS_BUSY());			/* �ȴ����� */	   
		
		if (retry++>5)
		{
			break; 
		}
	}	 				  

	vs1003_WriteCmd(SCI_CLOCKF,0x9800); 	    
	vs1003_WriteCmd(SCI_AUDATA,0xBB81); /* ������48k�������� */
	
	vs1003_WriteCmd(SCI_BASS, 0x0000);	/* */
    vs1003_WriteCmd(SCI_VOL, 0x2020); 	/* ����Ϊ�������,0�����  */
		 
	ResetDecodeTime();	/* ��λ����ʱ��	*/
	
    /* ��vs1003����4���ֽ���Ч���ݣ���������SPI���� */
    VS1003_DS_0();//ѡ�����ݴ���
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	vs1003_WriteByte(0xFF);
	VS1003_DS_1();//ȡ�����ݴ���
} 

/*
*********************************************************************************************************
*	�� �� ��: vs1003_SetVolume
*	����˵��: ����VS1003����������������0��������Ϊ0xFEFE   ��VS_VOL_MUTE��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void vs1003_SetVolume(uint8_t _ucVol)
{
	vs1003_WriteCmd(SCI_VOL, (_ucVol << 8) | _ucVol);
}

/*
*********************************************************************************************************
*	�� �� ��: ResetDecodeTime
*	����˵��: �������ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void ResetDecodeTime(void)
{
	vs1003_WriteCmd(SCI_DECODE_TIME, 0x0000);
}

/*
*********************************************************************************************************
*	����Ĵ��뻹δ����
*********************************************************************************************************
*/

#if 0

//ram ���� 																				 
void VsRamTest(void)
{
	uint16_t u16 regvalue ;
		   
	Mp3Reset();     
 	vs1003_CMD_Write(SPI_MODE,0x0820);// ����vs1003�Ĳ���ģʽ
	while ((GPIOC->IDR&MP3_DREQ)==0); // �ȴ�DREQΪ��
 	MP3_DCS_SET(0);	       			  // xDCS = 1��ѡ��vs1003�����ݽӿ�
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
	regvalue=vs1003_REG_Read(SPI_HDAT0); // ����õ���ֵΪ0x807F���������á�
	printf("regvalueH:%x\n",regvalue>>8);//������ 
	printf("regvalueL:%x\n",regvalue&0xff);//������ 
}     

//FOR WAV HEAD0 :0X7761 HEAD1:0X7665    
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ
const uint16_t bitrate[2][16]=
{ 
	{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0}, 
	{0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};
//����Kbps�Ĵ�С
//�õ�mp3&wma�Ĳ�����
uint16_t GetHeadInfo(void)
{
	unsigned int HEAD0;
	unsigned int HEAD1;    
	        
    HEAD0=vs1003_REG_Read(SPI_HDAT0); 
    HEAD1=vs1003_REG_Read(SPI_HDAT1);
    switch(HEAD1)
    {        
        case 0x7665:return 0;//WAV��ʽ
        case 0X4D54:return 1;//MIDI��ʽ 
        case 0X574D://WMA��ʽ
        {
            HEAD1=HEAD0*2/25;
            if((HEAD1%10)>5)return HEAD1/10+1;
            else return HEAD1/10;
        }
        default://MP3��ʽ
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03; 
            if(HEAD1==3)HEAD1=1;
            else HEAD1=0;
            return bitrate[HEAD1][HEAD0>>12];
        }
    } 
}  

//�õ�mp3�Ĳ���ʱ��n sec
uint16_t GetDecodeTime(void)
{ 
    return vs1003_REG_Read(SPI_DECODE_TIME);   
} 
//����Ƶ�׷����Ĵ��뵽VS1003
void LoadPatch(void)
{
	uint16_t i;

	for (i=0;i<943;i++)vs1003_CMD_Write(atab[i],dtab[i]); 
	delay_ms(10);
}
//�õ�Ƶ������
void GetSpec(u8 *p)
{
	u8 byteIndex=0;
	u8 temp;
	vs1003_CMD_Write(SPI_WRAMADDR,0x1804);                                                                                             
	for (byteIndex=0;byteIndex<14;byteIndex++) 
	{                                                                               
		temp=vs1003_REG_Read(SPI_WRAM)&0x63;//ȡС��100����    
		*p++=temp;
	} 
}
 
//�趨vs1003���ŵ������͸ߵ��� 
void set1003(void)
{
    uint8 t;
    uint16_t bass=0; //�ݴ������Ĵ���ֵ
    uint16_t volt=0; //�ݴ�����ֵ
    uint8_t vset=0;  //�ݴ�����ֵ 	 

    vset=255-vs1003ram[4];//ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ 
    volt=vset;
    volt<<=8;
    volt+=vset;//�õ��������ú��С
     //0,henh.1,hfreq.2,lenh.3,lfreq        
    for(t=0;t<4;t++)
    {
        bass<<=4;
        bass+=vs1003ram[t]; 
    }     
	vs1003_CMD_Write(SPI_BASS, 0x0000);//BASS   
    vs1003_CMD_Write(SPI_VOL, 0x0000); //������ 
}     

#endif
