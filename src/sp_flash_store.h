#ifndef __FLASH_STORE_H__
#define __FLASH_STORE_H__
#include "sp_info.h"
//#include "sp_pubfunc.h"
#include "sp_disp.h"





/////////////////////////////��װ����ṩ�Ľӿ�////////////////////////////////////
uint8 sp_SF_Read(uint32 page_no, uint32 offset_addr, uint8 *array, uint32 counter);

/**************************************************************
//����uIsErase:
   1-----�Ȳ���ԭ����������д���϶�����д��
   0-----ֱ��д���п����еĵط�д����
//����ֵ:1��ʾ�ɹ�
****************************************************************/
uint8 sp_SF_Write(uint32 page_no, uint32 offset_addr, uint8  *array, uint32 counter, uint8 uIsErase);

uint8 sp_SF_ErasePage(uint32 page_no);







#endif
