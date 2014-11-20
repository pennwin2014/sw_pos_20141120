#include "sp_flash_store.h"




/////////////////////////////////////封装汇多提供的接口/////////////////////////////////////////////////////
uint8 sp_SF_Read(uint32 page_no, uint32 offset_addr, uint8 *array, uint32 counter)
{
  return (SF_Read(page_no, offset_addr, array, counter) == 1) ? 0 : 1;
}
//uIsErase 0---直接写，1---擦除后再写
uint8 sp_SF_Write(uint32 page_no, uint32 offset_addr, uint8  *array, uint32 counter, uint8 uIsErase)
{
  return (SF_Write(page_no, offset_addr, array, counter, uIsErase) == 1) ? 0 : 1;
}

uint8 sp_SF_ErasePage(uint32 page_no)
{
  return (SF_ErasePage(page_no) == 1) ? 0 : 1;
}











