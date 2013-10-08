/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : msd.c
* Author             : MCD Application Team
* Version            : V3.1.0
* Date               : 10/30/2009
* Description        : MSD card driver source file.
*                      Pin assignment:
*             ----------------------------------------------
*             |  STM32F10x    |     MSD          Pin        |
*             ----------------------------------------------
*             | CS            |   ChipSelect      1         |
*             | MOSI          |   DataIn          2         |
*             |               |   GND             3 (0 V)   |
*             |               |   VDD             4 (3.3 V) |
*             | SCLK          |   Clock           5         |
*             |               |   GND             6 (0 V)   |
*             | MISO          |   DataOut         7         |
*             -----------------------------------------------
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "msd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Select MSD Card: ChipSelect pin low  */
#define MSD_CS_LOW()     GPIO_ResetBits(MSD_CS_PORT, MSD_CS_PIN)
/* Deselect MSD Card: ChipSelect pin high */
#define MSD_CS_HIGH()    GPIO_SetBits(MSD_CS_PORT, MSD_CS_PIN)

/* Private function prototypes -----------------------------------------------*/
static void SPI_Config(void);
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MSD_Init
* Description    : Initializes the MSD/SD communication.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_Init(void)
{
  uint32_t i = 0;

  /* Initialize MSD_SPI */
  SPI_Config();
  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF, 10 times with CS high*/
  /* rise CS and MOSI for 80 clocks cycles */
  for (i = 0; i <= 9; i++)
  {
    /* Send dummy byte 0xFF */
    MSD_WriteByte(DUMMY);
  }
  /*------------Put MSD in SPI mode--------------*/
  /* MSD initialized and set to SPI mode properly */
  return (MSD_GoIdleState());
}

