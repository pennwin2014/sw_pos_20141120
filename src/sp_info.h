#ifndef __SP_INFO_H
#define __SP_INFO_H
/*
**仅仅包含一些公用的头文件，实现功能有关的头文件不包含在这
*/
#include "math.h"
#include "string.h"
#include "stdio.h"
#include "config.h"
#include "M25PE16.h"
#include "sp_errdef.h"

//#define SP_DEBUG 0
//#define SP_DEBUG_MODE 0

typedef enum
{
  CPUCARD  = 0,
  M1CARD,
  SIMCARD,
  UNKNOWN_CARD
}
user_cardtype_t;
#define TRANSFLAG_UNCONFIRM 0x82
#define TRANSFLAG_LOSTCARD 0x13
/*
*  联机
*/
//正常消费上传成功
#define TRANSFLAG_OK_NO_PASSWD 0x89//普通无密码
#define TRANSFLAG_OK_PASSWD 0x8D//普通有密码
#define TRANSFLAG_OK_BOARD_NO_PASSWD 0xC9//带搭伙费无密码
#define TRANSFLAG_OK_BOARD_PASSWD 0xCD//带搭伙费有密码
#define TRANSFLAG_OK_DISCOUNT_NO_PASSWD 0xA9//带折扣无密码
#define TRANSFLAG_OK_DISCOUNT_PASSWD 0xAD//带折扣有密码
//中途拔卡
#define TRANSFLAG_HALFFWAY_NO_PASSWD 0x82//中途拔卡无密码
#define TRANSFLAG_HALFFWAY_PASSWD 0x86//中途拔卡有密码
#define TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD 0xC2//带搭伙费中途拔卡无密码
#define TRANSFLAG_HALFFWAY_BOARD_PASSWD 0xC6//带搭伙费中途拔卡有密码
#define TRANSFLAG_HALFFWAY_DISCOUNT_NO_PASSWD 0xA2//带折扣中途拔卡无密码
#define TRANSFLAG_HALFFWAY_DISCOUNT_PASSWD 0xA6//带折扣中途拔卡有密码
//黑卡消费
#define TRANSFLAG_BLACKCARD 0x10//00010000



#define CARD_SUPPORT_CPU (1<<7)//相当于二进制数10000000
#define CARD_SUPPORT_M1 (1<<6)//相当于二进制数1000000
#define CARD_SUPPORT_RFUIM (1<<5)//相当于二进制数100000




#define MAX_FLASH_WRITE_CNT 20
#define TIMEOUT_CNT 200
#define TCP_WAIT_SLEEP 10


#define MAX_MENU_SCREEN_CNT 3  //屏幕一次可以显示的菜单项目
//#define MAX_SCREEN_WORDS (16*3)
#define MAX_SCREEN_WORDS (16*4)
#define MAX_SCREEN_ROWS 16
#define SYSTEM_PASSWORD_LEN 6

//消费模式
#define SP_WORK_MODE_NORMAL 1
#define SP_WORK_MODE_FIXED_VALUE 2
//重新定义键盘按键
#define SP_KEY_0 '0'
#define SP_KEY_1 '1'
#define SP_KEY_2 '2'
#define SP_KEY_3 '3'
#define SP_KEY_4 '4'
#define SP_KEY_5 '5'
#define SP_KEY_6 '6'
#define SP_KEY_7 '7'
#define SP_KEY_8 '8'
#define SP_KEY_9 '9'
#define SP_KEY_ADD '+'//0x0b
#define SP_KEY_MINUS '-'//0x0d
#define SP_KEY_MUL '*'
#define SP_KEY_DOT '.'
#define SP_KEY_EQUAL 0x0d
#define SP_KEY_CONFIRM 0xb5
#define SP_KEY_CLEAR 0xb6
#define SP_KEY_MODE 0xb3
#define SP_KEY_FUNC 0xb4
//定义作为上翻下翻的按钮
#define SP_KEY_NEXT SP_KEY_ADD
#define SP_KEY_PREV SP_KEY_MINUS

//卡状态
#define CARD_STATUS_NORMAL 0//正常
#define CARD_STATUS_LOCK 1//锁卡
#define CARD_STATUS_FREEZE 2//冻结
#define CARD_STATUS_LOST 3 //挂失
#define CARD_STATUS_LOGOUT 4 //注销
#define CARD_STATUS_EXPIRE 5 // 过期



/////////////////////////////////////////
#define SP_PROCESS_CARD_FIRST 0x01//先放卡
#define SP_PROCESS_INPUT_FIRST 0x02//先输金额


