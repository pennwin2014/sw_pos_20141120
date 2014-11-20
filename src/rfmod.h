#ifndef _SP_RFMOD_H_
#define _SP_RFMOD_H_
/**
*
*   功能: 射频模块
*
**/
#include "config.h"
#include "sp_cardoper.h"
#include "sp_info.h"
#include "sp_disp.h"

typedef struct
{
  uint8 send_len;
  // 非接触卡底层指令保留值，外层不能使用
  uint8 send_dev_instruction;
  uint8 send_buf[256];
  uint8 recv_len;
  // 非接触卡底层指令保留值，外层不能使用
  uint8 recv_dev_instruction;
  uint8 recv_buf[256];
  uint16 sw;
} adpu_cmd_t;

#define ADPU_SUCCESS 0x9000
//#define ADPU_FAKE_SUCCESS 0x1601
////////////////////////////////////汇多底层函数包一层////////////////////////////////////
/*
*  读取卡片的uid
*/
extern int sp_read_uid(uint8* uid);

/*
*  复位非接cpu卡
*/
extern int sp_cpu_reset_card(void);

/*
*  取消卡片选择
*/
extern int sp_cpu_deselect_card(void);

/*
*  选择df03
*/
extern int sp_cpu_select_df03(void);


/*
*  卡片激活(含寻卡\防冲突\选卡\密码验证等操作)
*  Block_Adr :块地址
*  hKey: KEYA(0x10) 或 KEYB(0x20)认证
*  cKey:密码值数组
*/
extern uint8 sp_hl_active(uint8 Block_Adr, uint8 hKey, uint8 const *cKey);


/*
*******************************************************************************
** 函数功能 ：读块数据
** 入口参数 ：Adr:块号
** 出口参数 ：buff:读出的数据
*******************************************************************************
*/
extern uint8 sp_m1_readblock(uint8 *buff, uint8 Block_Adr);

/*
** 函数功能 ：读块数据(已含激活功能)
** 入口参数 ：cBlock : 读块数据，
: hKey 读块数据认证方式，KEYA_MODE 表示 Keya,
KEYB_MODE 表示 Keyb
: cKey 此块对应扇区的密钥
: pData 读出的块数据
: cTry 读卡失败后重复次数
*******************************************************************************
*/
extern bool sp_m1_readcard(uint8 cBlock, uint8 hKey, uint8 const *cKey, uint8 *pData, uint8 cTry);

/*
****************************************************
功能: request  card
入口: mode    工作模式 REQUEST_STD 表示标准寻卡  REQUEST_ALL表示寻所有卡
出口: 返回值0:成功，非0:失败
说明:目前REQUEST_ALL可以区分有没有放卡，暂时功能跟sp_read_uid重叠
*****************************************************
*/
extern uint8 sp_request(uint8 mode);



extern int sp_cpu_select_df03(void);


extern int sp_cpu_adpu_command(adpu_cmd_t* cmd);

extern int sp_sam_adpu_command(adpu_cmd_t* cmd);

extern int sp_cpucard_poweron(void);

extern int sp_cpucard_deselect(void);




extern int sp_cpu_reset_time(sp_context* ctx);
////////////////////////////////////////////////////////////////////////////////////




#endif // _SP_RFMOD_H_


