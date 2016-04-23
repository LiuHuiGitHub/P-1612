
//=============================2014-09-15========================================

//U3.2版本，无刷卡金额纪录，程序已经做了防快刷错误，数据可以自动恢复。

#include "reg52.h"
#include "normal.h"
#include "mifare.h"
#include "LED.h"
#include "fm1702sl.h"
#include "string.h"

bit Brushed;	           //用来判断当前是否进行价格中“分”的设置

unsigned char Card_Type[6];    //发行卡和空白卡之间的一个切换

unsigned char ucWorkState;
unsigned char ucPulseWeight;

unsigned char gPrice[2];   //gPrice[1]单位为元，gPrice[0]单位为分（0~99）

unsigned char RE_Card_Flag = 0;//在对用户卡进行充值，管理卡和用户卡之间不断切换标志位。

unsigned char idata PWD_Data[16]; 

unsigned char FC_FF_Card_count = 2;     //空白卡和发行卡之间的相互转换；
unsigned char Card_Type_Flag = 0;		//控制卡的类型，置1空白卡到用户卡、管理卡或者其他的卡；置2发行卡类型；
unsigned char PWD_Card_Count = 88;      //密码卡计数值，初始化88

unsigned char Field_Value = 1;          //制作分区卡是，记录区的编号，在没有刷分区卡的前提下，默认制作1区分区卡

unsigned char MGM_Card_PWD[] = {0,0,0,0,0,0};		//管理卡的通信密码
unsigned char User_Card_PWD[] = {0,0,0,0,0,0};		//用户卡的通信密码
unsigned char PWD_Card_PWD[] = {0,0,0,0,0,0};		//密码卡的通信密码

unsigned char FC_Card_Count = 0;   //把空白卡做成发行卡的数量
unsigned char FF_Card_Count = 0;   //把发行卡做成空白卡的数量
unsigned char USER_Card_Count[2];  //给用户卡加密时，用来计数的变量
unsigned char One_MGM_Card_Count[2];  //给1元管理卡加密时，用来计数的变量
unsigned char Ten_MGM_Card_Count[2];  //给10元管理卡加密时，用来计数的变量
unsigned char PWDS_Card_Count = 0;  //复制加密卡时，  用来计数的变量
unsigned char PLS_Card_Count[5];       //制作脉冲卡时，  用来计数的变量
unsigned char AREA_Card_Count[16];     //制作分区卡时，  用来计数的变量


/*
*	id: 		密码在RC500 中的索引
*	ucType:	函数执行类型：
*			0   将gBuff[0]~gBuff[5]内的密码存入
*			1   将gBuff[0]~gBuff[5]和gBuff[10]~gBuff[15]两组密码存入
*/
uchar ChangePWD(uchar id,uchar ucType)
{
	uchar mm0[6];
	uchar mm1[6];
    //1CB533FBAFC6
	mm0[0]=gBuff[0];mm0[1]=gBuff[1];mm0[2]=gBuff[2];mm0[3]=gBuff[3];mm0[4]=gBuff[4];mm0[5]=gBuff[5];
	mm1[0]=gBuff[10];mm1[1]=gBuff[11];mm1[2]=gBuff[12];mm1[3]=gBuff[13];mm1[4]=gBuff[14];mm1[5]=gBuff[15];
	if(ucType==0)
		return Load_Key(id,mm0);
	else if(Load_Key(id,mm0)||Load_Key(id+1,mm1))
		  return 1;
	return 0;
}


	//等待刷卡	
void WaitingBrush(void)
{
	do
	{
		WDT_CONTR = 0x3E;
		MIF_Halt();
		Request(RF_CMD_REQUEST_STD);
		AntiColl();
		if(SelectCard()==FM1702_OK)
			return;
		else if(Brushed ==1 )
		{
			Brushed = 0;
			Show(0,0);
		}
	}while(1);

}

/*
*	寻卡并使用A、B码建立通讯
*	ucField:	要进行读写的数据区
*	ucBlock:	要进行读写的数据块
*	ucKey:		需要验证的密码 ，按位判断  
*	成功：返回卡的类型：0 管理卡   1 用户卡    2 密码卡   3 发行卡
*	失败：返回0XFF
*/
uchar InitCard(uchar ucField,uchar ucBlock,uchar ucKey)	 //InitCard(1,0,0x04)==2
{
	uchar i;
	
	/*使用A密码进行寻卡认证*/
	for(i=0;i<4;i++)
	{
		if((ucKey>>i) & 0x01)
		{
			WaitingBrush();
			Load_Key_EE(i);
			if(Authentication(gCard_UID,ucField,0x60) ==FM1702_OK) //认证通过，进行读测试
			{
				if(Read_Block(gBuff,ucField*4+ucBlock)==FM1702_OK) //读测试通过，则返回，不再进行B密码认证
					return i;
				break;
			}
		}
	}
	/*使用B密码进行寻卡认证*/
	for(i=0;i<4;i++)
	{
		if((ucKey>>i) & 0x01)
		{
			WaitingBrush();
			Load_Key_EE(i);
			if(Authentication(gCard_UID,ucField,0x61) ==FM1702_OK) //认证通过，进行读测试
			{
				if(Read_Block(gBuff,ucField*4+ucBlock)==FM1702_OK)//读测试通过，则返回，不再进行B密码认证
					return i;
				break;
			}
		}
	}
	return 0xFF;
}


