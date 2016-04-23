#include <intrins.h>
#include "mifare.h"
#include "reg52.h"
#include "Led.h"
#include "fm1702sl.h"

////////////////////ȫ�ֱ���////////////////////////////////
uchar idata gBuff[16];
uchar idata gCard_UID[5];

//=======================================================
//���ƣ�SPIRead
//���ܣ�SPI��ȡ����
//���:
//SpiAddress:	Ҫ��ȡ��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//*ramadr:   	��Ŷ�ȡ���ݵ�Ram���׵�ַ
//width:    		��ȡ���ֽ���
//����: 		0:�ɹ�,1:ʧ�� (  ����ַ����λ��Ϊ��ʱ����1 ) 
//========================================================
uchar SPIRead(uchar SpiAddress,uchar *ramadr,uchar width)
{
	uchar i,j,adrtemp;

	//��ַ�ֽ����λ��1����ʾΪ�����ݣ����λ�̶�Ϊ0���м�6λΪ��ַ
	//�����ַ �Ӹ�λ����λ����
	adrtemp=SpiAddress;
	if ((adrtemp&0xc0)==0)
	{
		adrtemp=((adrtemp<<1)|0x80);
		RC500CS=0;
		for (i=0;i<8;i++)
		{
			if (adrtemp&0x80)RC500SO=1;
			else RC500SO=0;
			RC500SCK=1;
			adrtemp=adrtemp<<1;
			RC500SCK=0;
		}

		for (j=0;j<width;j++)
		{
			if (j!=width-1)  adrtemp=(SpiAddress|0x40)<<1;
			else   adrtemp=0;
			ramadr[j]=0;
			for (i=0;i<8;i++)
			{
				if ((adrtemp<<i)&0x80)RC500SO=1;
				else RC500SO=0;
				RC500SCK=1;
				ramadr[j]=ramadr[j]<<1;
				if (RC500SI)ramadr[j]+=0x1;
				RC500SCK=0;
			}
		}
		RC500CS=1;
		return(0);
	}
	else return(1);
}

