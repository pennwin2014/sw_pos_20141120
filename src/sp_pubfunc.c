#include "sp_pubfunc.h"



static uint16 crc_table[] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};


void sp_protocol_crc(const uint8* buffer, uint16 buffer_len, uint8 crc[2])
{
  uint16 accnum = 0xc78c;
  uint16 i;
  for(i = 0; i < buffer_len; ++i)
  {
    accnum = (accnum << 8) ^ crc_table[((accnum >> 8)
                                        ^ buffer[i]) & 0xFF];
  }
  crc[0] = (uint8)((accnum >> 8) & 0xFF);
  crc[1] = (uint8)(accnum & 0xFF);
}

uint8 sp_get_crc(uint8 record[23])
{
  unsigned char check_code = 0x00;
  int i = 0;
  for(i = 0; i < 23; ++i)
  {
    check_code |= record[i];
  }

  return check_code;
}
uint8 sp_check_crc(uint8 record[24])
{
  unsigned char check_code = 0x00;
  int i = 0;
  for(i = 0; i < 23; ++i)
  {
    check_code |= record[i];
  }

  if(check_code == 0 || check_code != record[23])
  {
    return 1;
  }
  return 0;
}
uint32 sp_atof_mul100(const char* m_str)
{
  uint32 result_value = 0;
  char int_part[10] = {0};
  char float_part[10] = {0};
  uint32 str_len = strlen(m_str);
  int i = 0;
  uint32 dot_index = 0;
  for(i = 0; i < str_len; i++)
  {
    if(m_str[i] == '.')
    {
      dot_index = i;
      break;
    }
  }
  if(dot_index > 0)
  {
    memcpy(int_part, m_str, dot_index);
    memcpy(float_part, m_str + dot_index + 1, 2);
    if(float_part[1] == 0)
    {
      float_part[1] = '0';
    }
    result_value = atoi(int_part) * 100 + atoi(float_part);
  }
  else
  {
    result_value = atoi(m_str);
    result_value = 100 * result_value;
  }

  return result_value;
}


void sp_bcd_hex(const uint8* bcd, uint8 bcd_len, char* hex)
{
  uint8 i;
  int j;
  for(i = 0; i < bcd_len; ++i)
  {
    j = i << 1;
    hex[j] = ((bcd[i] >> 4) & 0x0F) + 0x30;
    hex[j + 1] = (bcd[i] & 0x0F) + 0x30;
  }
  hex[i << 1] = 0;
}
void sp_hex_to_bcd(const uint8* hex, uint8 len, uint8* bcd)
{
  uint8 i;
  for(i = 0; i < len; ++i)
  {
    bcd[i] = hex[i] % 10;
    bcd[i] |= (uint8)(((uint32)(hex[i] / 10)) << 4);
  }
}

void sp_encode_hex(const uint8* data, uint8 data_len, char* hex)
{
  uint8 i, j;
  int k;
  for(i = 0; i < data_len; ++i)
  {
    k = i << 1;
    j = (data[i] >> 4) & 0x0F;
    if(j <= 9)
      hex[k] = j + 0x30;
    else if(j > 9)
      hex[k] = j + 0x37;

    j = data[i] & 0x0F;
    if(j <= 9)
      hex[k + 1] = j + 0x30;
    else if(j > 9)
      hex[k + 1] = j + 0x37;
  }
  hex[i << 1] = 0;
}

void sp_decode_hex(const char* hex, int hex_len, uint8* data)
{
  //  assert(0);
}

uint32 sp_bigend_to_int(uint8* data, uint8 len)
{
  uint8 temp[4];
  uint32 result;
  uint8* p = (uint8*)&result;
  // assert(len <= 4);
  memset(temp, 0 , sizeof temp);
  memcpy(temp + 4 - len, data, len);
  *p++ = temp[3];
  *p++ = temp[2];
  *p++ = temp[1];
  *p++ = temp[0];
  return result;
}
void sp_int_to_bigend(uint32 value, uint8 out_len, uint8* data)
{
  uint8 temp[4];
  uint8* p = (uint8*)&value;
  //assert(out_len <= 4);
  temp[3] = *p++;
  temp[2] = *p++;
  temp[1] = *p++;
  temp[0] = *p++;
  memcpy(data, temp + 4 - out_len, out_len);
}

