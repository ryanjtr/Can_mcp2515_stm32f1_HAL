/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#include "MCP2515.h"
#include "main.h"

/* Prototypes */
static void SPI_Tx(uint8_t data);
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length);
static uint8_t SPI_Rx(void);
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length);

/*Global variables*/

uint8_t msgTXERR[6] = "TXERR\n";
uint8_t msgMLOA[6] = "MLOA\n";
uint8_t msgABTF[6] = "ABTF\n";


/* initialize MCP2515 */
bool MCP2515_Initialize(void)
{
  MCP2515_CS_HIGH();

  uint8_t loop = 100;

  do
  {
    /* check SPI Ready */
    if (HAL_SPI_GetState(SPI_CAN) == HAL_SPI_STATE_READY)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* change mode as configuration mode */
bool MCP2515_SetConfigMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x80); // enable CLKN bit

  uint8_t loop = 100;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x80) // enable CLKN bit
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* change mode as normal mode */
bool MCP2515_SetNormalMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x00); // enable CLKN bit

  uint8_t loop = 100;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x00) // enable CLKN bit
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/*change mode as loop back mode*/
bool MCP2515_SetLoopBackMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x40); // enable CLKN bit

  uint8_t loop = 10;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x40) // enable CLKN bit
      return true;

    loop--;
  } while (loop > 0);

  return false;
}
/* Entering sleep mode */
bool MCP2515_SetSleepMode(void)
{
  /* configure CANCTRL Register */
  MCP2515_WriteByte(MCP2515_CANCTRL, 0x20);

  uint8_t loop = 10;

  do
  {
    /* confirm mode configuration */
    if ((MCP2515_ReadByte(MCP2515_CANSTAT) & 0xE0) == 0x20)
      return true;

    loop--;
  } while (loop > 0);

  return false;
}

/* MCP2515 SPI-Reset */
void MCP2515_Reset(void)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_RESET);

  MCP2515_CS_HIGH();
}

/* read single byte */
uint8_t MCP2515_ReadByte(uint8_t address)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_READ);
  SPI_Tx(address);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* read buffer */
void MCP2515_ReadRxSequence(uint8_t instruction, uint8_t *data, uint8_t length)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_RxBuffer(data, length);

  MCP2515_CS_HIGH();
}

/* write single byte */
void MCP2515_WriteByte(uint8_t address, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(address);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* write buffer */
void MCP2515_WriteByteSequence(uint8_t startAddress, uint8_t endAddress, uint8_t *data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_WRITE);
  SPI_Tx(startAddress);
  SPI_TxBuffer(data, (endAddress - startAddress + 1));

  MCP2515_CS_HIGH();
}

/* write to TxBuffer */
void MCP2515_LoadTxSequence(uint8_t instruction, uint8_t *idReg, uint8_t dlc, uint8_t *data)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_TxBuffer(idReg, 4);
  SPI_Tx(dlc);
  SPI_TxBuffer(data, dlc);

  MCP2515_CS_HIGH();
}

/* write to TxBuffer(1 byte) */
void MCP2515_LoadTxBuffer(uint8_t instruction, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* request to send */
void MCP2515_RequestToSend(uint8_t instruction)
{
  MCP2515_CS_LOW();

  SPI_Tx(instruction);

  MCP2515_CS_HIGH();
}

/* read status */
uint8_t MCP2515_ReadStatus(void)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_READ_STATUS);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* read RX STATUS register */
uint8_t MCP2515_GetRxStatus(void)
{
  uint8_t retVal;

  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_RX_STATUS);
  retVal = SPI_Rx();

  MCP2515_CS_HIGH();

  return retVal;
}

/* Use when changing register value */
void MCP2515_BitModify(uint8_t address, uint8_t mask, uint8_t data)
{
  MCP2515_CS_LOW();

  SPI_Tx(MCP2515_BIT_MOD);
  SPI_Tx(address);
  SPI_Tx(mask);
  SPI_Tx(data);

  MCP2515_CS_HIGH();
}

/* SPI Tx wrapper function  */
static void SPI_Tx(uint8_t data)
{
  HAL_SPI_Transmit(SPI_CAN, &data, 1, SPI_TIMEOUT);
}

/* SPI Tx wrapper function */
static void SPI_TxBuffer(uint8_t *buffer, uint8_t length)
{
  HAL_SPI_Transmit(SPI_CAN, buffer, length, SPI_TIMEOUT);
}

/* SPI Rx wrapper function */
static uint8_t SPI_Rx(void)
{
  uint8_t retVal;
  HAL_SPI_Receive(SPI_CAN, &retVal, 1, SPI_TIMEOUT);
  return retVal;
}

/* SPI Rx wrapper function */
static void SPI_RxBuffer(uint8_t *buffer, uint8_t length)
{
  HAL_SPI_Receive(SPI_CAN, buffer, length, SPI_TIMEOUT);
}

