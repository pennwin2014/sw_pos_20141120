#include "sp_consume.h"


static int is_blkcard(sp_context* ctx, uint32 cardno)
{
  uint32 page_no;
  uint8 blkbuffer[64], t;
  sp_bitmap_offset_t offset;
  uint8 page_offset = 0;
  sp_bitmap_transform(cardno, &offset);
  page_no = (ADDR_BLACKLIST + offset.byte_offset) / FLASH_PAGE_SIZE;
  page_offset = ADDR_BLACKLIST + offset.byte_offset - page_no * FLASH_PAGE_SIZE;
  sp_SF_Read(page_no, page_offset, blkbuffer, 1);
  t = blkbuffer[0];//[offset.byte_offset - 0];
  return ((t & (1 << offset.bit_offset)) == 0) ? 1 : 0;
}
static int do_input_and_check_card_passwd(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  char real_password[MAX_SCREEN_ROWS];
  ret = sp_input_password(ctx, "�����뿨����:", real_password, 6);
  if(ret)
    return ret;
  if(memcmp(real_password, card->passwd, strlen(card->passwd)) != 0)
  {
    return SP_E_CARD_PASSWD;
  }
  return 0;
}
static int do_check_consume_limit(sp_context * ctx, sp_card* card)
{
  int ret = 0;
  uint8 cancel_flag = 0;
  card->use_passwd = 0;
  if(ctx->syspara.limit_switch == SP_SWITCH_OFF)
    return 0;
  //����
  if(card->once_limit_amt > 0 && card->payamt >= card->once_limit_amt)
  {
    card->use_passwd = 1;
  }
  else if(card->day_sum_limit_amt > 0 && (card->day_sum_amt + card->payamt) >= card->day_sum_limit_amt)
  {
    card->use_passwd = 1;
  }
  //pos����
  else if(ctx->syspara.once_limit_amt > 0 && card->payamt >= ctx->syspara.once_limit_amt)
  {
    card->use_passwd = 1;
  }
  else if(ctx->syspara.day_sum_limit_amt > 0 && (card->day_sum_amt + card->payamt) >= ctx->syspara.day_sum_limit_amt)
  {
    card->use_passwd = 1;
  }
  if(card->use_passwd)
  {
    // �������
    ret = sp_read_card(ctx, card, SP_READ_FILE19);
    if(ret)
      return ret;
    while(1)
    {
      ret = do_input_and_check_card_passwd(ctx, card);
      if(SP_SUCCESS == ret)
        return SP_SUCCESS;
      else if(SP_R_CARD_PASSWD_CANCEL == ret)
      {
        sp_disp_by_type(SP_TP_IS_CANCEL_CONSUME, ctx, &cancel_flag);
        if(cancel_flag == SP_SWITCH_OFF)
        {
          continue;
        }
        else
          return ret;
      }
      else if(SP_E_CARD_PASSWD == ret)
      {
        sp_beep(1);
        sp_disp_by_type(SP_TP_PASSWORD_ERROR, ctx, NULL);
        continue;
      }
      return ret;
    }

  }
  return 0;
}
static int do_check_blkcard(sp_context * ctx, sp_card* card)
{
  if(memcmp(card->cardverno, ctx->syspara.blacklist_verno, 7) > 0)
  {
    return 0;
  }
  return is_blkcard(ctx, card->cardno);
}

