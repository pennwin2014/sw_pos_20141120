#include "sp_transfer.h"

static int do_send_recv_by_cmd(sp_context* ctx, uint8 cmd, uint16 time_out)
{
  int ret = 0;
  uint8 time_out_cnt = time_out / 100;
  ret = sp_send_request(ctx, cmd, NULL, time_out);
  if(ret)
    return ret;
  if(ret)
  {
    sp_disp_error("send cmd=[%02x] ret=%04x", cmd, ret);
    return ret;
  }
  while(1)
  {
    ret = sp_prcocess_message(ctx);
    if(ret == SP_SUCCESS)
      break;
    if(time_out_cnt -- <= 0)
      return SP_R_WAIT_TIMEOUT;
    sp_sleep(100);
  }
  return 0;
}

int sp_send_rt_transdtl(sp_context* ctx, sp_transdtl* ptransdtl)
{
  sp_send_request(ctx, SP_CMD_RT_TRANSDTL, ptransdtl, 3000);
  return 0;
}

int sp_download_feerate(sp_context* ctx)
{
  int ret = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_GET_FEERATE, 3000);
  if(ret)
    return ret;
  return 0;
}

int sp_download_syspara(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_GET_SYSPARA, 3000);
}

int sp_download_timepara(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_GET_TIMEPARA, 3000);
}

int sp_send_test(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_TEST, 500);
}

int sp_download_blacklist(sp_context* ctx)
{
  return do_send_recv_by_cmd(ctx, SP_CMD_GET_BLACKLIST, 500);
}

int sp_auth(sp_context* ctx)
{
  int ret = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_AUTH, 2000);
  if(ret)
    return ret;
  
  //���ְ汾��һ�£���ȡ�µİ汾�ķ���
  if(ctx->svrpara.feepara_verno != ctx->syspara.feepara_verno)
  {
    ret  = sp_download_feerate(ctx);
    if(ret)
    {
      return ret;
    }
    ctx->syspara.feepara_verno = ctx->svrpara.feepara_verno;
    // DO:����֧�ֿ����λͼ��flash�еĴ洢
    memcpy(ctx->syspara.feetype_bitmap, ctx->svrpara.feetype_bitmap, 32);
      ret = sp_write_syspara(ctx);
    if(ret)
      return ret;

    ret = sp_write_feerate_table(ctx);
    if(ret)
      return ret;
  }
  //ʱ��β����汾��
  if(ctx->svrpara.timepara_verno != ctx->syspara.timepara_verno)
  {
    ret = sp_download_timepara(ctx);
    if(ret)
      return ret;
    ctx->syspara.timepara_verno = ctx->svrpara.timepara_verno;
    ret = sp_write_timepara_table(ctx);
    if(ret)
      return ret;
  }

  return 0;
}


int sp_send_heartbeat(sp_context* ctx)
{
  int ret = 0;
  ret = do_send_recv_by_cmd(ctx, SP_CMD_HEARTBEAT, 500);
  if(ret)
    return ret;
  //�ȽϺ������汾��
  if(memcmp(ctx->syspara.blacklist_verno, ctx->svrpara.blacklist_verno, 6) != 0)
  {
    //���º�����
    ret = sp_download_blacklist(ctx);
    if(ret)
      return ret;
  }
  return 0;
}
int sp_check_deviceid(sp_context* ctx)
{
  ctx->syspara.enable_staus = SP_SUCCESS;//SP_S_DEVICEID_NOT_MATCH;
  return 0;
}
uint8 sp_recover_system(sp_context* ctx)
{
  uint8 choice = 0;
  int ret = 0;
  //��ʾ�Ƿ��ʼ���豸
  sp_disp_by_type(SP_TP_IS_INIT_DEVICE, ctx, &choice);
  if(choice)
  {
    sp_disp_msg(999, "��ȡ����ʼ���豸,�豸����ʹ��");
    return SP_R_CANCEL_CONSUME;
  }
  //�ָ���������
  ret = sp_recover_device(ctx);
  if(ret)
  {
    sp_disp_error("�ָ���������ʧ��!!,ret=%04x", ret);
    return ret;
  }
  //�����豸
  ret = sp_enable_device(ctx);
  if(ret)
  {
    sp_disp_error("�����豸ʧ��!!,ret=%04x", ret);
    return ret;
  }
  return 0;
}

int sp_enable_device(sp_context* ctx)
{
  int ret = 0;
  SP_CLS_FULLSCREEN;
  SP_PRINT(1, 0, "���������豸...");
  ret = sp_check_deviceid(ctx);
  if(ret)
    return ret;
  ret = sp_download_syspara(ctx);
  if(ret)
    return ret;
  ret = sp_download_feerate(ctx);
  if(ret)
    return ret;
  ret = sp_download_blacklist(ctx);
  if(ret)
    return ret;
  ret = sp_auth(ctx);
  if(ret)
    return ret;
  SP_PRINT(1, 0, "�����豸�ɹ�!! ");
  ctx->syspara.enable_staus = SP_SUCCESS;
  return 0;
}