void sp_bitmap_transform(uint32 value, sp_bitmap_offset_t* offset)
{
  if(value == 0)
    return;
  offset->byte_offset = (value - 1) / 8;
  offset->bit_offset = (value  - 1) % 8;
}

int sp_check_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size)
{
  sp_bitmap_offset_t offset;
  sp_bitmap_transform(value, &offset);
  if(offset.byte_offset >= bitmap_size)
    return -1;
  if(bitmap[offset.byte_offset] & (1 << offset.bit_offset))
    return 0;
  return -1;
}

int sp_set_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size)
{
  sp_bitmap_offset_t offset;
  sp_bitmap_transform(value, &offset);
  if(offset.byte_offset >= bitmap_size)
    return -1;
  bitmap[offset.byte_offset] |= 1 << offset.byte_offset;
  return 0;
}

int sp_clear_bitmap(uint32 value, uint8* bitmap, uint32 bitmap_size)
{
  sp_bitmap_offset_t offset;
  sp_bitmap_transform(value, &offset);
  if(offset.byte_offset >= bitmap_size)
    return -1;
  bitmap[offset.byte_offset] &= ~(1 << offset.bit_offset);
  return 0;
}

void sp_3bytes_to_uint32(const uint8 buffer[3], uint32* value)
{
  uint8 tmp_buffer[4];
  tmp_buffer[0] = buffer[2];
  tmp_buffer[1] = buffer[1];
  tmp_buffer[2] = buffer[0];
  tmp_buffer[3] = 0;
  memcpy((uint8*)value, tmp_buffer, 4);
}

void sp_uint32_to_3bytes(const uint32 value, uint8 buffer[3])
{
  uint8 tmp_buffer[4];
  memcpy(tmp_buffer, (uint8*)&value, 4);
  buffer[0] = tmp_buffer[3];
  buffer[1] = tmp_buffer[2];
  buffer[2] = tmp_buffer[1];
}

void sp_uint32_to_buffer(uint32 value, uint8 buffer[4])
{
  uint8 tmp_buffer[4];
  memcpy(tmp_buffer, (uint8*)&value, 4);
  buffer[0] = tmp_buffer[3];
  buffer[1] = tmp_buffer[2];
  buffer[2] = tmp_buffer[1];
  buffer[3] = tmp_buffer[0];
}

void sp_buffer_to_uint32(const uint8 buffer[4], uint32* value)
{
  uint8 tmp_buffer[4];
  tmp_buffer[0] = buffer[3];
  tmp_buffer[1] = buffer[2];
  tmp_buffer[2] = buffer[1];
  tmp_buffer[3] = buffer[0];
  memcpy((uint8*)value, tmp_buffer, 4);
}

void sp_uint16_to_buffer(uint16 value, uint8 buffer[2])
{
  memcpy(buffer, (uint8*)&value, 2);
}

void sp_buffer_to_uint16(const uint8 buffer[2], uint16* value)
{
  uint8 tmp_buffer[2];
  tmp_buffer[0] = buffer[1];
  tmp_buffer[1] = buffer[0];

  memcpy((uint8*)value, tmp_buffer, 2);
}

void sp_atoinet(const char* ip, uint8 addr[4])
{
  const char* p = ip;
  char* q;
  char temp[5];
  uint8 i = 0;
  while(*p != '\0')
  {
    q = strchr(p, '.');
    if(q != NULL)
    {
      memset(temp, 0 , sizeof temp);
      memcpy(temp, p, q - p);
      addr[i++] = atoi(temp);
      p = q + 1;
    }
    else
    {
      p = q;
      break;
    }
  }
  if(p != NULL && (*p != '\0') && i <= 3)
  {
    addr[i] = atoi(p);
  }
}

