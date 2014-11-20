#include "sp_flash_impl.h"
//////////////////////////˽�к���///////////////////////////////
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
      sp_disp_error("������ˮ�洢��ʧ��");
      return SP_E_FLASH_ERASE;
    }
  }
  //�ڵ�һ��λ��дһ��0����ˮ
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
  //�������洢��
  page_no = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_ErasePage(page_no);
  if(ret)
  {
    return SP_E_FLASH_ERASE;
  }
  //���ôӴ洢��
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
  //д����ˮ�Ŵ洢��
  page_no = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_Write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //д����ˮ�Ŵ洢��
  page_no = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
  ret = sp_SF_Write(page_no, 0, (uint8*)&transno_unit, sizeof(sp_transno_unit), 0);
  if(ret)
  {
    return SP_E_FLASH_WRITE;
  }
  //�ٶ�һ�鿴��crc�Բ���?
  //sp_SF_Read(uint32 page_no,uint32 offset_addr,uint8 * array,uint32 counter)
  ret = do_reset_transdtl_data(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}


/*
*����:�����ˮ�洢���Ƿ�����
*/
static int do_check_transdtl(sp_context* ctx)
{
  sp_transno_unit master_unit, slave_unit;
  sp_transdtl transdtl;
  uint8 ret = 0;
  //1����ȡ����ˮ�Ŵ洢��
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &master_unit);
  if(ret)
  {
    return ret;
  }
  //2����ȡ����ˮ�Ŵ洢��
  ret = sp_read_transno_unit(TRANSNO_FLAG_SLAVE, &slave_unit);
  if(ret)
  {
    return ret;
  }
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    return SP_E_FLASH_SEQNO_NOT_EQUAL;
  }
  //3����֤������Ч��
  ret = sp_read_transdtl(&transdtl, master_unit.last_trans_addr);
  if(ret)
  {
    sp_disp_error("��ȡ��ˮʧ��,ret=%d", ret);
    return ret;
  }
  sp_disp_debug("termseqno=%d,last_trans_no=%d", transdtl.termseqno, master_unit.last_trans_no);
  if(transdtl.termseqno != master_unit.last_trans_no)
  {
    sp_disp_error("��ˮ�����е���ˮ������ˮ�Ŵ洢������ˮ�Ų�ͬ����ˮ�Լ���ˮ�Ŵ洢��������");
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
  ret = sp_input_words(ctx, "�ն˱��: ", termno, 8);
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
  ret = sp_input_words(ctx, "�ն˻���: ", canid, 8);
  if(ret)
    return ret;
  ctx->syspara.canid = atoi(canid);
  return 0;
}

