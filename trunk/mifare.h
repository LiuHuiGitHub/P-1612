#ifndef _MIFARE_H
#define _MIFARE_H
#include "normal.h"

//;==============================================
//;FM1702SL控制寄存器定义
//;==============================================
#define		Page_Reg                0x00
#define		Command_Reg             0x01
#define		FIFO_Reg                0x02
#define		FIFOLength_Reg		    0x04
#define		SecondaryStatus_Reg  	0x05
#define		InterruptEn_Reg		    0x06
#define		InterruptRq_Reg		    0x07
#define		Control_Reg		        0x09
#define		ErrorFlag_Reg           0x0A
#define		BitFraming_Reg	     	0x0F
#define		TxControl_Reg           0x11
#define		CwConductance_Reg	    0x12
#define		RxControl2_Reg		    0x1E
#define     RxWait_Reg              0x21
#define		ChannelRedundancy_Reg	0x22


//;==============================================
//;FM1702SL发送命令代码
//;==============================================
#define		WriteEE			  	0x01
#define		LoadKeyE2		  	0x0B
#define    	Transmit			0x1A
#define		Transceive		  	0x1E
#define		Authent1		  	0x0C
#define		Authent2		  	0x14

//;==============================================
//;函数错误代码定义
//;==============================================
#define FM1702_OK			0		// 正确 
#define FM1702_NOTAGERR		1		// 无卡 
#define FM1702_CRCERR		2		//卡片CRC校验错误
#define FM1702_EMPTY		3		// 数值溢出错误 
#define FM1702_AUTHERR		4		//验证不成功
#define FM1702_PARITYERR    5		//卡片奇偶校验错误
#define FM1702_CODEERR		6		//通讯错误(BCC校验错)
#define FM1702_SERNRERR		8		//卡片序列号错误(anti-collision 错误)
#define FM1702_SELECTERR    9		//卡片数据长度字节错误(SELECT错误)
#define FM1702_NOTAUTHERR	10		// 0x0A 卡片没有通过验证 
#define FM1702_BITCOUNTERR	11		//从卡片接收到的位数错误
#define FM1702_BYTECOUNTERR	12		//从卡片接收到的字节数错误仅读函数有效
#define FM1702_RESTERR		13		//调用restore函数出错/
#define FM1702_TRANSERR		14		//调用transfer函数出错/
#define FM1702_WRITEERR		15		// 0x0F 调用write函数出错 
#define FM1702_INCRERR		16		// 0x10 调用increment函数出错 
#define FM1702_DECRERR		17      // 0x11 调用decrement函数出错 
#define FM1702_READERR		18      // 0x12 调用read函数出错 
#define FM1702_LOADKEYERR	19      // 0x13 调用LOADKEY函数出错 
#define FM1702_FRAMINGERR	20      // 0x14 FM1702帧错误 
#define FM1702_REQERR		21      // 0x15 调用req函数出错 
#define FM1702_SELERR		22      // 0x16 调用sel函数出错 
#define FM1702_ANTICOLLERR	23      // 0x17 调用anticoll函数出错 
#define FM1702_INTIVALERR	24      // 0x18 调用初始化函数出错 
#define FM1702_READVALERR	25      // 0x19 调用高级读块值函数出错
#define FM1702_DESELECTERR	26      // 0x1A
#define FM1702_CMD_ERR		42      // 0x2A 命令错误

//;==============================================
//;射频卡通信命令码定义
//;==============================================
#define RF_CMD_REQUEST_STD	0x26
#define RF_CMD_REQUEST_ALL	0x52
#define RF_CMD_ANTICOL		0x93
#define RF_CMD_SELECT		0x93
#define RF_CMD_AUTH_LA		0x60
#define RF_CMD_AUTH_LB		0x61
#define RF_CMD_READ         0x30
#define RF_CMD_WRITE		0xa0
#define RF_CMD_INC		    0xc1
#define RF_CMD_DEC		    0xc0
#define RF_CMD_RESTORE		0xc2
#define RF_CMD_TRANSFER		0xb0
#define RF_CMD_HALT		    0x50


extern uchar idata gBuff[16];             //M1卡数据块读取缓冲区
extern uchar idata gCard_UID[5];	//4个字节卡号（32位），一个校验字节

uchar SPIRead(uchar SpiAddress,uchar *ramadr,uchar width);
uchar SPIWrite(uchar SpiAddress,uchar *ramadr,uchar width);
////===============================================
//以下为FM1702读写的子程序
//===============================================
bool 	Init_FM1702(void);
uchar 	Read_FIFO(uchar idata *buff);
uchar 	SPIReadOne(uchar SpiAddress);
uchar   Command_Send(uchar Comm_Set,uchar idata *buff, uchar count);
uchar 	Request(uchar mode);
uchar 	AntiColl(void);
uchar 	SelectCard(void);
uchar 	Load_Key(uchar n,uchar *ramadr);
uchar 	Load_Key_EE(uchar index);
uchar 	Authentication(uchar idata *UID, uchar SecNR, uchar mode);
uchar 	rdbuff(uchar sq,uchar n,uchar ucKeyStyle);
uchar   Read_Block(uchar idata *buff,uchar index);
uchar   Write_Block(uchar idata *buff,uchar index);
uchar 	MIF_Halt(void);

#endif