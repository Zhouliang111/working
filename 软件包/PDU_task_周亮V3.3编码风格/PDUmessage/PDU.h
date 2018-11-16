#ifndef __pdu_h
#define __pdu_h


#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "time.h"

#define GSM_7BIT  0x00		//00h 7bit���ݱ���Ĭ���ַ���
#define GSM_8BIT  0x04		//F6h 8bit���ݱ��� Class1
#define GSM_UCS2  0x08		//08h USC2��16bit��˫�ֽ��ַ���

//���Ŵ洢�ṹ��
struct pdu
{

	char SCA[24];   //SMSC��ַ
	char TPA[24];   //TP-DA��ַ�ַ���,TPDU�λ���������Ŀ���ֻ�����

	char In_SCA[24];   //SMSC��ַ
	char In_TPA[24];   //TP-DA��ַ�ַ���,TPDU�λ���������Ŀ���ֻ�����

	unsigned char TP_PID;   //Э���ʶ ��Ϊ00 ��Ե�
	unsigned char TP_DCS;   //���뷽ʽ

	unsigned char In_TP_PID;   //Э���ʶ ��Ϊ00 ��Ե�
	unsigned char In_TP_DCS;   //���뷽ʽ

	char TP_SCTS[16];       //ʱ���
	unsigned char TP_UDL;            //�û���Ϣ����

	char TP_UD[1024];       //�û���Ϣ
	char In_TP_UD[1024];       //�û���Ϣ

	unsigned char UDH_head[14] ;  //��ӡ�û���Ϣͷ
	unsigned char UDH_head1[16];

	unsigned char UDH_headlength;  //�û�����ͷ����
	char data[1024];
};

extern struct pdu pdu_message;     //��������
extern struct pdu pdu_long[100];   //��������


#endif
