#include "sp_menu.h"
#include <time.h>
//////////////////////工具函数/////////////////
static int do_check_sys_passwd(sp_context* ctx, char input_password[MAX_SCREEN_ROWS])
{
  int ret = 0;
  while(1)
  {
    ret = sp_input_password(ctx, "请输入管理密码", input_password, SYSTEM_PASSWORD_LEN);
    if(ret)
      return SP_E_SYS_PASSWD;
    if(memcmp(ctx->syspara.password, input_password, 6) != 0)
    {
      sp_disp_error("管理密码错误,sys=[%02x%02x%02x%02x%02x%02x],in=[%02x%02x%02x%02x%02x%02x]", ctx->syspara.password[0],
                    ctx->syspara.password[1], ctx->syspara.password[2], ctx->syspara.password[3], ctx->syspara.password[4], ctx->syspara.password[5],
                    input_password[0], input_password[1], input_password[2], input_password[3], input_password[4], input_password[5]);
      continue;
    }
    break;
  }
  return 0;
}

//////////////////////菜单相关//////////////////////
static void do_set_password(sp_context* ctx)
{
  char input_password[MAX_SCREEN_ROWS];
  char renew_password[MAX_SCREEN_ROWS];
  int ret = 0;
  if(do_check_sys_passwd(ctx, input_password))
    return;
  while(1)
  {
    //请输入新密码
    while(1)
    {
      ret = sp_input_password(ctx, "请输入新密码", input_password, SYSTEM_PASSWORD_LEN);
      if(ret)
        return;
      if(strlen(input_password) != SYSTEM_PASSWORD_LEN)
      {
        sp_disp_msg(6, "密码长度错误");
        continue;
      }
      break;
    }
    //再次输入新密码
    ret = sp_input_password(ctx, "再次输入新密码", renew_password, SYSTEM_PASSWORD_LEN);
    if(ret)
      return;
    if(memcmp(renew_password, input_password, SYSTEM_PASSWORD_LEN) != 0)
    {
      sp_disp_msg(6, "两次密码不一致");
      continue;
    }
    break;
  }
  //修改密码成功
  memcpy(ctx->syspara.password, renew_password, 6);
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("更新系统信息flash失败，ret=%04x", ret);
    return;
  }
  sp_disp_msg(6, "修改密码成功!! ");
}
static void do_set_termno(sp_context* ctx)
{
  int ret = 0;
  char termno[MAX_SCREEN_ROWS];
  char sys_password[MAX_SCREEN_ROWS];
  char tmp[3];
  int i = 0, j = 0;
  ret = sp_input_words(ctx, "终端编号: ", termno, 8);
  if(ret)
    return;
  sp_disp_debug("%02x%02x%02x%02x%02x%02x%02x%02x", termno[0], termno[1], termno[2], termno[3], termno[4],
                termno[5], termno[6], termno[7]);
  if(do_check_sys_passwd(ctx, sys_password))
    return;
  tmp[2] = 0;
  for(i = 0; i < 4; i++)
  {
    tmp[0] = termno[i * 2];
    tmp[1] = termno[i * 2 + 1];
    j  = atoi(tmp);
    sp_uint8_to_bcd(j, ctx->syspara.termno + i);
  }
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("重写系统信息失败, ret=%04x", ret);
    return;
  }
  sp_disp_msg(8, "终端编号设置成功!! ");
}
static void do_set_communicate(sp_context* ctx)
{
  int ret = 0;
  char canid[MAX_SCREEN_ROWS];
  char sys_password[MAX_SCREEN_ROWS];
  ret = sp_input_words(ctx, "终端机号: ", canid, 8);
  if(ret)
    return;
  if(do_check_sys_passwd(ctx, sys_password))
    return;
  ctx->syspara.canid = atoi(canid);
  ret = sp_write_syspara(ctx);
  if(ret)
  {
    sp_disp_error("重写系统信息失败, ret=%04x", ret);
    return;
  }
  sp_disp_msg(8, "通讯参数设置成功!! ");
}
static int do_check_unconfirm_transdtl(sp_context* ctx)
{
  return 0;
}