void Init(void)
{
	P1M1 = 0;
	P1M0 = 0xFF;
	P3M1 = 0;
	P3M0 = 0xa0;
	Timer0_initialize();
	//OUT_PLUS = 0;
	//gPlusWidth = 0;
	while(Init_FM1702()==false);
	
	Ding();
}

void Card_Clear(void)
{
    unsigned char i;
	for(i = 0; i < 16; i++)
	{
	    gBuff[i] = 0;
	}
	if(Write_Block(gBuff,ucWorkState*4+0) != FM1702_OK) {Ding();Ding();return;}
	if(Write_Block(gBuff,ucWorkState*4+1) != FM1702_OK) {Ding();Ding();return;}
	if(Write_Block(gBuff,ucWorkState*4+2) != FM1702_OK) {Ding();Ding();return;}  //发行卡的密码：FC 27 30 C2 C4 A8
	if(Card_Type[0] == 0xfc)
	{
		gBuff[0] = 0xfc;gBuff[1] = 0x27;gBuff[2]  = 0x30;gBuff[3]  = 0xc2;gBuff[4]  = 0xc4;gBuff[5]  = 0xa8;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
		gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xfc;gBuff[11] = 0x27;gBuff[12] = 0x30;gBuff[13] = 0xc2;gBuff[14] = 0xc4;gBuff[15] = 0xa8;
	}
	else if(Card_Type[0] == 0xff)
	{
		gBuff[0] = 0xff;gBuff[1] = 0xff;gBuff[2]  = 0xff;gBuff[3]  = 0xff;gBuff[4]  = 0xff;gBuff[5]  = 0xff;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
		gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xff;gBuff[11] = 0xff;gBuff[12] = 0xff;gBuff[13] = 0xff;gBuff[14] = 0xff;gBuff[15] = 0xff;
	}
	if(Write_Block(gBuff,ucWorkState*4+3) != FM1702_OK) {Ding();Ding();return;}
	switch (PWD_Card_Count)
	{
		case 1: 
		        USER_Card_Count[1]++;
				if(USER_Card_Count[1] > 200) USER_Card_Count[1] = 1;
		        Show(USER_Card_Clear    + USER_Card_Count[1]    / 100,USER_Card_Count[1] % 100);         break; //用户卡到发行卡
		case 3: 
		        One_MGM_Card_Count[1]++;
				if(One_MGM_Card_Count[1] > 200) One_MGM_Card_Count[1] = 1; 
		        Show(One_MGM_Card_Clear + One_MGM_Card_Count[1] / 100,One_MGM_Card_Count[1] % 100);      break; //1元管理卡到发行卡
		case 5: 
		        Ten_MGM_Card_Count[1]++; 
				if(Ten_MGM_Card_Count[1] > 200) Ten_MGM_Card_Count[1] = 1;
		        Show(Ten_MGM_Card_Clear + Ten_MGM_Card_Count[1] / 100,Ten_MGM_Card_Count[1] % 100);      break; //10元管理卡到发行卡
		default:break;
	}
	gLedBuf[2]= 13;
	Ding();

}


