#ifndef _SP_CARDOPER_H_
#define _SP_CARDOPER_H_


//#include <assert.h>
#include "config.h"
#include "rfmod.h"
//#include "helper.h"
//#include "gnudes.h"
#include "sp_info.h"
//#include "sp_pubfunc.h"
#include "sp_disp.h"

/**
* \brief request card
*/
extern int sp_request_card(uint8 cardphyid[4]);
extern int sp_request_card_poweron(sp_card* card);

/**
* \brief - read card info, read card base information , eg . cardno,
*          feetype, last trans record
*/
	
int sp_select_card_app(sp_context* ctx);

int sp_read_card(sp_context*  ctx, sp_card* p_card, uint8 read_level);
/**
* \brief - init for purchase
* \return -  0 : success , -1 : error, 1 : short of balance
*/
int sp_payinit(sp_context* ctx, sp_card* p_card, uint32 money);
/**
* \brief - debit for payment
*/
int sp_payment(sp_context* ctx, sp_card* p_card, uint32 money);
int sp_halt(sp_context* ctx, sp_card* p_card);
/**
*判断交易是否成功

* @param  card
* @param  paycnt 判断的交易次数
* @return        返回 0 表示该交易成功 并在 card->tac 中保存 tac, 1 表示该交易不成功
*                   返回 -1 表示其它错误
*/
int sp_get_prove(sp_context* ctx, sp_card* p_card, uint16 paycnt);
int sp_set_card_loss(sp_context* ctx, sp_card* p_card);


/**
*初始化psam卡
*/
int	sp_init_sam_card(sp_context* ctx, uint8 samno[6]);
uint8  sp_rst_psam_card(uint8 *cData, uint8 *cLen);

/**
*  写卡
*/
uint8 sp_write_card(sp_card* p_card, sp_context* ctx);



#endif