static void do_set_recover(sp_context* ctx)
{
  int ret = 0;
  uint8 choice = 0;
  char input_password[MAX_SCREEN_ROWS];
  if(do_check_sys_passwd(ctx, input_password))
    return;
  // TODO: 检查一下是否有未上传的流水
  if(do_check_unconfirm_transdtl(ctx))
  {
    sp_disp_by_type(SP_TP_DISP_UNCONFIM_TRANSDTL, ctx, &choice);
    if(choice)
      return;
  }
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在恢复... ");
  ret = sp_recover_system(ctx);
  if(ret)
    return;
  SP_PRINT(1, 2, "恢复成功!! ");
  sp_sleep(3000);
}
static void do_set_clear_local_transdtl(sp_context* ctx)
{
  sp_disp_msg(8, "清空本地流水");
}
static void do_set_clear_blacklist(sp_context* ctx)
{
  sp_disp_msg(8, "清空黑名单");
}

static void do_consume_fixed_value(sp_context* ctx)
{
  sp_disp_msg(1000, "使用定值消费");
}

#define SP_HEART_BEART_SECONDS 500
static int check_heart_beat_time(sp_context* ctx)
{
  /*
  static uint32 old_tm = 0;
  uint32 new_tm = 0;
  new_tm = GetTickCount();
  */
  static uint16 gap_secs = 0;
  if(gap_secs++ > SP_HEART_BEART_SECONDS)
  {
    gap_secs = 0;
    return 1;
  }
  return 0;
}

static void react_by_type(sp_context* ctx, uint8 err_code)
{
  switch(err_code)
  {
    case SP_E_CARD_TYPE_NOT_SUPPORT:
      sp_disp_msg(3, "该卡类别不支持");
      ctx->disp_type = 0;
      break;
    case SP_NO_NUMBER_KEY_PRESSED:
    case SP_R_CANCEL_CONSUME:
      break;
    default:
      sp_disp_error("消费错误, 错误码=%04x", err_code);
      ctx->disp_type = 0;
      break;
  }
}

static void do_consume_normal(sp_context * ctx)
{
  int ret = 0;
  int32 key = 0;
  ctx->disp_type = 0;
  SP_CLS_FULLSCREEN;
  while(1)
  {
    sp_sleep(10);
    key = sp_get_key();
    if(SP_KEY_FUNC == key)
      return;
    //检查是否有存储空间
    ret = sp_pre_check(ctx);
    if(ret)
    {
      sp_disp_msg(10, "存储空间不足");
      continue;
    }
    ret = sp_check_date_change(ctx);
    if(ret)
    {
      sp_disp_error("致命错误, ret=%04x,[%02x%02x%02x%02x]->[%02x%02x%02x%02x%02x%02x%02x]", ret,
                    ctx->syspara.today_date[0], ctx->syspara.today_date[1], ctx->syspara.today_date[2], ctx->syspara.today_date[3],
                    ctx->today[0], ctx->today[1], ctx->today[2], ctx->today[3], ctx->current_datetime[4], ctx->current_datetime[5], ctx->current_datetime[6]);
      //sp_sleep(99999);
      sp_select_card_app(ctx);
    }
    ret = sp_prcocess_message(ctx);
    if(ret)
    {
      sp_disp_debug("proc msg,ret=%04x", ret);
    }
    ret = sp_consume_loop(ctx);
    if(SP_R_PRESS_FUNC == ret)
    {
      return;
    }
    if(ret)
    {
      react_by_type(ctx, ret);
    }

    Reset_Reader_TimeOut();
    //500ms或1S刷新一次 从Flash读取点阵需要时间可放定时器回调函数处理
    //喂狗定义防止溢出程序复位
    //  KillWatchDog();
    if(check_heart_beat_time(ctx))
    {
      //发送心跳
      ret = sp_send_heartbeat(ctx);
      if(ret)
        ctx->online_flag = false;
      else
        ctx->online_flag = true;
    }
  }
}


