/**
  ******************************************************************************
  * @file    stm3210b_eval_lcd.c
  * @author  MCD Application Team
  * @version V3.1.2
  * @date    09/28/2009
  * @brief   This file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm3210b_eval_lcd.h"
#include "fonts.h"

/** @addtogroup Utilities
  * @{
  */
  
/** @defgroup STM3210B_EVAL_LCD 
  * @briefThis file includes the LCD driver for AM-240320LTNQW00H (LCD_HX8312),
  *          AM-240320L8TNQW00H (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          Liquid Crystal Display Module of STM3210B-EVAL board.
  * @{
  */ 

/** @defgroup STM3210B_EVAL_LCD_Private_Types
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Defines
  * @{
  */ 
#define LCD_ILI9320     0x9320
#define LCD_HX8312      0x8312
#define LCD_SPFD5408    0x5408
#define START_BYTE      0x70
#define SET_INDEX       0x00
#define READ_STATUS     0x01
#define LCD_WRITE_REG   0x02
#define LCD_READ_REG    0x03
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Variables
  * @{
  */ 
  /* Global variables to set the written text color */
static __IO uint16_t TextColor = 0x0000, BackColor = 0xFFFF;
static __IO uint32_t LCDType = LCD_ILI9320;
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Function_Prototypes
  * @{
  */ 
#ifndef USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/

static void LCD_WriteRegHX8312(uint8_t LCD_Reg, uint8_t LCD_RegValue);
static void LCD_WriteRegILI9320(uint8_t LCD_Reg, uint16_t LCD_RegValue);
/**
  * @}
  */ 


/** @defgroup STM3210B_EVAL_LCD_Private_Functions
  * @{
  */ 


/**
  * @brief  Setups the LCD.
  * @param  None
  * @retval None
  */
void LCD_Setup(void)
{ 
/* Configure the LCD Control pins --------------------------------------------*/
  LCD_CtrlLinesConfig();
  
/* Configure the LCD_SPI interface ----------------------------------------------*/
  LCD_SPIConfig();
  if(LCDType == LCD_ILI9320)
  {
    _delay_(5); /* Delay 50 ms */
    /* Start Initial Sequence ------------------------------------------------*/
    LCD_WriteReg(R229, 0x8000); /* Set the internal vcore voltage */
    LCD_WriteReg(R0,  0x0001); /* Start internal OSC. */
    LCD_WriteReg(R1,  0x0100); /* set SS and SM bit */
    LCD_WriteReg(R2,  0x0700); /* set 1 line inversion */
    LCD_WriteReg(R3,  0x1030); /* set GRAM write direction and BGR=1. */
    LCD_WriteReg(R4,  0x0000); /* Resize register */
    LCD_WriteReg(R8,  0x0202); /* set the back porch and front porch */
    LCD_WriteReg(R9,  0x0000); /* set non-display area refresh cycle ISC[3:0] */
    LCD_WriteReg(R10, 0x0000); /* FMARK function */
    LCD_WriteReg(R12, 0x0000); /* RGB interface setting */
    LCD_WriteReg(R13, 0x0000); /* Frame marker Position */
    LCD_WriteReg(R15, 0x0000); /* RGB interface polarity */
    /* Power On sequence -----------------------------------------------------*/
    LCD_WriteReg(R16, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(R17, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(R18, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(R19, 0x0000); /* VDV[4:0] for VCOM amplitude */
    _delay_(20);               /* Dis-charge capacitor power voltage (200ms) */
    LCD_WriteReg(R16, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(R17, 0x0137); /* DC1[2:0], DC0[2:0], VC[2:0] */
    _delay_(5);                /* Delay 50 ms */
    LCD_WriteReg(R18, 0x0139); /* VREG1OUT voltage */
    _delay_(5);                /* Delay 50 ms */
    LCD_WriteReg(R19, 0x1d00); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(R41, 0x0013); /* VCM[4:0] for VCOMH */
    _delay_(5);                /* Delay 50 ms */
    LCD_WriteReg(R32, 0x0000); /* GRAM horizontal Address */
    LCD_WriteReg(R33, 0x0000); /* GRAM Vertical Address */
    /* Adjust the Gamma Curve ------------------------------------------------*/
    LCD_WriteReg(R48, 0x0006);
    LCD_WriteReg(R49, 0x0101);
    LCD_WriteReg(R50, 0x0003);
    LCD_WriteReg(R53, 0x0106);
    LCD_WriteReg(R54, 0x0b02);
    LCD_WriteReg(R55, 0x0302);
    LCD_WriteReg(R56, 0x0707);
    LCD_WriteReg(R57, 0x0007);
    LCD_WriteReg(R60, 0x0600);
    LCD_WriteReg(R61, 0x020b);
  
    /* Set GRAM area ---------------------------------------------------------*/
    LCD_WriteReg(R80, 0x0000); /* Horizontal GRAM Start Address */
    LCD_WriteReg(R81, 0x00EF); /* Horizontal GRAM End Address */
    LCD_WriteReg(R82, 0x0000); /* Vertical GRAM Start Address */
    LCD_WriteReg(R83, 0x013F); /* Vertical GRAM End Address */
    LCD_WriteReg(R96,  0x2700); /* Gate Scan Line */
    LCD_WriteReg(R97,  0x0001); /* NDL,VLE, REV */
    LCD_WriteReg(R106, 0x0000); /* set scrolling line */
    /* Partial Display Control -----------------------------------------------*/
    LCD_WriteReg(R128, 0x0000);
    LCD_WriteReg(R129, 0x0000);
    LCD_WriteReg(R130, 0x0000);
    LCD_WriteReg(R131, 0x0000);
    LCD_WriteReg(R132, 0x0000);
    LCD_WriteReg(R133, 0x0000);
    /* Panel Control ---------------------------------------------------------*/
    LCD_WriteReg(R144, 0x0010);
    LCD_WriteReg(R146, 0x0000);
    LCD_WriteReg(R147, 0x0003);
    LCD_WriteReg(R149, 0x0110);
    LCD_WriteReg(R151, 0x0000);
    LCD_WriteReg(R152, 0x0000);
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=01 (Horizontal : increment, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1018);
    LCD_WriteReg(R7, 0x0173); /* 262K color and display ON */
  }
  else if(LCDType == LCD_SPFD5408)
  {   
    /* Start Initial Sequence --------------------------------------------------*/
    LCD_WriteReg(R227, 0x3008); /* Set internal timing */
    LCD_WriteReg(R231, 0x0012); /* Set internal timing */
    LCD_WriteReg(R239, 0x1231); /* Set internal timing */
    LCD_WriteReg(R1, 0x0100);   /* Set SS and SM bit */
    LCD_WriteReg(R2, 0x0700);   /* Set 1 line inversion */
    LCD_WriteReg(R3, 0x1030);   /* Set GRAM write direction and BGR=1. */
    LCD_WriteReg(R4, 0x0000);   /* Resize register */
    LCD_WriteReg(R8, 0x0202);   /* Set the back porch and front porch */
    LCD_WriteReg(R9, 0x0000);   /* Set non-display area refresh cycle ISC[3:0] */
    LCD_WriteReg(R10, 0x0000);  /* FMARK function */
    LCD_WriteReg(R12, 0x0000);  /* RGB interface setting */
    LCD_WriteReg(R13, 0x0000);  /* Frame marker Position */
    LCD_WriteReg(R15, 0x0000);  /* RGB interface polarity */
    /* Power On sequence -------------------------------------------------------*/
    LCD_WriteReg(R16, 0x0000);  /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(R17, 0x0000);  /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(R18, 0x0000);  /* VREG1OUT voltage */
    LCD_WriteReg(R19, 0x0000);  /* VDV[4:0] for VCOM amplitude */
    _delay_(20);                /* Dis-charge capacitor power voltage (200ms) */  
    LCD_WriteReg(R17, 0x0007);	/* DC1[2:0], DC0[2:0], VC[2:0] */
    _delay_(5);                 /* Delay 50 ms */
    LCD_WriteReg(R16, 0x12B0);	/* SAP, BT[3:0], AP, DSTB, SLP, STB */
    _delay_(5);	                /* Delay 50 ms */
    LCD_WriteReg(R18, 0x01BD);  /* External reference voltage= Vci */
    _delay_(5);                 /* Delay 50 ms */ 
    LCD_WriteReg(R19, 0x1400);       /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(R41, 0x000E);  /* VCM[4:0] for VCOMH */
    _delay_(5);                 /* Delay 50 ms */
    LCD_WriteReg(R32, 0x0000);  /* GRAM horizontal Address */
    LCD_WriteReg(R33, 0x013F);  /* GRAM Vertical Address */
    /* Adjust the Gamma Curve --------------------------------------------------*/
    LCD_WriteReg(R48, 0x0007);
    LCD_WriteReg(R49, 0x0302);
    LCD_WriteReg(R50, 0x0105);
    LCD_WriteReg(R53, 0x0206);
    LCD_WriteReg(R54, 0x0808);
    LCD_WriteReg(R55, 0x0206);
    LCD_WriteReg(R56, 0x0504);
    LCD_WriteReg(R57, 0x0007);
    LCD_WriteReg(R60, 0x0105);
    LCD_WriteReg(R61, 0x0808);
    /* Set GRAM area -----------------------------------------------------------*/
    LCD_WriteReg(R80, 0x0000);  /* Horizontal GRAM Start Address */
    LCD_WriteReg(R81, 0x00EF);  /* Horizontal GRAM End Address */
    LCD_WriteReg(R82, 0x0000);  /* Vertical GRAM Start Address */
    LCD_WriteReg(R83, 0x013F);  /* Vertical GRAM End Address */
    LCD_WriteReg(R96,  0xA700); /* Gate Scan Line */
    LCD_WriteReg(R97,  0x0001); /* NDL,VLE, REV */
    LCD_WriteReg(R106, 0x0000); /* Set scrolling line */
    /* Partial Display Control -------------------------------------------------*/
    LCD_WriteReg(R128, 0x0000);
    LCD_WriteReg(R129, 0x0000);
    LCD_WriteReg(R130, 0x0000);
    LCD_WriteReg(R131, 0x0000);
    LCD_WriteReg(R132, 0x0000);
    LCD_WriteReg(R133, 0x0000);
    /* Panel Control -----------------------------------------------------------*/
    LCD_WriteReg(R144, 0x0010);
    LCD_WriteReg(R146, 0x0000);
    LCD_WriteReg(R147, 0x0003);
    LCD_WriteReg(R149, 0x0110);
    LCD_WriteReg(R151, 0x0000);
    LCD_WriteReg(R152, 0x0000);
    /* Set GRAM write direction and BGR = 1
       I/D=01 (Horizontal : increment, Vertical : decrement)
       AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1018);
    LCD_WriteReg(R7, 0x0112);   /* 262K color and display ON */
  }
  else if(LCDType == LCD_HX8312)
  {
    /* Enable the LCD Oscillator ---------------------------------------------*/
    LCD_WriteReg(R1, 0x10);
    LCD_WriteReg(R0, 0xA0);
    LCD_WriteReg(R3, 0x01);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(R3, 0x00);
    LCD_WriteReg(R43, 0x04);
  
    LCD_WriteReg(R40, 0x18);
    LCD_WriteReg(R26, 0x05);
    LCD_WriteReg(R37, 0x05);
    LCD_WriteReg(R25, 0x00);
    
    /* LCD Power On ----------------------------------------------------------*/
    LCD_WriteReg(R28, 0x73);
    LCD_WriteReg(R36, 0x74);
    LCD_WriteReg(R30, 0x01);
    LCD_WriteReg(R24, 0xC1);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(R24, 0xE1);
    LCD_WriteReg(R24, 0xF1);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(R24, 0xF5);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(R27, 0x09);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(R31, 0x11);
    LCD_WriteReg(R32, 0x0E);
    LCD_WriteReg(R30, 0x81);
    _delay_(1); /* Delay 10 ms */
    
    /* Chip Set --------------------------------------------------------------*/
    LCD_WriteReg(R157, 0x00);
    LCD_WriteReg(R192, 0x00);
   
    LCD_WriteReg(R14, 0x00);
    LCD_WriteReg(R15, 0x00);
    LCD_WriteReg(R16, 0x00);
    LCD_WriteReg(R17, 0x00);
    LCD_WriteReg(R18, 0x00);
    LCD_WriteReg(R19, 0x00);
    LCD_WriteReg(R20, 0x00);
    LCD_WriteReg(R21, 0x00);
    LCD_WriteReg(R22, 0x00);
    LCD_WriteReg(R23, 0x00);
   
    LCD_WriteReg(R52, 0x01);
    LCD_WriteReg(R53, 0x00);
    LCD_WriteReg(R75, 0x00);
    LCD_WriteReg(R76, 0x00);
    LCD_WriteReg(R78, 0x00);
    LCD_WriteReg(R79, 0x00);
    LCD_WriteReg(R80, 0x00);
  
    LCD_WriteReg(R60, 0x00);
    LCD_WriteReg(R61, 0x00);
    LCD_WriteReg(R62, 0x01);
    LCD_WriteReg(R63, 0x3F);
    LCD_WriteReg(R64, 0x02);
    LCD_WriteReg(R65, 0x02);
    LCD_WriteReg(R66, 0x00);
    LCD_WriteReg(R67, 0x00);
    LCD_WriteReg(R68, 0x00);
    LCD_WriteReg(R69, 0x00);
    LCD_WriteReg(R70, 0xEF);
    LCD_WriteReg(R71, 0x00);
    LCD_WriteReg(R72, 0x00);
    LCD_WriteReg(R73, 0x01);
    LCD_WriteReg(R74, 0x3F);
  
    LCD_WriteReg(R29, 0x08);  /* R29:Gate scan direction setting */
  
    LCD_WriteReg(R134, 0x00);
    LCD_WriteReg(R135, 0x30);
    LCD_WriteReg(R136, 0x02);
    LCD_WriteReg(R137, 0x05);
  
    LCD_WriteReg(R141, 0x01);  /* R141:Register set-up mode for one line clock */
    LCD_WriteReg(R139, 0x20);  /* R139:One line SYSCLK number in one-line */
    LCD_WriteReg(R51, 0x01);  /* R51:N line inversion setting */
    LCD_WriteReg(R55, 0x01);  /* R55:Scanning method setting */
    LCD_WriteReg(R118, 0x00);
   
    /* Gamma Set -------------------------------------------------------------*/
    LCD_WriteReg(R143, 0x10);
    LCD_WriteReg(R144, 0x67);
    LCD_WriteReg(R145, 0x07);
    LCD_WriteReg(R146, 0x65);
    LCD_WriteReg(R147, 0x07);
    LCD_WriteReg(R148, 0x01);
    LCD_WriteReg(R149, 0x76);
    LCD_WriteReg(R150, 0x56);
    LCD_WriteReg(R151, 0x00);
    LCD_WriteReg(R152, 0x06);
    LCD_WriteReg(R153, 0x03);
    LCD_WriteReg(R154, 0x00);
  
    /* Display On ------------------------------------------------------------*/
    LCD_WriteReg(R1, 0x50);
    LCD_WriteReg(R5, 0x04);
    LCD_WriteReg(R0, 0x80);
    LCD_WriteReg(R59, 0x01);
    _delay_(4);  /* Delay 40 ms */
    LCD_WriteReg(R0, 0x20);
  }  
}


/**
  * @brief  Initializes the LCD.
  * @param  None
  * @retval None
  */
void STM3210B_LCD_Init(void)
{
  /* Setups the LCD */
  LCD_Setup();
  /* Try to read new LCD controller ID 0x9320 */
  if (LCD_ReadReg(R0) == LCD_ILI9320)
  {
    LCDType = LCD_ILI9320;
  }
  else
  {
    LCDType = LCD_SPFD5408;
    /* Setups the LCD */
    LCD_Setup();
    /* Try to read new LCD controller ID 0x5408 */
    if (LCD_ReadReg(R0) != LCD_SPFD5408)
    {
      LCDType = LCD_HX8312;
      /* Setups the LCD */
      LCD_Setup();
    }
  }
}


/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetTextColor(__IO uint16_t Color)
{
  TextColor = Color;
}


/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetBackColor(__IO uint16_t Color)
{
  BackColor = Color;
}


/**
  * @brief  Clears the selected line.
  * @param  Line: the Line to be cleared. 
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @retval None
  */
void LCD_ClearLine(uint8_t Line)
{
  LCD_DisplayStringLine(Line, "                    ");
}


/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;
  
  LCD_SetCursor(0x00, 0x013F); 
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }
  for(index = 0; index < 76800; index++)
  {
    LCD_WriteRAM(Color);
  }
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET); 
  }  
}


/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position. 
  * @retval None
  */
void LCD_SetCursor(uint8_t Xpos, uint16_t Ypos)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteReg(R32, Xpos);
    LCD_WriteReg(R33, Ypos);
  }
  else if(LCDType == LCD_HX8312)
  {
    LCD_WriteReg(R66, Xpos);
    LCD_WriteReg(R67, ((Ypos & 0x100)>> 8));
    LCD_WriteReg(R68, (Ypos & 0xFF));
  }
}


