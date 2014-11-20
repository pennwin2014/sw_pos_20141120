#define SP_GLOBALS

#include "sp_communicate.h"

//静态全局变量，存放网络相关的所有数据
static DCB sDCB;
static uint8 global_send_buffer[MAX_SENDMSG_LEN];
static sp_com_buffer_handler global_tcp_handler;
//////////////////////////////工具函数/////////////////////////////////////
static void get_std_ctl_len(uint8* ctl_len, uint16 msg_len)
{
  memcpy(ctl_len, &msg_len, 2);
}

static uint16 get_data_len(void * data_len_buffer)
{
  uint16 data_len = 0;
  memcpy(&data_len, data_len_buffer, 2);
  return data_len;
}

static int compare_tcp_checksum(sp_context* ctx, uint8* data, uint16 data_len)
{
  uint8 crc1, crc2;
  crc1 = data[data_len - 1];
  crc2 = sp_calc_crc8(data, data_len - 1);
  ctx->crc1 = crc1;
  ctx->crc2 = crc2;
  return (crc1 == crc2) ? 0 : 1;
  //return (data[data_len - 1] == sp_calc_crc8(data, data_len - 1)) ? 0 : 1;
}

static uint8 do_com_sendbuf(uint8 uComm, uint8 *pcSendBuf, int iLen)
{
  return (0 == COM_SendBuf(uComm, pcSendBuf, iLen)) ? 1 : 0;
}

static uint8 do_send_msg(const uint8 * msg, uint16 msg_len)
{
  uint8* send_msg = global_send_buffer;//[MAX_SENDMSG_LEN] = {0};
  uint8 ctl_len[2] = {0};
  int send_len = 0;
  if(msg_len > MAX_SENDMSG_LEN)
    return 2;
  send_len = msg_len + 2;
  get_std_ctl_len(ctl_len, msg_len);
  memcpy(send_msg, ctl_len, 2);
  memcpy(send_msg + 2, msg, msg_len);
  return do_com_sendbuf(COM1, send_msg, send_len);
}


///////////////////////////内部函数///////////////////////////////////
static void do_init_tcp_handler()
{
  global_tcp_handler.recv_buffer = (uint8*)malloc(TCP_RECV_BUFFER_SIZE);
  memset(global_tcp_handler.recv_buffer, 0, TCP_RECV_BUFFER_SIZE);
  global_tcp_handler.recv_len = 0;
  global_tcp_handler.current_tcp_seqno = 0;
}
static void clear_recv_buffer()
{
  //清空缓冲区
  memset(global_tcp_handler.recv_buffer, 0 , TCP_RECV_BUFFER_SIZE);
  global_tcp_handler.recv_len = 0;
}
static void do_parse_resp(sp_context* ctx, uint8* recv_buffer, uint16* buffer_len)
{
  uint16 data_len = 0;
  data_len = get_data_len(global_tcp_handler.recv_buffer);
  memcpy(ctx->recv_data, global_tcp_handler.recv_buffer, 20);
  if(global_tcp_handler.recv_len > 0)
  {
    /*
    sp_print_row(FLAG_CLEAR, 0, 0, "recv len=%d", global_tcp_handler.recv_len);
    sp_sleep(5000);
    */
    if(data_len > 0)
    {
      if(global_tcp_handler.recv_len - 2 >= data_len)
      {
        //crc一致就认为是收到了完整的信息
        if(!compare_tcp_checksum(ctx, global_tcp_handler.recv_buffer + 2, data_len))
        {
          global_tcp_handler.recv_len -= 2;
          (*buffer_len) = data_len;
          memcpy(recv_buffer, global_tcp_handler.recv_buffer + 2, data_len);
          //更新接收缓冲区数据
          global_tcp_handler.recv_len -= data_len;
          memcpy(global_tcp_handler.recv_buffer, global_tcp_handler.recv_buffer + 2 + data_len, global_tcp_handler.recv_len);
        }
      }
    }
  }

}


