#ifndef __SP_FLASH_IMPL_H__
#define __SP_FLASH_IMPL_H__

#include "sp_info.h"
#include "sp_flash_store.h"

#define TRANSNO_FLAG_MASTER true
#define TRANSNO_FLAG_SLAVE false
//////////////��ַ///////////////////
#define ADDR_BLACKLIST 0x00000
#define ADDR_MASTER_TRANS_SEQNO 0x40000
#define ADDR_SLAVE_TRANS_SEQNO 0x40100
#define ADDR_SYSINFO 0x40200 //һҳ256B��Ż�����Ϣ
#define ADDR_FEERATE 0x40300 //һҳ256b��ŷ��ʱ�
#define ADDR_TIMEPARA 0x40400 //һҳ256b���ʱ��β����汾��
#define ADDR_TRUSS_ACCOUNT 0x40C00 //��ҳ256*4�������ʵ�1-4
#define ADDR_TRUSS_TRANSDTL 0x41000 //������ˮ�洢��
#define ADDR_TRANS_DATA 0x46900
#define ADDR_TRANS_LAST 0xE2D00
#define ADDR_HEX_FILE 0x10000//��������������ļ���ַ
///////ϵͳ����/////
#define OFFSET_WORK_MODE 0   //1byte,����ģʽ0����ţ�1����ֵ��2�����ۣ�
#define OFFSET_CONSUME_AMT 1  //4byte,���ѽ�� ���ڶ�ֵģʽ����Ч
#define OFFSET_CONSUME_GAP_SECONDS 5 //3byte,�������Ѽ����,0Ϊ�����ƣ� ��λ��ǰ
#define OFFSET_MAX_CARD_BAL 8 // 4byte, ������
#define OFFSET_MAX_CONSUME_AMT 12 // 4byte, ������ѽ� 0Ϊ�����ƣ���λ��ǰ
#define OFFSET_RESTART_TIME 16 // 8byte,��ʱ����������4�Σ�ÿ��ʱ��������ֽڣ���
//FF��ʾ������
//ʱ��ΪHEX��ʽ���磺13��30 ��ʾΪ 0x0D1
#define OFFSET_RETURN_FLAG 24 // 1byte,;//�˿�ܿ�����ر�
#define OFFSET_OFFLINE_FLAG 25 // 1byte,;//�ѻ�����ʱ������
#define OFFSET_MIN_CARD_BAL 26 // 2byte, //��С�����
#define OFFSET_TIMEOUT 28 // 1byte ����ʱʱ��
#define OFFSET_HEART_GAP 29 // 1byte,�������
#define OFFSET_SINGLE_CONSUME_LIMIT 30 // 4byte,���������޶�
#define OFFSET_DAY_SUM_COMSUME_LIMIT 34 // 4byte, /���ۼ������޶�
#define OFFSET_CARD_LIMIT_FLAG 38 // 1byte ,/���޿���
#define OFFSET_TERMNO 39 // 4byte,�����豸��
#define OFFSET_HD_VERSION 43 // 5byte, Ӳ���汾��
#define OFFSET_SYSTEM_CAPACITY 48 // 4byte, ϵͳ����
#define OFFSET_ADDR_SAMNO 52 //6byte, SAM����
#define OFFSET_ADDR_KEY_INDEX 58//1byte, ������Կ�汾��


/*
*  ��ˮ���
*/
uint8 sp_write_transno_unit(bool flag, sp_transno_unit* punit);
uint8 sp_read_transno_unit(bool flag, sp_transno_unit* punit);
uint32 sp_get_transno(void);
int32 sp_get_last_transaddr(void);
uint16 sp_get_transno_lastaddr(uint16 pageno);
uint16 sp_get_offline_transdtl_amout(sp_context* ctx);
//��д��ˮ
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr);
uint8 sp_write_transdtl(sp_transdtl* ptransdtl);
uint8 sp_update_left_transdtl_info(uint32 page_addr, sp_transdtl* ptransdtl);


/*
*��д������
*/
uint8 sp_read_blacklist(sp_context* ctx);
uint8 sp_write_blacklist(sp_context* ctx);


/*
*��дϵͳ��Ϣ
*/
uint8 sp_write_syspara(sp_context* ctx);
uint8 sp_read_syspara(sp_context* ctx);


/*
*��д���ʱ�
*/
uint8 sp_write_feerate_table(sp_context* ctx);
uint8 sp_read_feerate_table(sp_context* ctx);

/*
*��дʱ��β�����
*/
uint8 sp_write_timepara_table(sp_context* ctx);
uint8 sp_read_timepara_table(sp_context* ctx);

/*
*  ����:��鲢��ȡϵͳ����
*/
uint8 sp_read_system(sp_context* ctx);

/*
*  ����:�ָ���������
*  1������һЩĬ��ֵ
*  2����ǿ��Ҫ���û������ն˱��
*/
uint8 sp_recover_device(sp_context* ctx);


/*
*����
*/
void sp_disp_flash(int32 addr, int32 len);
int sp_check_date_change(sp_context* ctx);

#endif