/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint8_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, i = 0;
  uint8_t Xaddress = 0;
   
  Xaddress = Xpos;
  
  LCD_SetCursor(Xaddress, Ypos);
  
  for(index = 0; index < 24; index++)
  {
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    }
    for(i = 0; i < 16; i++)
    {
      if((c[index] & (1 << i)) == 0x00)
      {
        LCD_WriteRAM(BackColor);
      }
      else
      {
        LCD_WriteRAM(TextColor);
      }
    }
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
    }  
    Xaddress++;
    LCD_SetCursor(Xaddress, Ypos);
  }
}


/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint8_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_DrawChar(Line, Column, &ASCII_Table[Ascii * 24]);
}


/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint8_t Line, uint8_t *ptr)
{
  uint32_t i = 0;
  uint16_t refcolumn = 319;
  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (i < 20))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by 16 */
    refcolumn -= 16;
    /* Point on the next character */
    ptr++;
    /* Increment the character counter */
    i++;
  }
}


/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X buttom left position.
  * @param  Ypos: specifies the Y buttom left position.
  * @param  Height: display window height.
  * @param  Width: display window width.
  * @retval None
  */
void LCD_SetDisplayWindow(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Horizontal GRAM Start Address */
    if(Xpos >= Height)
    {
      LCD_WriteReg(R80, (Xpos - Height + 1));
    }
    else
    {
      LCD_WriteReg(R80, 0);
    }
    /* Horizontal GRAM End Address */
    LCD_WriteReg(R81, Xpos);
    /* Vertical GRAM Start Address */
    if(Ypos >= Width)
    {
      LCD_WriteReg(R82, (Ypos - Width + 1));
    }  
    else
    {
      LCD_WriteReg(R82, 0);
    }
    /* Vertical GRAM End Address */
    LCD_WriteReg(R83, Ypos);
  }
  else if(LCDType == LCD_HX8312)
  {  
    LCD_WriteReg(R1, 0xD0);
    LCD_WriteReg(R5, 0x14);
  
    LCD_WriteReg(R69, (Xpos - Height + 1));
    LCD_WriteReg(R70, Xpos);
 
    LCD_WriteReg(R71, (((Ypos - Width + 1) & 0x100)>> 8));
    LCD_WriteReg(R72, ((Ypos - Width + 1) & 0xFF));
    LCD_WriteReg(R73, ((Ypos & 0x100)>> 8));
    LCD_WriteReg(R74, (Ypos & 0xFF));
  }
  LCD_SetCursor(Xpos, Ypos);
}