/*
* 作用:接收网络数据的回调函数
*/
static void do_receive_data(void *pBuf, uint16 nLen)
{
  //就加入接收缓冲区
  if(global_tcp_handler.recv_len + nLen > TCP_RECV_BUFFER_SIZE)
  {
    clear_recv_buffer();
    sp_print_row(FLAG_CLEAR, 2, 6, "cls buf");
    sp_sleep(10000);
  }
  //加入缓冲区
  memcpy(global_tcp_handler.recv_buffer + global_tcp_handler.recv_len, pBuf, nLen);
  global_tcp_handler.recv_len += nLen;
  /*
    //覆盖缓冲区
    global_tcp_handler.recv_len = nLen;
    memcpy(global_tcp_handler.recv_buffer, pBuf, nLen);
    */
}



///////////////////////////////处理函数////////////////////////////////////////////////
static void prepare_header_by_cmd(uint8 cmd, sp_tcp_header* header)
{
  memcpy(header->guide_code, "HSV", 3);
  header->cmd_code = cmd;
  memcpy(header->deviceno, "\x00\x01", 2);
  header->seqno = global_tcp_handler.current_tcp_seqno;
}
static int prepare_output_parameter(uint8* send_buffer, uint16 buffer_max_len, uint16* data_len, sp_tcp_trans_unit* record)
{
  uint16 offset = 0;
  (*data_len) = sizeof(sp_tcp_header) + record->header.data_len;
  if(*data_len > buffer_max_len - 1)
    return SP_E_BUFFER_SIZE_OUT_OF_RANGE;
  memcpy(send_buffer, &record->header, sizeof(sp_tcp_header));
  offset = sizeof(sp_tcp_header);
  memcpy(send_buffer + offset, record->data, record->header.data_len);
  return 0;
}
static void calc_send_buffer_checksum(uint8* send_buffer, uint16* data_len)
{
  send_buffer[*data_len] = sp_calc_crc8(send_buffer, *data_len - 1);
  *data_len = *data_len + 1;
}

PROTOCOL_FUNC_PREPARE(do_prepare_auth)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  int offset = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_AUTH, &tcp_struct.header);
  //组data部分
  //设备的物理编号
  memcpy(tcp_struct.data, ctx->syspara.termno, 4);
  offset += 4;
  //设备软件版本号
  memcpy(tcp_struct.data + offset, ctx->syspara.soft_verno, 3);
  offset += 3;
  // 时钟
  memcpy(tcp_struct.data + offset, ctx->current_datetime, 7);
  offset += 7;
  //费率版本号
  sp_uint32_to_buffer(ctx->syspara.feepara_verno, tcp_struct.data + offset);
  offset += 4;
  //主参数版本号
  sp_uint32_to_buffer(ctx->syspara.syspara_verno, tcp_struct.data + offset);
  offset += 4;
  tcp_struct.header.data_len = offset;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(auth_resp_handler)
{
  int offset;
  int ret = 0;
  uint8 need_upd_time = 0;
  uint32 feepara_verno = 0;
  uint32 syspara_verno = 0;
  uint32 timepara_verno = 0;
  SYSTIME svr_tm;
  sp_print_row(FLAG_CLEAR, 2, 0, "%02x%02x%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5],
               buffer[6], buffer[7]);
  offset = 0;
  need_upd_time = buffer[offset++];
  if(need_upd_time)
  {
    ret = sp_get_hdtime(&svr_tm);
    sp_timestr_to_systime(buffer + offset, &svr_tm);
    //sp_print_row(FLAG_CLEAR, 0, 0, "gap=%d", svr_tm - local_tm);
    ret = sp_set_hdtime(&svr_tm);
    if(ret)
    {
      return ret;
    }
  }
  offset += 7;
  //费率版本号
  sp_buffer_to_uint32(buffer + offset, &feepara_verno);
  ctx->svrpara.feepara_verno = feepara_verno;
  offset += 4;
  //系统参数版本号
  sp_buffer_to_uint32(buffer + offset, &syspara_verno);
  ctx->svrpara.syspara_verno = syspara_verno;
  offset += 4;
  //时间段参数版本号
  sp_buffer_to_uint32(buffer + offset, &timepara_verno);
  ctx->svrpara.timepara_verno = timepara_verno;
  offset += 4;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_transdtl)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_RT_TRANSDTL, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = sizeof(sp_transdtl);
  memcpy(tcp_struct.data, arg, tcp_struct.header.data_len);
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(transdtl_resp_handler)
{
  //更新流水指针
  sp_recv_rt_transdtl* rt_record = NULL;
  //sp_print_row(FLAG_CLEAR, 0, 0, "rt,len=%d", data_len);
  rt_record = (sp_recv_rt_transdtl*) buffer;
  ctx->collect_record_ptr = rt_record->record_addr;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_prepare_blacklist)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_BLACKLIST, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 1;
  tcp_struct.data[0] = 0x01;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}