//=======================================================
//	���ƣ�	SPIReadOne
//	���ܣ�	SPI��ȡ����
//	���:
//		SpiAddress:	Ҫ��ȡ��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//	����: 	
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������	
//========================================================
uchar SPIReadOne(uchar SpiAddress)
{
	uchar data i,rdata; 
	//��ַ�ֽ����λ��1����ʾΪ�����ݣ����λ�̶�Ϊ0���м�6λΪ��ַ
	SpiAddress = SpiAddress<<1;
	SpiAddress = SpiAddress | 0x80;
	//�����ַ �Ӹ�λ����λ����
	RC500CS = 0;
	for (i=0;i<8;i++)
	{
		if(SpiAddress&0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		SpiAddress = SpiAddress<<1;
		RC500SCK = 0;
	}
	
	//��������
	rdata = 0;
	for (i=0;i<8;i++)
	{
		RC500SCK = 1;
		rdata = rdata<<1;
		if (RC500SI)
			rdata+= 1;
		RC500SCK = 0;
	}
	RC500CS=1;
	return (rdata);
}

//=======================================================
//���ƣ�SPIWrite
//���ܣ�SPIд�����
//���:	SpiAddress:	Ҫд��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//		*ramadr ��  	Ҫд���������Ram�е��׵�ַ
//		width:    		Ҫд����ֽ���
//����: 		0:�ɹ�,1:ʧ��(  ����ַ����λ��Ϊ��ʱ����1 ) 
//========================================================
uchar SPIWrite(uchar SpiAddress,uchar *ramadr,uchar width)
{
	uchar i,j,adrtemp;
	adrtemp=SpiAddress;
	if ((adrtemp&0xc0)==0)
	{
		adrtemp=((adrtemp<<1)&0x7e);
		RC500CS=0;
		for (i=0;i<8;i++)
		{
			if (adrtemp&0x80)RC500SO=1;
			else RC500SO=0;
			RC500SCK=1;
			adrtemp=adrtemp<<1;
			RC500SCK=0;
		}

		for (j=0;j<width;j++)
		{
			adrtemp= ramadr[j];
			for (i=0;i<8;i++)
			{
				if ((adrtemp<<i)&0x80)RC500SO=1;
				else RC500SO=0;
				RC500SCK=1;
				_nop_();
				RC500SCK=0;
			}
		}
		RC500CS=1;
		return(0);
	}
	else return(1);
}

//=======================================================
//	���ƣ�SPIWriteOne
//	���ܣ�SPIд�����
//	���:	SpiAddress:	Ҫд��FM1702SL�ڵļĴ�����ַ[0x01~0x3f]
//		wData ��  	Ҫд�������
//	����: 	
//	˵����Ϊ������ٶȣ��Ե�ַ����Ч�Բ������	
//========================================================
void SPIWriteOne(uchar SpiAddress,uchar wData)
{
	uchar i;
	SpiAddress = SpiAddress<<1;
	SpiAddress = SpiAddress & 0x7E;

	//�����ַ �Ӹ�λ����λ����
	RC500CS = 0;
	for (i=0;i<8;i++)
	{
		if(SpiAddress&0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		SpiAddress = SpiAddress<<1;
		RC500SCK = 0;
	}
	//�������ݣ��Ӹ�λ��ʼ
	for (i=0;i<8;i++)
	{
		if(wData & 0x80)
			RC500SO = 1;
		else
			RC500SO = 0;
		RC500SCK = 1;
		wData = wData<<1;
		RC500SCK = 0;
	}
	RC500CS = 1;
}


//=======================================================
//	����: Init_FM1702 
//	����: �ú���ʵ�ֶ�FM1702��ʼ������
//	����:N/A
//	���: N/A 
//=======================================================
bool Init_FM1702(void)
{
	
	uchar temp;
	uint	i;

	RC500SCK = 0;
	RC500SO = 1;
	RC500SI = 1;
	RC500RST = 1;	// FM1702��λ 
	Delay_1ms(2);
	RC500RST = 0;	
	//���븴λ�׶Σ���Ҫ512��FM1702SL��ʱ�����ڣ�Լ38us
	Delay_10us(1); 
	//�����ʼ���׶Σ���Ҫ128��ʱ�����ڣ�Լ10us
	for(i = 0;i<250;i++)// �ȴ�Command = 0,FM1702��λ�ɹ�
	{
		temp = SPIReadOne(Command_Reg);
		if(temp==0)
			break;
	}
	if(i==250)
	{
		Ding();
		Ding();
		return false;
	}
///////////////////////////////////////////////////////////////////////////////////
	SPIWriteOne(Page_Reg,0x80);	//��ʼ��SPI�ӿ�
	for(i = 0; i < 0x1fff; i++) // ��ʱ
	{
		temp = SPIReadOne(Command_Reg);
		if(temp == 0x00)	//SPI��ʼ���ɹ�
		{
			SPIWriteOne(Page_Reg,0);//����ʹ��SPI�ӿ�
			break;
		}
	}
////////////////////////////////////////////////////////////////////////////////////
	SPIWriteOne(InterruptEn_Reg,0x7F);	//  ��ֹ�����ж��������λ��0��
	SPIWriteOne(InterruptRq_Reg,0x7F);	// ��ֹ�����ж������ʶ��0�����λ��0��
	
	//���õ�����������ԴΪ�ڲ�������, ��������TX1��TX2
	SPIWriteOne(TxControl_Reg,0x5B); 		// ���Ϳ��ƼĴ��� 
	SPIWriteOne(RxControl2_Reg,0x01);
	SPIWriteOne(RxWait_Reg,5);
	return true;
}

//=======================================================
//	����: Request
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ��Request����
//			��������M1���ĸ�λ��Ӧ
//	����:	mode: ALL(�������FM1702������Χ֮�ڵĿ�Ƭ) 
//			STD(�����FM1702������Χ֮�ڴ���HALT״̬�Ŀ�Ƭ) 
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_REQERR: Ӧ�����
//=======================================================
uchar Request(uchar mode)
{
	
	uchar idata	temp;
	
	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg,0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg,0x03);
	//��������bit�ĸ�ʽ
	SPIWriteOne(BitFraming_Reg,0x07);
	gBuff[0] = mode;		//Requestģʽѡ�� 
	temp = SPIReadOne(Control_Reg);
	temp = temp & (0xf7);	
	SPIWriteOne(Control_Reg,temp);			//Control reset value is 00
	temp = Command_Send(Transceive, gBuff,1 );   //���ͽ������� 
	if(temp==0 )
	{
		return FM1702_NOTAGERR;
	}

	temp=Read_FIFO(gBuff);		//��FIFO�ж�ȡӦ����Ϣ��RevBuffer[]�� 
	// �ж�Ӧ���ź��Ƿ���ȷ 
	// 2  	Mifare Pro ��
	// 4 		Mifare One ��
	if((gBuff[0] == 0x04) & (gBuff[1] == 0x0) &(temp == 2))
	{
			return FM1702_OK;
	}
	return FM1702_REQERR;
}

//=======================================================
//���ƣ�Clear_FIFO
//���ܣ����FM1702��FIFO
//����:
//���: TRUE:�ɹ�,FALSE:ʧ��
//========================================================
uchar Clear_FIFO(void)
{
    uchar ucResult,i;
    ucResult = SPIReadOne(Control_Reg);
    ucResult |=0x01;
    SPIWriteOne(Control_Reg,ucResult);
    for(i=0;i<0xA0;i++)
    {
        ucResult = SPIReadOne(FIFOLength_Reg);
        if(ucResult == 0)
            return TRUE;
    }
    return FALSE;
}

//=======================================================
//���ƣ�Write_FIFO
//���ܣ�������д��FM1702��FIFO
//����:	buff��Ҫд���������Ram�е��׵�ַ
//		count��	Ҫд����ֽ���
//���: 
//========================================================
void Write_FIFO(uchar *buff,uchar count)
{
	if(count == 0)
        return;
	SPIWrite(FIFO_Reg,buff,count);
}

//=======================================================
//	����: ReadFIFO
//	����: �ú���ʵ�ִ�FM1702��FIFO�ж���x bytes����
//	����:	buff, ָ��������ݵ�ָ��
//	���: 	���ݳ��ȣ���λ���ֽڣ� 
//=======================================================
uchar Read_FIFO(uchar idata *buff)
{
	uchar	ucResult;
	ucResult = SPIReadOne(FIFOLength_Reg);
	if(ucResult == 0 || ucResult>16)
		return 0;
	SPIRead(FIFO_Reg,buff,ucResult);

	return ucResult;
}

//=======================================================
//	����: Command_Send
//	����: �ú���ʵ����FM1702��������Ĺ���
//	����:	count, ����������ĳ���  
//			buff, ָ����������ݵ�ָ�� 
//			Comm_Set, �����룺ָFM1702����IC��������
//	���: 	TRUE, �����ȷִ�� 
//			FALSE, ����ִ�д���
//=======================================================
uchar Command_Send(uchar Comm_Set,uchar idata *buff, uchar count)
{
    uchar ucResult1,ucResult2,i;
    SPIWriteOne(Command_Reg,0x00);
    if(Clear_FIFO()==FALSE)
        return FALSE;
    Write_FIFO(buff,count);
	SPIWriteOne(Command_Reg,Comm_Set);
	for (i=0;i<0xA0;i++)
	{
        ucResult1 = SPIReadOne(Command_Reg);
        ucResult2 = SPIReadOne(InterruptRq_Reg) & 0x80;
        if(ucResult1 == 0 || ucResult2 ==0x80)
            return TRUE;
	}
	return FALSE;
}

//=======================================================
//	����: AntiColl
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵĿ�Ƭ�ķ���ͻ���
//	����:	N/A
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//			FM1702_SERNRERR: ��Ƭ���к�Ӧ�����
//=======================================================
uchar AntiColl(void)
{
	
	uchar	temp,i;

	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg,0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg,0x03);
	
	gBuff[0] = RF_CMD_ANTICOL;
	gBuff[1] = 0x20;
	temp = Command_Send(Transceive,gBuff,2);
	if (temp==0)
	{
		return(FM1702_NOTAGERR);
	}

	temp = SPIReadOne(FIFOLength_Reg);
	if (temp == 0)
	{
		return FM1702_BYTECOUNTERR;
	}

	SPIRead(FIFO_Reg,gBuff,temp);
	temp = SPIReadOne(ErrorFlag_Reg);	// �жϽӅ������Ƿ��г�ͻλ
	temp = temp & 0x01;
	if (temp == 0x00)
	{
		for (i=0;i<5;i++)
			temp^=gBuff[i];
		if (temp)
			return(FM1702_SERNRERR);
		for (i=0;i<5;i++)
		{
			gCard_UID[i]=gBuff[i];
		}
		return(FM1702_OK);
	}
	else //�г�ͻλ
		return FM1702_SERNRERR;
}