/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void LCD_WindowModeDisable(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_SetDisplayWindow(239, 0x13F, 240, 320);
    LCD_WriteReg(R3, 0x1018);
  }
  else if(LCDType == LCD_HX8312)
  {
    LCD_WriteReg(R1, 0x50);
    LCD_WriteReg(R5, 0x04); 
  }
    
}


/**
  * @brief  Displays a line.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Length: line length.
  * @param  Direction: line direction.
  *   This parameter can be one of the following values: Vertical or Horizontal.
  * @retval None
  */
void LCD_DrawLine(uint8_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
  uint32_t i = 0;
  
  LCD_SetCursor(Xpos, Ypos);
  if(Direction == Horizontal)
  { 
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    }
    for(i = 0; i < Length; i++)
    {
      LCD_WriteRAM(TextColor);
    }
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
    }
  }
  else
  {
   for(i = 0; i < Length; i++)
    {
      if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
      {
        LCD_WriteRAMWord(TextColor);
      }
      else  if(LCDType == LCD_HX8312)
      {
        LCD_WriteRAM(TextColor);
      }
      Xpos++;
      LCD_SetCursor(Xpos, Ypos);
    }
  }
}


/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Height: display rectangle height.
  * @param  Width: display rectangle width.
  * @retval None
  */