PROTOCOL_FUNC_RESP(blacklist_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "处理黑名单返回");
  return 0;
}
PROTOCOL_FUNC_PREPARE(do_prepare_heartbeat)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_HEARTBEAT, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 10;
  memcpy(tcp_struct.data, ctx->current_datetime + 1, 6);
  sp_uint32_to_buffer(sp_get_transno(), tcp_struct.data + 6);
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(heartbeat_resp_handler)
{
  uint16 offset = 0;
  uint32 syspara_verno, recv_trans_no, cur_trans_no;
  uint8 pos_time[6];
  sp_print_row(FLAG_CLEAR, 1, 0, "处理心跳返回");
  //黑名单版本号
  offset = 0;
  memcpy(ctx->svrpara.blacklist_verno, buffer + offset, 6);
  //系统版本号
  offset += 6;
  sp_buffer_to_uint32(buffer + offset, &syspara_verno);
  ctx->svrpara.syspara_verno = syspara_verno;
  //已收到的流水号
  offset += 4;
  sp_buffer_to_uint32(buffer + offset, &recv_trans_no);
  //当前流水号
  offset += 4;
  sp_buffer_to_uint32(buffer + offset, &cur_trans_no);
  //pos机时间
  offset += 4;
  memcpy(pos_time, buffer + offset, 6);

  return 0;
}
PROTOCOL_FUNC_PREPARE(do_prepare_test)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_TEST, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  tcp_struct.data[0] = 0x06;
  tcp_struct.data[1] = 0x06;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(test_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "处理链路测试返回");
  return 0;
}


PROTOCOL_FUNC_PREPARE(do_download_feerate)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  uint16 download_index = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_FEERATE, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  sp_uint16_to_buffer(download_index, tcp_struct.data);
  ctx->svrpara.download_index = download_index;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(feerate_resp_handler)
{
  uint16 offset = 0;
  sp_print_row(FLAG_CLEAR, 1, 0, "下载费率表返回");
  //assert(data_len > 256 + 32)
  //是否需要校验一下crc
  memcpy(ctx->feerate_table, buffer, 256);
  offset += 256;
  //允许使用卡类别位图
  memcpy(ctx->svrpara.feetype_bitmap, buffer + offset, 32);
  offset += 32;
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_download_syspara)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_SYSPARA, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  tcp_struct.data[0] = 0x03;
  tcp_struct.data[0] = 0x06;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(syspara_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "下载系统参数返回");
  return 0;
}

PROTOCOL_FUNC_PREPARE(do_download_timepara)
{
  sp_tcp_trans_unit tcp_struct;
  int ret = 0;
  memset(&tcp_struct, 0, sizeof(sp_tcp_trans_unit));
  //header
  prepare_header_by_cmd(SP_CMD_GET_TIMEPARA, &tcp_struct.header);
  //data
  tcp_struct.header.data_len = 2;
  tcp_struct.data[0] = 0x01;
  tcp_struct.data[0] = 0x02;
  //传出参数
  ret = prepare_output_parameter(send_buffer, buffer_max_len, data_len, &tcp_struct);
  if(ret)
    return ret;
  //计算check_sum
  calc_send_buffer_checksum(send_buffer, data_len);
  return 0;
}

PROTOCOL_FUNC_RESP(timepara_resp_handler)
{
  sp_print_row(FLAG_CLEAR, 1, 0, "时间段参数返回");
  return 0;
}