//=======================================================
//	����: SelectCard
//	����: 	�ú���ʵ�ֶԷ���FM1702������Χ֮�ڵ�ĳ�ſ�Ƭ����ѡ��
//	����:	N/A
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_SELERR: ѡ������
//=======================================================
uchar SelectCard(void)
{
	
	uchar	temp, i;
	
	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg,0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg,0x0F);

	gBuff[0] = RF_CMD_SELECT;
	gBuff[1] = 0x70;
	for(i = 0; i < 5; i++)
	{
		gBuff[i + 2] = gCard_UID[i];
	}
	temp = Command_Send(Transceive, gBuff,7 );
	if(temp==0 )
	{
		return(FM1702_NOTAGERR);
	}
	else
	{
		temp = SPIReadOne(ErrorFlag_Reg);
		if((temp & 0x02) == 0x02) return(FM1702_PARITYERR);
		if((temp & 0x04) == 0x04) return(FM1702_FRAMINGERR);
		if((temp & 0x08) == 0x08) return(FM1702_CRCERR);
		temp = SPIReadOne(FIFOLength_Reg);
		if(temp != 1) return(FM1702_BYTECOUNTERR);
		SPIRead(FIFO_Reg,gBuff,temp);	//��FIFO�ж�ȡӦ����Ϣ
		if(gBuff[0]==0x08 || gBuff[0] ==0x28) 	  	// �ж�Ӧ���ź��Ƿ���ȷ 
			return(FM1702_OK);
		else
			return(FM1702_SELERR);
	}
}
//�洢��Կ
/* n:�����n�����루n��0��ʼ������ */
//*ramadr:��Կ
//gBuff[0]��Կ��Կ��ַ =0x80+n*12
//gBuff[1]:0
//gBuff[2]~gBuff[9]���ܷ�����//////////////
uchar Load_Key(uchar n,uchar *ramadr)
{
	uchar acktemp,temp[1],i;
	uint	unData;
	if (n>=32)
		return 1;
	unData=0x80+n*12;
	gBuff[0]=(uchar)(unData&0x00ff);
	gBuff[1]=(uchar)(unData>>8);
	for (i=0;i<6;i++)
	{
		temp[0]=ramadr[i];
		gBuff[2+i+i]=(((ramadr[i]&0xf0)>>4)|((~ramadr[i])&0xf0));
		gBuff[3+i+i]=((temp[0]&0xf)|(~(temp[0]&0xf)<<4));
	}
	acktemp=Command_Send(WriteEE,gBuff,0x0e);
	Delay_1ms(4);
	acktemp=SPIRead(SecondaryStatus_Reg,temp,1);
	if (acktemp) return(1);
	if (temp[0]&0x40)
	{
		temp[0]=0x0;
		acktemp=SPIWrite(Command_Reg,temp,0x1);
		if (acktemp) return(1);
		return(0);
	}
	temp[0]=0x0;
	acktemp=SPIWrite(Command_Reg,temp,0x1);
	return(1);
}