void LCD_DrawRect(uint8_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width)
{
  LCD_DrawLine(Xpos, Ypos, Width, Horizontal);
  LCD_DrawLine((Xpos + Height), Ypos, Width, Horizontal);
  
  LCD_DrawLine(Xpos, Ypos, Height, Vertical);
  LCD_DrawLine(Xpos, (Ypos - Width + 1), Height, Vertical);
}


/**
  * @brief  Displays a circle.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  Radius
  * @retval None
  */
void LCD_DrawCircle(uint8_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;/* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    LCD_SetCursor(Xpos + CurX, Ypos + CurY);
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos + CurX, Ypos - CurY);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos - CurX, Ypos + CurY);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos - CurX, Ypos - CurY);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos + CurY, Ypos + CurX);
    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos + CurY, Ypos - CurX);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos - CurY, Ypos + CurX);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
    LCD_SetCursor(Xpos - CurY, Ypos - CurX);

    if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
    {
      LCD_WriteRAMWord(TextColor);
    }
    else if(LCDType == LCD_HX8312)
    {
      LCD_WriteRAM(TextColor);
    }
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


/**
  * @brief  Displays a monocolor picture.
  * @param  Pict: pointer to the picture array.
  * @retval None
  */
void LCD_DrawMonoPict(const uint32_t *Pict)
{
  uint32_t index = 0, i = 0;
  LCD_SetCursor(0, 319); 

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }

  for(index = 0; index < 2400; index++)
  {
    for(i = 0; i < 32; i++)
    {
      if((Pict[index] & (1 << i)) == 0x00)
      {
        LCD_WriteRAM(BackColor);
      }
      else
      {
        LCD_WriteRAM(TextColor);
      }
    }
  }

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  }
}