//存放各种类型处理的接口
static protocol_process_handler global_process_handler[] =
{
  {SP_CMD_TEST, do_prepare_test, test_resp_handler},
  {SP_CMD_RT_TRANSDTL, do_prepare_transdtl, transdtl_resp_handler},
  {SP_CMD_AUTH, do_prepare_auth, auth_resp_handler},
  {SP_CMD_GET_BLACKLIST, do_prepare_blacklist, blacklist_resp_handler},
  {SP_CMD_HEARTBEAT, do_prepare_heartbeat, heartbeat_resp_handler},
  {SP_CMD_GET_FEERATE, do_download_feerate, feerate_resp_handler},
  {SP_CMD_GET_SYSPARA, do_download_syspara, syspara_resp_handler},
  {SP_CMD_GET_TIMEPARA, do_download_timepara, timepara_resp_handler}
};



static pprotocol_process_handler do_find_tcp_handler(uint8 cmd)
{
  int  i = 0;
  int list_len = 0;
  list_len = sizeof(global_process_handler) / sizeof(protocol_process_handler);
  for(i = 0; i < list_len; i++)
  {
    if(global_process_handler[i].cmd == cmd)
    {
      return &global_process_handler[i];
    }
  }
  return NULL;
}




////////////////////////////公开函数部分/////////////////////////////////////

/*
* 作用: 发送数据
*/
int sp_send_request(sp_context* ctx, uint8 cmd, void* arg, uint16 time_out)
{
  uint8* send_buffer;
  uint16 send_len = 0;
  int ret = 0;
  uint32 now_tick = 0;
  pprotocol_process_handler handler = NULL;
  //查看当前时间是否超过失效时间
  now_tick = GetTickCount();
  if(global_tcp_handler.expire_tick != 0)
  {
    if(now_tick > global_tcp_handler.expire_tick)
      return SP_E_EXCESSIVE_CALL;
  }
  //记录失效时间
  global_tcp_handler.expire_tick = now_tick + time_out;
  ++global_tcp_handler.current_tcp_seqno;
  handler = do_find_tcp_handler(cmd);
  send_buffer = (uint8*)malloc(MAX_SENDMSG_LEN);
  memset(send_buffer, 0, MAX_SENDMSG_LEN);
  ret = handler->prepare_req_data(ctx, send_buffer, MAX_SENDMSG_LEN, &send_len, arg);
  if(ret)
  {
    free(send_buffer);
    return ret;
  }
  ret = do_send_msg(send_buffer, send_len);
  if(ret)
  {
    free(send_buffer);
    return ret;
  }
  free(send_buffer);
  return 0;
}


/*
* 作用:等待数据返回
*/
int sp_prcocess_message(sp_context* ctx)
{
  uint8 recv_data[MAX_SENDMSG_LEN];
  uint16 data_len = 0;
  pprotocol_process_handler handler = NULL;
  sp_tcp_header* header = NULL;
  //检查buffer里的数据是否已经足够一个长度了
  do_parse_resp(ctx, recv_data, &data_len);
  ctx->recv_len = global_tcp_handler.recv_len;
  if(data_len <= 0)
  {
    return SP_E_BUFFER_NO_DATA;
  }
  header = (sp_tcp_header*)(recv_data);
  handler = do_find_tcp_handler(header->cmd_code);
  if(handler != NULL)
  {
    if(header->seqno != global_tcp_handler.current_tcp_seqno)
    {
      sp_print_row(FLAG_CLEAR, 3, 0, "seqno[%d-%d,%d]", header->seqno, global_tcp_handler.current_tcp_seqno, global_tcp_handler.recv_len);
      return SP_E_SEQNO_NOT_MATCH;
    }
    global_tcp_handler.expire_tick = 0;
    //只把data部分传进去
    return handler->resp_process_handler(ctx, recv_data + sizeof(sp_tcp_header), header->data_len);
    //return handler->resp_process_handler(ctx, recv_data, data_len);
  }
  else
  {
    sp_print_row(FLAG_CLEAR, 3, 0, "未cmd_code=%02x", header->cmd_code);
    return SP_E_CMD_NOT_FOUND;
  }
  //sp_print_row(FLAG_DO_NOT_CLEAR, 3, 0, "未data_len=%d", data_len);
}

