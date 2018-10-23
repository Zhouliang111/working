#ifndef __pdu_h
#define __pdu_h


#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "windows.h"
#include "time.h"

#define GSM_7BIT  0x00		//00h 7bit数据编码默认字符集
#define GSM_8BIT  0x04		//F6h 8bit数据编码 Class1
#define GSM_UCS2  0x08		//08h USC2（16bit）双字节字符集

//短信存储结构体
struct pdu
{

	char SCA[24] = { 0 };   //SMSC地址
	char TPA[24] = { 0 };   //TP-DA地址字符串,TPDU段基本参数、目标手机号码

	char In_SCA[24] = { 0 };   //SMSC地址
	char In_TPA[24] = { 0 };   //TP-DA地址字符串,TPDU段基本参数、目标手机号码

	unsigned char TP_PID = 0X00;   //协议标识 常为00 点对点
	unsigned char TP_DCS = 0X00;   //编码方式

	unsigned char In_TP_PID = 0X00;   //协议标识 常为00 点对点
	unsigned char In_TP_DCS = 0X00;   //编码方式

	char TP_SCTS[16] = { 0 };       //时间戳
	unsigned char TP_UDL = 140;            //用户信息长度

	char TP_UD[1024] = { 0 };       //用户信息
	char In_TP_UD[1024] = {};       //用户信息

	unsigned char UDH_head[14] = { 0 }; ;  //打印用户信息头
	unsigned char UDH_head1[16] = { 0 };

	unsigned char UDH_headlength = 0;  //用户数据头长度

	char data[1024] = { 0 };
};


#endif
