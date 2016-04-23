#include <intrins.h>
#include "normal.h"
#include "reg52.h"

void Delay_10us(uchar _10us)
{
    while(_10us--)
	{
	    _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
	    _nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
		_nop_();_nop_();		
//		_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();_nop_();
//		_nop_();	  //6.79MHz
    }
}

///////////////////////////////////////////////////////////////////////
// Delay 1ms
///////////////////////////////////////////////////////////////////////
void Delay_1ms(uchar _1ms)
{
    while (_1ms--)
    {
	    Delay_10us(100);
    }
}

///////////////////////////////////////////////////////////////////////
// 蜂鸣器 响一声
///////////////////////////////////////////////////////////////////////
void Ding(void)
{
	SPEAKER=1;
	Delay_1ms(150);
	SPEAKER=0;
	Delay_1ms(80);
}

/* 关闭IAP功能，清除相关特殊寄存器 */
void IAP_Disable(void)
{
	ISP_CONTR=0;
	ISP_CMD=0;
	ISP_TRIG=0;
	ISP_ADDRH = 0X80;
	ISP_ADDRL = 0X00;
}
/* 扇区擦除 */
void SectorErase(uint unDataAddr)
{
	uint unAddrTmp;
	unAddrTmp = unDataAddr + DATA_FLASH_START_ADDRESS;

	ISP_ADDRH=(uchar)(unAddrTmp>>8);
	ISP_ADDRL=0;
	ISP_CMD = ISP_IAP_BYTE_ERASE;
	ISP_CONTR = ENABLE_ISP;
	EA=0;
	ISP_TRIG=0x5A;
	ISP_TRIG=0xA5;
	EA=1;
}

/* 读一字节*/
uchar ByteRead(uint unDataAddr)
{
	uint unAddrTmp;
	unAddrTmp = unDataAddr + DATA_FLASH_START_ADDRESS;
	
	ISP_ADDRH=(uchar)(unAddrTmp>>8);
	ISP_ADDRL=(uchar)(unAddrTmp&0x00ff);
	ISP_CMD = ISP_IAP_BYTE_READ;
	ISP_CONTR = ENABLE_ISP;
	EA=0;
	ISP_TRIG=0X5A;
	ISP_TRIG=0XA5;
	EA=1;
	return ISP_DATA;
}

/* 编程一个字节 */
void ByteProgram(uint unAddr, uchar ucData)
{
	uint unAddrTmp;
	unAddrTmp = unAddr + DATA_FLASH_START_ADDRESS;

	ISP_ADDRH=(uchar)(unAddrTmp>>8);
	ISP_ADDRL=(uchar)(unAddrTmp&0x00ff);
	ISP_CMD = ISP_IAP_BYTE_PROGRAM;
	ISP_CONTR = ENABLE_ISP;
	ISP_DATA = ucData;
	EA=0;
	ISP_TRIG=0x5A;
	ISP_TRIG=0xA5;
	EA=1;
}
