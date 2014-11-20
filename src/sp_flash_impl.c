#include "sp_flash_impl.h"
//////////////////////////私有函数///////////////////////////////
static int do_reset_transdtl_data(sp_context* ctx)
{
  uint32 page_no = 0;
  uint32 tmp_addr = 0;
  uint8 ret = 0;
  sp_transdtl transdtl;
  for(tmp_addr = ADDR_TRANS_DATA; tmp_addr < ADDR_TRANS_LAST; tmp_addr += FLASH_PAGE_SIZE)
  {
    page_no = tmp_addr / FLASH_PAGE_SIZE;
    ret = sp_SF_ErasePage(page_no);
    if(ret)
    {
      sp_disp_error("擦除流水存储区失败");
      return SP_E_FLASH_ERASE;
    }
  }
  //在第一个位置写一笔0号流水
  memset(&transdtl, 0, sizeof(sp_transdtl));
  transdtl.termseqno = sp_get_transno();
  sp_protocol_crc((uint8*)(&transdtl), sizeof(sp_transdtl) - 2, transdtl.crc);
  page_no = ADDR_TRANS_DATA / FLASH_PAGE_SIZE;
  ret = sp_SF_Write(page_no, 0, (uint8*)(&transdtl), sizeof(sp_transdtl), 0);
  if(ret)
    return SP_E_FLASH_WRITE;
  return SP_SUCCESS;
}

