#ifndef __TRANSFER_H 
#define __TRANSFER_H

#include "sp_communicate.h"



/*
*  ҵ�����
*/
int sp_auth(sp_context* ctx);

int sp_send_rt_transdtl(sp_context* ctx, sp_transdtl* ptransdtl);

int sp_send_heartbeat(sp_context* ctx);

int sp_send_test(sp_context* ctx);

int sp_enable_device(sp_context* ctx);

int sp_check_deviceid(sp_context* ctx);

/*
*  �������
*/
int sp_download_feerate(sp_context* ctx);

int sp_download_syspara(sp_context* ctx);

int sp_download_blacklist(sp_context* ctx);

int sp_download_timepara(sp_context* ctx);



/*
*  ����
*/
uint8 sp_recover_system(sp_context* ctx);







#endif