#ifdef USE_LCD_DrawBMP 
/**
  * @brief  Displays a bitmap picture loaded in the SPI Flash.
  * @param  BmpAddress: Bmp picture address in the SPI Flash.
  * @retval None
  */
void LCD_DrawBMP(uint32_t BmpAddress)
{
  uint32_t i = 0, size = 0;
  /* Read bitmap size */
  SPI_FLASH_BufferRead((uint8_t*)&size, BmpAddress + 2, 4);
  /* get bitmap data address offset */
  SPI_FLASH_BufferRead((uint8_t*)&i, BmpAddress + 10, 4);
  
  size = (size - i)/2;
  SPI_FLASH_StartReadSequence(BmpAddress + i);
  /* Disable SPI1  */
  SPI_Cmd(SPI1, DISABLE);
  /* SPI in 16-bit mode */
  SPI_DataSizeConfig(SPI1, SPI_DataSize_16b);
  /* Enable SPI1  */
  SPI_Cmd(SPI1, ENABLE);
  
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
    /* AM=1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1008);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
  }
  
  /* Read bitmap data from SPI Flash and send them to LCD */
  for(i = 0; i < size; i++)
  {
    LCD_WriteRAM(__REV16(SPI_FLASH_SendHalfWord(0xA5A5)));
  }
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  }

  /* Deselect the FLASH: Chip Select high */
  SPI_FLASH_CS_HIGH();
  /* Disable SPI1  */
  SPI_Cmd(SPI1, DISABLE);
  /* SPI in 8-bit mode */
  SPI_DataSizeConfig(SPI1, SPI_DataSize_8b);
  /* Enable SPI1  */
  SPI_Cmd(SPI1, ENABLE);

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Set GRAM write direction and BGR = 1 */
    /* I/D = 01 (Horizontal : increment, Vertical : decrement) */
    /* AM = 1 (address is updated in vertical writing direction) */
    LCD_WriteReg(R3, 0x1018);
  }
}
#endif /* USE_LCD_DrawBMP */
 


