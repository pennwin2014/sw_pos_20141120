#ifndef _SP_PUBFUNC_H_
#define _SP_PUBFUNC_H_

#include "string.h"
#include "stdio.h"
#include "config.h"
#include "sp_info.h"
#include <time.h>


///////////////////////////////////////////////////
//#define SP_WAIT_AND_DISPMSG(key, fmt, ...) do{sp_wait_for_retkey(key, __FUNCTION__, __LINE__, fmt, __VA_ARGS__);}while(0)
#define SP_WAIT_AND_DISPMSG(key, fmt, args...) do{sp_wait_for_retkey(key, __FUNCTION__, __LINE__, fmt, args);}while(0)

#define SP_IS_SAME_CARDPHYID(x, y) (memcmp(x, y, 4) == 0)

///////////////////////////////工具函数/////////////////////////////////////////////
void sp_sleep(int ms);
void sp_beep(uint8 beep_count);
int sp_get_min(int a, int b);
void sp_protocol_crc(const uint8* buffer, uint16 buffer_len, uint8 crc[2]);
void sp_bcd_hex(const uint8* bcd, uint8 bcd_len, char* hex);
void sp_uint16_to_bcd(const uint16 value, uint8 bcd[2]);
void sp_uint8_to_bcd(const uint8 value, uint8* bcd);
void sp_encode_hex(const uint8* data, uint8 data_len, char* hex);
void sp_decode_hex(const char* hex, int hex_len, uint8* data);
uint32 sp_bigend_to_int(uint8* data, uint8 len);
void sp_int_to_bigend(uint32 value, uint8 out_len, uint8* data);
int sp_is_number_key(uint8 key, uint8* value);
uint32 sp_atof_mul100(const char* m_str);
uint8 sp_get_crc(uint8 record[23]);
uint8 sp_check_crc(uint8 record[24]);
uint8 sp_check_time(SYSTIME tm1, SYSTIME tm2);
void sp_timestr_to_systime(uint8 tm_str[7], SYSTIME* tm);
uint8 sp_get_hdtime(SYSTIME* tm);
uint8 sp_set_hdtime(SYSTIME* tm);
uint8 sp_bcd_to_uint8(const uint8 bcd);
void sp_hex_to_bcd(const uint8* hex, uint8 len, uint8* bcd);

typedef struct
{
  uint32 byte_offset;
  uint32 bit_offset;
} sp_bitmap_offset_t;

void sp_bitmap_transform(uint32 value, sp_bitmap_offset_t* offset);

int sp_check_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size);

int sp_set_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size);

int sp_clear_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size);

void sp_3bytes_to_uint32(const uint8 buffer[3], uint32* value);

void sp_uint32_to_3bytes(const uint32 value, uint8 buffer[3]);

void sp_uint32_to_buffer(uint32 value, uint8 buffer[4]);

void sp_buffer_to_uint32(const uint8 buffer[4], uint32* value);

void sp_uint16_to_buffer(uint16 value, uint8 buffer[2]);

void sp_buffer_to_uint16(const uint8 buffer[2], uint16* value);

void sp_atoinet(const char* ip, uint8 addr[4]);

void sp_inettoa(const uint8 addr[4], char* ip);

int sp_is_leap_year(uint32 year);

uint32 sp_bcd_to_uint32(uint8* bcd, uint8 len);

uint32 sp_day_to_year2k(uint8 date[4]);

void sp_encrypt_cardkey(const uint8* card_key, uint8* out_key);

uint8 sp_calc_crc8(const uint8* d, int len);

double sp_d4u5(double value, int num);

// 将浮点数转换为整型
int sp_d2i(double value);

///////////////////////////////////////////////////////
void sp_DelayNS2(int dly);

void sp_ByteToDecStr(unsigned char cValue, unsigned char *pChar);

void sp_get_time(sp_context* ctx);
/////////////////////////////////////////////////////////////////////////

int sp_check_context(sp_context* ctx);

int32 sp_get_key(void);


int sp_calc_diff_days(uint8 day1[4], uint8 day2[4]);

uint32 sp_calc_diff_seconds(SYSTIME* tm1, SYSTIME* tm2);

#endif


