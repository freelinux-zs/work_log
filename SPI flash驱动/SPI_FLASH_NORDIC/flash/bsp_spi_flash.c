#include "bsp_spi_flash.h"
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#if 1
void SPI_FLASH_Init(void)
{
	nrf_gpio_cfg_output(FLASH_SPI_SFSWP);
	nrf_gpio_cfg_output(FLASH_SPI_SFHOLD);
	nrf_gpio_cfg_output(FLASH_SPI_CS_PIN);
	
	nrf_gpio_pin_set(FLASH_SPI_SFSWP);
	nrf_gpio_pin_set(FLASH_SPI_SFHOLD);
	nrf_gpio_pin_set(FLASH_SPI_CS_PIN);
	
	nrf_gpio_cfg_output(FLASH_SPI_MOSI_PIN);
	nrf_gpio_cfg_input(FLASH_SPI_MISO_PIN,NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(FLASH_SPI_SCLK_PIN);
	
	nrf_gpio_pin_clear(FLASH_SPI_MOSI_PIN);
	nrf_gpio_pin_set(FLASH_SPI_SCLK_PIN);
	
	// Wait flash warm-up
	nrf_delay_ms(10);
}


static void InsertDummyCycle( uint8_t dummy_cycle )
{

    uint8_t i;
    for( i=0; i < dummy_cycle; i=i+1 )
    {
				SPI_SCLK_L;
        SPI_SCLK_H;
    }
}

ReturnMsg CMD_RDSR( uint8_t *StatusReg )
{
    uint8_t  gDataBuffer;

    // Chip select go low to start a flash command
    SPI_CS_L;

    // Send command
    SPI_FLASH_SendByte(FLASH_CMD_RDSR );
    gDataBuffer = SPI_FLASH_ReadByte();

    // Chip select go high to end a flash command
    SPI_CS_H;

    *StatusReg = gDataBuffer;

    return FlashOperationSuccess;
}

ReturnMsg CMD_RDSCUR( uint8_t *SecurityReg )
{
    uint8_t  gDataBuffer;

    // Chip select go low to start a flash command
    SPI_CS_L;

    //Send command
    SPI_FLASH_SendByte( FLASH_CMD_RDSCUR );
    gDataBuffer = SPI_FLASH_ReadByte();

    // Chip select go high to end a flash command
    SPI_CS_H;

    *SecurityReg = gDataBuffer;

    return FlashOperationSuccess;

}

ReturnMsg CMD_WREN( void )
{
    // Chip select go low to start a flash command
    SPI_CS_L;

    // Write Enable command = 0x06, Setting Write Enable Latch Bit
    SPI_FLASH_SendByte( FLASH_CMD_WREN);

    // Chip select go high to end a flash command
    SPI_CS_H;

    return FlashOperationSuccess;
}

void SendFlashAddr( uint32_t flash_address, uint8_t io_mode, _Bool addr_4byte_mode )
{
    /* Check flash is 3-byte or 4-byte mode.
       4-byte mode: Send 4-byte address (A31-A0)
       3-byte mode: Send 3-byte address (A23-A0) */
    if( addr_4byte_mode == TRUE ){
        SPI_FLASH_SendByte( (flash_address >> 24) ); // A31-A24
    }
    /* A23-A0 */
    SPI_FLASH_SendByte( (flash_address >> 16) );
    SPI_FLASH_SendByte( (flash_address >> 8) );
    SPI_FLASH_SendByte( (flash_address));
}


_Bool IsFlashBusy( void )
{
    uint8_t  gDataBuffer;

		CMD_RDSR( &gDataBuffer );
    if( (gDataBuffer & FLASH_WIP_MASK)  == FLASH_WIP_MASK )
        return TRUE;
    else
        return FALSE;
}

_Bool WaitFlashReady( uint32_t ExpectTime )
{
#ifndef NON_SYNCHRONOUS_IO
    uint32_t temp = 0;
    while( IsFlashBusy() )
    {
        if( temp > ExpectTime )
        {
            return FALSE;
        }
        temp = temp + 1;
    }
       return TRUE;
#else
    return TRUE;
#endif
}

_Bool IsFlash4Byte( void )
{
#ifdef FLASH_CMD_RDSCUR
    #ifdef FLASH_4BYTE_ONLY
        return TRUE;
    #elif FLASH_3BYTE_ONLY
        return FALSE;
    #else
        uint8_t  gDataBuffer;
        CMD_RDSCUR( &gDataBuffer );
        if( (gDataBuffer & FLASH_4BYTE_MASK) == FLASH_4BYTE_MASK )
            return TRUE;
        else
            return FALSE;
    #endif
#else
    return FALSE;
#endif
}

/*擦除扇区*/
ReturnMsg SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
		uint8_t  addr_4byte_mode;

    // Check flash address
    if( SectorAddr > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;

    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    SPI_CS_L;

    //Write Sector Erase command = 0x20;
    SPI_FLASH_SendByte( FLASH_CMD_SE );
    SendFlashAddr( SectorAddr, SIO, addr_4byte_mode );

    // Chip select go high to end a flash command
    SPI_CS_H;

    if( WaitFlashReady( SectorEraseCycleTime ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*擦除这个flash*/
//void SPI_FLASH_BulkErase(void)
//{


//}

/*对flash按页写入，需要先擦除再写入*/
ReturnMsg SPI_FLASH_PageWrite( uint32_t flash_address, uint8_t *source_address, uint32_t byte_length)
{
		uint32_t index;
    uint8_t  addr_4byte_mode;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;

    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    SPI_CS_L;

    // Write Page Program command
    SPI_FLASH_SendByte( FLASH_CMD_PP );
    SendFlashAddr( flash_address, SIO, addr_4byte_mode );

    // Set a loop to down load whole page data into flash's buffer
    // Note: only last 256 byte ( or 32 byte ) will be programmed
    for( index=0; index < byte_length; index++ )
    {
        SPI_FLASH_SendByte( *(source_address + index) );
    }

    // Chip select go high to end a flash command
    SPI_CS_H;

    if( WaitFlashReady( PageProgramCycleTime ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

void SPI_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	
	Addr = WriteAddr % SPI_FLASH_PageSize;
	
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage = NumByteToWrite  / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
	
	if(Addr == 0)
	{
		if(NumOfPage == 0){
			SPI_FLASH_PageWrite(WriteAddr, pBuffer, NumByteToWrite);
		}
		else
		{
			while(NumOfPage--)
			{
				SPI_FLASH_PageWrite(WriteAddr,pBuffer,SPI_FLASH_PageSize);
				WriteAddr += SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			SPI_FLASH_PageWrite(WriteAddr, pBuffer, NumOfSingle);
		}
	}
	else
	{
		if(NumOfPage == 0)
		{
			if(NumOfSingle > count)
			{
				temp = NumOfSingle - count;
				SPI_FLASH_PageWrite(WriteAddr, pBuffer, count);
				WriteAddr += count;
				pBuffer += count;
				SPI_FLASH_PageWrite(WriteAddr, pBuffer,temp);
			}else{
				SPI_FLASH_PageWrite(WriteAddr, pBuffer, NumByteToWrite);
			}
		}
		else
			{
				NumByteToWrite -= count;
				NumOfPage = NumByteToWrite / SPI_FLASH_PageSize;
				NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;
				
				SPI_FLASH_PageWrite(WriteAddr,  pBuffer, count);
				
				WriteAddr += count;
				pBuffer += count;
				
				while(NumOfPage--)
				{
					SPI_FLASH_PageWrite( WriteAddr,  pBuffer, SPI_FLASH_PageSize);
					WriteAddr += SPI_FLASH_PageSize;
					pBuffer += SPI_FLASH_PageSize;
				}
				
				if(NumOfSingle != 0)
				{
					SPI_FLASH_PageWrite(WriteAddr, pBuffer,  NumOfSingle);
				}
		}
	}
}

/*读取flash数据*/
ReturnMsg SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	uint32_t index;
    uint8_t  addr_4byte_mode;

    // Check flash address
    if( ReadAddr > FlashSize ) return FlashAddressInvalid;

    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Chip select go low to start a flash command
    SPI_CS_L;

    // Write READ command and address
    SPI_FLASH_SendByte( FLASH_CMD_READ );
    SendFlashAddr( ReadAddr, SIO, addr_4byte_mode );

    // Set a loop to read data into buffer
    for( index=0; index < NumByteToRead; index++ )
    {
        // Read data one byte at a time
        *(pBuffer + index) = SPI_FLASH_ReadByte();
    }

    // Chip select go high to end a flash command
    SPI_CS_H;

    return FlashOperationSuccess;

}


uint32_t SPI_FLASH_ReadID(void)
{
	  uint32_t temp;
    uint8_t  gDataBuffer[3];

    // Chip select go low to start a flash command
    SPI_CS_L;
    // Send command
    SPI_FLASH_SendByte(FLASH_CMD_RDID);

    // Get manufacturer identification, device identification
    gDataBuffer[0] = SPI_FLASH_ReadByte();
    gDataBuffer[1] = SPI_FLASH_ReadByte();
    gDataBuffer[2] = SPI_FLASH_ReadByte();

    // Chip select go high to end a command
    SPI_CS_H;

    // Store identification
    temp =  gDataBuffer[0];
    temp =  (temp << 8) | gDataBuffer[1];
    return ((temp << 8) | gDataBuffer[2]);
}	


void SPI_FLASH_ReadDeviceID(uint8_t *ElectricIdentification)
{
    SPI_CS_L;

    // Send flash command and insert dummy cycle
    SPI_FLASH_SendByte(FLASH_CMD_RES);
    InsertDummyCycle(24);

    // Get electric identification
    *ElectricIdentification = SPI_FLASH_ReadByte();

    // Chip select go high to end a flash command
    SPI_CS_H;
}	



uint8_t SPI_FLASH_ReadByte(void)
{
	uint8_t n ,dat;
	
	for(n=0;n<8;n++)
	{
		SPI_SCLK_L;
		dat<<=1;
		if(SPI_MISO_V)dat|=0x01;
		else dat &= 0xfe;
		SPI_SCLK_H;
	}
	
	SPI_SCLK_L;
	return dat;
}

void SPI_FLASH_SendByte(uint8_t byte_value)
{
		unsigned char n;
		for(n = 0; n < 8;n++)
		{
		SPI_SCLK_L;
		if(byte_value &0x80)SPI_MOSI_H;
		else SPI_MOSI_L;
		byte_value<<=1;
		SPI_SCLK_H;
		}
		SPI_SCLK_L;

}


void SPI_Flash_PowerDown(void)
{
	SPI_CS_L;
	SPI_FLASH_SendByte(FLASH_CMD_DP);
	SPI_CS_H;
}

void SPI_Flash_WAKEUP(void)
{
	SPI_CS_L;
	SPI_FLASH_SendByte(FLASH_CMD_RDP);
	SPI_CS_H;
}
#else

uint32_t SPI_FLASH_ReadID(void)
{
	return 0;
}

ReturnMsg SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
	return 0;
}

void SPI_FLASH_Init(void)
{

}

ReturnMsg SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	return 0;
}

void SPI_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{

}

void SPI_Flash_WAKEUP(void)
{

}
#endif






