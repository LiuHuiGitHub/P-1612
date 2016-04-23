#ifndef __FM1702SL_H__
#define __FM1702SL_H__

#define CALL_isr_UART()         TI = 1

#define USER_Card_Made          100
#define USER_Card_Clear         0
#define One_MGM_Card_Made       1100
#define One_MGM_Card_Clear      1000
#define Ten_MGM_Card_Made       2100
#define Ten_MGM_Card_Clear      2000
#define PWDS_Card_Made          3100
#define PLS_Card_10ms_Made      4100
#define PLS_Card_20ms_Made      4200
#define PLS_Card_30ms_Made      4300
#define PLS_Card_40ms_Made      4400
#define PLS_Card_50ms_Made      4500
#define AREA_0_Card_Made        5000
#define AREA_1_Card_Made        5100
#define AREA_2_Card_Made        5200
#define AREA_3_Card_Made        5300
#define AREA_4_Card_Made        5400
#define AREA_5_Card_Made        5500
#define AREA_6_Card_Made        5600
#define AREA_7_Card_Made        5700
#define AREA_8_Card_Made        5800
#define AREA_9_Card_Made        5900
#define AREA_A_Card_Made        5100
#define AREA_B_Card_Made        5100
#define AREA_C_Card_Made        5100
#define AREA_D_Card_Made        5100
#define AREA_E_Card_Made        5100
#define AREA_F_Card_Made        5100


extern unsigned char idata RevBuffer[40];

#endif