//��ˮ���������ⲿ��ctx���record��ֵ
static int prepare_transdtl(sp_context * ctx, sp_card* card, sp_transdtl* transdtl)
{
  int extra_amount = 0;
  uint32 trans_no = 0;
  uint16 record_used_size;
  //  memset(&(ctx->record), 0, sizeof(sp_transdtl));
  //��һ�ʽ���
  sp_uint32_to_3bytes(card->last_overdraft_limit, transdtl->last_limit_amt);
  transdtl->last_amt = card->last_trans_amt;
  transdtl->last_cnt = card->last_trans_seqno;
  transdtl->last_transflag = card->last_trans_flag;
  memcpy(transdtl->last_termno, card->last_samno, 6);
  memcpy(transdtl->last_datetime, card->last_datetime + 1, 6);
  //���ν���
  transdtl->amount = card->payamt;
  transdtl->cardbefbal = card->cardbefbal;
  transdtl->cardbefcnt = card->paycnt;
  transdtl->cardno = card->cardno;
  extra_amount = abs(card->payamt - card->amount);
  memcpy(transdtl->extra_amt, &extra_amount, 3);
  memset(transdtl->tac, 0 , sizeof transdtl->tac);
  memcpy(transdtl->tac, card->tac, 4);
  memcpy(transdtl->transdatetime, ctx->current_datetime + 1, 6);
  memcpy(transdtl->samno, ctx->syspara.samno, 6);
  //д��ˮ��ʱ����õ������ˮ��
  trans_no = sp_get_transno();
  transdtl->termseqno = trans_no + 1;
  //sp_disp_debug("termseqno=%d", transdtl->termseqno);
  //����crc
  record_used_size = sizeof(sp_transdtl) - 2;//transdtl->crc - &transdtl->termseqno;
  //sp_disp_msg(20, "Record used size=%d", record_used_size);
  sp_protocol_crc((uint8*)transdtl, record_used_size, transdtl->crc);
  return  0;
}



static void do_update_sum_amt(sp_context* ctx, sp_transdtl* record)
{
  uint8 ret = 0;
  int diff_days = 0;
  sp_get_time(ctx);
  diff_days = sp_calc_diff_days(ctx->today, ctx->syspara.today_date);
  if(diff_days > 1)
  {
    ctx->syspara.yesterday_total_cnt = 0;
    ctx->syspara.yesterday_total_amt = 0;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
  }
  else if(diff_days == 1)
  {
    ctx->syspara.yesterday_total_cnt = ctx->syspara.today_total_cnt;
    ctx->syspara.yesterday_total_amt = ctx->syspara.today_total_amt;
    ctx->syspara.today_total_cnt = 0;
    ctx->syspara.today_total_amt = 0;
    memcpy(ctx->syspara.today_date, ctx->today, 4);
  }
  else if(diff_days < 0)
  {
    sp_disp_error("��������,ʱ�䵹��,diffdays=%d", diff_days);
    ret = sp_select_card_app(ctx);
  }
  ctx->syspara.today_total_amt += record->amount;
  ++ctx->syspara.today_total_cnt;
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    //����������ϵ����Ա
    sp_disp_error("������������ϵ����Ա��������%04x", ret);
    sp_sleep(9999999);
  }
}






//////////////////////////////�����ĺ���//////////////////////////////////////////////
void sp_check_record_rom(sp_context* ctx)
{
  ctx->rec_flash_full = false;
}



uint8 sp_pre_check(sp_context* ctx)
{
  sp_check_record_rom(ctx);
  if(ctx->rec_flash_full)
  {
    sp_disp_error("��ˮ�洢��������ɼ�");
    return SP_E_FLASH_TRANSDTL_FULL;
  }
  return SP_SUCCESS;
}

