#ifndef __SP_ERRDEF_H__
#define __SP_ERRDEF_H__


#define COMMOM_ERROR "普通错误"
#define TYPE_ERROR "输入错误"
#define LEN_ERROR "长度错误"

/*
*  E--错误
*  R--返回
*  S--后台返回
*/
#define SP_SUCCESS 0x00
#define SP_FAIL 0x01
#define SP_E_FLASH_NOT_FOUNT 0x02 //flash未找到
#define SP_E_FLASH_READ 0x03 //flash读取失败
#define SP_E_FLASH_CRC 0x04 //flash校验失败
#define SP_E_FLASH_WRITE 0x05 //flash写失败
#define SP_E_FLASH_SEQNO_NOT_EQUAL 0x06 //主从流水号不一致
#define SP_E_FLASH_ERASE 0x07  //flash擦除失败
#define SP_E_FLASH_TRANSDTL_FULL 0x08 //流水存储已满
#define SP_R_PRESS_FUNC 0x09  //按了功能键返回
#define SP_R_BLACKCARD_CONSUME 0x0A  //黑卡消费
#define SP_E_CARD_STATUS 0x0B  //卡状态有误
#define SP_INSUFFICIENT_BALANCE 0x0C  //余额不足
#define SP_R_PRESS_CLEAR 0x0D //按了清除键返回
#define SP_E_LESS_THAN_MIN_CARD_BALANCE 0x0E //卡片余额小于最小卡余额
#define SP_E_CARD_PASSWD 0x0F //卡密码输入错误
#define SP_R_CARD_PASSWD_CANCEL 0x10 //取消卡密码输入
#define SP_E_WRITE_CARD 0x11 //写卡出错
#define SP_E_CALC_MAC 0x12 //计算mac出错
#define SP_E_NOT_SAME_CARD 0x13 //不是同一张卡
#define SP_NO_NUMBER_KEY_PRESSED 0x14 //没有输入金额
#define SP_R_CANCEL_CONSUME 0x15 //取消消费
#define SP_R_CARD_NOT_FOUND 0x16 //寻卡失败
#define SP_E_NOT_GET_PROVE 0x17 //没有扣掉卡上余额
#define SP_R_HALFWAY_PULL_CARD 0x18 //中途拔卡
#define SP_E_BUFFER_NO_DATA 0x19 //接收缓冲区没数据
#define SP_E_BUFFER_CRC_ERROR 0x1A //接收缓冲区crc错误
#define SP_R_WAIT_TIMEOUT 0x1B //等待响应超时
#define SP_E_SEQNO_NOT_MATCH 0x1C //接收到过大的顺序号
#define SP_E_CMD_NOT_MATCH 0x1D //接收到数据命令码不一致
#define SP_E_SERVER 0x1E //前置机收到数据后返回错误，暂时只定一个功能号
#define SP_E_DATE_REVERSE 0x1F //日期倒退
#define SP_R_DATE_CHANGE 0x20 //日期变化
#define SP_E_BUFFER_SIZE_OUT_OF_RANGE 0x21 //发送的数据长度超过最大长度
#define SP_E_EXCESSIVE_CALL 0x22 //未收到后台应答发送第二组数据
#define SP_E_CMD_NOT_FOUND 0x23 //命令码未找到
#define SP_R_FEERATE_CHANGE 0x24//需要更新费率
#define SP_R_SYSPARA_CHANGE 0x25//需要更新系统参数
#define SP_E_SYS_PASSWD 0x26//输入密码错误
#define SP_E_CARD_DATE_EXPIRE 0x27 //卡使用超过有效期
#define SP_E_CARD_PAYCNT_LIMIT 0x28 // 超过卡最大使用次数
#define SP_E_CARD_TYPE_NOT_SUPPORT 0x29 //卡片类别不支持
#define SP_S_DEVICEID_NOT_MATCH 0x2A //终端编号设置有误
/////////////////////////////////////////////////////////////
#define E_INSUFFICIENT_BALANCE 0x9401




#endif


