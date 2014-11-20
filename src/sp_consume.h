#ifndef __SP_CONSUME_H__
#define __SP_CONSUME_H__

#include "config.h"
#include "sp_info.h"

#include "sp_cardoper.h"
#include "sp_flash_impl.h"
#include "sp_transfer.h"
#include "sp_disp.h"

#define CONSUME_SUCCESS_BEEP 1
#define CONSUME_FAIL_BEEP 3

/*******************************************************
*** 函数名:   sp_consume_loop
*** 函数功能: 消费流程
*** 参数:  全局结构体指针
*** 返回值:  0--成功  1--失败
*********************************************************/
int sp_consume_loop(sp_context* ctx);



/*******************************************************
*** 函数名:   sp_pre_check
*** 函数功能: 消费前的检查流程
*** 参数:  全局结构体指针
*** 返回值:  0--成功  1--失败
*********************************************************/
uint8 sp_pre_check(sp_context* ctx);





/*******************************************************
*** 函数名:   sp_check_record_rom
*** 函数功能:检查流水存储区是否有足够的空间
*** 参数:  全局结构体指针
*** 返回值:  0--成功  1--失败
*********************************************************/
void sp_check_record_rom(sp_context* ctx);






#endif
