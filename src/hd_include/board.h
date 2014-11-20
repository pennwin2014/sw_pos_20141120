#ifndef __BOARD_H__
#define __BOARD_H__

// 蜂鸣器
#define    	BEEP         			1 << 21             // BEEP		(P1.21)
// TCP/IP模块配置引脚控制
// 电锁和电铃控制
#define     LOCK1       			1 << 11	         	// (P0.11)   
#define     LOCK2       			1 << 23            	// (P1.23)   

#define	 	TCPIP_EXIT()			IO1SET   	= LOCK2
#define 	TCPIP_ENTER()			IO1CLR		= LOCK2

#define LCDLightON()		IO1SET = LCD_BAK // 开LCD背光
#define LCDLightOFF()		IO1CLR = LCD_BAK // 关LCD背光
#define LEDON()			    IO0CLR = LED     // LED 开
#define LEDOFF()			IO0SET = LED     // LED 关

// 鸣叫提示
#define 	BP_SUPER_SHORT    		1           		// 极短促蜂鸣声 30ms
#define 	BP_SHORT				3		        	// 短促蜂鸣声 100ms
#define 	BP_MIDDLE				20		        	// 中蜂鸣声 200ms
#define 	BP_LONG					100		        	// 长蜂鸣声 1000ms
#define 	BP_LONG_LONG			255

//硬件选择
//KeyType
#define  KMKEY  	KMKEY                    //考勤机
#define  S2KEY  	S2KEY                    //挂式收费机
#define  SCKEY  	SCKEY                    //台式收费机
//DispType
#define  Led        Led                  //LED显示
#define  Lcd        Lcd                  //LCD显示
// 读头类型　
#define  IC			IC
#define  ID			ID
#define  SelectKeyAndDisp(KeyType,DispType,READER) {\
		GetKey=Get##KeyType;\
		KeyInit=KeyType##Init;\
		Print=DispType##Print;\
		DispInit=DispType##Init;\
		CLS=DispType##CLS;\
		Init_Reader=Init##READER##Reader;\
		ReadCardUID=Read##READER##CardUID;\
		}
extern uint8 (*GetKey)(void);//取键值
extern void  (*KeyInit)(bool bIsBeep);//按键初始化化
extern void  (*Print)(uint8 cRow,uint8 cArray, char *String , uint8 cFlashnum);//输出显示内容
extern void  (*DispInit)(void);//LCD或数码管显示初始化
extern void  (*CLS)(uint8 cRow);//清屏
extern void  (*Init_Reader)(void);// 初始化读头
extern uint8  (*ReadCardUID)(uint8 *cUID);// 读卡序列号
/****************************************************
功能: 控制蜂鸣器鸣叫次数
入口: cCount    鸣叫次数
出口: 无
*****************************************************/
void Beep(unsigned char cCount);
void InitBoard(void);
void BeeperCtr(void);                            //鸣叫控制
void DelayNS(int dly); 
extern uint8 BattCheck(uint8 IsCheckBatter);    // 电池检测
extern unsigned char	cBeep ;					// 蜂鸣器鸣叫时间

#endif