#define SP_READ_FILE10 0x01//00000001B
#define SP_READ_FILE12 0x02//00000010B
#define SP_READ_FILE15 0x04//00000100B
#define SP_READ_FILE16 0x08//00001000B
#define SP_READ_FILE18 0x10//00010000B
#define SP_READ_FILE19 0x20//00100000B
#define SP_READ_CARDBAL 0x40//01000000B

/////////////////////////////////////////

///////////////大小/////////////////////
#define SHOW_SYSTEM_INFO_COUNT 9

#define FLASH_PAGE_SIZE 256
#define MAX_SENDMSG_LEN 512 //最大发送长度
#define MAX_DATA_LEN 203
#define TCP_RECV_BUFFER_SIZE 1024 //  接收缓冲区大小

#define SP_SWITCH_ON 1
#define SP_SWITCH_OFF 0

#define SP_TP_ADD 1
#define SP_TP_REDUCE 0

#define SAMSLOT 1
#define SAMBAND 9600
#define SP_SAM_AID ((uint16)0xDF03)

/////////////结构体/////////////////

#pragma pack(push) //保存对齐状态
#pragma pack(1)

typedef struct
{
  uint32 last_trans_no;//最后一笔流水号
  uint32 last_trans_addr;//最后一笔流水地址
  uint8 date[3];//日期yymmdd
  uint32 sum_amt;//当日累计金额
  uint8 rev[3];//保留
  uint8 crc[2];//crc
} sp_transno_unit;

typedef struct
{
  uint32 termseqno;//4pos机的记录流水号
  uint32 cardno;//4用户流水号（即交易卡号）（低位在前）
  uint16 last_cnt;// 2 上一次交易次数
  uint8 last_limit_amt[3];//上一笔的透资限额
  uint32 last_amt;//上一笔的交易金额
  uint8 last_transflag;//上一笔的交易标志
  uint8 last_termno[6];//上一笔的终端编号
  uint8 last_datetime[6];//上一笔的日期时间yymmddhhmmss，bcd码
  uint32 cardbefbal;//4交易前卡余额（低位在前）
  uint16 cardbefcnt;//2交易前卡次数（低位在前）
  uint32 amount;//4真实交易金额（低位在前），包括折扣或搭伙费金额
  uint8 extra_amt[3];//3搭伙费或折扣金额（低位在前）
  uint8 transdatetime[6];//6交易日期时间 YYMMDDHHMMSS
  uint8 samno[6];//6PSAM卡号
  uint8 tac[4];//4TAC码
  uint8 transflag;//1交易标志
  uint8 reserve[2];//2保留
  uint8 crc[2];//2CRC校验
} sp_transdtl;

typedef struct
{
  uint8 work_mode;//工作模式0：编号，1：定值，2：单价，
  uint32 amount;//消费金额
  uint8 time_gap[3];//两次消费时间间隔
  uint32 max_cardbal;//最大卡余额
  uint32 max_amount;//最大消费金额
  uint8 restart_time[8];//定时重新启动
  uint8 return_flag;//退款功能开启与关闭
  uint8 offline_flag;//脱机消费时间限制
  uint16 min_cardbal;//最小卡余额
  uint8 timeout;//超时时间
  uint8 heartgap;//心跳间隔
  uint8 limit_switch;//卡限开关
  uint8 samno[6];//sam卡号
  uint8 key_index;  //消费密钥版本号
  uint8 today_date[4];  //当日日期
  uint8 password[6];  //管理密码
  uint32 max_pay_cnt;  //卡最大使用次数
  /*
  * 本地的统计
  */
  uint8 enable_staus;//是否启用的标志或者后台返回的错误码
  uint8 init_flag;//是否初始化标志
  uint32 today_total_amt;  // 当日消费总额
  uint32 today_total_cnt;  //当日消费笔数
  uint32 yesterday_total_amt;  // 昨天消费金额
  uint32 yesterday_total_cnt;  //昨天消费笔数
  /*
      需要从服务器获取的参数
  */
  uint8 termno[4];//终端编号
  uint32 canid;//帧id，设备id
  uint8 hd_verno[5];//硬件版本
  uint8 soft_verno[3];//软件版本
  uint32 timepara_verno;//时间段参数版本号
  uint32 feepara_verno;//收费参数版本号
  uint32 syspara_verno;//主参数版本号
  uint8 blacklist_verno[7]; // 当前黑名单版本
  uint8 feetype_bitmap[32];  // 允许使用卡类别1-255种
  uint32 once_limit_amt;//单笔消费限额
  uint32 day_sum_limit_amt;//日累计消费限额
  uint8 crc[2];  //2字节校验位
} sp_syspara;

