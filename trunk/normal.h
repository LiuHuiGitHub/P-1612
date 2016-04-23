#ifndef _NORMAL_H
#define _NORMAL_H

#define uchar 	unsigned char
#define uint	unsigned int
#define ulong	unsigned long

#define TRUE	1
#define FALSE	0	
#define true	1
#define false	0	
#define bool	bit

//����ISP/IAP����
#define ISP_IAP_BYTE_READ 		1	   //�ֽڶ�
#define ISP_IAP_BYTE_PROGRAM 	2	   //�ֽڱ�̣����Խ�1д��0
#define ISP_IAP_BYTE_ERASE 		3	   //�������������Խ�0����1

// ����Flash�����ȴ�ʱ�䲢��IAP 
#define ENABLE_ISP 0X82		//ϵͳʱ��<20MHz
// ����EEPROM��ʼ��ַ
#define DATA_FLASH_START_ADDRESS 0X0		//STC11/STC10xxϵ�е�Ƭ��EEPROM��ʼ��ַ

// EEPROM��д��غ��� һϵ��IAP��������Ҫ����IAP_Disable�ر�IAP�������ؼĴ��� 
void ByteProgram(uint unAddr,uchar ucData);
uchar ByteRead(uint unDataAddr);
void SectorErase(uint unDataAddr);
void IAP_Disable(void);

void Ding(void);
void Delay_10us(uchar _10us);
void Delay_1ms(uchar _1ms);


#endif