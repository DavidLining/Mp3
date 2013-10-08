/*
*********************************************************************************************************
*	                                  
*	ģ������ : WM8978��ƵоƬ����ģ��
*	�ļ����� : bsp_WM8978.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v0.1    2009-12-27 armfly  �������ļ���ST�̼���汾ΪV3.1.2
*		v1.0    2011-09-04 armfly  ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_WM8978_H
#define _BSP_WM8978_H

#include <inttypes.h>

#define WM8978_SLAVE_ADDRESS    0x34	/* WM8978 I2C�ӻ���ַ */


/* ����wm8978_Cfg()�������β� */
#define DAC_ON			1
#define DAC_OFF			0

#define AUX_ON			1
#define AUX_OFF			0

#define LINE_ON			1
#define LINE_OFF		0

#define EAR_ON			1
#define EAR_OFF			0

#define SPK_ON			1
#define SPK_OFF			0

#define MIC_ON			1
#define MIC_OFF			0

/* ����������� */
#define VOLUME_MAX		63		/* ������� */
#define VOLUME_STEP		3		/* �������ڲ��� */

/* �������MIC���� */
#define GAIN_MAX		63		/* ������� */
#define GAIN_STEP		3		/* ���沽�� */


/* ���ⲿ���õĺ������� */
uint8_t wm8978_Init(void);
uint8_t wm8978_ReadVolume(void);
void wm8978_Aux2Ear(void);
void wm8978_Aux2Spk(void);
void wm8978_CfgAudioIF(uint16_t _usStandard, uint8_t _ucWordLen, uint16_t _usMode);
void wm8978_ChangeVolume(uint8_t _ucLeftVolume, uint8_t _ucRightVolume);
void wm8978_Dac2Ear(void);
void wm8978_Dac2Spk(void);
void wm8978_Mic2Ear(void);
void wm8978_Mic2Spk(void);
void wm8978_Mic2Adc(void);
void wm8978_Mute(uint8_t _ucMute);
void wm8978_PowerDown(void);
void wm8978_SetMicGain(uint8_t _ucGain);

void I2S_CODEC_Init(void);
void I2S_StartPlay(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq);
void I2S_StartRecord(uint16_t _usStandard, uint16_t _usWordLen, uint16_t _usAudioFreq);
void I2S_Stop(void);
#endif