static int do_reset_transdtl(sp_context* ctx)
{
  uint8 ret = 0;
  uint32 page_no;
  sp_transno_unit transno_unit;
  //重置主存储区
  page_no = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_ErasePage(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  //重置从存储区
  page_no = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_ErasePage(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  memset(&transno_unit, 0, sizeof(sp_transno_unit));
  transno_unit.last_trans_no = 0;
  transno_unit.last_trans_addr = ADDR_TRANS_DATA;
  memcpy(transno_unit.date, ctx->current_datetime, 3);
  sp_protocol_crc((uint8*)&transno_unit, sizeof(sp_transno_unit) - 2, transno_unit.crc);
  //写主流水号存储区
  page_no = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_Write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //写从流水号存储区
  page_no = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_Write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //再读一遍看看crc对不对?
  //sp_SF_Read(uint32 page_no,uint32 offset_addr,uint8 * array,uint32 counter)
  ret = do_reset_transdtl_data(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}


/*
*作用:检测流水存储区是否正常
*/
static int do_check_transdtl(sp_context* ctx)
{
  sp_transno_unit master_unit, slave_unit;
  sp_transdtl transdtl;
  uint8 ret = 0;
  //1、读取主流水号存储区
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &master_unit);
  if(ret)
  {
    return ret;
  }
  //2、读取从流水号存储区
  ret = sp_read_transno_unit(TRANSNO_FLAG_SLAVE, &slave_unit);
  if(ret)
  {
    return ret;
  }
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  //3、验证数据有效性
  ret = sp_read_transdtl(&transdtl, master_unit.last_trans_addr);
  if(ret)
  {
    sp_disp_error("读取流水失败,ret=%d", ret);
    return ret;
  }
  sp_disp_debug("termseqno=%d,last_trans_no=%d", transdtl.termseqno, master_unit.last_trans_no);
  if(transdtl.termseqno != master_unit.last_trans_no)
  {
    sp_disp_error("流水数据中的流水号与流水号存储区的流水号不同，流水以及流水号存储区将重置");
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  return SP_SUCCESS;
}
static int do_init_blacklist(sp_context* ctx)
{
  return SP_SUCCESS;
}

static int do_input_termno(sp_context* ctx)
{
  int ret = 0;
  char termno[MAX_SCREEN_ROWS];
  char tmp[3];
  int i = 0, j = 0;
  ret = sp_input_words(ctx, "终端编号: ", termno, 8);
  if(ret)
    return ret;
  tmp[2] = 0;
  for(i = 0; i < 4; i++)
  {
    tmp[0] = termno[i * 2];
    tmp[1] = termno[i * 2 + 1];
    j  = atoi(tmp);
    sp_uint8_to_bcd(j, ctx->syspara.termno + i);
  }
  return 0;
}
static int do_input_canid(sp_context* ctx)
{
  int ret = 0;
  char canid[MAX_SCREEN_ROWS];
  ret = sp_input_words(ctx, "终端机号: ", canid, 8);
  if(ret)
    return ret;
  ctx->syspara.canid = atoi(canid);
  return 0;
}

static int do_init_syspara(sp_context* ctx)
{
  int ret = 0;
  memset(&ctx->syspara, 0, sizeof(sp_syspara));
  //其他
  ctx->syspara.work_mode = SP_WORK_MODE_NORMAL;
  ctx->syspara.amount = 0;
  memcpy(ctx->syspara.time_gap, "123", 3);
  ctx->syspara.max_cardbal = 100;
  ctx->syspara.max_amount = 100;
  memcpy(ctx->syspara.restart_time, "\x20\x14\x01\x01\x00\x00\x00\x00", 8);
  ctx->syspara.return_flag = 0;
  ctx->syspara.offline_flag = 0;
  ctx->syspara.min_cardbal = 1000;//十块钱
  ctx->syspara.timeout = 0;
  ctx->syspara.heartgap = 8;
  ctx->syspara.once_limit_amt = 0;
  ctx->syspara.day_sum_limit_amt = 50000;//五百元
  ctx->syspara.limit_switch = 1;
  memcpy(ctx->syspara.hd_verno, "\x09\x09\x09\x09\x09", 5);
  memcpy(ctx->syspara.soft_verno, "\x00\x00\x01", 3);
  memcpy(ctx->syspara.samno, "\x08\x08\x08\x08\x08\x08", 6);
  ctx->syspara.key_index = '\x01';
  ctx->syspara.yesterday_total_amt = 0;
  ctx->syspara.yesterday_total_cnt = 0;
  ctx->syspara.today_total_amt = 0;
  ctx->syspara.today_total_cnt = 0;
  memcpy(ctx->syspara.password, "\x31\x31\x31\x31\x31\x31", 6);
  ctx->syspara.max_pay_cnt = 5000;
  //影响硬件的
  memcpy(ctx->syspara.blacklist_verno, "\x14\x11\x07\x15\x05\x09\x11", 7);
  ctx->syspara.syspara_verno = 0;
  ctx->syspara.feepara_verno = 0;
  //提示用户输入设备编号和设备机号
  //memcpy(ctx->syspara.termno, "\x00\x00\x00\x00", 4);
  // ctx->syspara.canid = 1;
  ret = do_input_termno(ctx);
  if(ret)
    return ret;
  ret = do_input_canid(ctx);
  if(ret)
    return ret;
  ctx->syspara.init_flag = 1;
  ret = sp_write_syspara(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static int do_init_feerate_table(sp_context* ctx)
{
  int ret = 0;
  ret = sp_write_feerate_table(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

static int do_init_timepara_table(sp_context* ctx)
{
  int ret = 0;
  ret = sp_write_timepara_table(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}
////////////////////////////////外部函数//////////////////////////////////////
static uint32 sp_get_next_transdtl_addr(uint32 transdtl_addr)
{
  //假如要从最后一页跨到第一页了，也将第一页删除
  if(transdtl_addr + sizeof(sp_transdtl) > ADDR_TRANS_LAST)
  {
    //需不需要将第一页删除
    //是否要考虑正要被擦除的页的流水未上传的问题??????
    return ADDR_TRANS_DATA;
  }
  else
  {
    return transdtl_addr + sizeof(sp_transdtl);
  }
}

int sp_check_date_change(sp_context* ctx)
{
  int diff_days = 0;
  sp_get_time(ctx);
  diff_days = sp_calc_diff_days(ctx->today, ctx->syspara.today_date);
  //查看日期是否有切换,并且是切换了1天
  if(diff_days > 1)
  {
    ctx->syspara.yesterday_total_cnt = 0;
    ctx->syspara.yesterday_total_amt = 0;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return sp_write_syspara(ctx);
  }
  else if(diff_days == 1)
  {
    ctx->syspara.yesterday_total_cnt = ctx->syspara.today_total_cnt;
    ctx->syspara.yesterday_total_amt = ctx->syspara.today_total_amt;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
    return sp_write_syspara(ctx);
  }
  else if(diff_days < 0)
  {
    //日期倒退
    //sp_disp_error("日期倒退，联系管理员");
    return SP_E_DATE_REVERSE;
  }

  return 0;
}

uint8 sp_update_left_transdtl_info(uint32 page_addr, sp_transdtl* ptransdtl)
{
  uint8 ret = 0;
  uint32 page_no = 0;
  uint32 offset_addr = 0;
  sp_transdtl read_transdtl;
  //sp_disp_msg(30, "update left 9bytes ,page_addr=%08x", page_addr);
  // ptransdtl->transflag = 0x01;//表示消费
  memset(ptransdtl->reserve, 0xFF, 2);
  sp_protocol_crc((uint8 *)(ptransdtl), sizeof(sp_transdtl) - 2, ptransdtl->crc);
  //
  page_no = page_addr / FLASH_PAGE_SIZE;
  offset_addr = page_addr - page_no * FLASH_PAGE_SIZE + 55;

  ret = sp_SF_Write(page_no, offset_addr, (uint8*)(ptransdtl) + 55, 9, 1);
  if(ret)
    return SP_E_FLASH_WRITE;
  offset_addr = page_addr - page_no * FLASH_PAGE_SIZE;
  memset(&read_transdtl, 0, sizeof(read_transdtl));
  ret = sp_SF_Read(page_no, offset_addr, (uint8*)&read_transdtl, sizeof(sp_transdtl));
  if(ret)
    return SP_E_FLASH_READ;
  //sp_disp_msg(100, "crc1=[%02x%02x],crc_read=[%02x%02x]",ptransdtl->crc[0], ptransdtl->crc[1],
  //read_transdtl.crc[0], read_transdtl.crc[1]);
  if(memcmp(read_transdtl.crc, ptransdtl->crc, 2) != 0)
    return SP_E_FLASH_CRC;
  return SP_SUCCESS;
}


/*******************************************************
*** 函数名:   sp_write_transno_unit
*** 函数功能: 写主(从)流水号
*** 参数flag:    true----> 主流水号;       false-----> 从流水号
*** 参数punit: 待存入的结构体的指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
*********************************************************/
uint8 sp_write_transno_unit(bool flag, sp_transno_unit* punit)
{
  uint8 ret = 0;
  uint16 pageno = 0;
  uint32 lastaddr = 0, startaddr = 0;
  sp_protocol_crc((uint8*)(punit), sizeof(sp_transno_unit) - 2, punit->crc);
  if(flag == TRANSNO_FLAG_MASTER)
  {
    startaddr = ADDR_MASTER_TRANS_SEQNO;
  }
  else
  {
    startaddr = ADDR_SLAVE_TRANS_SEQNO;
  }
  pageno = startaddr / FLASH_PAGE_SIZE; //得到页号
  lastaddr = sp_get_transno_lastaddr(pageno);//获取到的是相对地址
  //sp_disp_msg(999, "lastaddr=%d,2*size=%d", lastaddr, 2 * sizeof(sp_transno_unit));
  if(lastaddr + 2 * sizeof(sp_transno_unit) > FLASH_PAGE_SIZE) //查看是否会超过最大地址
  {
    //sp_disp_msg(100, "流水号存储区已满，重新擦除");
    ret = sp_SF_ErasePage(pageno);    //擦除该页
    if(ret)
    {
      sp_disp_error("擦除流水号存储区失败");
      return SP_E_FLASH_ERASE;
    }
    lastaddr = 0;   //从0开始写
  }
  else
  {
    lastaddr = lastaddr + sizeof(sp_transno_unit);
  }
  //从lastaddr开始往下写
  ret = sp_SF_Write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 0);//直接写
  if(ret)
  {
    sp_disp_error("直接写失败，擦除后再写");
    ret = sp_SF_Write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 1);//擦除后再写
  }
  return ret;

}


/*******************************************************
*** 函数名:   sp_read_transno_unit
*** 函数功能: 读取主从流水号
*** 参数flag:    true 主流水号;false 从流水号
*** 参数punit:  最小单元的结构体指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
********************************************************/
uint8 sp_read_transno_unit(bool flag, sp_transno_unit* ptrans_unit)
{
  uint16 pageno = 0;
  uint8 ret = 0;
  uint8 read_buf[FLASH_PAGE_SIZE] = {0};
  uint32 offset = 0;
  sp_transno_unit tmp_unt;
  uint8 tmp_crc[2] = {0};
  uint16 unit_len  = 0;
  bool is_exist_before = false;

  unit_len = sizeof(sp_transno_unit);
  if(flag == TRANSNO_FLAG_MASTER)
  {
    pageno = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
  }
  else
  {
    pageno = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
  }
  ret = sp_SF_Read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
  if(ret)
    return SP_FAIL;
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&tmp_unt, read_buf + offset, unit_len);
    sp_protocol_crc((uint8*)&tmp_unt, unit_len - 2, tmp_crc);
    //sp_disp_msg(30, "transno=%d,crc1=%02x%02x,crc2=%02x%02x", tmp_unt.last_trans_no, tmp_crc[0], tmp_crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(tmp_crc, tmp_unt.crc, 2) == 0)
    {
      memcpy(ptrans_unit, &tmp_unt, unit_len);
      is_exist_before = true;
    }
    else
    {
      break;
    }
  }
  if(is_exist_before == true)
    return SP_SUCCESS;
  else
    return SP_E_FLASH_NOT_FOUNT;
}

uint32 sp_get_transno(void)
{
  uint8 ret = 0;
  sp_transno_unit transno_unit;
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &transno_unit);
  if(ret)
    return 0;
  return transno_unit.last_trans_no;
}

int32 sp_get_last_transaddr(void)
{
  uint8 ret = 0;
  sp_transno_unit transno_unit;
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &transno_unit);
  if(ret)
  {
    sp_disp_error("read transno error.,ret=%04x", ret);
    return 0;
  }
  return transno_unit.last_trans_addr;
}


/*******************************************************
*** 函数名:   sp_write_transdtl
*** 函数功能:   写交易流水到flash
*** 参数:   交易流水结构体指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
*********************************************************/
uint8 sp_write_transdtl(sp_transdtl* ptransdtl)
{
  //1、读取主从流水号
  uint8 ret = 0;
  uint16 pageno, write_cnt = 0, offset_addr = 0;
  uint32 next_addr = 0;
  sp_transno_unit master_unit, slave_unit;
  sp_transdtl read_transdtl;//tmp_trandtl
  uint8 read_buf[FLASH_PAGE_SIZE];
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &master_unit);
  if(ret)
    return ret;
  ret = sp_read_transno_unit(TRANSNO_FLAG_SLAVE, &slave_unit);
  if(ret)
    return ret;
  //2、对比主从流水号是否一致
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    sp_disp_error("主流水号和从流水号不一致");
    //将从流水号存储区的内容覆盖到主流水号中
    pageno = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
    sp_SF_Read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
    pageno = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
    ret = sp_SF_Write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//擦除后写
    //不成功就死循环写
    write_cnt = MAX_FLASH_WRITE_CNT;
    while(ret != SP_SUCCESS)
    {
      ret = sp_SF_Write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//擦除后写
      if(write_cnt-- < 1)
      {
        return SP_FAIL;//返回错误
      }
    }
  }
  //3、根据从流水号里的地址读取流水数据
  pageno = slave_unit.last_trans_addr / FLASH_PAGE_SIZE;
  offset_addr = slave_unit.last_trans_addr - pageno * FLASH_PAGE_SIZE;
  memset(&read_transdtl, 0, sizeof(read_transdtl));
  sp_SF_Read(pageno, offset_addr, (uint8*)(&read_transdtl), sizeof(read_transdtl));
  //4、对比流水里的流水号和从流水号存储区中的流水号是否一致
  if(read_transdtl.termseqno != slave_unit.last_trans_no)
  {
    sp_disp_error("流水号不相等,termseqno=%d,last_trans_no=%d", read_transdtl.termseqno, slave_unit.last_trans_no);
    return SP_FAIL;
  }
  //5、写新的主流水号
  //memcpy(&tmp_trandtl, ptransdtl, sizeof(sp_transdtl));
  ////5.1、获取下一个流水地址
  next_addr = sp_get_next_transdtl_addr(slave_unit.last_trans_addr);

  ////5.2、将该地址以及流水中的流水号写入主流水号存储区
  memset((&master_unit), 0, sizeof(master_unit));
  master_unit.last_trans_no = ptransdtl->termseqno;
  master_unit.last_trans_addr = next_addr;
  memcpy(master_unit.date, ptransdtl->transdatetime, 3);
  master_unit.sum_amt += ptransdtl->amount;
  //6、写新的主流水号
  ret = sp_write_transno_unit(TRANSNO_FLAG_MASTER, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_transno_unit(true, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  //7、写新的从流水号
  ret = sp_write_transno_unit(TRANSNO_FLAG_SLAVE, &master_unit);
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(ret != SP_SUCCESS)
  {
    ret = sp_write_transno_unit(TRANSNO_FLAG_SLAVE, &master_unit);
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }
  write_cnt = MAX_FLASH_WRITE_CNT;
  while(1)
  {
    //8、 写整条流水，非黑卡消费流水需要在后续步骤更新后面9字节
    // sp_disp_msg(100, "write transdtl in addr=%08x", master_unit.last_trans_addr);
    pageno = master_unit.last_trans_addr / FLASH_PAGE_SIZE;
    offset_addr = master_unit.last_trans_addr - FLASH_PAGE_SIZE * pageno;
    sp_SF_Write(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl), 1);//使用擦出后写的方式
    //9、读取一遍流水内容是否和内存中的一致
    sp_SF_Read(pageno, offset_addr, (uint8*)(&read_transdtl), sizeof(sp_transdtl));
    if(read_transdtl.termseqno == ptransdtl->termseqno)
    {
      break;
    }
    if(write_cnt-- < 1)
    {
      return SP_FAIL;
    }
  }

  return SP_SUCCESS;
}



/*******************************************************
*** 函数名:   sp_read_transdtl
*** 函数功能: 读取流水数据
*** 参数ptransdtl:  流水的结构体指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
*********************************************************/
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr)
{
  uint8 ret = 0;
  uint16 pageno = 0, offset_addr = 0;
  uint8 tmp_crc[2];
  //1、根据流水号里的地址读取流水
  pageno = trans_addr / FLASH_PAGE_SIZE;
  offset_addr = trans_addr - pageno * FLASH_PAGE_SIZE;
  ret =  sp_SF_Read(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl));
  if(ret)
    return ret;
  //2、检查crc是否正确
  sp_protocol_crc((uint8*)ptransdtl, sizeof(sp_transdtl) - 2, tmp_crc);
  if(memcmp(tmp_crc, ptransdtl->crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

/*
*  说明:该页最后两个字节存放crc
*/
uint8 sp_write_timepara_table(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  uint8 timepara_table[FLASH_PAGE_SIZE];
  uint16 table_size = 0;
  uint8 crc[2];
  //计算crc后将crc存入crc的位置
  table_size = 4 * sizeof(sp_timepara);
  memset(timepara_table, 0, FLASH_PAGE_SIZE);
  memcpy(timepara_table, ctx->timepara_table, table_size);
  sp_protocol_crc(timepara_table, table_size, timepara_table + FLASH_PAGE_SIZE - 2);
  //计算页号
  pageno = ADDR_TIMEPARA / FLASH_PAGE_SIZE;
  //先擦除原来的一整页
  ret = sp_SF_ErasePage(pageno);
  if(ret)
  {
    sp_disp_error("sp_SF_ErasePage fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_SF_Write(pageno, 0, timepara_table, FLASH_PAGE_SIZE, 1);
  if(ret)
  {
    sp_disp_error("sp_SF_Write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //重新读取，并用读取的数据校验crc
  memset(timepara_table, 0, FLASH_PAGE_SIZE);
  ret = sp_SF_Read(pageno, 0, timepara_table, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("sp_SF_Read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  sp_protocol_crc(timepara_table, table_size, crc);
  if(memcmp(timepara_table + FLASH_PAGE_SIZE - 2, crc, 2) != 0)
  {
    sp_disp_error("calc crc fail");
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

uint8 sp_read_timepara_table(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 read_buffer[FLASH_PAGE_SIZE];
  int32 ret = 0;
  uint8 crc[2];
  uint16 read_size = 0;
  pageno = ADDR_TIMEPARA / FLASH_PAGE_SIZE;
  ret = sp_SF_Read(pageno, 0, read_buffer, FLASH_PAGE_SIZE);
  if(ret)
    return SP_E_FLASH_READ;
  read_size = sizeof(ctx->timepara_table);
  //检验一下crc
  sp_protocol_crc(read_buffer, read_size, crc);
  if(memcmp(read_buffer + FLASH_PAGE_SIZE - 2, crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  memcpy(ctx->timepara_table, read_buffer, read_size);
  return SP_SUCCESS;
}

uint8 sp_write_feerate_table(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  uint8 read_feerate_table[256];
  memset(read_feerate_table, 0, 256);
  //计算crc后将crc存入crc的位置
  ctx->feerate_table[255] = sp_calc_crc8(ctx->feerate_table, 255);
  //计算页号
  pageno = ADDR_FEERATE / FLASH_PAGE_SIZE;
  //先擦除原来的一整页
  ret = sp_SF_ErasePage(pageno);
  if(ret)
  {
    sp_disp_error("sp_SF_ErasePage fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_SF_Write(pageno, 0, ctx->feerate_table, 256, 1);
  if(ret)
  {
    sp_disp_error("sp_SF_Write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //重新读取，并用读取的数据计算crc
  ret = sp_SF_Read(pageno, 0, read_feerate_table, 256);
  if(ret)
  {
    sp_disp_error("sp_SF_Read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  read_feerate_table[255] = sp_calc_crc8(read_feerate_table, 255);
  if(ctx->feerate_table[255] != read_feerate_table[255])
  {
    sp_disp_error("calc crc fail");
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}
uint8 sp_read_feerate_table(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 crc = 0;
  int32 ret = 0;
  pageno = ADDR_FEERATE / FLASH_PAGE_SIZE;
  ret = sp_SF_Read(pageno, 0, ctx->feerate_table, 256);
  if(ret)
    return SP_E_FLASH_READ;
  //检验一下crc如果不正常，返回1
  crc = sp_calc_crc8(ctx->feerate_table, 255);
  if(ctx->feerate_table[255] != crc)
  {
    return SP_E_FLASH_CRC;
  }
  else
  {
    return SP_SUCCESS;
  }

}

/*******************************************************
*** 函数名:   sp_write_sysinfo
*** 函数功能:   写系统参数到flash
*** 参数:   全局结构体指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
*********************************************************/
uint8 sp_write_syspara(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  sp_syspara read_sysinfo;
  memset(&read_sysinfo, 0, sizeof(sp_syspara));
  //计算crc后将crc存入crc的位置
  sp_protocol_crc((uint8*)(&ctx->syspara), sizeof(sp_syspara) - 2, ctx->syspara.crc);
  //计算页号
  pageno = ADDR_SYSINFO / FLASH_PAGE_SIZE;
  //先擦除原来的一整页
  ret = sp_SF_ErasePage(pageno);
  if(ret)
  {
    sp_disp_error("sp_SF_ErasePage fail,ret=%d", ret);
    return SP_E_FLASH_ERASE;
  }
  ret = sp_SF_Write(pageno, 0, (uint8*)(&ctx->syspara), sizeof(sp_syspara), 1);
  if(ret)
  {
    sp_disp_error("sp_SF_Write fail,ret=%d", ret);
    return SP_E_FLASH_WRITE;
  }
  //重新读取，并用读取的数据计算crc
  ret = sp_SF_Read(pageno, 0, (uint8*)(&read_sysinfo), sizeof(sp_syspara));
  if(ret)
  {
    sp_disp_error("sp_SF_Read fail,ret=%d", ret);
    return SP_E_FLASH_READ;
  }
  sp_protocol_crc((uint8*)(&read_sysinfo), sizeof(sp_syspara) - 2, read_sysinfo.crc);
  if(memcmp(ctx->syspara.crc, read_sysinfo.crc, 2) != 0)
  {
    sp_disp_error("calc crc fail,[%02x%02x]==[%02x%02x]", ctx->syspara.crc[0], ctx->syspara.crc[1]
                  , read_sysinfo.crc[0], read_sysinfo.crc[1]);
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}


/*******************************************************
*** 函数名:   sp_read_syspara
*** 函数功能: 读取系统信息
*** 参数:  全局结构体指针
*** 返回值:  0--成功  1--失败
*** 作者:   汪鹏
*** 时间:   2014-07-03
*********************************************************/
uint8 sp_read_syspara(sp_context* ctx)
{
  uint16 pageno = 0;
  uint8 crc[2] = {0};
  int32 ret = 0;
  pageno = ADDR_SYSINFO / FLASH_PAGE_SIZE;
  ret = sp_SF_Read(pageno, 0, (uint8*)(&ctx->syspara), sizeof(ctx->syspara));
  if(ret)
    return SP_E_FLASH_READ;
  //检验一下crc如果不正常，返回1
  sp_protocol_crc((uint8*)(&ctx->syspara), sizeof(sp_syspara) - 2, crc);
  if(memcmp(ctx->syspara.crc, crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  else
  {
    return SP_SUCCESS;
  }
}

uint8 sp_read_blacklist(sp_context* ctx)
{
  return 0;
}
uint8 sp_write_blacklist(sp_context* ctx)
{
  //ADDR_BLACKLIST
  return 0;
}

//获取流水号存储区中最后一个有效地址
uint16 sp_get_transno_lastaddr(uint16 pageno)
{
  int16 offset = 0;
  uint8 ret = 0;
  int16 unit_len = 0;
  sp_transno_unit tmp_unt;
  uint8 crc[2] = {0};
  uint8 page_buffer[FLASH_PAGE_SIZE];
  bool is_exist_before = false;
  uint16 result_offset = FLASH_PAGE_SIZE;

  unit_len = sizeof(sp_transno_unit);
  //一次性把一页全部读出
  memset(page_buffer, 0, FLASH_PAGE_SIZE);
  ret = sp_SF_Read(pageno, 0, page_buffer, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("读取错误, ret=%04x", ret);
    return FLASH_PAGE_SIZE;
  }
  for(offset = 0; offset < FLASH_PAGE_SIZE; offset += unit_len)
  {
    memcpy(&tmp_unt, page_buffer + offset, unit_len);
    sp_protocol_crc((uint8*)(&tmp_unt), unit_len - 2, crc);
    //sp_disp_error("trano=%d,c1=%02x%02x,c2=%02x%02x", tmp_unt.last_trans_no, crc[0], crc[1], tmp_unt.crc[0], tmp_unt.crc[1]);
    if(memcmp(tmp_unt.crc, crc, 2) == 0)
    {
      is_exist_before = true;
      result_offset = offset;
    }
    else
    {
      break;
    }
  }
  if(is_exist_before == true)
    return result_offset;
  else
    return FLASH_PAGE_SIZE;
}

void sp_disp_flash(int32 addr, int32 len)
{
  char disp_msg[1024];
  uint8 array[512];
  uint8 ret = 0;
  int page_no = 0, i = 0;
  uint32 counter = 0;
  uint32 offset_addr = 0;
  memset(disp_msg, 0, 1024);
  memset(array, 0, 512);
  page_no = addr / FLASH_PAGE_SIZE;
  offset_addr = addr - page_no * FLASH_PAGE_SIZE;
  counter = sp_get_min(len, 512);
  ret = sp_SF_Read(page_no, offset_addr, array, counter);
  if(ret)
  {
    sp_disp_error("sp_SF_Read error");
    return;
  }
  for(i = 0; i < counter; i++)
  {
    sprintf(disp_msg + 2 * i, "%02x", array[i]);
  }
  sp_wait_for_retkey(SP_KEY_MUL, __FUNCTION__, __LINE__, disp_msg);
}



uint16 sp_get_offline_transdtl_amout(sp_context* ctx)
{
  return 0;
  //offline_rec_amt
}


uint8 sp_read_system(sp_context* ctx)
{
  int ret = 0;
  memset(ctx, 0, sizeof(sp_context));
  //读取黑名单
  ret = sp_read_blacklist(ctx);
  if(ret)
    return ret;
  //读取系统参数
  ret = sp_read_syspara(ctx);
  if(ret)
    return ret;
  //读取费率
  ret = sp_read_feerate_table(ctx);
  if(ret)
    return ret;
  //读取时间段
  ret = sp_read_timepara_table(ctx);
  if(ret)
    return ret;
  //检查流水存储区
  ret = do_check_transdtl(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

uint8 sp_recover_device(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在初始化系统参数...");
  ret = do_init_syspara(ctx);
  if(ret)
    return ret;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在初始化黑名单...");
  ret = do_init_blacklist(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "正在初始化费率表...");
  ret = do_init_feerate_table(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "正在初始化时间段表...");
  ret = do_init_timepara_table(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "正在初始化流水存储区  ...");
  ret = do_reset_transdtl(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "初始化成功!!");
  return SP_SUCCESS;
}