static int do_input_money(sp_context* ctx, uint32* money, const uint8* cardphyid, char* value)
{
  uint8 upd = 1;
  uint32 key;
  int32 ret;
  uint8 req_card_phyid[4];
  char temp[20];
  uint8 t;
  while(1)
  {
    if(upd)
    {
      //����һ�½��
      sp_disp_by_type(SP_TP_DISP_MONEY, ctx, value);
      upd = 0;
    }
    ret = sp_request_card(req_card_phyid);
    if(ret)
    {
      //sp_disp_error(3, "==Ѱ��ʧ��==");
      return SP_R_CARD_NOT_FOUND;
    }

    if(memcmp(req_card_phyid, cardphyid, 4) != 0)
    {
      sp_disp_debug("phyid=[%02x%02x%02x%02x]", req_card_phyid[0], req_card_phyid[1], req_card_phyid[2], req_card_phyid[3]);
      return SP_E_NOT_SAME_CARD;
    }

    key = sp_get_key();
    if(!sp_is_number_key(key, &t))
    {
      if(strlen(value) >= 6)
        continue;
      strcpy(temp, value);
      if(t == SP_KEY_DOT)
      {
        //��һ���������ǵ�
        if(strlen(value) == 0)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          //�����Ѿ����ڵ��˾Ͳ�����
          if(strchr(temp, '.') != NULL)
            continue;
          sprintf(value, "%s%c", temp, '.');
        }
      }
      else
      {
        sprintf(value, "%s%d", temp, t);
      }
      upd = 1;
    }
    /*
    else if(key == SP_KEY_MODE)
    {
      return -1;
    }
    */
    else if(key == SP_KEY_CONFIRM)
    {
      //*money = atoi(value) * 100;
      *money = sp_atof_mul100(value);
      return 0;
    }
    else if(key == SP_KEY_CLEAR)
    {
      uint8 vl = strlen(value);
      if(vl == 0)
        return SP_R_CANCEL_CONSUME;
      if(vl > 0)
        value[vl - 1] = 0;
      upd = 1;
    }
  }
}
static int do_input_money_without_card(sp_context* ctx, uint32* money, uint8 t)
{
  uint8 upd = 1;
  uint32 key;
  char temp[20];
  char value[20] = {0};
  //uint8 t;
  //������
  if(t == SP_KEY_DOT)//�������.�ͱ��0.
  {
    memcpy(value, "0.", 2);
  }
  else
  {
    strcpy(temp, value);
    sprintf(value, "%s%d", temp, t);
  }
  while(1)
  {
    if(upd)
    {
      //����һ�½��
      sp_disp_by_type(SP_TP_DISP_MONEY, ctx, value);
      upd = 0;
    }

    key = sp_get_key();

    if(!sp_is_number_key(key, &t))
    {
      if(strlen(value) >= 6)
        continue;
      strcpy(temp, value);
      if(t == SP_KEY_DOT)
      {
        //��һ���������ǵ�
        if(strlen(value) == 0)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          //�����Ѿ����ڵ��˾Ͳ�����
          if(strchr(temp, '.') != NULL)
            continue;
          sprintf(value, "%s%c", temp, '.');
        }
      }
      else
      {
        sprintf(value, "%s%d", temp, t);
      }
      upd = 1;
    }
    else if(key == SP_KEY_CONFIRM)
    {
      //*money = atoi(value) * 100;
      *money = sp_atof_mul100(value);
      return 0;
    }
    else if(key == SP_KEY_CLEAR)
    {
      uint8 vl = strlen(value);
      if(vl == 0)
        return 1;
      if(vl > 0)
        value[vl - 1] = 0;
      upd = 1;
    }
  }
}

static int do_transdtl_halfway(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  int rate = 0;
  rate = ctx->feerate_table[card->feetype - 1] - 100;
  if(card->use_passwd)
  {
    if(rate == 0)//��û���ۿ�Ҳû�д���
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    else if(rate > 0) //���ۿ�
    {
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    }
    else//������
    {
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    }
  }
  else
  {
    if(rate == 0)//��û���ۿ�Ҳû�д���
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    else if(rate > 0) //���ۿ�
    {
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    }
    else//������
    {
      ctx->exception_record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
    }
  }
  ret = prepare_transdtl(ctx, card, &(ctx->exception_record));
  if(ret)
    return ret;
  ret = sp_write_transdtl(&(ctx->exception_record));//��һ�ʺڿ���ˮ
  if(ret)
  {
    sp_disp_error("������ˮʧ��,ret=%d", ret);
    return ret;
  }
  return 0;
}

static int do_purchase_wirte_card(sp_context* ctx, sp_card* card)
{
  int ret;
  ret = sp_payinit(ctx, card, card->payamt);
  if(ret)
  {
    sp_disp_error("pay init fail, ret=%04x", ret);
    return ret;
  }
  //sp_disp_debug("finish pay init=%d",1);
  // pay debit
  ret = sp_payment(ctx, card, card->payamt);
  if(ret)
  {
    //����������д���ױ�־
    ctx->record.transflag = TRANSFLAG_UNCONFIRM;
    //��һ����;�ο���ˮ
    card->is_halfway = 1;
    return ret;
  }
  else
  {
    //sp_disp_debug("pay confirm success,tac=%02x%02x%02x%02x", p_card->tac[0], p_card->tac[1], p_card->tac[2], p_card->tac[3]);
    ctx->record.transflag = TRANSFLAG_HALFFWAY_BOARD_NO_PASSWD;
  }
  return 0;
}