void Card_ClearMEM(void)
{
    unsigned char i;
	for(i = 0; i < 16; i++)
	{
	    gBuff[i] = 0;
	}
	if(Write_Block(gBuff,1*4+0) != FM1702_OK) {Ding();Ding();return;}
	if(Write_Block(gBuff,1*4+1) != FM1702_OK) {Ding();Ding();return;}
	if(Write_Block(gBuff,1*4+2) != FM1702_OK) {Ding();Ding();return;}  //发行卡的密码：FC 27 30 C2 C4 A8
	if(Card_Type[0] == 0xfc)
	{
		gBuff[0] = 0xfc;gBuff[1] = 0x27;gBuff[2]  = 0x30;gBuff[3]  = 0xc2;gBuff[4]  = 0xc4;gBuff[5]  = 0xa8;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
		gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xfc;gBuff[11] = 0x27;gBuff[12] = 0x30;gBuff[13] = 0xc2;gBuff[14] = 0xc4;gBuff[15] = 0xa8;
	}
	else if(Card_Type[0] == 0xff)
	{
		gBuff[0] = 0xff;gBuff[1] = 0xff;gBuff[2]  = 0xff;gBuff[3]  = 0xff;gBuff[4]  = 0xff;gBuff[5]  = 0xff;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
		gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xff;gBuff[11] = 0xff;gBuff[12] = 0xff;gBuff[13] = 0xff;gBuff[14] = 0xff;gBuff[15] = 0xff;
	}
	if(Write_Block(gBuff,1*4+3) != FM1702_OK) {Ding();Ding();return;}
	switch (PWD_Card_Count)
	{
		case 1: 
		        USER_Card_Count[1]++;
				if(USER_Card_Count[1] > 200) USER_Card_Count[1] = 1;
		        Show(USER_Card_Clear    + USER_Card_Count[1]    / 100,USER_Card_Count[1] % 100);         break; //用户卡到发行卡
		case 3: 
		        One_MGM_Card_Count[1]++;
				if(One_MGM_Card_Count[1] > 200) One_MGM_Card_Count[1] = 1; 
		        Show(One_MGM_Card_Clear + One_MGM_Card_Count[1] / 100,One_MGM_Card_Count[1] % 100);      break; //1元管理卡到发行卡
		case 5: 
		        Ten_MGM_Card_Count[1]++; 
				if(Ten_MGM_Card_Count[1] > 200) Ten_MGM_Card_Count[1] = 1;
		        Show(Ten_MGM_Card_Clear + Ten_MGM_Card_Count[1] / 100,Ten_MGM_Card_Count[1] % 100);      break; //10元管理卡到发行卡
		default:break;
	}
	gLedBuf[2]= 13;
	Ding();

}
//=======================================================
//	名称: main
//	说明: 主函数，
//FLASH：0扇区首地址为0：0、1数据存储ucPrice（元、分）,表示单次消费金额
//		1扇区首地址为0x200 ：0数据存储ucWorkState，表示工作状态，为0xFF时执行密码更新程序
//		2扇区首地址为0x400:0,1,2,3数据存储历史消费记录，ucTotal[4]
//		  消费记录 = ucTotal[2]*10000+ucTotal[1]*100+ucTotal[0]+ucTotal[4]/100 (元)
//=======================================================
void main(void)
{
	unsigned char i;

	Init();
    PWD_Card_PWD[0] = 0xAC;
    PWD_Card_PWD[1] = 0x1E;
    PWD_Card_PWD[2] = 0x57;
    PWD_Card_PWD[3] = 0xAF;
    PWD_Card_PWD[4] = 0x19;
    PWD_Card_PWD[5] = 0x4E;
//	PWD_Card_PWD[0] = ByteRead(0x0000);
//	PWD_Card_PWD[1] = ByteRead(0x0001);
//	PWD_Card_PWD[2] = ByteRead(0x0002);
//	PWD_Card_PWD[3] = ByteRead(0x0003);
//	PWD_Card_PWD[4] = ByteRead(0x0004);
//	PWD_Card_PWD[5] = ByteRead(0x0005);
	ucWorkState     = ByteRead(0x0200);
	Card_Type[0]    = ByteRead(0x0400);
	Card_Type[1]    = ByteRead(0x0401);
	Card_Type[2]    = ByteRead(0x0402);
	Card_Type[3]    = ByteRead(0x0403);
	Card_Type[4]    = ByteRead(0x0404);
	Card_Type[5]    = ByteRead(0x0405);
	Card_Type_Flag  = ByteRead(0x0406);	//FC2730C2C4A8
	if(Card_Type_Flag == 0xff)
	{
	    Card_Type[0] = 0xfc;
		Card_Type[1] = 0x27;
		Card_Type[2] = 0x30;
		Card_Type[3] = 0xc2;
		Card_Type[4] = 0xc4;
		Card_Type[5] = 0xa8;
        Card_Type_Flag = 1;
	}
	switch(ucWorkState)
	{
	case 0xFF://200905052154(出厂初始化密码卡密码)
		Show(1,0);
        SectorErase(0x0000);
        ByteProgram(0x0000,0xAC);
        ByteProgram(0x0001,0x1E);
        ByteProgram(0x0002,0x57);
        ByteProgram(0x0003,0xAF);
        ByteProgram(0x0004,0x19);
        ByteProgram(0x0005,0x4E);
        IAP_Disable();
        SectorErase(0x0200);
        ByteProgram(0x0200,1);
        IAP_Disable();
        Delay_1ms(200);
        Delay_1ms(200);
        break;
	default:
		break;
			
	}
	//显示版本号P-1612
	gShowDot=0;
	gLedBuf[0] = 20;
	gLedBuf[1] = 13;
	gLedBuf[2] = 1;
	gLedBuf[3] = 6;
	gLedBuf[4] = 1;
	gLedBuf[5] = 2;
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
    Show(ucWorkState,0);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	Delay_1ms(200);
	//显示88,88代表该机子是密码机	 
	gLedBuf[0] = 12;		 
	gLedBuf[1] = 12;
	if(Card_Type[0] == 0xfc) {gLedBuf[2] = 8;gLedBuf[3] = 8;}
	if(Card_Type[0] == 0xff) {gLedBuf[2] = 9;gLedBuf[3] = 9;}
	gLedBuf[4] = 12;
	gLedBuf[5] = 12;
    
    gBuff[0] = 0xAC;
    gBuff[1] = 0x1E;
    gBuff[2] = 0x57;
    gBuff[3] = 0xAF;
    gBuff[4] = 0x19;
    gBuff[5] = 0x4E;
    ChangePWD(2,0);

	Brushed = 0;
	while (1)
	{
		switch (InitCard(1,0,0x04))
		{
		case 2:	     //密码卡通信密码
			Brushed = 0;	       
			if(Read_Block(gBuff,0x4)!=FM1702_OK
                || Read_Block(PWD_Data,0x5)!=FM1702_OK
            )
			{
				Ding();
				Ding();
			}
			else
			{ 
			    if(gBuff[6] == 0xff)    //制作发行卡需要刷的卡
				{
				    PWD_Card_Count = 28;
					FC_Card_Count = 0;
					FF_Card_Count = 0;
					FC_FF_Card_count++;
					if(FC_FF_Card_count > 1) FC_FF_Card_count = 0;
					switch(FC_FF_Card_count)
					{
					    case 0:	Show(FC_Card_Count / 100,FC_Card_Count % 100);	 //FC_FF_Card_count = 0;FC状态下，空白卡到发行卡；
								gLedBuf[0]= 19;
								gLedBuf[1]= 16;
								gLedBuf[2]= 13;								   break;
						case 1:	Show(FF_Card_Count / 100,FF_Card_Count % 100);	//FF状态下，发行卡到空白卡；
								gLedBuf[0]= 19;
								gLedBuf[1]= 19;
								gLedBuf[2]= 13;								    break;
						default:break;
					}
					
					Ding();
					Delay_1ms(200);
					Delay_1ms(200);
				}
			    
			    if(gBuff[6] == 0x20) 	//卡型的选择 
				{
                    for(i = 0;i < 2;i++)  {USER_Card_Count[i] = 0;}
					for(i = 0;i < 2;i++)  {One_MGM_Card_Count[i] = 0;}
					for(i = 0;i < 2;i++)  {Ten_MGM_Card_Count[i] = 0;}
					PWDS_Card_Count = 0;
                    for(i = 0;i < 5;i++)  {PLS_Card_Count[i] = 0;}
					for(i = 0;i < 16;i++) {AREA_Card_Count[i] = 0;}
					if(Card_Type_Flag == 2)	 //发行卡
					{	
					    Card_Type_Flag = 1;	
				        for(i = 0;i < 6;i++)  {Card_Type[i] = gBuff[i];}
					    gLedBuf[2] = 8;
				        gLedBuf[3] = 8;
					    PWD_Card_Count = 88;
					} 
					else if(Card_Type_Flag == 1)   //
					{
					    Card_Type_Flag = 2;
				        for(i = 0;i < 6;i++)  {Card_Type[i] = gBuff[i+10];}
					    gLedBuf[2] = 9;
				        gLedBuf[3] = 9;
					    PWD_Card_Count = 99;
					}
					SectorErase(0x0400);
					ByteProgram(0x0400,Card_Type[0]);
					ByteProgram(0x0401,Card_Type[1]);
					ByteProgram(0x0402,Card_Type[2]);
					ByteProgram(0x0403,Card_Type[3]);
					ByteProgram(0x0404,Card_Type[4]);
					ByteProgram(0x0405,Card_Type[5]);
					ByteProgram(0x0406,Card_Type_Flag);
					IAP_Disable();
				    gLedBuf[0] = 12;		 
					gLedBuf[1] = 12;
					gLedBuf[4] = 12;
					gLedBuf[5] = 12;
					Ding();
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);	
				}
				Delay_1ms(100);		 //这个延时最好是有
				if(gBuff[6] >= 0x01 && gBuff[6] <= 15)	 //如果是密码卡，1区1块的第7byte是0x01
				{
					Ding();
				    FC_FF_Card_count = 2;
					ucWorkState = gBuff[6];
                    ucPulseWeight = gBuff[7];
					SectorErase(0x0200);
					ByteProgram(0x0200,ucWorkState);
					IAP_Disable();
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
					for(i = 0;i < 2;i++)  {USER_Card_Count[i] = 0;}
					for(i = 0;i < 2;i++)  {One_MGM_Card_Count[i] = 0;}
					for(i = 0;i < 2;i++)  {Ten_MGM_Card_Count[i] = 0;}
					PWDS_Card_Count = 0;
                    for(i = 0;i < 5;i++)  {PLS_Card_Count[i] = 0;}
					for(i = 0;i < 16;i++) {AREA_Card_Count[i] = 0;}
				   	for(i = 0;i < 6;i++)
					{
						MGM_Card_PWD[i]  = gBuff[i];
						User_Card_PWD[i] = gBuff[i+10];
					}
                    
				    if((PWD_Card_Count == 88) || (PWD_Card_Count == 99)) 
					{
						PWD_Card_Count = 0;
					}
					else
					{
						PWD_Card_Count++;
						if(PWD_Card_Count > 8)
						    PWD_Card_Count = 0;
					} 
				}
				if((gBuff[6] <= 0x10) || (gBuff[6] >= 0x01))	  //密码卡和分区卡能进来
				{
					switch (PWD_Card_Count)
					{
					    case 0:  Show(USER_Card_Made     + USER_Card_Count[0]    / 100,USER_Card_Count[0] % 100);     break; //发行卡到用户卡
						case 1:  Show(USER_Card_Clear    + USER_Card_Count[1]    / 100,USER_Card_Count[1] % 100);     break; //用户卡到发行卡
						case 2:  Show(One_MGM_Card_Made  + One_MGM_Card_Count[0] / 100,One_MGM_Card_Count[0] % 100);  break; //发行卡到1元管理卡
						case 3:  Show(One_MGM_Card_Clear + One_MGM_Card_Count[1] / 100,One_MGM_Card_Count[1] % 100);  break; //1元管理卡到发行卡
						case 4:  Show(Ten_MGM_Card_Made  + Ten_MGM_Card_Count[0] / 100,Ten_MGM_Card_Count[0] % 100);  break; //发行卡到10元管理卡
						case 5:  Show(Ten_MGM_Card_Clear + Ten_MGM_Card_Count[1] / 100,Ten_MGM_Card_Count[1] % 100);  break; //10元管理卡到发行卡
						case 6:  Show(PWDS_Card_Made     + PWDS_Card_Count / 100,PWDS_Card_Count % 100);              break; //复制密码卡
						case 7:  Show(4100  + One_MGM_Card_Count[0] / 100,One_MGM_Card_Count[0] % 100);  break; //发行卡到测试卡
						case 8:  Show(4000 + One_MGM_Card_Count[1] / 100,One_MGM_Card_Count[1] % 100);  break; //测试卡到发行卡
						default:break;
					}
					gLedBuf[2]= 13;
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
				}	
			}
			break;
		case 0xff: //管理卡、用户卡加密；制作脉冲卡、分区卡；复制密码卡	
			if(FC_FF_Card_count == 0)	  //空白卡到发行卡
			{
				gBuff[0]=0xff;gBuff[1]=0xff;gBuff[2]=0xff;gBuff[3]=0xff;gBuff[4]=0xff;gBuff[5]=0xff;
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(ucWorkState,0,0x08)==3)
				{
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					if(Write_Block(gBuff,ucWorkState*4+0) != FM1702_OK) {Ding();Ding();break;}
                    
					if(Write_Block(gBuff,ucWorkState*4+1) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,ucWorkState*4+2) != FM1702_OK) {Ding();Ding();break;} //FC2730C2C4A8
					gBuff[0] = 0xfc;gBuff[1] = 0x27;gBuff[2]  = 0x30;gBuff[3]  = 0xc2;gBuff[4]  = 0xc4;gBuff[5]  = 0xa8;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
	                gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xfc;gBuff[11] = 0x27;gBuff[12] = 0x30;gBuff[13] = 0xc2;gBuff[14] = 0xc4;gBuff[15] = 0xa8;
					if(Write_Block(gBuff,ucWorkState*4+3) != FM1702_OK) {Ding();Ding();break;}
					FC_Card_Count++;
					if(FC_Card_Count > 200)	 FC_Card_Count = 1;
					Show(USER_Card_Made + FC_Card_Count / 100,FC_Card_Count % 100);
					gLedBuf[0]= 19;
					gLedBuf[1]= 16;
					gLedBuf[2]= 13;
					Ding();
					break;	
				}
			}
			if(FC_FF_Card_count == 1)	  //发行卡到空白卡
			{
				gBuff[0]=0xfc;gBuff[1]=0x27;gBuff[2]=0x30;gBuff[3]=0xc2;gBuff[4]=0xc4;gBuff[5]=0xa8;
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(ucWorkState,0,0x08)==3)
				{
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					if(Write_Block(gBuff,ucWorkState*4+0) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,ucWorkState*4+1) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,ucWorkState*4+2) != FM1702_OK) {Ding();Ding();break;} 
					gBuff[0] = 0xff;gBuff[1] = 0xff;gBuff[2]  = 0xff;gBuff[3]  = 0xff;gBuff[4]  = 0xff;gBuff[5]  = 0xff;gBuff[6]  = 0xff;gBuff[7]  = 0x07;
	                gBuff[8] = 0x80;gBuff[9] = 0x69;gBuff[10] = 0xff;gBuff[11] = 0xff;gBuff[12] = 0xff;gBuff[13] = 0xff;gBuff[14] = 0xff;gBuff[15] = 0xff;
					if(Write_Block(gBuff,ucWorkState*4+3) != FM1702_OK) {Ding();Ding();break;}
					FF_Card_Count++;
					if(FF_Card_Count > 200)	 FF_Card_Count = 1;
					Show(FF_Card_Count / 100,FF_Card_Count % 100);
					gLedBuf[0]= 19;
					gLedBuf[1]= 19;
					gLedBuf[2]= 13;
					Ding();
					break;	
				}
