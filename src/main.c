#include "config.h"
#include "Mifare.h"


#include "sp_communicate.h"
#include "sp_disp.h"

#include "sp_menu.h"
#include <stdio.h>

#define HD_POS HD_SC
static int do_init_hd_stuff(sp_context* ctx)
{
  InitBoard();
  //���Ź���ʼ��
#ifdef OPEN_WDT
  InitWatchDog();
#endif
  //��������
  sp_init_com();
  return 0;
}
static int do_init_sam_card(sp_context* ctx)
{
  int ret = 0;
  uint8 samno[6];
  //��ʼ��psam��
  ret = sp_init_sam_card(ctx, samno);
  if(ret)
    return ret;
  if(memcmp(samno, ctx->syspara.samno, 6) != 0)
  {
    memcpy(ctx->syspara.samno, samno, 6);
    ret = sp_write_syspara(ctx);
    if(ret)
    {
      sp_disp_error("samno write fail,samno=%s", samno);
      return ret;
    }
  }
  return 0;
}


static void do_auth(sp_context* ctx)
{
  int ret = 0;
  sp_print_row(FLAG_CLEAR, 1, 2, "����ǩ��...");
  ret = sp_auth(ctx);
  if(ret)
  {
    sp_disp_error("ǩ��ʧ��!!ret=%04x", ret);
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 2, "ǩ���ɹ�!! ");
  sp_sleep(2000);
}

int main()
{
  sp_context *main_ctx = (sp_context*)malloc(sizeof(sp_context));
  int ret = 0;
  ret = do_init_hd_stuff(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "��ʼ��������ʧ��!!��ȷ�ϼ��˳�����,ret=%04x", ret);
    return 1;
  }
  ret = sp_read_system(main_ctx);
  if(ret)
  {
    ret = sp_recover_system(main_ctx);
    if(ret)
      return ret;
  }

  //�Ѿ�init����Ҫ���������豸
  if(main_ctx->syspara.init_flag)
  {
    if(main_ctx->syspara.enable_staus == SP_S_DEVICEID_NOT_MATCH)
    {
      //��Ҫ���³�ʼ��
      ret = sp_recover_system(main_ctx);
      if(ret)
        return ret;
    }
    else if(main_ctx->syspara.enable_staus == SP_SUCCESS)
    {
      //ǩ����ʹʧ��Ҳ���Խ���ϵͳ
      do_auth(main_ctx);
    }
    else
    {
      //�����豸
      ret = sp_enable_device(main_ctx);
      if(ret)
      {
        sp_disp_error("�����豸ʧ��!!,ret=%04x", ret);
        main_ctx->syspara.enable_staus = ret;
        ret = sp_write_syspara(main_ctx);
        return ret;
      }
    }
  }

  // ��������Ƿ�仯
  ret = sp_check_date_change(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "������ڱ仯ʧ��,ret=%04x", ret);
    return 1;
  }
  //��ʼ��psam��
  ret = do_init_sam_card(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "��ʼ��psam��ʧ��!!��ȷ�ϼ��˳�����,ret=%04x", ret);
    return 1;
  }
  //���Ѳ˵�
  sp_menu_consume(main_ctx);
  free(main_ctx);
  return 0;
}
