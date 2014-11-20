#ifndef __SP_DISP_H__
#define __SP_DISP_H__

#include "config.h"
#include "sp_info.h"
#include "sp_pubfunc.h"
#include "sp_communicate.h"

//#define SP_DEBUG_MODE 0
/*
* 显示类型
*/
#define SP_TP_REQUEST_CARD 0x01
#define SP_TP_USER_BALANCE 0x02
#define SP_TP_CONSUME_SUCCESS 0x03
#define SP_TP_INSUFFICIENT_BALANCE 0x04
#define SP_TP_CANCEL_CONSUME 0x05
#define SP_TP_REQUEST_CONSUME_CARD 0x06
#define SP_TP_DISP_MONEY 0x07
#define SP_TP_PASSWORD_ERROR 0x08
#define SP_TP_IS_CANCEL_CONSUME 0x09
#define SP_TP_REVENUE_QUERY 0x0A
#define SP_TP_SHOW_SYSTEM_INFO 0x0B
#define SP_TP_IS_INIT_DEVICE 0x0C//是否初始化设备界面
#define SP_TP_DISP_UNCONFIM_TRANSDTL 0x0D //显示未上传流水笔数


#ifdef SP_DEBUG_MODE
#define OUTPUT_MSG(t, fmt, ...) do{ sp_disp_msg(t, fmt, ...); }while(0)
#else
#define OUTPUT_MSG(t, fmt, ...) do{ }while(0)
#endif
/////////////////////封装汇多屏幕显示函数////////////////////////////
#define FLAG_CLEAR 0
#define FLAG_DO_NOT_CLEAR 1
#define SP_PRINT(row, col, str) do{LcdPrint(row, col, str, 0);}while(0)
#define SP_CLS(row) do{LcdCLS(row);}while(0)
#define SP_CLS_FULLSCREEN SP_CLS(0xff)



///////////////////////////////////////////////
void sp_disp_msg(uint32 nsec, const char* fmt, ...);

int sp_input_password(sp_context* ctx, char* title, char real_password[MAX_SCREEN_ROWS], uint16 max_passwd_len);

int sp_input_words(sp_context* ctx, char* title, char real_word[MAX_SCREEN_ROWS], uint16 max_word_len);

void sp_disp_by_type(uint8 disp_type, sp_context* ctx, void* arg);

void sp_wait_for_retkey(const int32 retkey, const char* func_name,
                        const int lineno, const char* fmt, ...);

void sp_disp_error(const char* fmt, ...);

void sp_disp_debug(const char* fmt, ...);

void sp_wait_seconds(int wait_sec);

void sp_print_row(uint8 flag, uint8 row, uint8 col, const char* fmt, ...);


#endif

