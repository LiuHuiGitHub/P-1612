#ifndef _LED_H
#define _LED_H

extern bit gShowDot;
extern unsigned char idata gLedBuf[6]; // ������Ϊ0,1,2
void Show(unsigned int unData,unsigned char ucData);
void Timer0_initialize(void);

#endif