//=======================================================
//	����: Load_keyE2
//	����: �ú���ʵ�ְ�E2���������FM1702��keyRevBuffer��
//	����:	index: ��Կ����(ͨ����Ӧ������0~15��A���룬16~31��B����)
//	���: 	TRUE, ��Կװ�سɹ� 
//			FALSE, ��Կװ��ʧ��
//=======================================================
//������Կ
//���:gBuff[0]��Կ��Կ��ַ
//gBuff[1]:0
uchar Load_Key_EE(uchar index)
{
    uchar ucData[2],ucResult;
    uint unData;
    if(index>=32)
        return FALSE;
    unData = 0x80+index*12;
    ucData[0] = (uchar)(unData&0x00FF);
    ucData[1] = (uchar)(unData>>8);
    ucResult = Command_Send(LoadKeyE2,ucData,2);
    if(ucResult == FALSE)
        return FALSE;
    Delay_1ms(1);
    ucResult= SPIReadOne(ErrorFlag_Reg);
    if(ucResult & 0x40)
        return FALSE;
	return TRUE;
}

//=======================================================
//	����: Authentication
//	����: 	�ú���ʵ��������֤�Ĺ���
//	����:	UID: ��Ƭ���кŵ�ַ
//			SecNR: ������
//			mode: ģʽ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_AUTHERR: Ȩ����֤�д�
//=======================================================
uchar Authentication(uchar idata *UID, uchar SecNR, uchar mode)
{
	
	uchar idata	i;
	uchar idata	temp, temp1;

	if(SecNR >= 16)
		SecNR = SecNR % 16;
	
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg,0x0F);
	gBuff[0] = mode;
	gBuff[1] = SecNR * 4 + 3;
	for(i = 0; i < 4; i++)
	{
		gBuff[2 + i] = UID[i];
	}

	temp = Command_Send(Authent1, gBuff,6 );
	if(temp==0)
	{
		return 0x99;
	}

	temp = SPIReadOne(ErrorFlag_Reg);   
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp = Command_Send(Authent2, gBuff,0 );	
	if(temp ==0)
	{
		return 0x88;
	}

	temp = SPIReadOne(ErrorFlag_Reg);
