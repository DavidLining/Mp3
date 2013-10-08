/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��������ģ��    
*	�ļ����� : bsp_button.h
*	��    �� : V2.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

/* �����˲�ʱ��50ms, ��λ10ms
 ֻ��������⵽50ms״̬�������Ϊ��Ч����������Ͱ��������¼�
*/
#define BUTTON_FILTER_TIME 	5
#define BUTTON_LONG_TIME 	100		/* ����1�룬��Ϊ�����¼� */

/*
	ÿ��������Ӧ1��ȫ�ֵĽṹ�������
	���Ա������ʵ���˲��Ͷ��ְ���״̬�������
*/
typedef struct
{
	/* ������һ������ָ�룬ָ���жϰ����ַ��µĺ��� */
	uint8_t (*IsKeyDownFunc)(void); /* �������µ��жϺ���,1��ʾ���� */

	uint8_t Count;			/* �˲��������� */
	uint8_t FilterTime;		/* �˲�ʱ��(���255,��ʾ2550ms) */
	uint16_t LongCount;		/* ���������� */
	uint16_t LongTime;		/* �������³���ʱ��, 0��ʾ����ⳤ�� */
	uint8_t  State;			/* ������ǰ״̬�����»��ǵ��� */
	uint8_t KeyCodeUp;		/* ��������ļ�ֵ����, 0��ʾ����ⰴ������ */
	uint8_t KeyCodeDown;	/* �������µļ�ֵ����, 0��ʾ����ⰴ������ */
	uint8_t KeyCodeLong;	/* ���������ļ�ֵ����, 0��ʾ����ⳤ�� */
	uint8_t RepeatSpeed;	/* ������������ */
	uint8_t RepeatCount;	/* �������������� */
}BUTTON_T;

/* �����ֵ����
	�Ƽ�ʹ��enum, ����#define��ԭ��
	(1) ����������ֵ,�������˳��ʹ���뿴���������
	(2)	�������ɰ����Ǳ����ֵ�ظ���
*/
typedef enum
{
	KEY_NONE = 0,			/* 0 ��ʾ�����¼� */

	KEY_DOWN_USER,			/* User������ */
	KEY_UP_USER,			/* User������ */
	KEY_LONG_USER,			/* User������ */

	KEY_DOWN_WAKEUP,		/* WakeUp������ */
	KEY_UP_WAKEUP,			/* WakeUp������ */

	KEY_DOWN_TAMPER,		/* Tamper������ */
	KEY_UP_TAMPER,			/* Tamper������ */

	KEY_DOWN_JOY_UP,		/* ҡ��UP������ */
	KEY_DOWN_JOY_DOWN,		/* ҡ��DOWN������ */
	KEY_DOWN_JOY_LEFT,		/* ҡ��LEFT������ */
	KEY_DOWN_JOY_RIGHT,		/* ҡ��RIGHT������ */
	KEY_DOWN_JOY_OK,		/* ҡ��OK������ */

	KEY_UP_JOY_OK,			/* ҡ��OK���ͷ� */
	
	KEY_DOWN_USER_TAMPER	/* ��ϼ���USER����WAKEUP��ͬʱ���� */
}KEY_ENUM;

/* ����ID */
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

/* ����FIFO�õ����� */
#define KEY_FIFO_SIZE	20
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* ��ֵ������ */
	uint8_t Read;					/* ��������ָ�� */
	uint8_t Write;					/* ������дָ�� */
}KEY_FIFO_T;

/* ���ⲿ���õĺ������� */
void bsp_InitButton(void);
void bsp_PutKey(uint8_t _KeyCode);
uint8_t bsp_GetKey(void);
void bsp_KeyPro(void);
uint8_t bsp_KeyState(uint8_t _ucKeyID);

#endif


