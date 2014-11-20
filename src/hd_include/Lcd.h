#ifndef __LCD_H__		//内部接口
#define  __LCD_H__

#define   LCD_BAK       	1<< 19   			// 背光     	(P1.19)
#define    LED         		1 << 14             // led		(P0.14)

//LCD
#define 	LCDLightON()		IO1SET = LCD_BAK // 开LCD背光
#define 	LCDLightOFF()		IO1CLR = LCD_BAK // 关LCD背光 
#define 	LEDON()			    	IO0CLR = LED     // LED 开
#define 	LEDOFF()			IO0SET = LED     // LED 关


/* 宏定义*/
#define 		CLRSCR			0xff


//液晶类型
#define	LCDTPYE_HAVEWORD		0x00			//带字库LCD
#define	LCDTPYE_NOWORD		0x01			//不带字库LCD

//定义字体
#define     SIMPLIFIED  							0x00			//简体
#define     TRADITIONAL 							0x01			//繁体



//GB字库,简体和繁体之间的切换
#define FPA_CODING_ADDR_START    						(uint16)3490		// GB字库开始页面
#define FPA_CODING_ADDR_END        						(uint16)3560   	// GB字库结束页面 



//字库起始地址    //each page 256B  ascii码共6Page,其它为汉字
#define FPA_ASCIILIB_ADDR_START    	(uint16)7122	// ASCII码字库起始页面
#define FPA_FONT_ADDR_START         	(uint16)7128   // 汉字库起始页面
#define FONTLIB_ADDR_END			(uint16)8192    // 汉字库结束页面



//函数
/*
****************************************************************************************
**函数名称:SetLcdType()
**函数功能:设置液晶类型
**入口参数:cLCDType液晶类型:LCDTPYE_HAVEWORD:0 带字库LCD, LCDTPYE_NOWORD:1 不带字库LCD
**出口参数:无
**返回:无
****************************************************************************************
*/
extern void SetLcdType(uint8 cLCDType);

/*
****************************************************************************************
**函数名称:GetLcdType()
**函数功能:获取液晶类型
**入口参数:无
**出口参数:无
**返回:无
****************************************************************************************
*/
extern uint8 GetLcdType(void);

/*
****************************************************************************************
**函数名称:SetLcdContrast()
**函数功能:设置液晶对比度
**入口参数:cContrast对比度:范围0x20~0x3f,默认为0x28
**出口参数:无
**返回:无
****************************************************************************************
*/
extern void SetLcdContrast(uint8 cContrast);


/*
****************************************************************************************
**函数名称:CLS()
**函数功能:清屏
**入口参数:无 
**出口参数:无
**返回:无
****************************************************************************************
*/
extern void LcdCLS(uint8 cRow);                           	//LCD 清屏

/*
****************************************************************************************
**函数名称:CldInit()
**函数功能:液晶初始化
**入口参数:无
**出口参数:无
**返回:无
****************************************************************************************
*/
extern void LcdInit(void);                            // LCD显示初始化

/*
****************************************************************************************
**函数名称:LcdPrint()
**函数功能:显示
**入口参数:cArray:列(0-15),cRow:行(0-3),String:字符串指针, cFlashnum:不起用此参数
**出口参数:无
**返回:无
****************************************************************************************
*/
extern void LcdPrint(uint8 cRow, uint8 cArray, char *String,uint8 cFlashnum);//LCD输出显示	



#endif

