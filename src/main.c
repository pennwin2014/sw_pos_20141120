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
  //看门狗初始化
#ifdef OPEN_WDT
  InitWatchDog();
#endif
  //串口设置
  sp_init_com();
  return 0;
}
static int do_init_sam_card(sp_context* ctx)
{
  int ret = 0;
  uint8 samno[6];
  //初始化psam卡
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
  sp_print_row(FLAG_CLEAR, 1, 2, "正在签到...");
  ret = sp_auth(ctx);
  if(ret)
  {
    sp_disp_error("签到失败!!ret=%04x", ret);
    return;
  }
  sp_print_row(FLAG_CLEAR, 1, 2, "签到成功!! ");
  sp_sleep(2000);
}

int main()
{
  sp_context *main_ctx = (sp_context*)malloc(sizeof(sp_context));
  int ret = 0;
  ret = do_init_hd_stuff(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "初始化汇多相关失败!!按确认键退出程序,ret=%04x", ret);
    return 1;
  }
  ret = sp_read_system(main_ctx);
  if(ret)
  {
    ret = sp_recover_system(main_ctx);
    if(ret)
      return ret;
  }

  //已经init的需要重新启用设备
  if(main_ctx->syspara.init_flag)
  {
    if(main_ctx->syspara.enable_staus == SP_S_DEVICEID_NOT_MATCH)
    {
      //就要重新初始化
      ret = sp_recover_system(main_ctx);
      if(ret)
        return ret;
    }
    else if(main_ctx->syspara.enable_staus == SP_SUCCESS)
    {
      //签到即使失败也可以进入系统
      do_auth(main_ctx);
    }
    else
    {
      //启用设备
      ret = sp_enable_device(main_ctx);
      if(ret)
      {
        sp_disp_error("启用设备失败!!,ret=%04x", ret);
        main_ctx->syspara.enable_staus = ret;
        ret = sp_write_syspara(main_ctx);
        return ret;
      }
    }
  }

  // 检查日期是否变化
  ret = sp_check_date_change(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "检查日期变化失败,ret=%04x", ret);
    return 1;
  }
  //初始化psam卡
  ret = do_init_sam_card(main_ctx);
  if(ret)
  {
    SP_WAIT_AND_DISPMSG(SP_KEY_CONFIRM, "初始化psam卡失败!!按确认键退出程序,ret=%04x", ret);
    return 1;
  }
  //消费菜单
  sp_menu_consume(main_ctx);
  free(main_ctx);
  return 0;
}