static void do_menu_sign(sp_context* ctx)
{
  int32 key = 0;
  int ret = 0;
  SP_CLS_FULLSCREEN;
  while(1)
  {
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_CLEAR:
        return;
      case SP_KEY_0:
        SP_PRINT(1, 0, "清空展示区");
        SP_CLS(2);
        SP_CLS(3);
        break;
      case SP_KEY_1:
        SP_PRINT(1, 0, "上传流水请求");
        SP_CLS(2);
        ret = sp_send_request(ctx, SP_CMD_RT_TRANSDTL, &ctx->record, 10000);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        ret = sp_prcocess_message(ctx);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        break;
      case SP_KEY_2:
        SP_PRINT(1, 0, "签到请求");
        SP_CLS(2);
        ret = sp_send_request(ctx, SP_CMD_AUTH, NULL, 10000);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        sp_sleep(1000);
        ret = sp_prcocess_message(ctx);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        break;
      case SP_KEY_3:
        SP_PRINT(1, 0, "获取黑名单请求");
        SP_CLS(2);
        ret = sp_send_request(ctx, SP_CMD_GET_BLACKLIST, NULL, 10000);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        ret = sp_prcocess_message(ctx);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        break;
      case SP_KEY_4:
        ret = sp_prcocess_message(ctx);
        if(ret)
        {
          sp_disp_error("ret=%04x", ret);
          continue;
        }
        break;
      case SP_KEY_5:
        sp_disp_error("size of head=%d", sizeof(sp_tcp_header));
        break;
      case SP_KEY_6:
        sp_print_row(FLAG_CLEAR, 0, 0, "%02x%02x%02x%02x%02x%02x%02x%02x", ctx->recv_data[0],  ctx->recv_data[1],  ctx->recv_data[2],  ctx->recv_data[3],
                     ctx->recv_data[4],  ctx->recv_data[5],  ctx->recv_data[6],  ctx->recv_data[7]);
        sp_print_row(FLAG_CLEAR, 1, 0, "%02x%02x%02x%02x%02x%02x%02x%02x", ctx->recv_data[8],  ctx->recv_data[9],  ctx->recv_data[10],  ctx->recv_data[11],
                     ctx->recv_data[12],  ctx->recv_data[13],  ctx->recv_data[14],  ctx->recv_data[15]);
        sp_print_row(FLAG_CLEAR, 2, 0, "%02x%02x%02x%02x", ctx->recv_data[16],  ctx->recv_data[17],  ctx->recv_data[18],  ctx->recv_data[19]);
        sp_print_row(FLAG_CLEAR, 3, 0, "%d-%d-%02x-%02x", ctx->collect_record_ptr,
                     ctx->recv_len, ctx->crc1, ctx->crc2);
        break;
      case SP_KEY_7:
        SP_CLS_FULLSCREEN;
        SP_PRINT(1, 2, "正在签到...");
        ret = sp_auth(ctx);
        if(ret)
        {
          sp_disp_msg(3, "签到失败, ret=%04x", ret);
          break;
        }
        sp_print_row(FLAG_CLEAR, 1, 2, "签到成功!!");
        break;
    }
  }
}
static void do_menu_show_sysinfo(sp_context* ctx)
{
  //sp_disp_flash(ADDR_SYSINFO, sizeof(sp_syspara));
  uint8 upd_flag = 1;
  uint8 index = 0;
  int key = 0;
  uint8 page_count = SHOW_SYSTEM_INFO_COUNT;
  ctx->transno = sp_get_transno();
  while(1)
  {
    if(upd_flag)
    {
      sp_disp_by_type(SP_TP_SHOW_SYSTEM_INFO, ctx, &index);
      upd_flag = 0;
    }
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_CLEAR:
        return;
      case SP_KEY_NEXT:
        if(index < page_count - 1)
        {
          ++index;
          upd_flag = 1;
        }
        break;
      case SP_KEY_PREV:
        if(index > 0)
        {
          --index;
          upd_flag = 1;
        }
        break;
    }

  }
}
static void do_menu_disp_transdtl(sp_context * ctx)
{
  uint8 ret = 0;
  int i = 0;
  int start_pos = 0;
  sp_transno_unit transno_unt;
  ret = sp_read_transno_unit(TRANSNO_FLAG_MASTER, &transno_unt);
  if(ret)
  {
    sp_disp_msg(999, "读取主流水号失败");
    return;
  }
  sp_disp_msg(999, "last_addr=%06x,last_no=%d", transno_unt.last_trans_addr, transno_unt.last_trans_no);
  if(transno_unt.last_trans_no > 2)
    start_pos = -2;
  for(i = start_pos; i < start_pos + 5; i++)
    sp_disp_flash(transno_unt.last_trans_addr + i * 64, 64); //打印五笔流水的数据
}

