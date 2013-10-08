/*
*********************************************************************************************************
*	                                  
*	ģ������ : ��������ģ��
*	�ļ����� : bsp_button.c
*	��    �� : V2.0
*	˵    �� : ʵ�ְ����ļ�⣬��������˲����ƣ����Լ�������¼���
*				(1) ��������
*				(2) ��������
*				(3) ������
*				(4) ����ʱ�Զ�����
*				(5) ��ϼ�
*
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v0.1    2009-12-27 armfly  �������ļ���ST�̼���汾ΪV3.1.2
*		v1.0    2011-01-11 armfly  ST�̼���������V3.4.0�汾��
*       v2.0    2011-10-16 armfly  ST�̼���������V3.5.0�汾��
*
*	Copyright (C), 2010-2011, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "stm32f10x.h"
#include <stdio.h>

#include "bsp_button.h"

static BUTTON_T s_BtnUser;		/* USER �� */
static BUTTON_T s_BtnTamper;	/* TAMPER �� */
static BUTTON_T s_BtnWakeUp;	/* WAKEUP �� */
static BUTTON_T s_BtnUp;		/* ҡ��UP�� */
static BUTTON_T s_BtnDown;		/* ҡ��DOWN�� */
static BUTTON_T s_BtnLeft;		/* ҡ��LEFT�� */
static BUTTON_T s_BtnRight;		/* ҡ��RIGHT�� */
static BUTTON_T s_BtnOk;		/* ҡ��OK�� */

static BUTTON_T s_BtnUserTamper;/* ��ϼ���USER��TAMPER�� */

static KEY_FIFO_T s_Key;		/* ����FIFO����,�ṹ�� */

static void bsp_InitButtonVar(void);
static void bsp_InitButtonHard(void);
static void bsp_DetectButton(BUTTON_T *_pBtn);