/**
  * @brief  Reset LCD control line(/CS) and Send Start-Byte
  * @param  Start_Byte: the Start-Byte to be sent
  * @retval None
  */
void LCD_nCS_StartByte(uint8_t Start_Byte)
{
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);

  SPI_I2S_SendData(LCD_SPI, Start_Byte);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }
}


/**
  * @brief  Writes index to select the LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @retval None
  */
void LCD_WriteRegIndex(uint8_t LCD_Reg)
{
  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | SET_INDEX);

  /* Write 16-bit Reg Index (High Byte is 0) */
  SPI_I2S_SendData(LCD_SPI, 0x00);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  SPI_I2S_SendData(LCD_SPI, LCD_Reg);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}


/**
  * @brief  Writes to the selected LCD ILI9320 register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
static void LCD_WriteRegILI9320(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  /* Write 16-bit Index (then Write Reg) */
  LCD_WriteRegIndex(LCD_Reg);

  /* Write 16-bit Reg */
  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | LCD_WRITE_REG);

  SPI_I2S_SendData(LCD_SPI, LCD_RegValue>>8);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  SPI_I2S_SendData(LCD_SPI, (LCD_RegValue & 0xFF));

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}


/**
  * @brief  Reads the selected LCD Register.
  * @param  LCD_Reg: address of the selected register.
  * @retval LCD Register Value.
  */
uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  uint16_t tmp = 0;
  uint8_t i = 0;
  
  /* LCD_SPI prescaler: 4 */
  LCD_SPI->CR1 &= 0xFFC7;
  LCD_SPI->CR1 |= 0x0008;
  /* Write 16-bit Index (then Read Reg) */
  LCD_WriteRegIndex(LCD_Reg);
  /* Read 16-bit Reg */
  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | LCD_READ_REG);
  
  for(i = 0; i < 5; i++)
  {
    SPI_I2S_SendData(LCD_SPI, 0xFF);
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
    {
    }
    /* One byte of invalid dummy data read after the start byte */
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
    {    
    }
    SPI_I2S_ReceiveData(LCD_SPI); 
  }

  SPI_I2S_SendData(LCD_SPI, 0xFF);

  /* Read upper byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  /* Read lower byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }
  tmp = SPI_I2S_ReceiveData(LCD_SPI);
  
  
  SPI_I2S_SendData(LCD_SPI, 0xFF);
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }

  /* Read lower byte */
  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
  {
  }

  tmp = ((tmp & 0xFF) << 8) | SPI_I2S_ReceiveData(LCD_SPI);
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);

  /* LCD_SPI prescaler: 2 */
  LCD_SPI->CR1 &= 0xFFC7;

  return tmp;
}


/**
  * @brief  Prepare to write to the LCD RAM.
  * @param  None
  * @retval None
  */
void LCD_WriteRAM_Prepare(void)
{
  LCD_WriteRegIndex(R34); /* Select GRAM Reg */

  /* Reset LCD control line(/CS) and Send Start-Byte */
  LCD_nCS_StartByte(START_BYTE | LCD_WRITE_REG);
}


/**
  * @brief  Writes 1 word to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void LCD_WriteRAMWord(uint16_t RGB_Code)
{

  LCD_WriteRAM_Prepare();

  LCD_WriteRAM(RGB_Code);

  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}


/**
  * @brief  Writes to the selected LCD HX8312 register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
static void LCD_WriteRegHX8312(uint8_t LCD_Reg, uint8_t LCD_RegValue)
{
  uint16_t tmp = 0;
  
  LCD_CtrlLinesWrite(LCD_NWR_GPIO_PORT, LCD_NWR_PIN, Bit_RESET);
  LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_RESET);
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
  
  tmp = LCD_Reg << 8;
  tmp |= LCD_RegValue;
   
  SPI_I2S_SendData(LCD_SPI, tmp);

  while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
  {
  }
  
  LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
}


/**
  * @brief  Writes to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    LCD_WriteRegILI9320(LCD_Reg, LCD_RegValue);
  }
  else if(LCDType == LCD_HX8312)
  {
    LCD_WriteRegHX8312(LCD_Reg, ((uint8_t)LCD_RegValue));
  }
}


/**
  * @brief  Writes to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void LCD_WriteRAM(uint16_t RGB_Code)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    SPI_I2S_SendData(LCD_SPI, RGB_Code >> 8);
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
    {
    }
    SPI_I2S_SendData(LCD_SPI, RGB_Code & 0xFF);
    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
    {
    }
  }

  if(LCDType == LCD_HX8312)
  {
    LCD_CtrlLinesWrite(LCD_NWR_GPIO_PORT, LCD_NWR_PIN, Bit_RESET);
    LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_RESET);
    SPI_I2S_SendData(LCD_SPI, RGB_Code);

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) != RESET)
    {
    }
  
    LCD_CtrlLinesWrite(LCD_NCS_GPIO_PORT, LCD_NCS_PIN, Bit_SET);
  }
}


/**
  * @brief  Power on the LCD.
  * @param  None
  * @retval None
  */