static int do_wait_reload_card(sp_context* ctx, sp_card* card)
{
  sp_card this_card;
  int ret = 0;
  while(sp_request_card_poweron(&this_card))
  {
    sp_beep(1);
    SP_CLS(0);
    if(SP_KEY_CLEAR == sp_get_key())
      return SP_R_CANCEL_CONSUME;
  }
  //Ѱ�������ж����ǲ���ͬһ�ſ�
  if(!SP_IS_SAME_CARDPHYID(this_card.cardphyid, card->cardphyid))
  {
    return SP_E_NOT_SAME_CARD;
  }
  ret = sp_select_card_app(ctx);
  if(ret)
  {
    sp_disp_error("select card app error, ret=%04x", ret);
    return ret;
  }
  return 0;
}

static int do_wait_and_prove_card(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  ret = do_wait_reload_card(ctx, card);
  if(ret)
    return ret;
  ret = sp_get_prove(ctx, card, ++card->paycnt);
  if(ret)
  {
    return SP_E_NOT_GET_PROVE;
  }
  return 0;
}


static int do_purchase(sp_context* ctx, sp_card* card)
{
  int ret;
  bool has_save_transdtl = false;
  while(1)
  {
    ret = do_purchase_wirte_card(ctx, card);
    if(ret)
    {
      //������������Ҫ
      if(E_INSUFFICIENT_BALANCE == ret)
        return ret;
      //��¼һ����;�ο���ˮ
      if(has_save_transdtl == false)
      {
        if(card->is_halfway)
        {
          //��һ����;�ο���ˮ
          ret = do_transdtl_halfway(ctx, card);
          if(!ret)
          {
            sp_send_rt_transdtl(ctx, &ctx->exception_record);
            has_save_transdtl = true;
          }
        }
      }
      sp_disp_msg(30, "�����·ſ�");
      ret = do_wait_and_prove_card(ctx, card);
      // success,  ��¼�ɹ�������ˮһ��
      if(SP_SUCCESS == ret)
        break;
      sp_disp_debug("do_wait_and_prove_card,ret=%04x", ret);
      // �û�ȡ���� return
      if(SP_R_CANCEL_CONSUME == ret)
      {
        sp_disp_msg(30, "�û�ȡ��");
        return ret;
      }
      //��δ�۳ɹ�������
      if(SP_E_NOT_GET_PROVE == ret)
        continue;
    }
    else
      break;
  }
  //  ��¼�ɹ�������ˮһ��
  return 0;
}
static int check_and_input_money(sp_context* ctx, uint32* money, sp_card* p_card, char value[20], uint8 clear_value_flag, uint8* press_flag)
{
  int32 key = 0;
  //char value[20] = {0};
  char temp[20];
  uint8 req_card_phyid[4];
  uint8 t;
  if(clear_value_flag)
  {
    memset(value, 0, 20);
    while(1)
    {
      // Ѱ������ҲҪreturn
      if(sp_request_card(req_card_phyid))
      {
        return SP_R_CANCEL_CONSUME;//û�����������Ҳ��Ϊ���û�ȡ��
      }
      // �ж��ǲ���ͬһ�ſ�
      if(!SP_IS_SAME_CARDPHYID(req_card_phyid, p_card->cardphyid))
      {
        return SP_E_NOT_SAME_CARD;
      }
      key = sp_get_key();
      if(!sp_is_number_key(key, &t))
      {
        if(t == SP_KEY_DOT)
        {
          memcpy(value, "0.", 2);
        }
        else
        {
          strcpy(temp, value);
          sprintf(value, "%s%d", temp, t);
        }
        *press_flag = 0;
        break;
      }
    }
  }
  //������
  return do_input_money(ctx, money, p_card->cardphyid, value);
}