static int do_init_syspara(sp_context* ctx)
{
  int ret = 0;
  memset(&ctx->syspara, 0, sizeof(sp_syspara));
  //����
  ctx->syspara.work_mode = SP_WORK_MODE_NORMAL;
  ctx->syspara.amount = 0;
  memcpy(ctx->syspara.time_gap, "123", 3);
  ctx->syspara.max_cardbal = 100;
  ctx->syspara.max_amount = 100;
  memcpy(ctx->syspara.restart_time, "\x20\x14\x01\x01\x00\x00\x00\x00", 8);
  ctx->syspara.return_flag = 0;
  ctx->syspara.offline_flag = 0;
  ctx->syspara.min_cardbal = 1000;//ʮ��Ǯ
  ctx->syspara.timeout = 0;
  ctx->syspara.heartgap = 8;
  ctx->syspara.once_limit_amt = 0;
  ctx->syspara.day_sum_limit_amt = 50000;//���Ԫ
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
  //Ӱ��Ӳ����
  memcpy(ctx->syspara.blacklist_verno, "\x14\x11\x07\x15\x05\x09\x11", 7);
  ctx->syspara.syspara_verno = 0;
  ctx->syspara.feepara_verno = 0;
  //��ʾ�û������豸��ź��豸����
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
////////////////////////////////�ⲿ����//////////////////////////////////////
static uint32 sp_get_next_transdtl_addr(uint32 transdtl_addr)
{
  //����Ҫ�����һҳ�絽��һҳ�ˣ�Ҳ����һҳɾ��
  if(transdtl_addr + sizeof(sp_transdtl) > ADDR_TRANS_LAST)
  {
    //�費��Ҫ����һҳɾ��
    //�Ƿ�Ҫ������Ҫ��������ҳ����ˮδ�ϴ�������??????
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
  //�鿴�����Ƿ����л�,�������л���1��
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
    //���ڵ���
    //sp_disp_error("���ڵ��ˣ���ϵ����Ա");
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
  // ptransdtl->transflag = 0x01;//��ʾ����
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
*** ������:   sp_write_transno_unit
*** ��������: д��(��)��ˮ��
*** ����flag:    true----> ����ˮ��;       false-----> ����ˮ��
*** ����punit: ������Ľṹ���ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
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
  pageno = startaddr / FLASH_PAGE_SIZE; //�õ�ҳ��
  lastaddr = sp_get_transno_lastaddr(pageno);//��ȡ��������Ե�ַ
  //sp_disp_msg(999, "lastaddr=%d,2*size=%d", lastaddr, 2 * sizeof(sp_transno_unit));
  if(lastaddr + 2 * sizeof(sp_transno_unit) > FLASH_PAGE_SIZE) //�鿴�Ƿ�ᳬ������ַ
  {
    //sp_disp_msg(100, "��ˮ�Ŵ洢�����������²���");
    ret = sp_SF_ErasePage(pageno);    //������ҳ
    if(ret)
    {
      sp_disp_error("������ˮ�Ŵ洢��ʧ��");
      return SP_E_FLASH_ERASE;
    }
    lastaddr = 0;   //��0��ʼд
  }
  else
  {
    lastaddr = lastaddr + sizeof(sp_transno_unit);
  }
  //��lastaddr��ʼ����д
  ret = sp_SF_Write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 0);//ֱ��д
  if(ret)
  {
    sp_disp_error("ֱ��дʧ�ܣ���������д");
    ret = sp_SF_Write(pageno, lastaddr, (uint8*)punit, sizeof(sp_transno_unit), 1);//��������д
  }
  return ret;

}


/*******************************************************
*** ������:   sp_read_transno_unit
*** ��������: ��ȡ������ˮ��
*** ����flag:    true ����ˮ��;false ����ˮ��
*** ����punit:  ��С��Ԫ�Ľṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
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
*** ������:   sp_write_transdtl
*** ��������:   д������ˮ��flash
*** ����:   ������ˮ�ṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_write_transdtl(sp_transdtl* ptransdtl)
{
  //1����ȡ������ˮ��
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
  //2���Ա�������ˮ���Ƿ�һ��
  if(master_unit.last_trans_no != slave_unit.last_trans_no)
  {
    sp_disp_error("����ˮ�źʹ���ˮ�Ų�һ��");
    //������ˮ�Ŵ洢�������ݸ��ǵ�����ˮ����
    pageno = ADDR_SLAVE_TRANS_SEQNO / FLASH_PAGE_SIZE;
    sp_SF_Read(pageno, 0, read_buf, FLASH_PAGE_SIZE);
    pageno = ADDR_MASTER_TRANS_SEQNO / FLASH_PAGE_SIZE;
    ret = sp_SF_Write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
    //���ɹ�����ѭ��д
    write_cnt = MAX_FLASH_WRITE_CNT;
    while(ret != SP_SUCCESS)
    {
      ret = sp_SF_Write(pageno, 0, read_buf, FLASH_PAGE_SIZE, 1);//������д
      if(write_cnt-- < 1)
      {
        return SP_FAIL;//���ش���
      }
    }
  }
  //3�����ݴ���ˮ����ĵ�ַ��ȡ��ˮ����
  pageno = slave_unit.last_trans_addr / FLASH_PAGE_SIZE;
  offset_addr = slave_unit.last_trans_addr - pageno * FLASH_PAGE_SIZE;
  memset(&read_transdtl, 0, sizeof(read_transdtl));
  sp_SF_Read(pageno, offset_addr, (uint8*)(&read_transdtl), sizeof(read_transdtl));
  //4���Ա���ˮ�����ˮ�źʹ���ˮ�Ŵ洢���е���ˮ���Ƿ�һ��
  if(read_transdtl.termseqno != slave_unit.last_trans_no)
  {
    sp_disp_error("��ˮ�Ų����,termseqno=%d,last_trans_no=%d", read_transdtl.termseqno, slave_unit.last_trans_no);
    return SP_FAIL;
  }
  //5��д�µ�����ˮ��
  //memcpy(&tmp_trandtl, ptransdtl, sizeof(sp_transdtl));
  ////5.1����ȡ��һ����ˮ��ַ
  next_addr = sp_get_next_transdtl_addr(slave_unit.last_trans_addr);

  ////5.2�����õ�ַ�Լ���ˮ�е���ˮ��д������ˮ�Ŵ洢��
  memset((&master_unit), 0, sizeof(master_unit));
  master_unit.last_trans_no = ptransdtl->termseqno;
  master_unit.last_trans_addr = next_addr;
  memcpy(master_unit.date, ptransdtl->transdatetime, 3);
  master_unit.sum_amt += ptransdtl->amount;
  //6��д�µ�����ˮ��
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
  //7��д�µĴ���ˮ��
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
    //8�� д������ˮ���Ǻڿ�������ˮ��Ҫ�ں���������º���9�ֽ�
    // sp_disp_msg(100, "write transdtl in addr=%08x", master_unit.last_trans_addr);
    pageno = master_unit.last_trans_addr / FLASH_PAGE_SIZE;
    offset_addr = master_unit.last_trans_addr - FLASH_PAGE_SIZE * pageno;
    sp_SF_Write(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl), 1);//ʹ�ò�����д�ķ�ʽ
    //9����ȡһ����ˮ�����Ƿ���ڴ��е�һ��
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
*** ������:   sp_read_transdtl
*** ��������: ��ȡ��ˮ����
*** ����ptransdtl:  ��ˮ�Ľṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_read_transdtl(sp_transdtl* ptransdtl, uint32 trans_addr)
{
  uint8 ret = 0;
  uint16 pageno = 0, offset_addr = 0;
  uint8 tmp_crc[2];
  //1��������ˮ����ĵ�ַ��ȡ��ˮ
  pageno = trans_addr / FLASH_PAGE_SIZE;
  offset_addr = trans_addr - pageno * FLASH_PAGE_SIZE;
  ret =  sp_SF_Read(pageno, offset_addr, (uint8*)ptransdtl, sizeof(sp_transdtl));
  if(ret)
    return ret;
  //2�����crc�Ƿ���ȷ
  sp_protocol_crc((uint8*)ptransdtl, sizeof(sp_transdtl) - 2, tmp_crc);
  if(memcmp(tmp_crc, ptransdtl->crc, 2) != 0)
  {
    return SP_E_FLASH_CRC;
  }
  return SP_SUCCESS;
}

/*
*  ˵��:��ҳ��������ֽڴ��crc
*/
uint8 sp_write_timepara_table(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  uint8 timepara_table[FLASH_PAGE_SIZE];
  uint16 table_size = 0;
  uint8 crc[2];
  //����crc��crc����crc��λ��
  table_size = 4 * sizeof(sp_timepara);
  memset(timepara_table, 0, FLASH_PAGE_SIZE);
  memcpy(timepara_table, ctx->timepara_table, table_size);
  sp_protocol_crc(timepara_table, table_size, timepara_table + FLASH_PAGE_SIZE - 2);
  //����ҳ��
  pageno = ADDR_TIMEPARA / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
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
  //���¶�ȡ�����ö�ȡ������У��crc
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
  //����һ��crc
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
  //����crc��crc����crc��λ��
  ctx->feerate_table[255] = sp_calc_crc8(ctx->feerate_table, 255);
  //����ҳ��
  pageno = ADDR_FEERATE / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
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
  //���¶�ȡ�����ö�ȡ�����ݼ���crc
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
  //����һ��crc���������������1
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
*** ������:   sp_write_sysinfo
*** ��������:   дϵͳ������flash
*** ����:   ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
*********************************************************/
uint8 sp_write_syspara(sp_context* ctx)
{
  uint16 pageno = 0;
  int32 ret = 0;
  sp_syspara read_sysinfo;
  memset(&read_sysinfo, 0, sizeof(sp_syspara));
  //����crc��crc����crc��λ��
  sp_protocol_crc((uint8*)(&ctx->syspara), sizeof(sp_syspara) - 2, ctx->syspara.crc);
  //����ҳ��
  pageno = ADDR_SYSINFO / FLASH_PAGE_SIZE;
  //�Ȳ���ԭ����һ��ҳ
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
  //���¶�ȡ�����ö�ȡ�����ݼ���crc
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
*** ������:   sp_read_syspara
*** ��������: ��ȡϵͳ��Ϣ
*** ����:  ȫ�ֽṹ��ָ��
*** ����ֵ:  0--�ɹ�  1--ʧ��
*** ����:   ����
*** ʱ��:   2014-07-03
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
  //����һ��crc���������������1
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

//��ȡ��ˮ�Ŵ洢�������һ����Ч��ַ
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
  //һ���԰�һҳȫ������
  memset(page_buffer, 0, FLASH_PAGE_SIZE);
  ret = sp_SF_Read(pageno, 0, page_buffer, FLASH_PAGE_SIZE);
  if(ret)
  {
    sp_disp_error("��ȡ����, ret=%04x", ret);
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
  //��ȡ������
  ret = sp_read_blacklist(ctx);
  if(ret)
    return ret;
  //��ȡϵͳ����
  ret = sp_read_syspara(ctx);
  if(ret)
    return ret;
  //��ȡ����
  ret = sp_read_feerate_table(ctx);
  if(ret)
    return ret;
  //��ȡʱ���
  ret = sp_read_timepara_table(ctx);
  if(ret)
    return ret;
  //�����ˮ�洢��
  ret = do_check_transdtl(ctx);
  if(ret)
    return ret;
  return SP_SUCCESS;
}

uint8 sp_recover_device(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "���ڳ�ʼ��ϵͳ����...");
  ret = do_init_syspara(ctx);
  if(ret)
    return ret;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "���ڳ�ʼ��������...");
  ret = do_init_blacklist(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "���ڳ�ʼ�����ʱ�...");
  ret = do_init_feerate_table(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "���ڳ�ʼ��ʱ��α�...");
  ret = do_init_timepara_table(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "���ڳ�ʼ����ˮ�洢��  ...");
  ret = do_reset_transdtl(ctx);
  if(ret)
    return ret;
  sp_print_row(FLAG_CLEAR, 1, 0, "��ʼ���ɹ�!!");
  return SP_SUCCESS;
}