uint8_t MCP2515_Trans0(uint8_t *data)
{
  uint8_t retVal;

  // write ID high
  MCP2515_WriteByte(MCP2515_TXB0SIDH, 0x14);
  retVal = MCP2515_ReadByte(MCP2515_TXB0SIDH);
  // write ID low
  MCP2515_WriteByte(MCP2515_TXB0SIDL, 0x60);
  retVal = MCP2515_ReadByte(MCP2515_TXB0SIDL);
  //  //write ID_ext high
  //  MCP2515_WriteByte(MCP2515_TXB0EID8,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB0EID8);
  //  //write ID_ext low
  //  MCP2515_WriteByte(MCP2515_TXB0EID0,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB0EID0);
  // write DLC
  MCP2515_WriteByte(MCP2515_TXB0DLC, 0x08);
  retVal = MCP2515_ReadByte(MCP2515_TXB0DLC);
  // write 8 byte data
  for (int i = 0; i < 8; ++i)
  {
    MCP2515_WriteByte(MCP2515_TXB0Dm + i, data[i]);
  }

  MCP2515_WriteByte(MCP2515_TXB0CTRL, 0x0B); // enable TXREQ, enable bit highest message priority: TXP1 & TXP0
  retVal = MCP2515_ReadByte(MCP2515_TXB0CTRL);
//  if ((retVal >> 4) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgTXERR,6,100);
//	  return 0;
//  }
//  else if((retVal >> 5) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgMLOA,6,100);
//	  return 0;
//  }
//  else if((retVal >> 6) & 0x01)
//	{
//			HAL_UART_Transmit(&huart1,msgABTF,6,100);
//			return 0;
//	}

//  MCP2515_WriteByte(MCP2515_TXRTSCTRL, 0x01);//enable B0RTSM: Pin is used to request message transmission of TXB0 buffer (on falling edge)
  MCP2515_CS_LOW();
  //  SPI_Tx(MCP2515_LOAD_TXB0D0);
  SPI_Tx(MCP2515_RTS_TX0);
  MCP2515_CS_HIGH();
  return 1;

}

uint8_t MCP2515_Trans1(uint8_t *data)
{
  uint8_t retVal;

  // write ID high
  MCP2515_WriteByte(MCP2515_TXB1SIDH, 0x14);
  retVal = MCP2515_ReadByte(MCP2515_TXB1SIDH);
  // write ID low
  MCP2515_WriteByte(MCP2515_TXB1SIDL, 0x60);
  retVal = MCP2515_ReadByte(MCP2515_TXB1SIDL);
  //  //write ID_ext high
  //  MCP2515_WriteByte(MCP2515_TXB1EID8,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB1EID8);
  //  //write ID_ext low
  //  MCP2515_WriteByte(MCP2515_TXB1EID0,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB1EID0);
  // write DLC
  MCP2515_WriteByte(MCP2515_TXB1DLC, 0x08);
  retVal = MCP2515_ReadByte(MCP2515_TXB1DLC);
  // write 8 byte data
  for (int i = 0; i < 8; ++i)
  {
    MCP2515_WriteByte(MCP2515_TXB1Dm + i, data[i]);
  }
  MCP2515_WriteByte(MCP2515_TXB1CTRL, 0x0B);
  retVal = MCP2515_ReadByte(MCP2515_TXB1CTRL);
//  if ((retVal >> 4) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgTXERR,6,100);
//	  return 0;
//  }
//  else if((retVal >> 5) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgMLOA,6,100);
//	  return 0;
//  }
//  else if((retVal >> 6) & 0x01)
//	{
//			HAL_UART_Transmit(&huart1,msgABTF,6,100);
//			return 0;
//	}

//  MCP2515_WriteByte(MCP2515_TXRTSCTRL, 0x02);
  MCP2515_CS_LOW();
  //  SPI_Tx(MCP2515_LOAD_TXB1D0);
  SPI_Tx(MCP2515_RTS_TX1);
  MCP2515_CS_HIGH();
  return 1;

}

uint8_t MCP2515_Trans2(uint8_t *data)
{
  uint8_t retVal;

  MCP2515_WriteByte(MCP2515_TXB2CTRL, 0x0B);
  retVal = MCP2515_ReadByte(MCP2515_TXB2CTRL);

  // write ID high
  MCP2515_WriteByte(MCP2515_TXB2SIDH, 0x14);
  retVal = MCP2515_ReadByte(MCP2515_TXB2SIDH);
  // write ID low
  MCP2515_WriteByte(MCP2515_TXB2SIDL, 0x60);
  retVal = MCP2515_ReadByte(MCP2515_TXB2SIDL);
  //  //write ID_ext high
  //  MCP2515_WriteByte(MCP2515_TXB2EID8,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB2EID8);
  //  //write ID_ext low
  //  MCP2515_WriteByte(MCP2515_TXB2EID0,0x00);
  //  retVal=MCP2515_ReadByte(MCP2515_TXB2EID0);
  // write DLC
  MCP2515_WriteByte(MCP2515_TXB2DLC, 0x08);
  retVal = MCP2515_ReadByte(MCP2515_TXB2DLC);
  // write 8 byte data
  for (int i = 0; i < 8; ++i)
  {
    MCP2515_WriteByte(MCP2515_TXB2Dm + i, data[i]);
  }

//  MCP2515_WriteByte(MCP2515_TXB0CTRL, 0x0B);
//  retVal = MCP2515_ReadByte(MCP2515_TXB0CTRL);
//  if ((retVal >> 4) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgTXERR,6,100);
//	  return 0;
//  }
//  else if((retVal >> 5) & 0x01)
//  {
//	  HAL_UART_Transmit(&huart1,msgMLOA,6,100);
//	  return 0;
//  }
//  else if((retVal >> 6) & 0x01)
//	{
//			HAL_UART_Transmit(&huart1,msgABTF,6,100);
//			return 0;
//	}
//  MCP2515_WriteByte(MCP2515_TXRTSCTRL, 0x04);
  MCP2515_CS_LOW();
    SPI_Tx(MCP2515_LOAD_TXB2D0);
  SPI_Tx(MCP2515_RTS_TX2);
  MCP2515_CS_HIGH();
  return 1;
}