static int do_transdtl_blackcard(sp_context* ctx, sp_card* the_card)
{
  int ret = 0;
  // black card
  sp_disp_msg(100, "�ÿ��Ǻڿ�");
  sp_set_card_loss(ctx, the_card);
  ctx->exception_record.transflag = TRANSFLAG_LOSTCARD;
  ret = prepare_transdtl(ctx, the_card, &(ctx->exception_record));
  if(ret)
    return ret;
  ret = sp_write_transdtl(&(ctx->exception_record));//��һ�ʺڿ���ˮ
  if(ret)
  {
    sp_disp_error("������ˮʧ��,ret=%d", ret);
    return ret;
  }
  ret = sp_send_rt_transdtl(ctx, &ctx->exception_record);
  if(ret)
  {
    sp_disp_error("�ϴ��ڿ�������ˮʧ��,ret=%d", ret);
    return ret;
  }
  return 0;

}
static int do_check_card_validity(sp_context* ctx, sp_card* card)
{
  int ret = 0;
  if(memcmp(ctx->today, card->expiredate, 4) > 0)
  {
    return SP_E_CARD_DATE_EXPIRE;
  }
  if(card->paycnt >= ctx->syspara.max_pay_cnt)
  {
    return SP_E_CARD_PAYCNT_LIMIT;
  }
  if(sp_check_bitmap(card->feetype, ctx->syspara.feetype_bitmap, sizeof(ctx->syspara.feetype_bitmap)))
  {
    //sp_disp_error("card feetype=%02x", card->feetype);
    return SP_E_CARD_TYPE_NOT_SUPPORT;
  }
  // TODO: �ѻ�ʹ������(����Ų�������ط����)
  // TODO : �жϺڿ�ǰ�Ƶ������������Ǻڿ����ϼ�һ�ʺڿ����ѵ���ˮ������
  if(do_check_blkcard(ctx, card))
  {
    ret = do_transdtl_blackcard(ctx, card);
    if(ret)
    {
      // TODO: ����
    }
    return SP_R_BLACKCARD_CONSUME;
  }

  return 0;
}
static int do_prepare_pay_with_card(sp_context* ctx, sp_card* the_card)
{
  int ret;
  uint32 amount;
  uint8 clear_value_flag = 1;
  uint8 press_num_flag = 1;
  char value[20] = {0};
  //  uint8 is_first_time = 1;
  //Ѱ��������һ�ſ��Ͳ��������������

  ret = sp_read_card(ctx, the_card, SP_READ_FILE10 | SP_READ_FILE12 | SP_READ_FILE15 | SP_READ_FILE16 | SP_READ_FILE18 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_halt(ctx, the_card);
    return ret;
  }
  ret = do_check_card_validity(ctx, the_card);
  if(ret)
    return ret;
  while(1)
  {
    //��ʾ�����û�������ѧ��
    sp_disp_by_type(SP_TP_USER_BALANCE, ctx, NULL);
    ret = check_and_input_money(ctx, &amount, the_card, value, clear_value_flag, &press_num_flag);
    if(ret)
    {
      if(SP_R_CANCEL_CONSUME == ret)
        return ret;
      //û������ּ������,����һ�ſ�Ҳ�൱��ȡ��������
      if((press_num_flag) && (SP_E_NOT_SAME_CARD == ret))
        return SP_R_CANCEL_CONSUME;
      while(1)
      {
        SP_CLS_FULLSCREEN;
        if(SP_E_NOT_SAME_CARD == ret)
        {
          sp_print_row(FLAG_DO_NOT_CLEAR, 0, 0, "����ͬһ�ſ�");
          clear_value_flag = 1;
        }
        else
        {
          clear_value_flag = 0;
        }
        sp_print_row(FLAG_DO_NOT_CLEAR, 1, 0, "������ˢ��>>");
        ret = do_wait_reload_card(ctx, &ctx->card);
        if(SP_R_CANCEL_CONSUME == ret)
          return SP_R_CANCEL_CONSUME;
        else if(SP_SUCCESS == ret)
          break;
      }
    }
    else
      break;
  }
  //sp_disp_debug("%d,%8x", money, money);
  if(amount <= 0)
  {
    sp_disp_msg(10, "������Ϊ0 ,���׳���");
    return SP_R_CANCEL_CONSUME;
  }
  the_card->amount = amount;
  return 0;
}
static int check_number_key(uint8* t)
{
  int key;
  key = sp_get_key();
  if(key == SP_KEY_FUNC)
    return SP_R_PRESS_FUNC;
  //sp_disp_debug("key=%02x,0=%02x,9=%02x", key, SP_KEY_0, SP_KEY_9);
  //if(sp_is_number_key(key, &t))
  if(sp_is_number_key(key, t))
    return SP_NO_NUMBER_KEY_PRESSED;
  else
    return 0;
}
static int do_prepare_pay_without_card(sp_context* ctx, sp_card* the_card)
{
  int ret = 0;
  uint8 t;
  uint32 amount = 0;
  ret = check_number_key(&t);
  if(ret)
    return ret;
  //������
  if(do_input_money_without_card(ctx, &amount, t))
    return SP_R_CANCEL_CONSUME;
  if(amount <= 0)
  {
    sp_disp_msg(5, "������Ϊ0 ,���׳���");
    return SP_R_CANCEL_CONSUME;
  }

  the_card->amount = amount;
  //���ǰ�������������һֱѰ��
  while(sp_request_card_poweron(the_card))
  {
    sp_disp_by_type(SP_TP_REQUEST_CONSUME_CARD, ctx, NULL);
    if(SP_KEY_CLEAR == sp_get_key())
      return SP_R_CANCEL_CONSUME;
  }
  ret = sp_read_card(ctx, the_card, SP_READ_FILE10 | SP_READ_FILE12 | SP_READ_FILE15 | SP_READ_FILE18 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_halt(ctx, the_card);
    return ret;
  }
  ret = do_check_card_validity(ctx, the_card);
  if(ret)
    return ret;
  //sp_request_card(the_card->cardphyid);
  return 0;
}