void sp_init_com(void)
{
  //DCB sDCB;
  sDCB.baudrate   = 38400;
  sDCB.datab    = 8;
  sDCB.parity   = 0;
  sDCB.stopb    = 1;
  sDCB.limitlen = 1000;
  sDCB.timeout  = 100;//300
  do_init_tcp_handler();
  sDCB.pRecvfuc = do_receive_data;
  StartComm(COM1, &sDCB);

}
































#ifdef SP_DEBUG
///////////////////////////////////////////////////////////////////////
//
// 函数名       : SendAddrToCan
// 功能描述     : 配置CAN的地址
// 参数         : unsigned char cAddr －设置的CAN地址
// 返回值       : void
//
///////////////////////////////////////////////////////////////////////
uint8 SetAddrForCan(unsigned char cAddr)
{
  uint8 cTemp[7];
  uint8 i;
  uint8 count;
  int cLength;

  //组包
  cTemp[0] = 0x5a;                // 标志
  cTemp[1] = 0x01;                // 长度
  cTemp[2] = cAddr;         // 要发送的地址
  cTemp[3] = CANH;        // 终端子系统

  /*
      SF_Read(FPA_CFGS, FLA_CANbps, &cTemp[4],1); //　CAN通信速率
      if(cTemp[4] == 0x02)    //20k
      cLength = 4;
    else
    {
        cTemp[1] = 0x02;             // 长度
      cLength = 5;
    }
    */

  SF_Read(FPA_CFGS, FLA_CANbps, &cTemp[4], 1); //　CAN通信速率0x00:5K,0x01:10K,0x02:20K,0x03:50K
  if(cTemp[4] > 0x07) // 0x04:100K,0x05:125K,0x06:250K,0x07:500K,0x08:1000K
    cTemp[4] = 0x02;
  cLength = 5;

#if(POS_COMM == COMM_INTERRUPT)
  memcpy(DataBuf, cTemp, cLength);
#endif
  // 等待响应
  count = 3;

  do
  {
    UART_SendBuf(COM1, cTemp, cLength);   // 发地址给CAN
    for(i = 0x2f; i > 0 ; i--)
    {
      KillWatchDog();
      DelayNS(10);          //需要加延时,不然不会成功(原因待查)
      if(giRcvLen != 0)                    // 接收长度不为0, 收到数据
      {
        if(giRcvLen == 4)                // 接收完毕
        {
          if((rcv_buf[0] == 0x5a)  // 判断数据是否为发送过去的数据
             && (rcv_buf[1] == 0x01)) //&&(rcv_buf[2] == cAddr)
            //&&(rcv_buf[3] == CANH)
          {
            if(rcv_buf[3] < 0x08)
            {
              rcv_buf[2] = 0;
              rcv_buf[3] = 0;
            }
            SF_Write(FPA_CFGS, FLA_CAN_VER, (uint8 *)&rcv_buf[2], 2, TRUE);
            gbResetCanAddr = FALSE;
            giRcvLen = 0;
            Beep(3);
            return TRUE;          // 成功，则返回
          }
          else
          {
            break;
          }
        }
      }
    }
    gnBaudrate = gnBaudrateBuf[3 - count];
    SF_Write(FPA_CFGS, FLA_COMM_BAUDRATE, (uint8*)&gnBaudrate, 2, TRUE);
    COMM1_Init();
    DelayNS(10);
    giRcvLen = 0;
  }
  while(--count > 0);

  // 超时或接收到的数据不正确，提示操作员重新上电取地址
  gsPosCurrentState.sPosSystemState = State_Error;
#if(HD_POS == HD_SC)
  CLS();
  DelayNS(1);
  SP_PRINT(0, 0, "配置通讯模块失败");
  SP_PRINT(0, 1, "▲请重启--确认键");
  SP_PRINT(0, 2, "▲或重设通信方式");
  DelayNS(DISPDELAY);
#elif(HD_POS == HD_S2)
  ShowState(LED_ERROR, ERR_CommCAN);
  Beep(5) ;
#endif
  giRcvLen = 0;
  return FALSE;

}

#endif

