#ifndef __MIFARE_H__		//内部接口
#define __MIFARE_H__
	// 宏定义
#define 	KEYA_MODE			0x10		//用keya认证模式
#define 	KEYB_MODE			0x20		//用keyb认证模式

#define 	INCREMENT			1		//钱包加值
#define	DECREMENT			0		//钱包减值

#define	REQUEST_STD 		0x26	
#define	REQUEST_ALL 		0x52

#define	BLOCK0				0x01
#define	BLOCK1				0x02
#define	BLOCK2				0x04
#define	BLOCK3				0x08

//******************* 函数错误代码定义********************
#define	FM1715_OK				0			//正确
#define	FM1715_NOTAGERR		1			//无卡
#define	FM1715_CRCERR			2			//卡片CRC校验错误
#define	FM1715_EMPTY			3			//数值溢出错误
#define	FM1715_AUTHERR		4			//验证不成功
#define	FM1715_PARITYERR		5			//卡片奇偶校验错误
#define	FM1715_CODEERR		6			//通讯错误(BCC校验错)
#define	FM1715_SERNRERR		8			//卡片序列号错误(anti-collision 错误)
#define	FM1715_SELECTERR		9			//卡片数据长度字节错误(SELECT错误)
#define	FM1715_NOTAUTHERR	10			//卡片没有通过验证
#define	FM1715_BITCOUNTERR	11			//从卡片接收到的位数错误
#define	FM1715_BYTECOUNTERR	12			//从卡片接收到的字节数错误（仅读函数有效）
#define	FM1715_RESTERR		13			//调用restore函数出错
#define	FM1715_TRANSERR		14			//调用transfer函数出错
#define	FM1715_WRITEERR		15			//调用write函数出错
#define	FM1715_INCRERR		16			//调用increment函数出错
#define	FM1715_DECRERR		17			//调用decrement函数出错
#define	FM1715_READERR		18			//调用read函数出错
#define 	FM1715_LOADKEYERR		19		//调用LOADKEY函数出错
#define 	FM1715_FRAMINGERR		20      	//FM1715帧错误
#define 	FM1715_REQERR			21      		//调用req函数出错
#define 	FM1715_SELERR			22			//调用sel函数出错
#define 	FM1715_ANTICOLLERR		23      	//调用anticoll函数出错
#define	FM1715_INTIVALERR		24			//调用初始化函数出错
#define 	FM1715_READVALERR		25		//调用高级读块值函数出错
#define	FM1715_DESELECTERR	26
#define	FM1715_CMD_ERR		42			//命令错误	
#define 	FM1715_READ_ERR		43			// 读卡错误
#define 	FM1715_WRITE_ERR		44			// 写卡错误
#define   FM1715_PROTOCOL_ERR   45                //协议错误

//  变量
extern unsigned char	gcUID[5]    ;			// 序列号
extern unsigned long     glPrevIDBak;           	// 序列号

// 函数
/*
***************************************************
功能: 初始化Mifare读头及其相关部分
入口: 无
出口: 无
****************************************************
*/
void InitICReader(void);
// 函数
unsigned char MIF_ReadBlock(
				 unsigned char *buff,
				 unsigned char Block_Adr);

unsigned char MIF_WriteBlock(
				 unsigned char *buff,
				 unsigned char Block_Adr);

unsigned char HL_Active(
				 unsigned char Block_Adr,
				 unsigned char hKeyunsigned,
				 unsigned char const *cKey);

//电子钱包部分
unsigned char MIF_Value(
				 unsigned char cmd,
				 unsigned char *buff,
				 unsigned char Block_Adr);

/*
*********************************************************************************************************
** 函数名称 ：MIF_InitPurse
** 函数功能 ：将一个块初始化为钱包格式、金额为0
** 入口参数 : cBlock : 读块数据，
			: hKey 写块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: cTry  写卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/

unsigned char MIF_InitPurse(
						 unsigned char cBlock,
						 unsigned char hKey, 
						 unsigned char const *cKey, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** 函数名称 ：MIF_Purse
** 函数功能 ：对钱包进行加/减操作
** 入口参数 ：cmd 命令字 INCREMENT 表示加钱包，DECREMENT 表示减钱包
			: cBlock : 读块数据，
			: hKey 写块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: pData 写入的块数据
			: cTry  写卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/

bool MIF_Purse(
						 unsigned char cmd,
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);


/*
****************************************************
功能: request  card
入口: mode    工作模式 REQUEST_STD 表示标准寻卡  REQUEST_ALL表示寻所有卡
出口: 返回值
*****************************************************
*/