static void do_menu_disp_transno(sp_context * ctx)
{
  int i = 0;
  int unit_len = sizeof(sp_transno_unit);
  sp_disp_msg(999, "主流水号存储区数据");
  for(i = 0; i < FLASH_PAGE_SIZE / unit_len; i++)
    sp_disp_flash(ADDR_MASTER_TRANS_SEQNO + i * unit_len, unit_len);
  sp_disp_msg(999, "从流水号存储区数据");
  for(i = 0; i < FLASH_PAGE_SIZE / unit_len; i++)
    sp_disp_flash(ADDR_SLAVE_TRANS_SEQNO + i * unit_len, unit_len);
}



static int do_cancel_last_trans(sp_card * p_card)
{
  sp_disp_msg(20, "撤销上一笔消费");
  return 0;
}

static void do_menu_cancel(sp_context * ctx)
{
  int key = 0;
  int ret = 0;
  //char phyid_hex[33] = {0};
  sp_card this_card;
  sp_disp_by_type(SP_TP_REQUEST_CARD, ctx, NULL);
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    ret = sp_request_card(this_card.cardphyid);
    if(ret == 0)
      break;
  }
  if(CPUCARD == this_card.cardtype)
  {
    sp_disp_msg(6, "不支持消费撤销");
    return;
  }
  //重新寻卡
  ret = sp_request_card_poweron(&this_card);
  if(ret)
  {
    sp_disp_error("sp_request_card_purchase,ret=%04x", ret);
    return;
  }
  ret = sp_read_card(ctx, &this_card, SP_READ_FILE10 | SP_READ_FILE15 | SP_READ_FILE16 | SP_READ_CARDBAL);
  if(ret)
  {
    sp_disp_error("sp_read_card,ret=%04x", ret);
    return;
  }
  sp_disp_by_type(SP_TP_CANCEL_CONSUME, ctx, NULL);
  while(1)
  {
    key = sp_get_key();
    if(SP_KEY_CLEAR == key)
      return;
    if(SP_KEY_CONFIRM == key)
      do_cancel_last_trans(&this_card);
  }
}
static void do_menu_query_revenue(sp_context * ctx)
{
  int key = 0;
  uint8 init_disp_flag = 0;
  uint8 upd_flag = 1;
  while(1)
  {
    if(upd_flag)
    {
      SP_CLS_FULLSCREEN;
      SP_PRINT(1, 0, "1.当日营业额查询");
      SP_PRINT(2, 0, "2.昨日营业额查询");
      upd_flag = 0;
    }
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_1:
        init_disp_flag = 1;
        sp_disp_by_type(SP_TP_REVENUE_QUERY, ctx, &init_disp_flag);
        upd_flag = 1;
        break;
      case SP_KEY_2:
        init_disp_flag = 2;
        sp_disp_by_type(SP_TP_REVENUE_QUERY, ctx, &init_disp_flag);
        upd_flag = 1;
        break;
      case SP_KEY_CLEAR:
        return;
    }
  }
}
static void do_menu_update_blacklist(sp_context * ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "正在更新黑名单..");
  ret = sp_download_blacklist(ctx);
  if(ret)
  {
    sp_disp_msg(6, "更新黑名单失败，请稍后重试!! ");
    return;
  }
  sp_sleep(2000);
  sp_print_row(FLAG_CLEAR, 1, 3, "更新成功!! ");
  sp_sleep(1500);
}

static void do_menu_upload_transdtl(sp_context * ctx)
{

}

static void do_menu_test_net(sp_context * ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 2, "正在检测...");
  ret = sp_send_test(ctx);
  if(ret)
  {
    ctx->online_flag = false;
    sp_disp_msg(6, "链路检测失败");
    return;
  }
  ctx->online_flag = true;
  sp_print_row(FLAG_CLEAR, 1, 2, "通讯成功!!");
  sp_sleep(3000);
}

