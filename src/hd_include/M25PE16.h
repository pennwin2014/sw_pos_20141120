#ifndef __M25P16_H__
#define __M25P16_H__

typedef unsigned char  ST_uint8; /* All HW dependent Basic Data Types */       
typedef          char  ST_sint8;   
typedef unsigned short ST_uint16;       
typedef          short ST_sint16;       
typedef unsigned long  ST_uint32;       
typedef          long  ST_sint32;  


/******** SectorType ********/  
 
typedef ST_uint16 uSectorType;  
  
/******** PageType ********/  
 
typedef ST_uint16 uPageType;  
  
/******** AddrType ********/  
 
typedef ST_uint32 uAddrType;  

/******************************************************************************* 
Flash Status Register Definitions (see Datasheet) 
*******************************************************************************/ 
enum 
{ 
	SPI_FLASH_WIP	= 0x01,				// write/program/erase in progress indicator 
	SPI_FLASH_WEL	= 0x02				// write enable latch 
}; 
 


typedef enum {  
    Flash_AddressInvalid,  
    Flash_MemoryOverflow,  
    Flash_PageEraseFailed,  
    Flash_PageNrInvalid,  
    Flash_SectorNrInvalid, 
    Flash_FunctionNotSupported, 
    Flash_NoInformationAvailable, 
    Flash_OperationOngoing,  
    Flash_OperationTimeOut,  
    Flash_ProgramFailed,  
    Flash_WrongType, 
    Flash_Success 
} ReturnType;  



/******** InstructionCode ********/  
enum 
{ 
	//instruction set 
	SPI_FLASH_INS_WREN      = 0x06,		// Write Enable 
	SPI_FLASH_INS_WRDI      = 0x04,		// Write Disable 
	SPI_FLASH_INS_RDSR	    = 0x05,		// Read Status Register 
	SPI_FLASH_INS_WRSR      = 0x01,		// write status register 
	SPI_FLASH_INS_READ	    = 0x03,		// Read Data Bytes 
	SPI_FLASH_INS_FAST_READ = 0x0B,		// Read Data Bytes at Higher Speed 
	SPI_FLASH_INS_PP	    = 0x02,		// Page Program 
	SPI_FLASH_INS_SE	    = 0xD8,		// Sector Erase 
	SPI_FLASH_INS_SSE	    = 0x20,		// SubSector Erase 
	SPI_FLASH_INS_BE        = 0xC7,     // Bulk Erase 
	SPI_FLASH_INS_DP	    = 0xB9,		// Deep Power-down 
	SPI_FLASH_INS_RDID	    = 0x9F,		// Read Identification 
	SPI_FLASH_INS_PW	    = 0x0A,		// Page Write 
	SPI_FLASH_INS_PE	    = 0xDB,		// Page Erase  
	SPI_FLASH_INS_WRLR      = 0xE5,     // Write to Lock Register	 
	SPI_FLASH_INS_RDLR      = 0xE8,     // Read Lock Register 
	SPI_FLASH_INS_RDP	    = 0xAB,		// Release from Deep Power-down 
	SPI_FLASH_INS_DUMMY	    = 0xAA		// dummy byte to check Deep Power-down. 
}; 


  //extern ReturnType  Flash( InstructionType insInstruction, ParameterType *fp ); 
  extern ReturnType  FlashWriteEnable( void ); 
  extern ReturnType  FlashWriteDisable( void ); 
  extern ReturnType  FlashReadDeviceIdentification( ST_uint16 *uwpDeviceIdentification); 
  extern ReturnType  FlashReadManufacturerIdentification( ST_uint8 *ucpManufacturerIdentification); 
  extern ReturnType  FlashReadStatusRegister( ST_uint8 *ucpStatusRegister); 
  extern ReturnType  FlashRead( uAddrType udAddr, ST_uint8 *ucpElements, ST_uint32 udNrOfElementsToRead); 
  extern ReturnType  FlashFastRead( uAddrType udAddr, ST_uint8 *ucpElements, ST_uint32 udNrOfElementsToRead); 

  extern ReturnType  FlashPageWrite( uAddrType udAddr, ST_uint8 *pArray , ST_uint16 udNrOfElementsInArray); 
  extern ReturnType  FlashPageProgram( uAddrType udAddr, ST_uint8 *pArray, ST_uint16 udNrOfElementsInArray ); 
  extern ReturnType  FlashPageErase( uPageType upgPageNr ); 
  extern ReturnType  FlashSectorErase( uSectorType uscSectorNr ); 
  extern ReturnType  FlashBulkErase( void ); 
  extern ReturnType  FlashDeepPowerDown( void ); 
  extern ReturnType  FlashReleaseFromDeepPowerDown( void ); 
  extern ReturnType  FlashWrite( ST_uint32 udAddr, ST_uint8 *pArray, ST_uint32 udNrOfElementsInArray ); 
  extern ReturnType  FlashProgram( ST_uint32 udAddr, ST_uint8 *pArray , ST_uint32 udNrOfElementsInArray); 
  extern ReturnType  FlashReadLockRegister( ST_uint32 udAddr, ST_uint8 *ucLockRegister ); 
  extern ReturnType  FlashWriteLockRegister( ST_uint32 udAddr, ST_uint8 ucLockRegister ); 



	// FLASH特性字
	#define AT45DB041B_MAXBLOCK		256	    	// AT45DB041B共256块(8页/块)
	#define AT45DB041B_MAXPAGE		2048	// AT45DB041B共2048页
	//#define AT45DB041B_PAGESIZE	    	264		// 每页264Bytes
	#define AT45DB041B_PAGESIZE	    	256		// 每页264Bytes

	// FLASH特性字
	#define AT45DB161B_MAXBLOCK		512	    	// AT45DB041B共256块(8页/块)
	#define AT45DB161B_MAXPAGE		4096		// AT45DB041B共2048页
	#define AT45DB161B_PAGESIZE	    	528		// 每页264Bytes
	//函数
	extern void  SF_Init(void);
	extern uint8 SF_Read(uint16 page_addr, uint16 in_page_byte_addr, uint8 *array, uint16 counter);  
	extern uint8 SF_Write(uint16 page_addr, uint16 in_page_begin_addr, uint8  *array, uint16 counter,uint8 uIsErase);
	extern uint8  SF_ErasePage(uint16 page_addr);
  	extern bool SF_ReadContinuous(uint32 lAddr,uint8 *array, uint16 counter);// 以线性地址读数据(可跨页)
	ReturnType FlashTimeOut(ST_uint32 udSeconds) ;
	uint8 IsFlashBusy(void)  ;

#endif

