//用于发行版
#include 	"LED.h"			//包含这个头文件到你的工程			   
#include 	"reg52.h"		//reg 操作


//定时器0的装载值，决定显示速度
//每个数码管刷新的时间间隔为：(0x10000-count)*12/11.0592
//共6个数码管，设刷新间隔为100ms，则(0x10000-count)*12/11.0592=100000/6
//所以count取：0x10000-100000*11.0592/6/12=0x10000-0x05FA=0xFA06
#define TIMER0_COUNT 		0xFA06	   //后面程序里的 TIMER0_COUNT全部赋值 0xFA06

//与显示相关变量
unsigned char idata gLedBuf[6]; // 从左到右为0,1,2
unsigned char data gCurLed;//从左到右为0,1,2
bit gShowDot;

/* LED动态显示 */
void DisplayLed(void)
{
	LED_CS0 = 1;
	LED_CS1 = 1;
	LED_CS2 = 1;
	LED_CS3 = 1;
	LED_CS4 = 1;
	LED_CS5 = 1;
	switch ( gLedBuf[gCurLed])		 //switch case 语句中 switch中的条件case都不满足就执行default
	{
    case 0:	P1 = 0xf5; break;  //"0"
	case 1:	P1 = 0x05; break;  //"1"
	case 2:	P1 = 0xb3; break;  //"2"
	case 3:	P1 = 0x97; break;  //"3"
	case 4:	P1 = 0x47; break;  //"4"
	case 5:	P1 = 0xd6; break;  //"5"
	case 6:	P1 = 0xf6; break;  //"6"
	case 7:	P1 = 0x85; break;  //"7"
	case 8:	P1 = 0xf7; break;  //"8"
	case 9:	P1 = 0xd7; break;  //"9"
	case 10:P1 = 0x75; break;  //"U"
	case 11:P1 = 0x9f; break;  //“3.”
	case 12:P1 = 0x00; break;  //不显示
	case 13:P1 = 0x02; break;  //“-”
	case 14:P1 = 0xe7; break;  //“A”
	case 15:P1 = 0x76; break;  //“b”
	case 16:P1 = 0xf0; break;  //“C”
	case 17:P1 = 0x37; break;  //“d”
	case 18:P1 = 0xf2; break;  //“E”
	case 19:P1 = 0xe2; break;  //“F”
	case 20:P1 = 0xE3; break;  //“P”
	
	default:P1 = 0x0FF;break;
	}
 	if (gCurLed==3 && gShowDot==1 )		//&&逻辑与
 		P1 |= 0X08;
// 	P1 ^=0xFF;
	switch (gCurLed)
	{
	case 0:
		LED_CS0 = 0;
		LED_CS1 = 1;
		LED_CS2 = 1;
		LED_CS3 = 1;
		LED_CS4 = 1;
		LED_CS5 = 1;
		break;
	case 1:
		LED_CS0 = 1;
		LED_CS1 = 0;
		LED_CS2 = 1;
		LED_CS3 = 1;
		LED_CS4 = 1;
		LED_CS5 = 1;
		break;
	case 2:
		LED_CS0 = 1;
		LED_CS1 = 1;
		LED_CS2 = 0;
		LED_CS3 = 1;
		LED_CS4 = 1;
		LED_CS5 = 1;
		break;
	case 3:
		LED_CS0 = 1;
		LED_CS1 = 1;
		LED_CS2 = 1;
		LED_CS3 = 0;
		LED_CS4 = 1;
		LED_CS5 = 1;
		break;
	case 4:
		LED_CS0 = 1;
		LED_CS1 = 1;
		LED_CS2 = 1;
		LED_CS3 = 1;
		LED_CS4 = 0;
		LED_CS5 = 1;
		break;
	case 5:
		LED_CS0 = 1;
		LED_CS1 = 1;
		LED_CS2 = 1;
		LED_CS3 = 1;
		LED_CS4 = 1;
		LED_CS5 = 0;
		break;
	default:
		break;
	}
}

/* 送显函数 数据不大于0x270f (9999) */
/* 函数改变显示缓冲LedBuf的内容     */
void Show(unsigned int unData,unsigned char ucData)
{
	unsigned int unTmp;
	unsigned char ucTmp;
	if (unData > 9999)
		unTmp = 9999;
	else
		unTmp = unData;
	if (ucData > 99)
		ucTmp = 99;
	else
		ucTmp = ucData;
	gLedBuf[0]=0;
	gLedBuf[1]=0;
	gLedBuf[2]=0;
	gLedBuf[3]=0;
	gLedBuf[4]=0;
	gLedBuf[5]=0;

	if (unTmp>=1000)
	{
		gLedBuf[0] = (unsigned char)(unTmp/1000);
		unTmp -=gLedBuf[0]*1000;
	}
	if (unTmp>=100)
	{
		gLedBuf[1] = (unsigned char)(unTmp/100);
		unTmp -=gLedBuf[1]*100;
	}
	if (unTmp >= 10)
	{
		gLedBuf[2] = (unsigned char)(unTmp/10);
		unTmp -=gLedBuf[2]*10;
	}
	gLedBuf[3]=unTmp;
	if (ucTmp >= 10)
	{
		gLedBuf[4] = (unsigned char)(ucTmp/10);
		ucTmp -=gLedBuf[4]*10;
	}
	gLedBuf[5]=ucTmp;
}

/* 定时器0初始化 */
void Timer0_initialize(void)
{
	EA=0;
	TR0=0;
	TMOD &= 0x0F0;
	TMOD |= 0x01;			   		//工作在方式1：16位定时、计数器
	TL0 =(unsigned char)(TIMER0_COUNT&0x00FF);
	TH0 =(unsigned char)(TIMER0_COUNT >> 8);

	PT0 = 0;
	ET0 = 1;
	TR0 = 1;
	EA = 1;
}

/* 定时器0中断服务程序 */
static void Timer0_isr(void) interrupt 1 using 1
{
	TR0=0;
	TL0 =(unsigned char)(TIMER0_COUNT&0x00cF);
	TH0 =(unsigned char)(TIMER0_COUNT >> 8);
	TR0 = 1;
	if (gCurLed<5)
		gCurLed++;
	else
		gCurLed=0;
	DisplayLed();
}