static void do_show_menu(const menu_info_t * menu_info, int menu_count, void * arg)
{
  int32 index = 0;
  int32 page_index = 0;
  int32 page_cnt = 0;
  int32 key = 0;
  int32 start = 0;
  bool need_upd = true;//是否需要刷新
  int32 i = 0, cnt = 0, last = 0;
  bool no_key_pressed = false;
  bool is_quit = false;
  page_cnt = ceil(menu_count * 1.0 / MAX_MENU_SCREEN_CNT);
  while(1)
  {
    if(need_upd)
    {
      SP_CLS_FULLSCREEN;
      start = page_index * MAX_MENU_SCREEN_CNT;
      cnt = 0;
      last = start + MAX_MENU_SCREEN_CNT;
      if(last > menu_count)
        last = menu_count;
      for(i = start; i < last; i++)
      {
        SP_PRINT(cnt++, 0, (char*)menu_info[i].menu_desc);
      }
      SP_PRINT(3, 2, "按+/ - 上下翻页");
      need_upd = false;
    }
    is_quit = false;
    no_key_pressed = true;
    key = sp_get_key();
    switch(key)
    {
      case SP_KEY_CLEAR:
        no_key_pressed = false;
        is_quit = true;
        return;
        // break;
      case SP_KEY_NEXT:
        if(page_index < page_cnt - 1)
        {
          page_index ++;
          need_upd = true;
        }
        break;
      case SP_KEY_PREV:
        if(page_index > 0)
        {
          page_index--;
          need_upd = true;
        }
        break;
      case SP_KEY_1:
        no_key_pressed = false;
        index = 1;
        break;
      case SP_KEY_2:
        no_key_pressed = false;
        index = 2;
        break;
      case SP_KEY_3:
        no_key_pressed = false;
        index = 3;
        break;
      case SP_KEY_4:
        no_key_pressed = false;
        index = 4;
        break;
      case SP_KEY_5:
        no_key_pressed = false;
        index = 5;
        break;
      case SP_KEY_6:
        no_key_pressed = false;
        index = 6;
        break;
      case SP_KEY_7:
        no_key_pressed = false;
        index = 7;
        break;
      case SP_KEY_8:
        no_key_pressed = false;
        index = 8;
        break;
      case SP_KEY_9:
        no_key_pressed = false;
        index = 9;
        break;
      case SP_KEY_0:
        no_key_pressed = false;
        index = 10;
        break;
      default:
        break;
    }
    if(no_key_pressed == false)
    {
      if((index > 0) && (index <= menu_count))
      {
        menu_info[index - 1].menu_func(arg);
        need_upd = true;
      }
      if(is_quit)
        break;
    }
  }
}
static void do_menu_set(sp_context* ctx)
{
  menu_info_t menus[] =
  {
    {"1. 管理密码修改", (menu_func_t)do_set_password},
    {"2. 终端编号设置", (menu_func_t)do_set_termno},
    {"3. 通讯参数设置", (menu_func_t)do_set_communicate},
    {"4. 恢复出厂参数", (menu_func_t)do_set_recover},
    {"5. 清空本地流水", (menu_func_t)do_set_clear_local_transdtl},
    {"6. 清空黑名单", (menu_func_t)do_set_clear_blacklist}
  };
  do_show_menu(menus, sizeof(menus) / sizeof(menus[0]), ctx);

}
static void do_menu_main(sp_context * ctx)
{
  menu_info_t menus[] =
  {
    {"1. 消费撤销", (menu_func_t)do_menu_cancel},
    {"2. 营业额查询", (menu_func_t)do_menu_query_revenue},
    {"3. 手工更新黑名单", (menu_func_t)do_menu_update_blacklist},
    {"4. 手动上传流水", (menu_func_t)do_menu_upload_transdtl},
    {"5. 链路检测", (menu_func_t)do_menu_test_net},
    {"6. 手工签到", (menu_func_t)do_menu_sign},
    {"7. 终端参数查询", (menu_func_t)do_menu_show_sysinfo},
    {"8. 管理功能", (menu_func_t)do_menu_set},
    {"9. 流水号存储区", (menu_func_t)do_menu_disp_transno},
    {"0. 流水存储区", (menu_func_t)do_menu_disp_transdtl}
  };
  do_show_menu(menus, sizeof(menus) / sizeof(menus[0]), ctx);
}

void sp_menu_consume(sp_context * ctx)
{
  while(1)
  {
    if(ctx->syspara.work_mode == SP_WORK_MODE_NORMAL)
      do_consume_normal(ctx);
    else if(ctx->syspara.work_mode == SP_WORK_MODE_FIXED_VALUE)
      do_consume_fixed_value(ctx);
    do_menu_main(ctx);
    //KillWatchDog();
  }
}