void LCD_PowerOn(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Power On sequence ---------------------------------------------------------*/
    LCD_WriteReg(R16, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(R17, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(R18, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(R19, 0x0000); /* VDV[4:0] for VCOM amplitude */
    _delay_(20);               /* Dis-charge capacitor power voltage (200ms) */
    LCD_WriteReg(R16, 0x17B0); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(R17, 0x0137); /* DC1[2:0], DC0[2:0], VC[2:0] */
    _delay_(5);                /* Delay 50 ms */
    LCD_WriteReg(R18, 0x0139); /* VREG1OUT voltage */
    _delay_(5);                /* delay 50 ms */
    LCD_WriteReg(R19, 0x1d00); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(R41, 0x0013); /* VCM[4:0] for VCOMH */
    _delay_(5);                /* delay 50 ms */
    LCD_WriteReg(R7, 0x0173);  /* 262K color and display ON */
  }
  else if(LCDType == LCD_HX8312)
  {  
    /* Power On Set */
    LCD_WriteReg(R28, 0x73);
    LCD_WriteReg(R36, 0x74);
    LCD_WriteReg(R30, 0x01);
    LCD_WriteReg(R24, 0xC1);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(R24, 0xE1);
    LCD_WriteReg(R24, 0xF1);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(R24, 0xF5);
    _delay_(6); /* Delay 60 ms */
    LCD_WriteReg(R27, 0x09);
    _delay_(1); /* Delay 10 ms */
    LCD_WriteReg(R31, 0x11);
    LCD_WriteReg(R32, 0x0E);
    LCD_WriteReg(R30, 0x81);
    _delay_(1); /* Delay 10 ms */
  }
}


/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOn(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Display On */
    LCD_WriteReg(R7, 0x0173); /* 262K color and display ON */
  }
  else if(LCDType == LCD_HX8312)
  {  
    LCD_WriteReg(R1, 0x50);
    LCD_WriteReg(R5, 0x04);
    /* Display On */
    LCD_WriteReg(R0, 0x80);
    LCD_WriteReg(R59, 0x01);
    _delay_(4); /* Delay 40 ms */
    LCD_WriteReg(R0, 0x20);
  }  
}


/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void LCD_DisplayOff(void)
{
  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    /* Display Off */
    LCD_WriteReg(R7, 0x0);
  }
  else if(LCDType == LCD_HX8312)
  { 
    /* Display Off */
    LCD_WriteReg(R0, 0xA0);
    _delay_(4);  /* Delay 40 ms */
    LCD_WriteReg(R59, 0x00);
  } 
}


/**
  * @brief  Configures LCD control lines in Output Push-Pull mode.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(LCD_RSNWR_GPIO_CLK | LCD_NCS_GPIO_CLK, ENABLE);

  /* Configure NCS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_NCS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LCD_NCS_GPIO_PORT, &GPIO_InitStructure);
  
  /* Configure NWR(RNW), RS in Output Push-Pull mode */
  GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN | LCD_NWR_PIN;
  GPIO_Init(LCD_RS_GPIO_PORT, &GPIO_InitStructure);
  
  LCD_CtrlLinesWrite(LCD_NWR_GPIO_PORT, LCD_NWR_PIN, Bit_SET);
  LCD_CtrlLinesWrite(LCD_RS_GPIO_PORT, LCD_RS_PIN, Bit_SET);
}


/**
  * @brief  Sets or reset LCD control lines.
  * @param  GPIOx: where x can be B or D to select the GPIO peripheral.
  * @param  CtrlPins: the Control line.
  *   This parameter can be:
  *     @arg LCD_NCS_PIN: Chip Select pin
  *     @arg LCD_NWR_PIN: Read/Write Selection pin
  *     @arg LCD_RS_PIN: Register/RAM Selection pin
  * @param  BitVal: specifies the value to be written to the selected bit.
  *   This parameter can be:
  *     @arg Bit_RESET: to clear the port pin
  *     @arg Bit_SET: to set the port pin
  * @retval None
  */
void LCD_CtrlLinesWrite(GPIO_TypeDef* GPIOx, uint16_t CtrlPins, BitAction BitVal)
{
  /* Set or Reset the control line */
  GPIO_WriteBit(GPIOx, CtrlPins, BitVal);
}


/**
  * @brief  Configures the LCD_SPI interface.
  * @param  None
  * @retval None
  */
void LCD_SPIConfig(void)
{
  SPI_InitTypeDef    SPI_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(LCD_SPI_GPIO_CLK, ENABLE);

  /* Enable SPI clock  */
  RCC_APB1PeriphClockCmd(LCD_SPI_CLK, ENABLE);

  /* Configure SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = LCD_SPI_SCK_PIN | LCD_SPI_MISO_PIN | LCD_SPI_MOSI_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(LCD_SPI_GPIO_PORT, &GPIO_InitStructure);

  SPI_I2S_DeInit(LCD_SPI);
  
  /* SPI Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

  if((LCDType == LCD_ILI9320) || (LCDType == LCD_SPFD5408))
  {
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  }
  else if(LCDType == LCD_HX8312)
  {
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  }
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

  SPI_Init(LCD_SPI, &SPI_InitStructure);

  /* SPI enable */
  SPI_Cmd(LCD_SPI, ENABLE);
}

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (100000 * nCount); index != 0; index--)
  {
  }
}
#endif /* USE_Delay*/
/**
  * @}
  */ 


/**
  * @}
  */ 

/**
  * @}
  */ 
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