/*
	������STM32F103ZE-EK �������߷��䣺
	USER��     : PG8  (�͵�ƽ��ʾ����)
	TAMPEER��  : PC13 (�͵�ƽ��ʾ����)
	WKUP��     : PA0  (!!!�ߵ�ƽ��ʾ����)
	ҡ��UP��   : PG15 (�͵�ƽ��ʾ����)
	ҡ��DOWN�� : PD3  (�͵�ƽ��ʾ����)
	ҡ��LEFT�� : PG14 (�͵�ƽ��ʾ����)
	ҡ��RIGHT��: PG13 (�͵�ƽ��ʾ����)
	ҡ��OK��   : PG7 (�͵�ƽ��ʾ����)

	���庯���жϰ����Ƿ��£�����ֵ1 ��ʾ���£�0��ʾδ����
*/
static uint8_t IsKeyDownUser(void) 		{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_8) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownTamper(void) 	{if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownWakeUp(void) 	{if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_SET) return 1; return 0;}
static uint8_t IsKeyDownUp(void) 		{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_15) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownDown(void) 		{if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownLeft(void) 		{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_14) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownRight(void) 	{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_13) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownOk(void) 		{if (GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_7) == Bit_SET) return 0; return 1;}
static uint8_t IsKeyDownUserTamper(void) {if (IsKeyDownUser() && IsKeyDownTamper()) return 1; return 0;}	/* ��ϼ� */

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitButton
*	����˵��: ��ʼ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitButton(void)
{
	bsp_InitButtonVar();		/* ��ʼ���������� */
	bsp_InitButtonHard();		/* ��ʼ������Ӳ�� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_PutKey
*	����˵��: ��1����ֵѹ�밴��FIFO��������������ģ��һ��������
*	��    �Σ�_KeyCode : ��������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode)
{
	s_Key.Buf[s_Key.Write] = _KeyCode;

	if (++s_Key.Write  >= KEY_FIFO_SIZE)
	{
		s_Key.Write = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_GetKey
*	����˵��: �Ӱ���FIFO��������ȡһ����ֵ��
*	��    �Σ���
*	�� �� ֵ: ��������
*********************************************************************************************************
*/
uint8_t bsp_GetKey(void)
{
	uint8_t ret;

	if (s_Key.Read == s_Key.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_Key.Buf[s_Key.Read];

		if (++s_Key.Read >= KEY_FIFO_SIZE)
		{
			s_Key.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_KeyState
*	����˵��: ��ȡ������״̬
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t bsp_KeyState(uint8_t _ucKeyID)
{
	uint8_t ucState = 0;

	switch (_ucKeyID)
	{
		case KID_TAMPER:
			ucState = s_BtnTamper.State;
			break;

		case KID_WAKEUP:
			ucState = s_BtnWakeUp.State;
			break;

		case KID_USER:
			ucState = s_BtnUser.State;
			break;

		case KID_JOY_UP:
			ucState = s_BtnUp.State;
			break;

		case KID_JOY_DOWN:
			ucState = s_BtnDown.State;
			break;

		case KID_JOY_LEFT:
			ucState = s_BtnLeft.State;
			break;

		case KID_JOY_RIGHT:
			ucState = s_BtnRight.State;
			break;

		case KID_JOY_OK:
			ucState = s_BtnOk.State;
			break;
	}

	return ucState;
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitButtonHard
*	����˵��: ��ʼ������Ӳ��
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_InitButtonHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*
	������STM32F103ZE-EK �������߷��䣺
	USER��     : PG8  (�͵�ƽ��ʾ����)
	TAMPEER��  : PC13 (�͵�ƽ��ʾ����)
	WKUP��     : PA0  (!!!�ߵ�ƽ��ʾ����)
	ҡ��UP��   : PG15 (�͵�ƽ��ʾ����)
	ҡ��DOWN�� : PD3  (�͵�ƽ��ʾ����)
	ҡ��LEFT�� : PG14 (�͵�ƽ��ʾ����)
	ҡ��RIGHT��: PG13 (�͵�ƽ��ʾ����)
	ҡ��OK��   : PG7 (�͵�ƽ��ʾ����)
	*/
			
	/* ��1������GPIOA GPIOC GPIOD GPIOF GPIOG��ʱ��
	   ע�⣺����ط�����һ����ȫ��
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
			| RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOG, ENABLE);
	
	/* ��2�����������еİ���GPIOΪ��������ģʽ(ʵ����CPU��λ���������״̬) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	/* PA0 */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);	/* PC13 */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure);	/* PD3 */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_13
					  | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOG, &GPIO_InitStructure);	/* PG7,8,13,14,15 */
}
	
/*
*********************************************************************************************************
*	�� �� ��: bsp_InitButtonVar
*	����˵��: ��ʼ����������
*	��    �Σ�strName : ���������ַ���
*			  strDate : ���̷�������
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_InitButtonVar(void)
{
	/* �԰���FIFO��дָ������ */
	s_Key.Read = 0;
	s_Key.Write = 0;

	/* ��ʼ��USER����������֧�ְ��¡����𡢳��� */
	s_BtnUser.IsKeyDownFunc = IsKeyDownUser;		/* �жϰ������µĺ��� */
	s_BtnUser.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnUser.LongTime = BUTTON_LONG_TIME;			/* ����ʱ�� */
	s_BtnUser.Count = s_BtnUser.FilterTime / 2;		/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnUser.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnUser.KeyCodeDown = KEY_DOWN_USER;			/* �������µļ�ֵ���� */
	s_BtnUser.KeyCodeUp = KEY_UP_USER;				/* ��������ļ�ֵ���� */
	s_BtnUser.KeyCodeLong = KEY_LONG_USER;			/* �������������µļ�ֵ���� */
	s_BtnUser.RepeatSpeed = 0;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnUser.RepeatCount = 0;						/* ���������� */		

	/* ��ʼ��TAMPER����������֧�ְ��� */
	s_BtnTamper.IsKeyDownFunc = IsKeyDownTamper;	/* �жϰ������µĺ��� */
	s_BtnTamper.FilterTime = BUTTON_FILTER_TIME;	/* �����˲�ʱ�� */
	s_BtnTamper.LongTime = 0;						/* ����ʱ��, 0��ʾ����� */
	s_BtnTamper.Count = s_BtnTamper.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnTamper.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnTamper.KeyCodeDown = KEY_DOWN_TAMPER;		/* �������µļ�ֵ���� */
	s_BtnTamper.KeyCodeUp = KEY_UP_TAMPER;			/* ��������ļ�ֵ���� */
	s_BtnTamper.KeyCodeLong = 0;					/* �������������µļ�ֵ���� */
	s_BtnTamper.RepeatSpeed = 0;					/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnTamper.RepeatCount = 0;					/* ���������� */	

	/* ��ʼ��WAKEUP����������֧�ְ��� */
	s_BtnWakeUp.IsKeyDownFunc = IsKeyDownWakeUp;	/* �жϰ������µĺ��� */
	s_BtnWakeUp.FilterTime = BUTTON_FILTER_TIME;	/* �����˲�ʱ�� */
	s_BtnWakeUp.LongTime = 0;						/* ����ʱ�� */
	s_BtnWakeUp.Count = s_BtnWakeUp.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnWakeUp.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnWakeUp.KeyCodeDown = KEY_DOWN_WAKEUP;		/* �������µļ�ֵ���� */
	s_BtnWakeUp.KeyCodeUp = KEY_UP_WAKEUP;			/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnWakeUp.KeyCodeLong = 0;					/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnWakeUp.RepeatSpeed = 0;					/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnWakeUp.RepeatCount = 0;					/* ���������� */	

	/* ��ʼ��UP����������֧�ְ��¡�����������10ms�� */
	s_BtnUp.IsKeyDownFunc = IsKeyDownUp;			/* �жϰ������µĺ��� */
	s_BtnUp.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnUp.LongTime = 20;							/* ����ʱ�� */
	s_BtnUp.Count = s_BtnUp.FilterTime / 2;			/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnUp.State = 0;								/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnUp.KeyCodeDown = KEY_DOWN_JOY_UP;			/* �������µļ�ֵ���� */
	s_BtnUp.KeyCodeUp = 0;							/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnUp.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnUp.RepeatSpeed = 5;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnUp.RepeatCount = 0;						/* ���������� */		

	/* ��ʼ��DOWN����������֧�ְ��¡�����������10ms�� */
	s_BtnDown.IsKeyDownFunc = IsKeyDownDown;		/* �жϰ������µĺ��� */
	s_BtnDown.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnDown.LongTime = 20;							/* ����ʱ�� */
	s_BtnDown.Count = s_BtnDown.FilterTime / 2;		/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnDown.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnDown.KeyCodeDown = KEY_DOWN_JOY_DOWN;		/* �������µļ�ֵ���� */
	s_BtnDown.KeyCodeUp = 0;						/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnDown.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnDown.RepeatSpeed = 5;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnDown.RepeatCount = 0;						/* ���������� */		

	/* ��ʼ��LEFT����������֧�ְ��� */
	s_BtnLeft.IsKeyDownFunc = IsKeyDownLeft;		/* �жϰ������µĺ��� */
	s_BtnLeft.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnLeft.LongTime = 20;							/* ����ʱ�� */
	s_BtnLeft.Count = s_BtnLeft.FilterTime / 2;		/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnLeft.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnLeft.KeyCodeDown = KEY_DOWN_JOY_LEFT;		/* �������µļ�ֵ���� */
	s_BtnLeft.KeyCodeUp = 0;						/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnLeft.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnLeft.RepeatSpeed = 5;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnLeft.RepeatCount = 0;						/* ���������� */	

	/* ��ʼ��RIGHT����������֧�ְ��� */
	s_BtnRight.IsKeyDownFunc = IsKeyDownRight;		/* �жϰ������µĺ��� */
	s_BtnRight.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnRight.LongTime = 20;						/* ����ʱ�� */
	s_BtnRight.Count = s_BtnRight.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnRight.State = 0;							/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnRight.KeyCodeDown = KEY_DOWN_JOY_RIGHT;	/* �������µļ�ֵ���� */
	s_BtnRight.KeyCodeUp = 0;						/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnRight.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnRight.RepeatSpeed = 5;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnRight.RepeatCount = 0;						/* ���������� */	

	/* ��ʼ��OK����������֧�ְ��� */
	s_BtnOk.IsKeyDownFunc = IsKeyDownOk;			/* �жϰ������µĺ��� */
	s_BtnOk.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnOk.LongTime = 0;							/* ����ʱ�� */
	s_BtnOk.Count = s_BtnOk.FilterTime / 2;			/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnOk.State = 0;								/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnOk.KeyCodeDown = KEY_DOWN_JOY_OK;			/* �������µļ�ֵ���� */
	s_BtnOk.KeyCodeUp = KEY_UP_JOY_OK;				/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnOk.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnOk.RepeatSpeed = 0;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnOk.RepeatCount = 0;						/* ���������� */	

	/* ��ʼ����ϰ���������֧�ְ��� */
	s_BtnUserTamper.IsKeyDownFunc = IsKeyDownUserTamper;	/* �жϰ������µĺ��� */
	s_BtnUserTamper.FilterTime = BUTTON_FILTER_TIME;		/* �����˲�ʱ�� */
	s_BtnUserTamper.LongTime = 0;							/* ����ʱ�� */
	s_BtnUserTamper.Count = s_BtnUserTamper.FilterTime / 2;	/* ����������Ϊ�˲�ʱ���һ�� */
	s_BtnUserTamper.State = 0;								/* ����ȱʡ״̬��0Ϊδ���� */
	s_BtnUserTamper.KeyCodeDown = KEY_DOWN_USER_TAMPER;		/* �������µļ�ֵ���� */
	s_BtnUserTamper.KeyCodeUp = 0;							/* ��������ļ�ֵ���룬0��ʾ����� */
	s_BtnUserTamper.KeyCodeLong = 0;						/* �������������µļ�ֵ���룬0��ʾ����� */
	s_BtnUserTamper.RepeatSpeed = 0;						/* �����������ٶȣ�0��ʾ��֧������ */
	s_BtnUserTamper.RepeatCount = 0;						/* ���������� */
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_DetectButton
*	����˵��: ���һ��������������״̬�����뱻�����Եĵ��á�
*	��    �Σ������ṹ����ָ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void bsp_DetectButton(BUTTON_T *_pBtn)
{
	/* ���û�г�ʼ�������������򱨴�
	if (_pBtn->IsKeyDownFunc == 0)
	{
		printf("Fault : DetectButton(), _pBtn->IsKeyDownFunc undefine");
	}
	*/

	if (_pBtn->IsKeyDownFunc())
	{
		if (_pBtn->Count < _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count < 2 * _pBtn->FilterTime)
		{
			_pBtn->Count++;
		}
		else
		{
			if (_pBtn->State == 0)
			{
				_pBtn->State = 1;

				/* ���Ͱ�ť���µ���Ϣ */
				if (_pBtn->KeyCodeDown > 0)
				{
					/* ��ֵ���밴��FIFO */
					bsp_PutKey(_pBtn->KeyCodeDown);
				}
			}

			if (_pBtn->LongTime > 0)
			{
				if (_pBtn->LongCount < _pBtn->LongTime)
				{
					/* ���Ͱ�ť�������µ���Ϣ */
					if (++_pBtn->LongCount == _pBtn->LongTime)
					{
						/* ��ֵ���밴��FIFO */
						bsp_PutKey(_pBtn->KeyCodeLong);						
					}
				}
				else
				{
					if (_pBtn->RepeatSpeed > 0)
					{
						if (++_pBtn->RepeatCount >= _pBtn->RepeatSpeed)
						{
							_pBtn->RepeatCount = 0;
							/* ��������ÿ��10ms����1������ */
							bsp_PutKey(_pBtn->KeyCodeDown);														
						}
					}
				}
			}
		}
	}
	else
	{
		if(_pBtn->Count > _pBtn->FilterTime)
		{
			_pBtn->Count = _pBtn->FilterTime;
		}
		else if(_pBtn->Count != 0)
		{
			_pBtn->Count--;
		}
		else
		{
			if (_pBtn->State == 1)
			{
				_pBtn->State = 0;

				/* ���Ͱ�ť�������Ϣ */
				if (_pBtn->KeyCodeUp > 0)
				{
					/* ��ֵ���밴��FIFO */
					bsp_PutKey(_pBtn->KeyCodeUp);			
				}
			}
		}

		_pBtn->LongCount = 0;
		_pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_KeyPro
*	����˵��: ������а�����������״̬�����뱻�����Եĵ��á�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_KeyPro(void)
{
	bsp_DetectButton(&s_BtnUser);		/* USER �� */
	bsp_DetectButton(&s_BtnTamper);		/* TAMPER �� */
	bsp_DetectButton(&s_BtnWakeUp);		/* WAKEUP �� */
	bsp_DetectButton(&s_BtnUp);			/* ҡ��UP�� */
	bsp_DetectButton(&s_BtnDown);		/* ҡ��DOWN�� */
	bsp_DetectButton(&s_BtnLeft);		/* ҡ��LEFT�� */
	bsp_DetectButton(&s_BtnRight);		/* ҡ��RIGHT�� */
	bsp_DetectButton(&s_BtnOk);			/* ҡ��OK�� */
	bsp_DetectButton(&s_BtnUserTamper);	/* ��ϼ� */
}