/*
*  ����ִ�в���:
*   1���ȼ�һ�����ˮ
*   2�����ѳ�ʼ���Ѿ�����ȷ�ϣ�����ȷ�ϻ᷵��tac��
*   3����������ȷ�ϳɹ�����¼ʣ�ಿ��(�������9�ֽ�)
*/
static int do_purchase_and_write_transdtl(sp_context * ctx, sp_card* the_card)
{
  int ret = 0;
  int32 last_transdtl_addr = 0;
  ret = prepare_transdtl(ctx, the_card, &ctx->record);
  if(ret)
    return ret;
  ret = sp_write_transdtl(&ctx->record);
  if(ret)
  {
    sp_disp_error("������ˮʧ��,ret=%d", ret);
    return ret;
  }
  last_transdtl_addr = sp_get_last_transaddr();//������ˮ�ĵ�ַ
  the_card->is_halfway = 0;//��;�ο����
  ret = do_purchase(ctx, the_card);
  if(ret)
  {
    sp_beep(CONSUME_FAIL_BEEP);
    return ret;
  }
  //12�������9���ֽ�
  ret = sp_update_left_transdtl_info(last_transdtl_addr, &(ctx->record));
  if(ret)
  {
    sp_disp_error("�������Ÿ��ֽ�ʧ�ܣ�ret=%04x", ret);
    return ret;
  }
  //  sp_disp_debug( "������ˮ�ɹ�,��ˮ��=%d", transdtl.termseqno);
  return 0;
}



static int do_calc_paymoney(sp_context* ctx, sp_card* card)
{
  int rate = 0;
  rate = ctx->feerate_table[card->feetype - 1];
  card->payamt = sp_d2i((card->amount * rate) * 1.0 / 100.0);
  return 0;
}

static void do_update_card_day_sum_amt(sp_context* ctx, sp_card* the_card)
{
  //������ڸ�������������һ�ν�������
  //sp_disp_debug("lastdate=[%02x%02x%02x%02x]", the_card->last_trans_date[0], the_card->last_trans_date[1], the_card->last_trans_date[2], the_card->last_trans_date[3]);
  if(memcmp(the_card->last_datetime, ctx->today, 4) != 0)
  {
    memcpy(the_card->last_datetime, ctx->today, 4);
    the_card->day_sum_amt = 0;
    sp_disp_debug("today=[%02x%02x%02x%02x],reset date", ctx->today[0], ctx->today[1], ctx->today[2], ctx->today[3]);
  }
  the_card->day_sum_amt += the_card->payamt;
}

