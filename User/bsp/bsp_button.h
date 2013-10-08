/*
*********************************************************************************************************
*	                                  
*	模块名称 : 按键驱动模块    
*	文件名称 : bsp_button.h
*	版    本 : V2.0
*	说    明 : 头文件
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

/* 按键滤波时间50ms, 单位10ms
 只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
*/
#define BUTTON_FILTER_TIME 	5
#define BUTTON_LONG_TIME 	100		/* 持续1秒，认为长按事件 */

/*
	每个按键对应1个全局的结构体变量。
	其成员变量是实现滤波和多种按键状态所必须的
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t Count;			/* 滤波器计数器 */
	uint8_t FilterTime;		/* 滤波时间(最大255,表示2550ms) */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t KeyCodeUp;		/* 按键弹起的键值代码, 0表示不检测按键弹起 */
	uint8_t KeyCodeDown;	/* 按键按下的键值代码, 0表示不检测按键按下 */
	uint8_t KeyCodeLong;	/* 按键长按的键值代码, 0表示不检测长按 */
	uint8_t RepeatSpeed;	/* 连续按键周期 */
	uint8_t RepeatCount;	/* 连续按键计数器 */
}BUTTON_T;

/* 定义键值代码
	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2)	编译器可帮我们避免键值重复。
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 表示按键事件 */

	KEY_DOWN_USER,			/* User键按下 */
	KEY_UP_USER,			/* User键弹起 */
	KEY_LONG_USER,			/* User键长按 */

	KEY_DOWN_WAKEUP,		/* WakeUp键按下 */
	KEY_UP_WAKEUP,			/* WakeUp键弹起 */

	KEY_DOWN_TAMPER,		/* Tamper键按下 */
	KEY_UP_TAMPER,			/* Tamper键弹起 */

	KEY_DOWN_JOY_UP,		/* 摇杆UP键按下 */
	KEY_DOWN_JOY_DOWN,		/* 摇杆DOWN键按下 */
	KEY_DOWN_JOY_LEFT,		/* 摇杆LEFT键按下 */
	KEY_DOWN_JOY_RIGHT,		/* 摇杆RIGHT键按下 */
	KEY_DOWN_JOY_OK,		/* 摇杆OK键按下 */

	KEY_UP_JOY_OK,			/* 摇杆OK键释放 */
	
	KEY_DOWN_USER_TAMPER	/* 组合键，USER键和WAKEUP键同时按下 */
}KEY_ENUM;

/* 按键ID */
enum
{
	KID_TAMPER = 0,
	KID_WAKEUP,
	KID_USER,
	KID_JOY_UP,
	KID_JOY_DOWN,
	KID_JOY_LEFT,
	KID_JOY_RIGHT,
	KID_JOY_OK
};

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	20
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_InitButton(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
void bsp_KeyPro(void);
uint8_t bsp_KeyState(uint8_t _ucKeyID);

#endif