typedef struct
{
  uint16 aid;
  uint8 is_halfway; // 是否有中途拔卡
  uint8 cardphyid[8];
  char username[32];
  char stuempno[20];
  uint32 cardno;
  uint8 cardtype;
  uint8 status;
  uint8 cardverno[7];
  uint8 feetype;
  uint8 expiredate[4];
  uint32 cardbefbal;
  uint16 paycnt;
  uint32 amount;
  uint32 payamt;
  uint8 keyver;
  uint8 keyindex;
  uint8 pay_random[4];
  uint8 tac[4];
  uint32 sam_seqno;
  uint32 day_sum_limit_amt;
  uint32 once_limit_amt;
  //0x18文件
  uint16 last_trans_seqno;//ED或EP联机或脱机交易序号
  uint32 last_overdraft_limit;//上一笔的透资限额
  uint32 last_trans_amt;//交易金额
  uint8 last_trans_flag;//交易类型标志
  uint8 last_samno[6];//终端机编号
  uint8 last_datetime[7];//上一笔交易日期4+上一笔交易时间3
  //0x10文件
  uint32 day_sum_amt;//日累计交易金额
  /*
  uint8 last_trans_dev[6];
  uint8 last_trans_date[4];
  uint8 last_trans_time[3];
  uint8 last_trans_amt[4];
  uint8 last_trans_cnt[2];
  uint8 last_tac[4];
  */
  uint8 use_passwd;
  char passwd[16];//卡密码
  // m1 card package block data
  uint8 m1_package_block[16];
} sp_card;

typedef struct
{
  uint32 base_amt;
  uint16 consume_interval;
  uint8 offline_day;
  uint8 blacklist_index;
  uint8 record_index;
  uint8 m1_alg;
  uint8 card_support;
  uint8 rfu[51];
  uint8 crc;
} sp_param_t;

typedef struct
{
  uint32 feepara_verno;
  uint32 timepara_verno;
  uint8 blacklist_verno[6];
  uint32 syspara_verno;
  uint16 download_index;
  uint8 feetype_bitmap[32];
} sp_svrpara;

typedef struct
{
  uint8 period[4];
  uint8 use_times;
  uint8 forbidden_bitmap[32];
} sp_timepara;

typedef struct
{
  uint8 crc1, crc2;
  uint8 recv_data[20];
  uint32 recv_len;
  uint32 disp_cnt;
  uint8 disp_type;
  uint8 system_enable_flag;
  // 大学名称标题
  char school_name[17];
  //是否在线
  bool online_flag;
  //是否是第一次reset cpu卡
  uint8 reset_cpucard_count;
  // 终端物理ID
  uint8 devphyid[4];
  uint16 sam_aid;
  // 当前消费统计日期
  uint8 today[4];
  // 当前时间
  uint8 current_datetime[7];//yy-yy-mm-dd-hh-mi-ss
  // 卡密钥
  uint8 card_key[16];
  // 未上传流水最小日期
  uint8 offline_min_day[4];
  // 未上传流水笔数
  uint32 offline_rec_amt;
  // 当前流水 flash 是否已经存满
  uint8 rec_flash_full;
  //当前交易流水号
  uint32 transno;
  // 基础参数
  sp_param_t param;
  // 黑名单启用标志
  uint16 black_enable_flag;
  uint8 user_code[6];
  uint8 user_index;
  uint8 sam_ok;
  uint32 collect_record_ptr;
  // 当前流水
  sp_transdtl record;
  //异常流水
  sp_transdtl exception_record;
  // 当前卡
  sp_card card;
  //下载等时候暂存的服务器上的值
  sp_svrpara svrpara;
  //系统参数
  sp_syspara syspara;
  // 费率表
  uint8 feerate_table[256];
  //时间段参数版本号
  sp_timepara timepara_table[4];
} sp_context;

////////////////////////////////定义网络通讯结构体////////////////////////////////////////////
typedef struct
{
  uint8 guide_code[3];//引导码
  uint8 cmd_code;//命令码
  uint16 data_len;//数据长度
  uint8 deviceno[2];//机器地址
  uint32 seqno;
} sp_tcp_header;

typedef struct
{
  sp_tcp_header header;
  uint8 data[MAX_DATA_LEN];
  uint8 check_sum;
} sp_tcp_trans_unit;

//签到接收
typedef struct
{
  uint8 retcode;
  //商户号
  uint8 shopno[4];
  //M1分散密钥
  uint8 m1_key[8];
  //M1卡钱包号
  uint8 m1_bagno;
  //时间
  uint8 auth_time[6];
  //黑名单版本
  uint8 blacklist_verno[6];
  //CPU卡密钥引索参数
  uint8 cpu_key_para;
} sp_tcp_auth_recv;
#pragma pack(pop)//恢复对齐状态



#endif
