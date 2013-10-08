#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

#include "stm32f10x.h"

#define VOLUME_MAX		180
#define VOLUME_STEP		5

/* 定义一个用于MP3播放器的结构体 
便于全局变量操作
*/
typedef struct
{
	uint8_t ucMuteOn;			/* 0 : 静音， 1: 放音 */
	uint8_t ucVolume;			/* 当前音量 */
	uint32_t uiProgress;		/* 当前进度(已读取的字节数) */
	uint8_t ucPauseEn;			/* 暂停使能 */
}MP3_T;

void Mp3Player(void);

#endif