//				if(InitCard(ucWorkState,0,0x03) == 1)
//				{
//					Ding();
//					break;
//				}
			}
		    if(PWD_Card_Count == 0)	  //发行卡到用户卡
			{
				gBuff[0]=Card_Type[0];gBuff[1]=Card_Type[1];gBuff[2]=Card_Type[2];gBuff[3]=Card_Type[3];gBuff[4]=Card_Type[4];gBuff[5]=Card_Type[5];
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(ucWorkState,0,0x08)==3)
				{
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					if(Write_Block(gBuff,ucWorkState*4+0) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,ucWorkState*4+1) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,ucWorkState*4+2) != FM1702_OK) {Ding();Ding();break;}
					gBuff[0]  = User_Card_PWD[0];gBuff[1]  = User_Card_PWD[1];gBuff[2]  = User_Card_PWD[2];gBuff[3]  = User_Card_PWD[3];
					gBuff[4]  = User_Card_PWD[4];gBuff[5]  = User_Card_PWD[5];gBuff[6]  = 0xff;            gBuff[7]  = 0x07;
					gBuff[8]  = 0x80;            gBuff[9]  = 0x69;            gBuff[10] = User_Card_PWD[0];gBuff[11] = User_Card_PWD[1];
					gBuff[12] = User_Card_PWD[2];gBuff[13] = User_Card_PWD[3];gBuff[14] = User_Card_PWD[4];gBuff[15] = User_Card_PWD[5];
					if(Write_Block(gBuff,ucWorkState*4+3) != FM1702_OK) {Ding();Ding();break;}
					USER_Card_Count[0]++;
					if(USER_Card_Count[0] > 200)	 USER_Card_Count[0] = 1;
					Show(USER_Card_Made + USER_Card_Count[0] / 100,USER_Card_Count[0] % 100);
					gLedBuf[2]= 13;
					Ding();
					break;	
				}
				if(InitCard(ucWorkState,0,0x03) == 1)
				{
					Ding();
					break;
				}
			}
			if(PWD_Card_Count == 1)	  //用户卡到发行卡
			{
			    gBuff[0]=User_Card_PWD[0];gBuff[1]=User_Card_PWD[1];gBuff[2]=User_Card_PWD[2];gBuff[3]=User_Card_PWD[3];gBuff[4]=User_Card_PWD[4];gBuff[5]=User_Card_PWD[5];
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(ucWorkState,0,0x08)==3)
				{
					Card_Clear();
					break;
				}
			}
			if((PWD_Card_Count == 2) || (PWD_Card_Count == 4) || (PWD_Card_Count == 7)) //1元，10元管理卡加密，脉冲卡加密
			{
			    gBuff[0]=Card_Type[0];gBuff[1]=Card_Type[1];gBuff[2]=Card_Type[2];gBuff[3]=Card_Type[3];gBuff[4]=Card_Type[4];gBuff[5]=Card_Type[5];
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(1,0,0x08)==3)
				{
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					switch (PWD_Card_Count)
					{
					    case 2:  gBuff[0] = 1;    gBuff[1] = 10; break; //1元管理卡
						case 4:  gBuff[0] = 0xfa; gBuff[1] = 1;  break; //10元管理卡
						case 7:  gBuff[0] = 0xff; gBuff[1] = 0xff;  break; //10ms脉冲管理卡
						case 8:  gBuff[0] = 0xf0; gBuff[1] = 2;  break; //20ms脉冲管理卡
						case 9:  gBuff[0] = 0xf0; gBuff[1] = 3;  break; //30ms脉冲管理卡
						case 10: gBuff[0] = 0xf0; gBuff[1] = 4;  break; //40ms脉冲管理卡
						case 11: gBuff[0] = 0xf0; gBuff[1] = 5;  break; //50ms脉冲管理卡
						default:break;
					}
					if(Write_Block(gBuff,1*4+0) != FM1702_OK) {Ding();Ding();break;}
					gBuff[0] = 0; gBuff[1] = 0; 
					if(Write_Block(gBuff,1*4+1) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,1*4+2) != FM1702_OK) {Ding();Ding();break;}

					gBuff[0]  = MGM_Card_PWD[0];gBuff[1]  = MGM_Card_PWD[1];gBuff[2]  = MGM_Card_PWD[2];gBuff[3]  = MGM_Card_PWD[3];
					gBuff[4]  = MGM_Card_PWD[4];gBuff[5]  = MGM_Card_PWD[5];gBuff[6]  = 0xff;           gBuff[7]  = 0x07;
					gBuff[8]  = 0x80;           gBuff[9]  = 0x69;           gBuff[10] = MGM_Card_PWD[0];gBuff[11] = MGM_Card_PWD[1];
					gBuff[12] = MGM_Card_PWD[2];gBuff[13] = MGM_Card_PWD[3];gBuff[14] = MGM_Card_PWD[4];gBuff[15] = MGM_Card_PWD[5];
					if(Write_Block(gBuff,1*4+3) != FM1702_OK) {Ding();Ding();break;}
					switch (PWD_Card_Count)
					{
					    case 2: One_MGM_Card_Count[0]++;
					            if(One_MGM_Card_Count[0] > 200)	 One_MGM_Card_Count[0] = 1;
					            Show(One_MGM_Card_Made + One_MGM_Card_Count[0] / 100,One_MGM_Card_Count[0] % 100);break; //1元管理卡
						case 4: Ten_MGM_Card_Count[0]++;
					            if(Ten_MGM_Card_Count[0] > 200)	 Ten_MGM_Card_Count[0] = 1;
					            Show(Ten_MGM_Card_Made + Ten_MGM_Card_Count[0] / 100,Ten_MGM_Card_Count[0] % 100);break; //10元管理卡     
						case 7: PLS_Card_Count[0]++;
					            if(PLS_Card_Count[0] > 200)	 PLS_Card_Count[0] = 1;
					            Show(PLS_Card_10ms_Made + PLS_Card_Count[0] / 100,PLS_Card_Count[0] % 100);	     break; //脉冲管理卡
						case 8: PLS_Card_Count[1]++;
					            if(PLS_Card_Count[1] > 200)	 PLS_Card_Count[1] = 1;
					            Show(PLS_Card_20ms_Made + PLS_Card_Count[1] / 100,PLS_Card_Count[1] % 100);	     break; //脉冲管理卡
						case 9: PLS_Card_Count[2]++;
					            if(PLS_Card_Count[2] > 200)	 PLS_Card_Count[2] = 1;
					            Show(PLS_Card_30ms_Made + PLS_Card_Count[2] / 100,PLS_Card_Count[2] % 100);	     break; //脉冲管理卡
						case 10:PLS_Card_Count[3]++;
					            if(PLS_Card_Count[3] > 200)	 PLS_Card_Count[3] = 1;
					            Show(PLS_Card_40ms_Made + PLS_Card_Count[3] / 100,PLS_Card_Count[3] % 100);	     break; //脉冲管理卡
						case 11:PLS_Card_Count[4]++;
					            if(PLS_Card_Count[4] > 200)	 PLS_Card_Count[4] = 1;
					            Show(PLS_Card_50ms_Made + PLS_Card_Count[4] / 100,PLS_Card_Count[4] % 100);	     break; //脉冲管理卡  
						default:break;
					}
					gLedBuf[2]= 13;
					Ding();
					break;
				}
				if(InitCard(1,0,0x03) == 0)  //已经加密过的管理卡
				{
					Ding();
					break;
				}
			}
			if((PWD_Card_Count == 3) || (PWD_Card_Count == 5)|| (PWD_Card_Count == 8))
			{
			    gBuff[0]=MGM_Card_PWD[0];gBuff[1]=MGM_Card_PWD[1];gBuff[2]=MGM_Card_PWD[2];gBuff[3]=MGM_Card_PWD[3];gBuff[4]=MGM_Card_PWD[4];gBuff[5]=MGM_Card_PWD[5];
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(1,0,0x08)==3)
				{
					Card_ClearMEM();
					break;
				}
			}
			if((PWD_Card_Count == 6) || ((PWD_Card_Count >= 12) && (PWD_Card_Count <= 27)))	  //复制密码卡,制作分区卡
			{
			    gBuff[0]=Card_Type[0];gBuff[1]=Card_Type[1];gBuff[2]=Card_Type[2];gBuff[3]=Card_Type[3];gBuff[4]=Card_Type[4];gBuff[5]=Card_Type[5];
				if(ChangePWD(3,0))
				{
					Ding();
					Ding();
					break;
				}
				if(InitCard(1,0,0x08)==3)
				{
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					switch (PWD_Card_Count)
					{
					    case 6:	 gBuff[0]  = MGM_Card_PWD[0]; gBuff[1]  = MGM_Card_PWD[1];gBuff[2]   = MGM_Card_PWD[2];gBuff[3]   = MGM_Card_PWD[3];
						         gBuff[4]  = MGM_Card_PWD[4]; gBuff[5]  = MGM_Card_PWD[5];gBuff[6]   = ucWorkState;    gBuff[7]   = ucPulseWeight;
						         gBuff[8]  = 0x00;            gBuff[9]  = 0x00;           gBuff[10]  = User_Card_PWD[0];gBuff[11] = User_Card_PWD[1];
						         gBuff[12] = User_Card_PWD[2];gBuff[13] = User_Card_PWD[3];gBuff[14] = User_Card_PWD[4];gBuff[15] = User_Card_PWD[5];	break;
					    case 12: gBuff[6] = 0x10; gBuff[7] = 0x00;	  break;
						case 13: gBuff[6] = 0x10; gBuff[7] = 0x01;	  break;
						case 14: gBuff[6] = 0x10; gBuff[7] = 0x02;	  break;
						case 15: gBuff[6] = 0x10; gBuff[7] = 0x03;	  break;
						case 16: gBuff[6] = 0x10; gBuff[7] = 0x04;	  break;
						case 17: gBuff[6] = 0x10; gBuff[7] = 0x05;	  break;
						case 18: gBuff[6] = 0x10; gBuff[7] = 0x06;	  break;
						case 19: gBuff[6] = 0x10; gBuff[7] = 0x07;	  break;
						case 20: gBuff[6] = 0x10; gBuff[7] = 0x08;	  break;
						case 21: gBuff[6] = 0x10; gBuff[7] = 0x09;	  break;
						case 22: gBuff[6] = 0x10; gBuff[7] = 0x0a;	  break;
						case 23: gBuff[6] = 0x10; gBuff[7] = 0x0b;	  break;
						case 24: gBuff[6] = 0x10; gBuff[7] = 0x0c;	  break;
						case 25: gBuff[6] = 0x10; gBuff[7] = 0x0d;	  break;
						case 26: gBuff[6] = 0x10; gBuff[7] = 0x0e;	  break;
						case 27: gBuff[6] = 0x10; gBuff[7] = 0x0f;	  break;
					} 
					if(Write_Block(gBuff,1*4+0) != FM1702_OK) {Ding();Ding();break;}
					for(i = 0; i < 16; i++)
					{
					    gBuff[i] = 0;
					}
					if(Write_Block(PWD_Data,1*4+1) != FM1702_OK) {Ding();Ding();break;}
					if(Write_Block(gBuff,1*4+2) != FM1702_OK) {Ding();Ding();break;}
					gBuff[0]  = PWD_Card_PWD[0];gBuff[1]  = PWD_Card_PWD[1];gBuff[2]  = PWD_Card_PWD[2];gBuff[3]  = PWD_Card_PWD[3];
					gBuff[4]  = PWD_Card_PWD[4];gBuff[5]  = PWD_Card_PWD[5];gBuff[6]  = 0xff;           gBuff[7]  = 0x07;
					gBuff[8]  = 0x80;           gBuff[9]  = 0x69;           gBuff[10] = PWD_Card_PWD[0];gBuff[11] = PWD_Card_PWD[1];
					gBuff[12] = PWD_Card_PWD[2];gBuff[13] = PWD_Card_PWD[3];gBuff[14] = PWD_Card_PWD[4];gBuff[15] = PWD_Card_PWD[5];
					if(Write_Block(gBuff,1*4+3) != FM1702_OK) {Ding();Ding();break;}
				    switch (PWD_Card_Count)
					{
					    case 6:	 PWDS_Card_Count++;
						         if(PWDS_Card_Count > 200) PWDS_Card_Count = 1;
					             Show(PWDS_Card_Made + PWDS_Card_Count / 100,PWDS_Card_Count % 100);	       break;
					    case 12: AREA_Card_Count[0]++;
						         if(AREA_Card_Count[0] > 200) AREA_Card_Count[0] = 1;
					             Show(AREA_0_Card_Made + AREA_Card_Count[0] / 100,AREA_Card_Count[0] % 100);   break;
						case 13: AREA_Card_Count[1]++;
						         if(AREA_Card_Count[1] > 200) AREA_Card_Count[1] = 1;
					             Show(AREA_1_Card_Made + AREA_Card_Count[1] / 100,AREA_Card_Count[1] % 100);   break;
						case 14: AREA_Card_Count[2]++;
						         if(AREA_Card_Count[2] > 200) AREA_Card_Count[2] = 1;
					             Show(AREA_2_Card_Made + AREA_Card_Count[2] / 100,AREA_Card_Count[2] % 100);   break;
						case 15: AREA_Card_Count[3]++;
						         if(AREA_Card_Count[3] > 200) AREA_Card_Count[3] = 1;
					             Show(AREA_3_Card_Made + AREA_Card_Count[3] / 100,AREA_Card_Count[3] % 100);   break;
						case 16: AREA_Card_Count[4]++;
						         if(AREA_Card_Count[4] > 200) AREA_Card_Count[4] = 1;
					             Show(AREA_4_Card_Made + AREA_Card_Count[4] / 100,AREA_Card_Count[4] % 100);   break;
						case 17: AREA_Card_Count[5]++;
						         if(AREA_Card_Count[5] > 200) AREA_Card_Count[5] = 1;
					             Show(AREA_5_Card_Made + AREA_Card_Count[5] / 100,AREA_Card_Count[5] % 100);   break;
						case 18: AREA_Card_Count[6]++;
						         if(AREA_Card_Count[6] > 200) AREA_Card_Count[6] = 1;
					             Show(AREA_6_Card_Made + AREA_Card_Count[6] / 100,AREA_Card_Count[6] % 100);   break;
						case 19: AREA_Card_Count[7]++;
						         if(AREA_Card_Count[7] > 200) AREA_Card_Count[7] = 1;
					             Show(AREA_7_Card_Made + AREA_Card_Count[7] / 100,AREA_Card_Count[7] % 100);   break;
						case 20: AREA_Card_Count[8]++;
						         if(AREA_Card_Count[8] > 200) AREA_Card_Count[8] = 1;
					             Show(AREA_8_Card_Made + AREA_Card_Count[8] / 100,AREA_Card_Count[8] % 100);   break;
						case 21: AREA_Card_Count[9]++;
						         if(AREA_Card_Count[9] > 200) AREA_Card_Count[9] = 1;
					             Show(AREA_9_Card_Made + AREA_Card_Count[9] / 100,AREA_Card_Count[9] % 100);   break;
						case 22: AREA_Card_Count[10]++;
						         if(AREA_Card_Count[10] > 200) AREA_Card_Count[10] = 1;
					             Show(AREA_A_Card_Made + AREA_Card_Count[10] / 100,AREA_Card_Count[10] % 100); 
								 gLedBuf[1] = 14;															   break;
						case 23: AREA_Card_Count[11]++;
						         if(AREA_Card_Count[11] > 200) AREA_Card_Count[11] = 1;
					             Show(AREA_B_Card_Made + AREA_Card_Count[11] / 100,AREA_Card_Count[11] % 100); 
								 gLedBuf[1] = 15;															   break;
						case 24: AREA_Card_Count[12]++;
						         if(AREA_Card_Count[12] > 200) AREA_Card_Count[12] = 1;
					             Show(AREA_C_Card_Made + AREA_Card_Count[12] / 100,AREA_Card_Count[12] % 100); 
								 gLedBuf[1] = 16;															   break;
						case 25: AREA_Card_Count[13]++;
						         if(AREA_Card_Count[13] > 200) AREA_Card_Count[13] = 1;
					             Show(AREA_D_Card_Made + AREA_Card_Count[13] / 100,AREA_Card_Count[13] % 100); 
								 gLedBuf[1] = 17;															   break;
						case 26: AREA_Card_Count[14]++;
						         if(AREA_Card_Count[14] > 200) AREA_Card_Count[14] = 1;
					             Show(AREA_E_Card_Made + AREA_Card_Count[14] / 100,AREA_Card_Count[14] % 100); 
								 gLedBuf[1] = 18;															   break;
						case 27: AREA_Card_Count[15]++;
						         if(AREA_Card_Count[15] > 200) AREA_Card_Count[15] = 1;
					             Show(AREA_F_Card_Made + AREA_Card_Count[15] / 100,AREA_Card_Count[15] % 100); 
								 gLedBuf[1] = 19;															   break;
					} 
					gLedBuf[2]= 13;
					Ding();
					Delay_1ms(200);
					Delay_1ms(200);	
					Delay_1ms(200);
					Delay_1ms(200);
					Delay_1ms(200);
				}
			}
			break;
			default:break;
		}
	}	  
}



































