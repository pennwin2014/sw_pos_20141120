

#include "rfmod.h"


typedef uint16(* adpu_device_func)(adpu_cmd_t* cmd);

/////////////////////////////////////////////////////////////////////////////////////////
static uint16 sam_adpu_proxy(adpu_cmd_t* cmd)
{
  /*
    *******************************************************************************
    **函数名称：CosCommand
    **函数功能：PSAM 卡 Apdu 指令操作函数
    **入口参数：
    **pSData :发送数据指针，cSNum：发送数据长度
    **出口参数:pRData :接收数据指针，cRNum：接收数据长度指针
    **返回：COS 指令返回码
    *******************************************************************************
    */
  uint16 ret = 0;//, i = 0;
  // char show_msg[1000];
  ret = CosCommand(cmd->send_buf, cmd->send_len, cmd->recv_buf, &cmd->recv_len);
  /*
    memset(show_msg, 0 , 1000);
    for(i = 0; i < cmd->send_len; i++)
    {
      sprintf(show_msg + 2 * i, "%02x", cmd->send_buf[i]);
    }
    sprintf(show_msg + 2 * i, "  [sendlen=%d]", cmd->send_len);
    sp_wait_for_retkey(SP_KEY_CONFIRM, __FUNCTION__, __LINE__, show_msg);

    memset(show_msg, 0 , 1000);
    for(i = 0; i < cmd->recv_len; i++)
    {
      sprintf(show_msg + 2 * i, "%02x", cmd->recv_buf[i]);
    }
    sprintf(show_msg + 2 * i, "  [ret=%04x]{recvlen=%d}", ret, cmd->recv_len);
    sp_wait_for_retkey(SP_KEY_CONFIRM,  __FUNCTION__, __LINE__, show_msg);

  */
  cmd->sw = ret;
  return ret;

}

static uint16 cpu_adpu_proxy(adpu_cmd_t* cmd)
{
  /*
  Name: CpuCard_Apdu
  Function:应用协议数据单元信息交换函数
  InPara: cCID:固定为 0x0F 不使用 CID
  cSendLen:发送数据长度
  cSendBuf:发送数据内容
  OutPara: cRcvLen: 接收数据长度
  cRcvBuf: 接收数据内容
  return: 命令执行成功 0x9000
  发送命令失败 0x0001
  接收长度错误 0x0002
  协议错误 0x0003
  CID 超过最大值 0x0004
  其它的值为错误代码

  */
  uint8 sw1, sw2;
  uint16 ret = 0;
  ret = CpuCard_Apdu(0x0F, cmd->send_len, cmd->send_buf, &cmd->recv_len, cmd->recv_buf);
  sw1 = cmd->recv_buf[cmd->recv_len - 2];
  sw2 = cmd->recv_buf[cmd->recv_len - 1];
  cmd->sw = (sw1 << 8) | sw2;
  return ret;
}

static int do_adpu_command(adpu_cmd_t* cmd, adpu_device_func func)
{
  func(cmd);
  if(cmd->sw != 0x9000)
    return cmd->sw;
  return 0;
}

int sp_cpu_adpu_command(adpu_cmd_t* cmd)
{
  return do_adpu_command(cmd, cpu_adpu_proxy);
}
int sp_sam_adpu_command(adpu_cmd_t* cmd)
{
  return do_adpu_command(cmd, sam_adpu_proxy);
}

int sp_cpucard_poweron(void)
{
  int ret = 0;
  sp_cpucard_deselect();//不关心返回值
  ret = sp_cpu_reset_card();
  if(ret)
  {
    //sp_disp_error("CPU ResetCard_A fail");
    return 1;
  }

  return 0;
}
int sp_cpucard_deselect(void)
{
  int ret = 0;
  ret = Deselect_A();
  if(FM1715_NOTAGERR == ret)
  {
    //sp_disp_error("CPU Deselect_A fail");
  }
  else if(FM1715_OK == ret)
  {
    //sp_disp_error("CPU Deselect_A ok");
  }
  else
  {
    sp_disp_error("CPU Deselect_A else, ret = %d", ret);
  }
  return ret;
}

int sp_cpu_reset_card()
{
  int ret = 0;
 // uint8 cSendBuf[7] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0x3F, 0x00};
//  uint8 cRcvBuf[64], cRcvLen;
  //SYSTIME tm;
  ret = (ResetCard_A() == TRUE) ? 0 : 1;
  /*
  sp_disp_error("ret=%04x,cnt=%d", ret, ctx->reset_cpucard_count);
  if(ret == TRUE)
  {
    if(ctx->reset_cpucard_count == 0)
    {
      ctx->reset_cpucard_count++;
      sp_select_df03();
      sp_deselect_card();
      sp_get_hdtime(&tm);
      sp_reset_time(ctx);
      sp_reset_time(ctx);
    }
  }
  else
  {
    sp_disp_error("ret=%d", ret);
  }
  */
  if(ret)
    return ret;

  //ret = CpuCard_Apdu(0x0f, 7, cSendBuf, &cRcvLen, cRcvBuf);
  //sp_disp_error("select 3f00, ret=%04x", ret);
  //Deselect_A();
  //Reset_Reader_TimeOut();

  return 0;//ret == 0 ? 1 : 0;
}

int sp_cpu_deselect_card(void)
{
  return Deselect_A() == 0 ? 1 : 0;
}

int sp_cpu_select_df03(void)
{
  uint8 cSendBuf[7] = {0x00, 0xA4, 0x00, 0x00, 0x02, 0xDF, 0x03};
  uint8 cRcvBuf[64], cRcvLen;
  return CpuCard_Apdu(0x0f, 7, cSendBuf, &cRcvLen, cRcvBuf);
}


int sp_cpu_reset_time(sp_context* ctx)
{
  int ret = 0;
  SYSTIME tm;
  ret = sp_cpu_reset_card();
  sp_disp_error("reset,ret=%04x", ret);
  ret = sp_cpu_select_df03();
  sp_disp_error("select df03,ret=%04x", ret);
  ret = sp_cpu_deselect_card();
  sp_disp_error("deselect,ret=%04x", ret);
  ret = sp_get_hdtime(&tm);
  sp_disp_error("sp_get_hdtime,ret=%04x", ret);
  return ret;
}

int sp_read_uid(uint8* uid)
{
  return ReadICCardUID(uid);
}

uint8 sp_hl_active(uint8 Block_Adr, uint8 hKey, uint8 const *cKey)
{
  return HL_Active(Block_Adr, hKey, cKey) == 0 ? 1 : 0;
}

uint8 sp_m1_readblock(uint8 *buff, uint8 Block_Adr)
{
  return MIF_ReadBlock(buff, Block_Adr) == 0 ? 1 : 0;
}

bool sp_m1_readcard(uint8 cBlock, uint8 hKey, uint8 const *cKey, uint8 *pData, uint8 cTry)
{
  return MIF_ReadCard(cBlock, hKey, cKey, pData, cTry);
}

uint8 sp_request(uint8 mode)
{
  return Request(mode);// == 0 ? 1 : 0;
}