void sp_inettoa(const uint8 addr[4], char* ip)
{
  sprintf(ip, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

void sp_timestr_to_systime(uint8 tm_str[7], SYSTIME* tm)
{
  tm->chYear = sp_bcd_to_uint32(tm_str, 2);
  tm->chMonth = sp_bcd_to_uint8(tm_str[2]);
  tm->chDay = sp_bcd_to_uint8(tm_str[3]);
  tm->chHour = sp_bcd_to_uint8(tm_str[4]);
  tm->chMinute = sp_bcd_to_uint8(tm_str[5]);
  tm->chSecond = sp_bcd_to_uint8(tm_str[6]);
  //tm->chWeek
}

uint8 sp_bcd_to_uint8(const uint8 bcd)
{
  uint8 t = ((bcd >> 4) & 0x0F) * 10;
  t += bcd & 0x0F;
  return t;
}

int sp_is_number_key(uint8 key, uint8* value)
{
  if(key >= SP_KEY_0 && key <= SP_KEY_9)
  {
    *value = key - SP_KEY_0;
  }
  else if(key == SP_KEY_DOT)
  {
    *value = SP_KEY_DOT;
  }
  else
  {
    return -1;
  }
  return 0;
}

int sp_is_leap_year(uint32 year)
{
  if(year % 100 == 0)
  {
    return (year % 400 == 0) ? 1 : 0;
  }
  else
  {
    return (year % 4 == 0) ? 1 : 0;
  }
}

uint32 sp_bcd_to_uint32(uint8* bcd, uint8 len)
{
  uint8 i;
  uint32 r = 0;
  for(i = 0; i < len; ++i)
  {
    uint8 t = ((bcd[i] >> 4) & 0x0F) * 10;
    t += bcd[i] & 0x0F;
    r = r * 100 + t;
  }
  return r;
}

uint32 sp_day_to_year2k(uint8 date[4])
{
  uint32 begin_year = 2000;
  uint32 days = 0;
  uint32 year, mon, mday;
  uint32 t;
  uint8 mon_days [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  year = sp_bcd_to_uint32(date, 2);
  mon = sp_bcd_to_uint32(date + 2, 1);
  if(mon < 1 || mon > 12)
    return 0;
  mday = sp_bcd_to_uint32(date + 3, 1);
  while(begin_year < year)
  {
    days += sp_is_leap_year(begin_year) ? 366 : 365;
    ++begin_year;
  }
  for(t = 1; t < mon; ++t)
  {
    days += mon_days[t - 1];
  }
  days += mday;
  return days;
}

void sp_encrypt_cardkey(const uint8* card_key, uint8* out_key)
{
  uint8 i;
  uint8 statickey[] = "\x32\x30\x30\x35\x30\x31\x33\x31";
  for(i = 0; i < 8; ++i)
  {
    out_key[i] = ~(card_key[i] ^ statickey[i]);
  }
}

uint8 sp_calc_crc8(const uint8* d, int len)
{
  uint8 crc;
  uint8 i;
  crc = 0;
  while(len--)
  {
    crc ^= *d++;
    for(i = 0; i < 8; ++i)
    {
      if(crc & 0x01)
      {
        crc = (crc >> 1) ^ 0x8C;
      }
      else crc >>= 1;
    }
  }
  return crc;
}

//////////////////////////////////////////////////////////////////////////
void sp_sleep(int ms)
{
  sp_DelayNS2(ms * 5000);
}

/*__inline */void sp_DelayNS2(int dly)
{
  int i;
  for(i = 0; i < dly; i++);
}


void sp_ByteToDecStr(unsigned char cValue, unsigned char *pChar)
{
  unsigned char i;
  for(i = 0; i < 3; i++)
  {
    pChar[2 - i] = cValue % 10 + '0';
    cValue /= 10;
  }
  pChar[3] = '\0';
}

void sp_uint16_to_bcd(const uint16 value, uint8 bcd[2])
{
  uint8 part1 = 0, part2 = 0;
  part1 = value % 100;
  part2 = (value - part1) / 100;
  sp_uint8_to_bcd(part2, &bcd[0]);
  sp_uint8_to_bcd(part1, &bcd[1]);
}
void sp_uint8_to_bcd(const uint8 value, uint8* bcd)
{
  *bcd = (((value / 10) << 4) & (0xF0)) | ((value % 10) & (0x0F));
}

void sp_get_time(sp_context* ctx)
{
  uint8 current_datetime[7];
  SYSTIME tmptime;
  sp_get_hdtime(&tmptime);
  tmptime.chYear = 2014;
  sp_uint16_to_bcd(tmptime.chYear, current_datetime);
  sp_uint8_to_bcd(tmptime.chMonth, &current_datetime[2]);
  sp_uint8_to_bcd(tmptime.chDay, &current_datetime[3]);
  sp_uint8_to_bcd(tmptime.chHour, &current_datetime[4]);
  sp_uint8_to_bcd(tmptime.chMinute, &current_datetime[5]);
  sp_uint8_to_bcd(tmptime.chSecond, &current_datetime[6]);
  memcpy(ctx->current_datetime, current_datetime, 7);
  memcpy(ctx->today, ctx->current_datetime, 4);
}

int sp_get_min(int a, int b)
{
  return a > b ? b : a;
}
void sp_beep(uint8 beep_count)
{
  Beep(beep_count);
}

//四舍五入函数,保留num位小数
double sp_d4u5(double value, int num)
{
  int sign = 1;
  long long i = 0;
  double k = 1.0;

  for(i = 0; i < num + 1; i++)
    k = 10.0 * k;
  if(value < 0)
  {
    sign = -1;
    value = -value;
  }
  value = value * k + 5;
  i = value;
  i -= i % 10;
  value = i / k;
  return value * sign;
}
// 将浮点数转换为整型
int sp_d2i(double value)
{
  return (int) sp_d4u5(value, 0);
}


/////////////////////////////////////////////////////////////////////////////////


uint32 sp_calc_diff_seconds(SYSTIME* tm1, SYSTIME* tm2)
{
  time_t t1, t2;
  struct tm tt1, tt2;
  memset(&tt1, 0x0, sizeof(tt1));
  tt1.tm_year = tm1->chYear;
  tt1.tm_mon = tm1->chMonth;
  tt1.tm_mday = tm1->chDay;
  tt1.tm_hour = tm1->chHour;
  tt1.tm_min = tm1->chMinute;
  tt1.tm_sec = tm1->chSecond;
  t1 = mktime(&tt1);
  //
  memset(&tt2, 0x0, sizeof(tt2));
  tt2.tm_year = tm2->chYear;
  tt2.tm_mon = tm2->chMonth;
  tt2.tm_mday = tm2->chDay;
  tt2.tm_hour = tm2->chHour;
  tt2.tm_min = tm2->chMinute;
  tt2.tm_sec = tm2->chSecond;
  t2 = mktime(&tt2);
  return t1 - t2;
}

int sp_calc_diff_days(uint8 day1[4], uint8 day2[4])
{
  return sp_day_to_year2k(day1) - sp_day_to_year2k(day2);
}
//////////////////////////////////////////////////////////////////////////////////////


int sp_check_context(sp_context* ctx)
{
  memset(ctx, 0, sizeof(sp_context));
  ctx->offline_rec_amt = 0;
  ctx->rec_flash_full = false;

  sprintf(ctx->school_name, "上海大学");
  ctx->online_flag = false;
  ctx->reset_cpucard_count = 0;


  //param部分
  ctx->param.base_amt = 200;
  ctx->param.blacklist_index = 0;
  ctx->param.card_support = 0;
  ctx->param.consume_interval = 10;
  ctx->param.m1_alg = 1;
  ctx->param.offline_day = 0;
  ctx->param.record_index = 1;
  ctx->disp_cnt = 0;
  return 0;
}

int32 sp_get_key(void)
{
  return GetSCKEY();
}
uint8 sp_get_hdtime(SYSTIME* tm)
{
  return GetTime(tm) == 1 ? 0 : 1;
}
uint8 sp_set_hdtime(SYSTIME* tm)
{
  return SetTime(tm) == 1 ? 0 : 1;
}

uint8 sp_check_time(SYSTIME tm1, SYSTIME tm2)
{
  if((tm1.chYear != tm2.chYear) || (tm1.chMonth != tm2.chMonth) || (tm1.chDay != tm2.chDay))
    return SP_E_CMD_NOT_MATCH;
  if((tm1.chHour != tm2.chHour) || (tm1.chMinute != tm2.chMinute) || (tm1.chSecond != tm2.chSecond))
    return SP_E_CMD_NOT_MATCH;
  return 0;
}

/////////////////////////////////////////////////////////////






