#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

#include "stm32f10x.h"

#define VOLUME_MAX		180
#define VOLUME_STEP		5

/* ����һ������MP3�������Ľṹ�� 
����ȫ�ֱ�������
*/
typedef struct
{
	uint8_t ucMuteOn;			/* 0 : ������ 1: ���� */
	uint8_t ucVolume;			/* ��ǰ���� */
	uint32_t uiProgress;		/* ��ǰ����(�Ѷ�ȡ���ֽ���) */
	uint8_t ucPauseEn;			/* ��ͣʹ�� */
}MP3_T;

void Mp3Player(void);

#endif