static int do_purchase_with_card(sp_context* ctx, sp_card* card, uint8 card_reset_flag)
{
  int ret;
  sp_card current_card;
  memset(&current_card, 0, sizeof current_card);
  /*
  //�۷Ѳ���(��Ҫ������һ������)
  ret = sp_request_card_poweron(&current_card);
  if(ret)
  {
    sp_disp_error("����Ѱ��ʧ��");
    return SP_R_CARD_NOT_FOUND;
  }
  if(!SP_IS_SAME_CARDPHYID(the_card->cardphyid, current_card.cardphyid))
  {
    return SP_E_NOT_SAME_CARD;
  }
  */
  if(card_reset_flag)
  {
    ret = sp_cpu_reset_card();
    if(ret)
    {
      sp_disp_error("sp_cpu_reset_card error,ret=%04x", ret);
      return ret;
    }
    SP_CLS(0);
    SP_PRINT(0, 0, "���Ѱ����ɹ�!!");
  }

  if(card->cardbefbal <= ctx->syspara.min_cardbal)
  {
    //��ʾ�ÿ�����
    sp_disp_by_type(SP_TP_INSUFFICIENT_BALANCE, ctx, NULL);
    return SP_E_LESS_THAN_MIN_CARD_BALANCE;
  }
  ret = sp_select_card_app(ctx);
  if(ret)
    return ret;
  //�жϿ�״̬
  if(CARD_STATUS_NORMAL != card->status)
  {
    return SP_E_CARD_STATUS;
  }
  // DO:�����ۿۺ����ʵ���ѽ��
  ret = do_calc_paymoney(ctx, card);
  if(ret)
  {
    sp_disp_error("�����ۿ�ʧ��, ret=%04x", ret);
    return ret;
  }

  // DO:  �ж������޶�
  ret = do_check_consume_limit(ctx, card);
  if(ret)
  {
    sp_disp_error("�����޶ret=%04x", ret);
    return ret;
  }

  //���Ѳ�����ˮ
  ret = do_purchase_and_write_transdtl(ctx, card);
  if(ret)
  {
    sp_disp_error("write card error,ret= %04x", ret);
    return ret;
  }
  // DO:���¿������Ϣ
  do_update_card_day_sum_amt(ctx, card);

  ret = sp_write_card(card, ctx);
  if(ret)
  {
    Beep(100);//����
    sp_disp_msg(30, "������������ˢ��");
    // TODO: �ȴ��û�ˢ����Ѱ���ɹ���д�����0x10�ļ�����Ϣ
    ret = do_wait_reload_card(ctx, card);
  }
  else
  {
    do_update_sum_amt(ctx, &(ctx->record));
  }

  sp_disp_by_type(SP_TP_CONSUME_SUCCESS, ctx, NULL);

  sp_beep(CONSUME_SUCCESS_BEEP);
  return 0;
}

int sp_consume_loop(sp_context* ctx)
{
  int ret = 0;
  uint8 card_reset_flag = 0;

  memset(&ctx->card, 0 , sizeof(ctx->card));
  sp_disp_by_type(SP_TP_REQUEST_CARD, ctx, NULL);

  //�Ƿ�Ѱ��������������
  ret = sp_request_card_poweron(&ctx->card);
  if(ret)
  {
    ret = do_prepare_pay_without_card(ctx, &ctx->card);
    card_reset_flag = 0;
  }
  else
  {
    ret = do_prepare_pay_with_card(ctx, &ctx->card);
    card_reset_flag = 1;
  }
  if(ret)
  {
    return ret;
  }
  ret = do_purchase_with_card(ctx, &ctx->card, card_reset_flag);
  //����ret�����;����Ƿ��ϴ���ˮ
  if(SP_SUCCESS == ret)
  {
    ret = sp_send_rt_transdtl(ctx, &ctx->record);
    if(ret)
    {
      sp_disp_error("�ϴ���ˮʧ��,ret=%d", ret);
      return ret;
    }
    //�����ڴ��е���ˮ�洢��
    memset(&ctx->record, 0, sizeof(sp_transdtl));
    memset(&ctx->exception_record, 0, sizeof(sp_transdtl));
  }
  else
  {
    if(E_INSUFFICIENT_BALANCE == ret)
    {
      //��ʾ����
      sp_disp_by_type(SP_TP_INSUFFICIENT_BALANCE, ctx, NULL);
    }
  }
  if(ret)
    return ret;
  return 0;
}