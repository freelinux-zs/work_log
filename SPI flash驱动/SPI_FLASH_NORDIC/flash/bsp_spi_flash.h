#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include <stdint.h>
#include <string.h>

#define sFLASH_ID 													  0xc22015  //MX25L1606E
#define SPI_FLASH_PageSize										256
#define SPI_FLASH_PerWritePageSize						256

/*命令定义*/
#define    FLASH_CMD_RDID      0x9F    //RDID (Read Identification)
#define    FLASH_CMD_RES       0xAB    //RES (Read Electronic ID)
#define    FLASH_CMD_REMS      0x90    //REMS (Read Electronic & Device ID)

//Register comands
#define    FLASH_CMD_WRSR      0x01    //WRSR (Write Status Register)
#define    FLASH_CMD_RDSR      0x05    //RDSR (Read Status Register)
#define    FLASH_CMD_WRSCUR    0x2F    //WRSCUR (Write Security Register)
#define    FLASH_CMD_RDSCUR    0x2B    //RDSCUR (Read Security Register)

//READ comands
#define    FLASH_CMD_READ        0x03    //READ (1 x I/O)
#define    FLASH_CMD_FASTREAD    0x0B    //FAST READ (Fast read data)
#define    FLASH_CMD_DREAD       0x3B    //DREAD (1In/2 Out fast read)
#define    FLASH_CMD_RDSFDP      0x5A    //RDSFDP (Read SFDP)

//Program comands
#define    FLASH_CMD_WREN     0x06    //WREN (Write Enable)
#define    FLASH_CMD_WRDI     0x04    //WRDI (Write Disable)
#define    FLASH_CMD_PP       0x02    //PP (page program)

//Erase comands
#define    FLASH_CMD_SE       0x20    //SE (Sector Erase)
#define    FLASH_CMD_BE       0xD8    //BE (Block Erase)
#define    FLASH_CMD_CE       0x60    //CE (Chip Erase) hex code: 60 or C7

//Mode setting comands
#define    FLASH_CMD_DP       0xB9    //DP (Deep Power Down)
#define    FLASH_CMD_RDP      0xAB    //RDP (Release form Deep Power Down)
#define    FLASH_CMD_ENSO     0xB1    //ENSO (Enter Secured OTP)
#define    FLASH_CMD_EXSO     0xC1    //EXSO  (Exit Secured OTP)


// variable
#define    TRUE     1
#define    FALSE    0
#define    BYTE_LEN          8
#define    IO_MASK           0x80
#define    HALF_WORD_MASK    0x0000ffff

#define    FlashID          0xc22015
#define    ElectronicID     0x14
#define    RESID0           0xc214
#define    RESID1           0x14c2
#define    FlashSize        0x200000       // 2 MB
#define    CE_period        10416667       // tCE /  ( CLK_PERIOD * Min_Cycle_Per_Inst *One_Loop_Inst)
#define    tW               40000000       // 40ms
#define    tDP              10000          // 10us
#define    tBP              50000          // 50us
#define    tPP              3000000        // 3ms
#define    tSE              200000000      // 200ms
#define    tBE              2000000000     // 2s
#define    tPUW             10000000       // 10ms
#define    tWSR             tBP

#define    SIO              0
#define    DIO              1

/*
  Flash Related Parameter Define
*/

#define    Block_Offset       0x10000     // 64K Block size
#define    Block32K_Offset    0x8000      // 32K Block size
#define    Sector_Offset      0x1000      // 4K Sector size
#define    Page_Offset        0x0100      // 256 Byte Page size
#define    Page32_Offset      0x0020      // 32 Byte Page size (some products have smaller page size)
#define    Block_Num          (FlashSize / Block_Offset)

// Flash control register mask define
// status register
#define    FLASH_WIP_MASK         0x01
#define    FLASH_LDSO_MASK        0x02
#define    FLASH_QE_MASK          0x40
// security register
#define    FLASH_OTPLOCK_MASK     0x03
#define    FLASH_4BYTE_MASK       0x04
#define    FLASH_WPSEL_MASK       0x80
// configuration reigster
#define    FLASH_DC_MASK          0x80
#define    FLASH_DC_2BIT_MASK     0xC0
// other
#define    BLOCK_PROTECT_MASK     0xff
#define    BLOCK_LOCK_MASK        0x01


/*SPI接口定义*/
#define FLASH_SPI_SFSWP  										13
#define FLASH_SPI_SFHOLD  									14
#define FLASH_SPI_CS_PIN   									4
#define FLASH_EN_PIN  											30
#define FLASH_SPI_MOSI_PIN									11
#define FLASH_SPI_MISO_PIN                  12
#define FLASH_SPI_SCLK_PIN									5

#define  SPI_MOSI_H    nrf_gpio_pin_set(FLASH_SPI_MOSI_PIN)
#define  SPI_MOSI_L    nrf_gpio_pin_clear(FLASH_SPI_MOSI_PIN)

#define  SPI_MISO_V		 nrf_gpio_pin_read(FLASH_SPI_MISO_PIN)
#define  SPI_MISO_H		 nrf_gpio_pin_set(FLASH_SPI_MISO_PIN)

#define  SPI_SCLK_H		 nrf_gpio_pin_set(FLASH_SPI_SCLK_PIN)
#define  SPI_SCLK_L		 nrf_gpio_pin_clear(FLASH_SPI_SCLK_PIN)

#define  SPI_CS_H			 nrf_gpio_pin_set(FLASH_SPI_CS_PIN)	
#define  SPI_CS_L			 nrf_gpio_pin_clear(FLASH_SPI_CS_PIN)

// Return Message
typedef enum {
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid
}ReturnMsg;

// Flash status structure define
struct sFlashStatus{
    /* Mode Register:
     * Bit  Description
     * -------------------------
     *  7   RYBY enable
     *  6   Reserved
     *  5   Reserved
     *  4   Reserved
     *  3   Reserved
     *  2   Reserved
     *  1   Parallel mode enable
     *  0   QPI mode enable
    */
    uint8_t    ModeReg;
    _Bool    ArrangeOpt;
};

typedef struct sFlashStatus FlashStatus;

#define    CLK_PERIOD             20     // unit: ns
#define    Min_Cycle_Per_Inst     12     // use 12T 8051
#define    One_Loop_Inst          8      // instruction count of one loop (estimate)

// Flash information define
#define    WriteStatusRegCycleTime     tW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    PageProgramCycleTime        tPP / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    SectorEraseCycleTime        tSE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    BlockEraseCycleTime         tBE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    ChipEraseCycleTime          CE_period
#define    FlashFullAccessTime         tPUW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)


void SPI_FLASH_Init(void);
ReturnMsg SPI_FLASH_SectorErase(uint32_t SectorAddr);
void SPI_FLASH_BulkErase(void);
ReturnMsg SPI_FLASH_PageWrite( uint32_t flash_address, uint8_t *source_address, uint32_t byte_length);
void SPI_FLASH_BufferWrite(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
ReturnMsg SPI_FLASH_BufferRead(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI_FLASH_ReadID(void);
void  SPI_FLASH_ReadDeviceID(uint8_t *ElectricIdentification);
void InsertDummyCycle( uint8_t dummy_cycle );
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr);
void SPI_Flash_PowerDown(void);
void SPI_Flash_WAKEUP(void);

uint8_t SPI_FLASH_ReadByte(void);
void SPI_FLASH_SendByte(uint8_t byte);
uint16_t SPI_FLASH_SendHalfWord(uint16_t HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);




#endif /*__SPI_FLASH_H*/