/*******************************************************************************
* Function Name  : MSD_WriteBlock
* Description    : Writes a block on the MSD
* Input          : - pBuffer : pointer to the buffer containing the data to be
*                    written on the MSD.
*                  - WriteAddr : address to write on.
*                  - NumByteToWrite: number of data to write
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint32_t i = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD24 (MSD_WRITE_BLOCK) to write multiple block */
  MSD_SendCmd(MSD_WRITE_BLOCK, WriteAddr, 0xFF);

  /* Check if the MSD acknowledged the write block command: R1 response (0x00: no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    /* Send a dummy byte */
    MSD_WriteByte(DUMMY);
    /* Send the data token to signify the start of the data */
    MSD_WriteByte(0xFE);
    /* Write the block data to MSD : write count data by block */
    for (i = 0; i < NumByteToWrite; i++)
    {
      /* Send the pointed byte */
      MSD_WriteByte(*pBuffer);
      /* Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /* Put CRC bytes (not really needed by us, but required by MSD) */
    MSD_ReadByte();
    MSD_ReadByte();
    /* Read data response */
    if (MSD_GetDataResponse() == MSD_DATA_OK)
    {
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_ReadBlock
* Description    : Reads a block of data from the MSD.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the MSD.
*                  - ReadAddr : MSD's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the MSD.
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  uint32_t i = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
  MSD_SendCmd(MSD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);

  /* Check if the MSD acknowledged the read block command: R1 response (0x00: no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    /* Now look for the data token to signify the start of the data */
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Read the MSD block data : read NumByteToRead data */
      for (i = 0; i < NumByteToRead; i++)
      {
        /* Save the received data */
        *pBuffer = MSD_ReadByte();
        /* Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /* Get CRC bytes (not really needed by us, but required by MSD) */
      MSD_ReadByte();
      MSD_ReadByte();
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_WriteBuffer
* Description    : Writes many blocks on the MSD
* Input          : - pBuffer : pointer to the buffer containing the data to be
*                    written on the MSD.
*                  - WriteAddr : address to write on.
*                  - NumByteToWrite: number of data to write
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
  uint32_t i = 0, NbrOfBlock = 0, Offset = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;

  /* Calculate number of blocks to write */
  NbrOfBlock = NumByteToWrite / BLOCK_SIZE;
  /* MSD chip select low */
  MSD_CS_LOW();

  /* Data transfer */
  while (NbrOfBlock --)
  {
    /* Send CMD24 (MSD_WRITE_BLOCK) to write blocks */
    MSD_SendCmd(MSD_WRITE_BLOCK, WriteAddr + Offset, 0xFF);

    /* Check if the MSD acknowledged the write block command: R1 response (0x00: no errors) */
    if (MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
    {
      return MSD_RESPONSE_FAILURE;
    }
    /* Send dummy byte */
    MSD_WriteByte(DUMMY);
    /* Send the data token to signify the start of the data */
    MSD_WriteByte(MSD_START_DATA_SINGLE_BLOCK_WRITE);
    /* Write the block data to MSD : write count data by block */
    for (i = 0; i < BLOCK_SIZE; i++)
    {
      /* Send the pointed byte */
      MSD_WriteByte(*pBuffer);
      /* Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /* Set next write address */
    Offset += 512;
    /* Put CRC bytes (not really needed by us, but required by MSD) */
    MSD_ReadByte();
    MSD_ReadByte();
    /* Read data response */
    if (MSD_GetDataResponse() == MSD_DATA_OK)
    {
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_RESPONSE_FAILURE;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_ReadBuffer
* Description    : Reads multiple block of data from the MSD.
* Input          : - pBuffer : pointer to the buffer that receives the data read
*                    from the MSD.
*                  - ReadAddr : MSD's internal address to read from.
*                  - NumByteToRead : number of bytes to read from the MSD.
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
  uint32_t i = 0, NbrOfBlock = 0, Offset = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;

  /* Calculate number of blocks to read */
  NbrOfBlock = NumByteToRead / BLOCK_SIZE;
  /* MSD chip select low */
  MSD_CS_LOW();

  /* Data transfer */
  while (NbrOfBlock --)
  {
    /* Send CMD17 (MSD_READ_SINGLE_BLOCK) to read one block */
    MSD_SendCmd (MSD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
    /* Check if the MSD acknowledged the read block command: R1 response (0x00: no errors) */
    if (MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
    {
      return  MSD_RESPONSE_FAILURE;
    }
    /* Now look for the data token to signify the start of the data */
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Read the MSD block data : read NumByteToRead data */
      for (i = 0; i < BLOCK_SIZE; i++)
      {
        /* Read the pointed data */
        *pBuffer = MSD_ReadByte();
        /* Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /* Set next read address*/
      Offset += 512;
      /* get CRC bytes (not really needed by us, but required by MSD) */
      MSD_ReadByte();
      MSD_ReadByte();
      /* Set response value to success */
      rvalue = MSD_RESPONSE_NO_ERROR;
    }
    else
    {
      /* Set response value to failure */
      rvalue = MSD_RESPONSE_FAILURE;
    }
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);
  /* Returns the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_GetCSDRegister
* Description    : Read the CSD card register.
*                  Reading the contents of the CSD register in SPI mode
*                  is a simple read-block transaction.
* Input          : - MSD_csd: pointer on an SCD register structure
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_GetCSDRegister(sMSD_CSD* MSD_csd)
{
  uint32_t i = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;
  uint8_t CSD_Tab[16];

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD9 (CSD register) or CMD10(CSD register) */
  MSD_SendCmd(MSD_SEND_CSD, 0, 0xFF);

  /* Wait for response in the R1 format (0x00 is no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      for (i = 0; i < 16; i++)
      {
        /* Store CSD register value on CSD_Tab */
        CSD_Tab[i] = MSD_ReadByte();
      }
    }
    /* Get CRC bytes (not really needed by us, but required by MSD) */
    MSD_WriteByte(DUMMY);
    MSD_WriteByte(DUMMY);
    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);

  /* Byte 0 */
  MSD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  MSD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  MSD_csd->Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  MSD_csd->TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  MSD_csd->NSAC = CSD_Tab[2];
  /* Byte 3 */
  MSD_csd->MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  MSD_csd->CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  MSD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  MSD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  MSD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  MSD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  MSD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  MSD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  MSD_csd->Reserved2 = 0; /* Reserved */
  MSD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  MSD_csd->DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  MSD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  MSD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  MSD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  MSD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  MSD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  MSD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  MSD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  MSD_csd->EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  MSD_csd->EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  MSD_csd->EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  MSD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  MSD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  MSD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  MSD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  MSD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  MSD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  MSD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  MSD_csd->Reserved3 = 0;
  MSD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  MSD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  MSD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  MSD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  MSD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  MSD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  MSD_csd->ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  MSD_csd->msd_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  MSD_csd->Reserved4 = 1;

  /* Return the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_GetCIDRegister
* Description    : Read the CID card register.
*                  Reading the contents of the CID register in SPI mode
*                  is a simple read-block transaction.
* Input          : - MSD_cid: pointer on an CID register structure
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_GetCIDRegister(sMSD_CID* MSD_cid)
{
  uint32_t i = 0;
  uint8_t rvalue = MSD_RESPONSE_FAILURE;
  uint8_t CID_Tab[16];

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD10 (CID register) */
  MSD_SendCmd(MSD_SEND_CID, 0, 0xFF);

  /* Wait for response in the R1 format (0x00 is no errors) */
  if (!MSD_GetResponse(MSD_RESPONSE_NO_ERROR))
  {
    if (!MSD_GetResponse(MSD_START_DATA_SINGLE_BLOCK_READ))
    {
      /* Store CID register value on CID_Tab */
      for (i = 0; i < 16; i++)
      {
        CID_Tab[i] = MSD_ReadByte();
      }
    }
    /* Get CRC bytes (not really needed by us, but required by MSD) */
    MSD_WriteByte(DUMMY);
    MSD_WriteByte(DUMMY);
    /* Set response value to success */
    rvalue = MSD_RESPONSE_NO_ERROR;
  }

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte: 8 Clock pulses of delay */
  MSD_WriteByte(DUMMY);

  /* Byte 0 */
  MSD_cid->ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  MSD_cid->OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  MSD_cid->OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  MSD_cid->ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  MSD_cid->ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  MSD_cid->ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  MSD_cid->ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  MSD_cid->ProdName2 = CID_Tab[7];
  /* Byte 8 */
  MSD_cid->ProdRev = CID_Tab[8];
  /* Byte 9 */
  MSD_cid->ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  MSD_cid->ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  MSD_cid->ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  MSD_cid->ProdSN |= CID_Tab[12];
  /* Byte 13 */
  MSD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  MSD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  MSD_cid->ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  MSD_cid->msd_CRC = (CID_Tab[15] & 0xFE) >> 1;
  MSD_cid->Reserved2 = 1;

  /* Return the reponse */
  return rvalue;
}

/*******************************************************************************
* Function Name  : MSD_SendCmd
* Description    : Send 5 bytes command to the MSD card.
* Input          : - Cmd: the user expected command to send to MSD card
*                  - Arg: the command argument
*                  - Crc: the CRC
* Output         : None
* Return         : None
*******************************************************************************/
void MSD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
  uint32_t i = 0x00;
  uint8_t Frame[6];

  /* Construct byte1 */
  Frame[0] = (Cmd | 0x40);
  /* Construct byte2 */
  Frame[1] = (uint8_t)(Arg >> 24);
  /* Construct byte3 */
  Frame[2] = (uint8_t)(Arg >> 16);
  /* Construct byte4 */
  Frame[3] = (uint8_t)(Arg >> 8);
  /* Construct byte5 */
  Frame[4] = (uint8_t)(Arg);
  /* Construct CRC: byte6 */
  Frame[5] = (Crc);

  /* Send the Cmd bytes */
  for (i = 0; i < 6; i++)
  {
    MSD_WriteByte(Frame[i]);
  }
}

/*******************************************************************************
* Function Name  : MSD_GetDataResponse
* Description    : Get MSD card data response.
* Input          : None
* Output         : None
* Return         : The MSD status: Read data response xxx0<status>1
*                   - status 010: Data accecpted
*                   - status 101: Data rejected due to a crc error
*                   - status 110: Data rejected due to a Write error.
*                   - status 111: Data rejected due to other error.
*******************************************************************************/
uint8_t MSD_GetDataResponse(void)
{
  uint32_t i = 0;
  uint8_t response, rvalue;

  while (i <= 64)
  {
    /* Read resonse */
    response = MSD_ReadByte();
    /* Mask unused bits */
    response &= 0x1F;

    switch (response)
    {
      case MSD_DATA_OK:
      {
        rvalue = MSD_DATA_OK;
        break;
      }

      case MSD_DATA_CRC_ERROR:
        return MSD_DATA_CRC_ERROR;

      case MSD_DATA_WRITE_ERROR:
        return MSD_DATA_WRITE_ERROR;

      default:
      {
        rvalue = MSD_DATA_OTHER_ERROR;
        break;
      }
    }
    /* Exit loop in case of data ok */
    if (rvalue == MSD_DATA_OK)
      break;
    /* Increment loop counter */
    i++;
  }
  /* Wait null data */
  while (MSD_ReadByte() == 0);
  /* Return response */
  return response;
}

/*******************************************************************************
* Function Name  : MSD_GetResponse
* Description    : Returns the MSD response.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_GetResponse(uint8_t Response)
{
  uint32_t Count = 0xFFF;

  /* Check if response is got or a timeout is happen */
  while ((MSD_ReadByte() != Response) && Count)
  {
    Count--;
  }

  if (Count == 0)
  {
    /* After time out */
    return MSD_RESPONSE_FAILURE;
  }
  else
  {
    /* Right response got */
    return MSD_RESPONSE_NO_ERROR;
  }
}

/*******************************************************************************
* Function Name  : MSD_GetStatus
* Description    : Returns the MSD status.
* Input          : None
* Output         : None
* Return         : The MSD status.
*******************************************************************************/
uint16_t MSD_GetStatus(void)
{
  uint16_t Status = 0;

  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD13 (MSD_SEND_STATUS) to get MSD status */
  MSD_SendCmd(MSD_SEND_STATUS, 0, 0xFF);

  Status = MSD_ReadByte();
  Status |= (uint16_t)(MSD_ReadByte() << 8);

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF */
  MSD_WriteByte(DUMMY);

  return Status;
}

/*******************************************************************************
* Function Name  : MSD_GoIdleState
* Description    : Put MSD in Idle state.
* Input          : None
* Output         : None
* Return         : The MSD Response: - MSD_RESPONSE_FAILURE: Sequence failed
*                                    - MSD_RESPONSE_NO_ERROR: Sequence succeed
*******************************************************************************/
uint8_t MSD_GoIdleState(void)
{
  /* MSD chip select low */
  MSD_CS_LOW();
  /* Send CMD0 (GO_IDLE_STATE) to put MSD in SPI mode */
  MSD_SendCmd(MSD_GO_IDLE_STATE, 0, 0x95);

  /* Wait for In Idle State Response (R1 Format) equal to 0x01 */
  if (MSD_GetResponse(MSD_IN_IDLE_STATE))
  {
    /* No Idle State Response: return response failue */
    return MSD_RESPONSE_FAILURE;
  }
  /*----------Activates the card initialization process-----------*/
  do
  {
    /* MSD chip select high */
    MSD_CS_HIGH();
    /* Send Dummy byte 0xFF */
    MSD_WriteByte(DUMMY);

    /* MSD chip select low */
    MSD_CS_LOW();

    /* Send CMD1 (Activates the card process) until response equal to 0x0 */
    MSD_SendCmd(MSD_SEND_OP_COND, 0, 0xFF);
    /* Wait for no error Response (R1 Format) equal to 0x00 */
  }
  while (MSD_GetResponse(MSD_RESPONSE_NO_ERROR));

  /* MSD chip select high */
  MSD_CS_HIGH();
  /* Send dummy byte 0xFF */
  MSD_WriteByte(DUMMY);

  return MSD_RESPONSE_NO_ERROR;
}

/*******************************************************************************
* Function Name  : MSD_WriteByte
* Description    : Write a byte on the MSD.
* Input          : Data: byte to send.
* Output         : None
* Return         : None.
*******************************************************************************/
void MSD_WriteByte(uint8_t Data)
{
  /* Wait until the transmit buffer is empty */
  while (SPI_I2S_GetFlagStatus(MSD_SPI, SPI_I2S_FLAG_TXE) == RESET);
  /* Send the byte */
  SPI_I2S_SendData(MSD_SPI, Data);

  /* Wait until a data is received */
  while (SPI_I2S_GetFlagStatus(MSD_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  /* Get the received data */
  SPI_I2S_ReceiveData(MSD_SPI);
  
}

/*******************************************************************************
* Function Name  : MSD_ReadByte
* Description    : Read a byte from the MSD.
* Input          : None.
* Output         : None
* Return         : The received byte.
*******************************************************************************/
uint8_t MSD_ReadByte(void)
{
  uint8_t Data = 0;

  /* Wait until the transmit buffer is empty */
  while (SPI_I2S_GetFlagStatus(MSD_SPI, SPI_I2S_FLAG_TXE) == RESET);
  /* Send the byte */
  SPI_I2S_SendData(MSD_SPI, DUMMY);

  /* Wait until a data is received */
  while (SPI_I2S_GetFlagStatus(MSD_SPI, SPI_I2S_FLAG_RXNE) == RESET);
  /* Get the received data */
  Data = SPI_I2S_ReceiveData(MSD_SPI);

  /* Return the shifted data */
  return Data;
}

/*******************************************************************************
* Function Name  : SPI_Config
* Description    : Initializes the MSD_SPI and CS pins.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  SPI_InitTypeDef   SPI_InitStructure;

  /* MSD_SPI_PORT and MSD_CS_PORT Periph clock enable */
  RCC_APB2PeriphClockCmd(MSD_SPI_GPIO_PORT_CLOCK | MSD_CS_GPIO_PORT_CLOCK | \
                         RCC_APB2Periph_AFIO, ENABLE);

  /* MSD_SPI Periph clock enable */
#ifdef USE_STM3210B_EVAL
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
#elif defined (USE_STM3210C_EVAL)
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
#endif /* USE_STM3210B_EVAL */

  /* Remap the SPI pins if needed */
#ifdef SPI_REMAPPED
  GPIO_PinRemapConfig(MSD_SPI_GPIO_REMAP, ENABLE);
#endif /* SPI_REMAPPED */

  /* Configure MSD_SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = MSD_SPI_PIN_SCK | MSD_SPI_PIN_MISO | MSD_SPI_PIN_MOSI;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(MSD_SPI_PORT, &GPIO_InitStructure);

  /* Configure CS pin */
  GPIO_InitStructure.GPIO_Pin = MSD_CS_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(MSD_CS_PORT, &GPIO_InitStructure);

  /* MSD_SPI Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(MSD_SPI, &SPI_InitStructure);

  /* MSD_SPI enable */
  SPI_Cmd(MSD_SPI, ENABLE);
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