unsigned char Request(unsigned char mode);
/*
***************************************************
功能: halt  card 停卡
入口: 无
出口: 返回值
****************************************************
*/

unsigned char MIF_Halt(void);

/*
*********************************************************************************************************
** 函数名称 ：MIF_ReadCard
** 函数功能 ：读块数据
** 入口参数 ：cBlock : 读块数据，
			: hKey 读块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: pData 读出的块数据
			: cTry  读卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/

bool MIF_ReadCard(
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** 函数名称 ：MIF_ReadCard2
** 函数功能 ：读同一扇区前二块数据
** 入口参数 ：cBlock : 读块数据，
			: hKey 读块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: pData 读出的块数据
			: cTry  读卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/
bool MIF_ReadCard2(
						 unsigned char cBlock, 
						 unsigned char hKey,
						 unsigned char const *cKey, 
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** 函数名称 ：MIF_ReadCard3
** 函数功能 ：读同一扇区三块数据
** 入口参数 ：cBlock : 读块数据，
			: hKey 读块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: pData 读出的块数据
			: cTry  读卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/
	
bool MIF_ReadCard3(
						 unsigned char cBlock, 
						 unsigned char hKey,
						 unsigned char const *cKey, 
						 unsigned char *pData, 
						 unsigned char cTry);	
/*
*********************************************************************************************************
** 函数名称 ：MIF_WriteCard
** 函数功能 ：写块数据
** 入口参数 ：cBlock : 读块数据，
			: hKey 写块数据认证方式，KEYA_MODE 表示Keya,KEYB_MODE表示Keyb
			: cKey 此块对应扇区的密钥
			: pData 写入的块数据
			: cTry  写卡失败后重复次数
** 出口参数 ：无
** 返    回 : 0 : 表示不成功， 1 : 表示成功
*********************************************************************************************************
*/
bool MIF_WriteCard(
						 unsigned char cBlock, 
						 unsigned char hKey, 
						 unsigned char const *cKey,
						 unsigned char *pData, 
						 unsigned char cTry);
/*
*********************************************************************************************************
** 函数名称 ：ReadICCardUID
** 函数功能 ：读卡序列号4个字节
** 入口参数 ：cUID 成功则表示卡片序列号
** 出口参数 ：无
** 返    回 : 0 : 表示成功， 非0 : 表示不成功
*********************************************************************************************************
*/

uint8 ReadICCardUID(unsigned char *cUID);	




/*
*********************************************************************************************************

***********************************************CPU卡操作函数***************************************

*********************************************************************************************************
*/

/*
*********************************************************************************************************
** 函数名称 ：ResetCard_A
** 函数功能 ：复位非接CPU卡
** 入口参数 ：无
** 出口参数 ：无
** 返    回 : TRUE:成功False:失败
*********************************************************************************************************
*/
uint8 ResetCard_A(void);            


/*
*****************************************************************************
Name: 		CpuCard_Apdu	
Function:	应用协议数据单元信息交换函数
InPara:     	cCID:固定为0x0F　不使用CID
			cSendLen:发送数据长度
            		cSendBuf:发送数据内容　
            
OutPara:    	cRcvLen: 接收数据长度
            		cRcvBuf: 接收数据内容
return:     命令执行成功 0x9000
			发送命令失败 0x0001
			接收长度错误 0x0002
			协议错误     0x0003
			CID超过最大值0x0004
		    其它的值为错误代码
*****************************************************************************
*/
uint16 CpuCard_Apdu(uint8 cCID, uint8 cSendLen, uint8 *pSendBuf, uint8 *pRcvLen, uint8 *pRcvBuf);


/*
*********************************************************************************************************
** 函数名称 ：Deselect_A
** 函数功能 ：取消被选择的卡片
** 入口参数 ：无
** 出口参数 ：无
** 返    回 : FM1715_OK:成功FM1715_NOTAGERR:失败
*********************************************************************************************************
*/
uint8 Deselect_A(void) ;



/*
*********************************************************************************************************
** 函数名称 ：Reset_Reader_TimeOut
** 函数功能 ：重设读头指令执行超时时间
** 入口参数 ：无
** 出口参数 ：无
** 返    回 : 无
*********************************************************************************************************
*/
void Reset_Reader_TimeOut(void);
#endif
