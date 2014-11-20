#ifndef __MENU_H__
#define __MENU_H__
#include "config.h"
#include "lcd.h"
#include "sp_info.h"

//#include "sp_pubfunc.h"
#include "sp_disp.h"

#include "sp_consume.h"
#include "sp_communicate.h"
typedef void (* menu_func_t)(void* arg);
typedef struct
{
  const char* menu_desc;//�˵�����
  menu_func_t menu_func;//�˵�����
} menu_info_t;

////////////////////�˵�������////////////////////////////
void sp_menu_consume(sp_context* ctx);

//////////////////////////////////////



#endif
