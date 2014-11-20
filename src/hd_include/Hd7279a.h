#ifndef __HD7279_H__	//内部接口 
#define  __HD7279_H__
/************************************************************
			7279 键值映射
*************************************************************/
//  (数字键0-9分别为ASCII码的'0'-'9',
//  +/A,-B,*/C, . 分别映射为'+','-','*','.'
//	其它功能键定义如下
#define 	KB_MODE		0xb3 	//模式键
#define 	KB_MENU		0xb4	//功能键
#define   KB_ENTER		0xb5	//确认键
#define 	KB_CANCEL		0xb6	//清除键
#define	KB_NONE         	0x00

#define 		NOFLASH 	0xff
#define     	STANDMODE   	(uint8)0         //标准消费模式指示灯
#define     	FIXMODE      		(uint8)1         //定值消费模式指示灯
#define     	NUMMODE      	(uint8)2         //编号消费模式指示灯	
#define     	S2KEYInit      SCKEYInit	
uint8       GetS2KEY(void);     			//S2取键值函数
uint8       GetSCKEY(void);     			//SC取键值函数
void        SCKEYInit(bool bIsBeep);    	//7279初始化   非0按键鸣叫
void        LedPrint(uint8 cRow, uint8 cArray, char *String, uint8 cFlashNum);//S2输出显示内容	            
void 	LedCLS(uint8 cRow);         	//S2清屏
void        LedInit(void);        			//S2显示初始化
void        SelectModeLed(uint8 mode);	//选择消费模式指示灯
#endif