//	Show(temp,0);
	if((temp & 0x02) == 0x02) return FM1702_PARITYERR;
	if((temp & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((temp & 0x08) == 0x08) return FM1702_CRCERR;
	temp1 = SPIReadOne(Control_Reg);
	temp1 = temp1 & 0x08;	
	if(temp1 == 0x08)
	{
		return FM1702_OK;
	}

	return FM1702_AUTHERR;
}

//=======================================================
//	����: Read_Block
//	����: 	�ú���ʵ�ֶ�MIFARE�������ֵ
//	����:	buff: �������׵�ַ
//			index: ���ַ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================

uchar Read_Block(uchar idata *buff,uchar index)
{
    uchar ucCmdLine[2],ucResult;
    SPIWriteOne(ChannelRedundancy_Reg,0x0F);
    ucCmdLine[0] = RF_CMD_READ;
    ucCmdLine[1] = index;
    ucResult = Command_Send(Transceive,ucCmdLine,2);
    if(ucResult == FALSE)
        return FM1702_NOTAGERR;//�޿�
    ucResult = SPIReadOne(ErrorFlag_Reg);
	if((ucResult & 0x02) == 0x02) return FM1702_PARITYERR;
	if((ucResult & 0x04) == 0x04) return FM1702_FRAMINGERR;
	if((ucResult & 0x08) == 0x08) return FM1702_CRCERR;
    ucResult = Read_FIFO(buff);
    if(ucResult!=0x10)
        return FM1702_BYTECOUNTERR;
    else
        return FM1702_OK;
}

//=======================================================
//	����: Write_Block
//	����: 	�ú���ʵ��дMIFARE�������ֵ
//	����:	buff: �������׵�ַ
//			index: ���ַ
//	���: 	FM1702_NOTAGERR: �޿�
//			FM1702_NOTAUTHERR: δ��Ȩ����֤
//			FM1702_EMPTY: �����������
//			FM1702_WRITEERR: д�������ݳ���
//			FM1702_OK: Ӧ����ȷ
//			FM1702_PARITYERR: ��żУ���
//			FM1702_CRCERR: CRCУ���
//			FM1702_BYTECOUNTERR: �����ֽڴ���
//=======================================================
uchar Write_Block(uchar idata *buff,uchar index)
{
    uchar ucCmdLine[2],ucResult,ucData[16];
    SPIWriteOne(ChannelRedundancy_Reg,0x07);   /* Note: this line is for 1702, different from RC500*/
    ucCmdLine[0] = RF_CMD_WRITE;
    ucCmdLine[1] = index;
    ucResult = Command_Send(Transceive,ucCmdLine,2);
    if(ucResult == FALSE)
        return FM1702_NOTAGERR;
    ucResult = Read_FIFO(ucData);
    if(ucResult == 0)
        return FM1702_BYTECOUNTERR;
    switch(ucData[0])
    {
	    case 0x00:	return(FM1702_NOTAUTHERR);	
	    case 0x04:	return(FM1702_EMPTY);
	    case 0x0a:	break;                   //����
	    case 0x01:	return(FM1702_CRCERR);
	    case 0x05:	return(FM1702_PARITYERR);
	    default:	return(FM1702_WRITEERR);
    }
    ucResult = Command_Send(Transceive,buff,16);
    if(ucResult == TRUE)
        return FM1702_OK;
    else
    {
        ucResult = SPIReadOne(ErrorFlag_Reg);
	    if((ucResult & 0x02) == 0x02) return FM1702_PARITYERR;
	    else if((ucResult & 0x04) == 0x04) return FM1702_FRAMINGERR;
	    else if((ucResult & 0x08) == 0x08) return FM1702_CRCERR;
        else return FM1702_WRITEERR;
    }
}

//=======================================================
//	����: MIF_Halt
//	����: �ú���ʵ����ͣMIFARE��
//	����:	N/A 
//	���: 	FM1702_OK: Ӧ����ȷ 
//			FM1702_PARITYERR: ��żУ���
//			FM1702_FRAMINGERR:FM1702֡����
//			FM1702_CRCERR: CRCУ���
//			FM1702_NOTAGERR: �޿�
//=======================================================
uchar MIF_Halt(void)
{

	uchar	temp;

	//ѡ��TX1��TX2�ķ��������迹
	SPIWriteOne(CwConductance_Reg,0x3f);
	//ѡ������У�������ģʽ
	SPIWriteOne(ChannelRedundancy_Reg,0x03);
	*gBuff = RF_CMD_HALT;
	*(gBuff + 1) = 0x00;
	temp = Command_Send(Transmit, gBuff, 2);//����FIFO�����ַ
	if(temp == TRUE)
		return FM1702_OK;
	else
	{
		temp = SPIReadOne(ErrorFlag_Reg);
		if((temp & 0x02) == 0x02)
		{
			return(FM1702_PARITYERR);
		}

		if((temp & 0x04) == 0x04)
		{
			return(FM1702_FRAMINGERR);
		}
		return(FM1702_NOTAGERR);
	}
}
//*/