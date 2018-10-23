#include "pdu_encode.h"
#include "PDU.h"



extern int Cnt ;              //短信分片的数量
extern int RN ;        //用于产生的编码标识
extern int ok ;        //用于编号计数
extern int left ;      //处理余数
extern struct pdu pdu_message;

//************************************编码函数****************************************


// 7bit编码
// 输入: pSrc - 源字符串指针
//       nSrcLength - 源字符串长度
// 输出: pDst - 目标编码串指针
// 返回: 目标编码串长度
int PDUencode_7bit(const char* Src, unsigned char* Dst, int n_Srclength)
{
	int n_src;		// 源字符串的计数值
	int n_dst;		// 目标编码串的计数值
	int n_char;		// 当前正在处理的组内字符字节的序号，范围是0-7
	unsigned char n_left;	// 上一字节残余的数据

							// 计数值初始化
	n_src = 0;
	n_dst = 0;


	// 将源串每8个字节分为一组，压缩成7个字节
	// 循环该处理过程，直至源串被处理完
	// 如果分组不到8字节，也能正确处理
	//+1 是为了让最后一个字节的高4位可以补上0000
	while (n_src < (n_Srclength + 1))
	{
		// 取源字符串的计数值的最低3位
		n_char = n_src & 7;

		// 处理源串的每个字节
		if (n_char == 0)
		{
			// 组内第一个字节，只是保存起来，待处理下一个字节时使用
			n_left = *Src;
		}
		else
		{
			// 组内其它字节，将其右边部分与残余数据相加，得到一个目标编码字节
			*Dst = (*Src << (8 - n_char)) | n_left;

			// 将该字节剩下的左边部分，作为残余数据保存起来
			n_left = *Src >> n_char;

			// 修改目标串的指针和计数值
			Dst++;
			n_dst++;
		}

		// 修改源串的指针和计数值
		Src++;
		n_src++;
	}

	// 返回目标串长度
	return n_dst;
}


// 8bit编码
int PDUencode_8bit(const char* Src, unsigned char* Dst, int n_srclength)
{
	// 简单复制
	memcpy(Dst, Src, n_srclength);

	return n_srclength;
}


// UCS2编码
int PDUencode_Unicode(const char* Src, unsigned char* Dst, int n_srclength)
{
	int n_dstlength;		// UNICODE宽字符数目
	wchar_t wchar[128];	// UNICODE串缓冲区

						// GB字符串-->UNICODE串
						//nDstLength = strGB2Unicode(pSrc, wchar, nSrcLength);
	n_dstlength = MultiByteToWideChar(CP_ACP, 0, Src, n_srclength, wchar, 128);

	// 高低字节对调，输出
	for (int i = 0; i<n_dstlength; i++)
	{
		*Dst++ = wchar[i] >> 8;		// 先输出高位字节
		*Dst++ = wchar[i] & 0xff;		// 后输出低位字节
	}

	// 返回目标编码串长度
	return n_dstlength * 2;
}


// 正常顺序的字符串转换为两两颠倒的字符串，若长度为奇数，补'F'凑成偶数
// 如："8613851872468" --> "683158812764F8"
int PDU_InvertNumbers(const char* Src, char* Dst, int n_srclength)
{
	int n_dstlength;		// 目标字符串长度
	char ch;			// 用于保存一个字符

						// 复制串长度
	n_dstlength = n_srclength;

	// 两两颠倒
	for (int i = 0; i<n_srclength; i += 2)
	{
		ch = *Src++;		// 保存先出现的字符
		*Dst++ = *Src++;	// 复制后出现的字符
		*Dst++ = ch;		// 复制先出现的字符
	}

	// 源串长度是奇数吗？   一般为13位电话号码
	if (n_srclength & 1)
	{
		*(Dst - 2) = 'F';	// 补'F'
		n_dstlength++;		// 目标串长度加1  加到14位
	}

	// 输出字符串加个结束符
	*Dst = '\0';

	// 返回目标字符串长度
	return n_dstlength;
}


// 字节数据转换为可打印字符串
// 如：{0xC8} --> "C8" 
int PDU_BytestoString(const unsigned char* Src, char* Dst, int n_srclength)
{
	const char tab[] = "0123456789ABCDEF";	// 0x0-0xf的字符查找表

	for (int i = 0; i < n_srclength; i++)
	{
		*Dst++ = tab[*Src >> 4];			// 输出高4位 低四位移出去 从左边补0000 
		*Dst++ = tab[*Src & 0x0f];		// 输出低4位  0x0f 为 00001111 取出低四位
		Src++;
	}

	// 输出字符串加个结束符
	*Dst = '\0';

	// 返回目标字符串长度
	return (n_srclength * 2);
}


// PDU编码，用于编制、发送短消息
//测试成功
//中英文输入ok
//7bit成功
int PDU_Encode(char* Dst)
{

	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[256];									// 内部用的缓冲区

															/*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//设置短信中心号码
		// SMSC地址信息长度, lenghth为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x11;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[3] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

																						/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// 用户信息字符串的长度
	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)

	buf[2] = 0xC4;							// 有效期(TP-VP)为30天：A8-C4（VP-166）x 1天
	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit编码方式
		//buf[3] = n_length;					// 编码前长度
		//n_length = PDUencode_7bit(pdu_message.In_TP_UD, &buf[4], n_length) + 4;		// 转换TP-DA到目标PDU串
		buf[3] = PDUencode_7bit(pdu_message.In_TP_UD, &buf[4], n_length);
		n_length = buf[3] + 4;
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2编码方式
		buf[3] = PDUencode_Unicode(pdu_message.In_TP_UD, &buf[4], n_length);			// 转换TP-DA到目标PDU串
		n_length = buf[3] + 4;											// nLength等于该段数据长度
	}
	else
	{
		// 8-bit编码方式
		buf[3] = PDUencode_8bit(pdu_message.In_TP_UD, &buf[4], n_length);			// 转换TP-DA到目标PDU串
		n_length = buf[3] + 4;											// nLength等于该段数据长度
	}
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
																		// 返回目标字符串长度
	return n_dstlength;
}

//正常短信加入时间戳  
//测试成功
//中英文输入ok
//7bit成功
int PDU_Encode_time(char* Dst)
{
	//添加时间戳的变量
	char now_time[24];
	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	

	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[256];									// 内部用的缓冲区

	memset(now_time, 0, sizeof(now_time));
	PDU_time = localtime(&t);
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //将时间戳转为字符串 18082708525108
															/*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//设置短信中心号码
		// SMSC地址信息长度, lenghth为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x84;					// 是发送短信(TP-MTI=01)，TP-VP用相对格式(TP-VPF=10)
	buf[1] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[2] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

						/*********** TPDU段协议标识、编码方式、用户信息等 ***********/

	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)

	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);          //转换时间戳到PDU串

	n_length = (int)strlen(pdu_message.In_TP_UD);			// 用户信息字符串的长度
	
	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit编码方式
		buf[0] = PDUencode_7bit(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;

	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2编码方式
		buf[0] = PDUencode_Unicode(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;

	}
	else
	{
		// 8-bit编码方式
		buf[0] = PDUencode_8bit(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;
	}
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
																			// 返回目标字符串长度
	return n_dstlength;
}



//编码添加用户头
//中英文输入解决
//7bit解决
int PDU_Encode_longmessage_NO1(char* Dst)      //不超过字符添加用户信息头的长短信编码 只有一条
{

	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[1024];									// 内部用的缓冲区

	char Temp[1024];

																/*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//手动设置短信中心号码
		// SMSC地址信息长度, lenghth = Type + Address. Address为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x51;					// 支持用户信息头的标识
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[3] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

																						/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// 用户信息字符串的长度


	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)
													//buf[2] = 0;							// 有效期(TP-VP)为5分钟:00-8F（VP+1）x 5分钟  从5分钟间隔到12小时
	buf[2] = 0xC4;							// 有效期(TP-VP)为30天：A8-C4（VP-166）x 1天



	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit编码方式
		buf[3] = n_length + 7;					// 编码前长度

		/*buf[3] =   PDUencode_7bit(pdu_message.In_TP_UD, &buf[11], n_length) + 7 ;*/
		buf[4] = 0x05;                     //添加超长短信参数
		buf[5] = 0x00;                     //长短信拆分标识
		buf[6] = 0x03;                     //后面有3个数据
		buf[7] = RN;                       //判断标识
		buf[8] = 0x01;                      //子短信分片总数
		buf[9] = 0x01;                       //子短信分片序号
																					 //处理用户头
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// 转换4个字节到目标PDU串

		Temp[0] = 0x05;
		Temp[1] = 0x00;
		Temp[2] = 0x03;
		Temp[3] = RN;
		Temp[4] = 0x01;
		Temp[5] = 0x01;
		Temp[6] = 0;
		memcpy(&Temp[7], pdu_message.In_TP_UD, n_length);                     //SUCCESS

		//打印调试
		/*for (int i=0;i<n_length + 7;i++)
		{
			printf("%c", Temp[i]);
		}*/
		n_length = PDUencode_7bit(Temp, buf, n_length + 8) ;		// 经过7bit转换后长度少一个
		n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)   //解决中英文输入
	{
		// UCS2编码方式

		buf[3] =  PDUencode_Unicode(pdu_message.In_TP_UD, &buf[10], n_length) + 6;
		buf[4] = 0x05;                     //添加超长短信参数
		buf[5] = 0x00;                     //长短信拆分标识
		buf[6] = 0x03;                     //后面有3个数据
		buf[7] = RN;                       //判断标识
		buf[8] = 0x01;                      //子短信分片总数
		buf[9] = 0x01;                       //子短信分片序号

		n_length = buf[3] + 10 - 6;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
	}
	else
	{
		// 8-bit编码方式

		buf[3] = n_length + 6;					// 编码前长度
		//buf[3] =   PDUencode_8bit(pdu_message.In_TP_UD, &buf[11], n_length) + 6;
		buf[4] = 0x05;                     //添加超长短信参数
		buf[5] = 0x00;                     //长短信拆分标识
		buf[6] = 0x03;                     //后面有3个数据
		buf[7] = RN;                       //判断标识
		buf[8] = 0x01;                      //子短信分片总数
		buf[9] = 0x01;                       //子短信分片序号

		n_length = PDUencode_8bit(pdu_message.In_TP_UD, &buf[10], n_length) + 10;
		//n_length = buf[3] +10 -6

		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
	}
	

																			// 返回目标字符串长度
	return n_dstlength;
}

//编码添加用户头的时间戳
//中英文输入正确
//7bit成功
int PDU_Encode_longmessage_NO1time(char* Dst)      //不超过字符添加用户信息头的长短信编码 只有一条
{
	char now_time[24];
	char Temp[1024];

	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[1024];									// 内部用的缓冲区

	memset(now_time, 0, sizeof(now_time));

	//数据处理
	PDU_time = localtime(&t);

	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //将时间戳转为字符串 18082708525108	

																/*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//手动设置短信中心号码
		// SMSC地址信息长度, lenghth = Type + Address. Address为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x64;					// 支持用户信息头的标识
	buf[1] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[2] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength],3);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// 转换2个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);
																						
														/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// 用户信息字符串的长度


	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit编码方式
		buf[0] = n_length + 7;					// 编码前长度
		buf[1] = 0x05;                     //添加超长短信参数
		buf[2] = 0x00;                     //长短信拆分标识
		buf[3] = 0x03;                     //后面有3个数据
		buf[4] = RN;                       //判断标识
		buf[5] = 0x01;                      //子短信分片总数
		buf[6] = 0x01;                       //子短信分片序号

		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

		Temp[0] = 0x05;
		Temp[1] = 0x00;
		Temp[2] = 0x03;
		Temp[3] = RN;
		Temp[4] = 0x01;
		Temp[5] = 0x01;
		Temp[6] = 0;
		memcpy(&Temp[7], pdu_message.In_TP_UD, n_length);                     //SUCCESS

		n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// 经过7bit转换后长度少一个
		n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2编码方式
		buf[0] =   PDUencode_Unicode(pdu_message.In_TP_UD, &buf[7], n_length) + 6;
		buf[1] = 0x05;                     //添加超长短信参数
		buf[2] = 0x00;                     //长短信拆分标识
		buf[3] = 0x03;                     //后面有3个数据
		buf[4] = RN;                       //判断标识
		buf[5] = 0x01;                      //子短信分片总数
		buf[6] = 0x01;                       //子短信分片序号
		n_length = buf[0] + 7 - 6;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
	}
	else
	{
		// 8-bit编码方式

		buf[0] = n_length + 6;					// 编码前长度

		//buf[0] =   PDUencode_8bit(pdu_message.In_TP_UD, &buf[11], n_length) + 6;
		buf[1] = 0x05;                     //添加超长短信参数
		buf[2] = 0x00;                     //长短信拆分标识
		buf[3] = 0x03;                     //后面有3个数据
		buf[4] = RN;                       //判断标识
		buf[5] = 0x01;                      //子短信分片总数
		buf[6] = 0x01;                       //子短信分片序号

		n_length = PDUencode_8bit(pdu_message.In_TP_UD, &buf[7], n_length) + 7;
		//n_length = buf[0] +7 -6
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
	}
	
				
	return n_dstlength;														// 返回目标字符串长度
}



//长短信拆分编码
//中文拆分修正
//7bit拆分ok
int PDU_Encode_longmessage_NO2(char* Dst)              //超过字符的长短信拆分编码 不带时间戳
{

	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[1024];									// 内部用的缓冲区

	char Temp[1024];
																/*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//手动设置短信中心号码
		// SMSC地址信息长度, lenghth = Type + Address. Address为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x51;					// 支持用户信息头的标识
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[3] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

													/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
													//n_length = strlen(pdu_message.In_TP_UD);			// 用户信息字符串的长度    拆分时候长度需要变化了

	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)
													//buf[2] = 0;							// 有效期(TP-VP)为5分钟:00-8F（VP+1）x 5分钟  从5分钟间隔到12小时
	buf[2] = 0xC4;							// 有效期(TP-VP)为30天：A8-C4（VP-166）x 1天

	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit编码方式
		if (ok == Cnt - 1)                       //未处理到最后一条子短信
		{
			n_length = 160 - 7;                                    //7bit拆分
			buf[3] = left + 7 ;					// 编码前长度
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// 转换10个字节到目标PDU串

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], left);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// 经过7bit转换后长度少一个
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串

		}

		else
		{
			n_length = 160 - 7;                                    //7bit拆分
			buf[3] = 160;					// 编码前长度
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// 转换10个字节到目标PDU串

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], n_length);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// 经过7bit转换后长度少一个
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;*/

		}
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2编码方式
		if (ok == Cnt - 1)                       //未处理到最后一条子短信
		{
			n_length = 140 - 6;                  
			//buf[3] = left + 6;					// 编码前长度 6个数据长度
			buf[3] = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 6;          //更改
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号

			n_length = buf[3] + 10 - 6;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串

			/*n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 10;*/
		}

		else
		{
			n_length = 140 - 6;                                    //7bit拆分
			buf[3] = 140;					// 编码前长度
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号
			n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}
	}
	else
	{
		// 8-bit编码方式
		if (ok == Cnt - 1)                       //未处理到最后一条子短信
		{
			n_length = 140 - 6;                                    //7bit拆分
			buf[3] = left + 6;					// 编码前长度  6位数据长度
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}
		else
		{
			n_length = 140 - 6;                                    //7bit拆分

			buf[3] = 140;					// 编码前长度
			buf[4] = 0x05;                     //添加超长短信参数
			buf[5] = 0x00;                     //长短信拆分标识
			buf[6] = 0x03;                     //后面有3个数据
			buf[7] = RN;                       //判断标识
			buf[8] = Cnt;                      //子短信分片总数
			buf[9] = ok + 1;                       //子短信分片序号

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}
	}
																			// 返回目标字符串长度
	return n_dstlength;
}

//长短信拆分编码加时间戳
//7bit修改ok
//中英文拆分修正
int PDU_Encode_longmessage_NO2time(char* Dst)              //超过字符的长短信拆分编码
{
	//添加时间戳的变量
	char now_time[24];
	time_t t;
	t = time(NULL);

	char Temp[1024];

	struct tm *PDU_time = NULL;
	//
	int n_length;											// 内部用的串长度
	int n_dstlength;											// 目标PDU串长度
	unsigned char buf[1024];									// 内部用的缓冲区
	
	memset(now_time, 0, sizeof(now_time));
	
																//数据处理
	PDU_time = localtime(&t);
	
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //将时间戳转为字符串 18082708525108													   
																   /*********** SMSC地址信息段 ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//短信服务中心号码长度

	if (n_length > 0)
	{
		//手动设置短信中心号码
		// SMSC地址信息长度, lenghth = Type + Address. Address为奇数时, 要补上'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// 固定: 用国际格式号码
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// 转换2个字节到目标PDU串 

																	// 转换短信服务中心号码到目标PDU串
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//终端将自动从SIM卡中读取短信中心号码并填充SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// 转换1个字节到目标PDU串
	}

	/*********** TPDU段基本参数、目标地址等 ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA地址字符串的长度
	buf[0] = 0x64;					// 支持用户信息头的标识
	buf[1] = (char)n_length;			// 目标地址数字个数(目标手机号码字符串真实长度)
	buf[2] = 0x91;					// 固定: 用国际格式号码
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);				// 转换4个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// 转换TP-DA到目标PDU串

												/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
												//n_length = strlen(pdu_message.In_TP_UD);			


	buf[0] = pdu_message.In_TP_PID;					// 协议标识(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// 用户信息编码方式(TP-DCS)

	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// 转换2个字节到目标PDU串
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);

	//printf("时间戳交换顺序为%s\n", now_time);

	if (pdu_message.In_TP_DCS == GSM_7BIT)    //拆分有问题
	{
		// 7-bit编码方式
		if (ok == Cnt - 1)                       //未处理到最后一条子短信
		{
			n_length = 160 - 7;                                    //7bit最大数160
			buf[0] = left + 7;					// 编码前长度
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length * ok], left);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, left + 8);		// 经过7bit转换后长度少一个
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 7;*/
		}
		else
		{
			n_length = 160 - 7;                                    //7bit最大数160
			buf[0] = 160;					// 编码前长度
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], n_length);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// 经过7bit转换后长度少一个
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// 转换该段数据到目标PDU串

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;*/
		}

	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		if (ok == Cnt-1 )                       //未处理到最后一条子短信
		{
			// UCS2编码方式
			n_length = 140 - 6;                                    //USC2最大数140
			buf[0] = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 6;          //更改
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号
			n_length = buf[0] + 7 - 6;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}

		else 
		{
			n_length = 140 - 6;                                    //USC2最大数140
			buf[0] = 140;					// 编码前长度
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号

			n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串

		}
	}
	else
	{
		// 8-bit编码方式
		if (ok == Cnt - 1)                       //未处理到最后一条子短信
		{
			n_length = 140 - 6;                                    //USC2最大数140
			buf[0] = left + 6;					// 编码前长度
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}
		else
		{
			// 8-bit编码方式
			n_length = 140 - 6;                                    //USC2最大数140
			buf[0] = 140;					// 编码前长度
			buf[1] = 0x05;                     //添加超长短信参数
			buf[2] = 0x00;                     //长短信拆分标识
			buf[3] = 0x03;                     //后面有3个数据
			buf[4] = RN;                       //判断标识
			buf[5] = Cnt;                      //子短信分片总数
			buf[6] = ok + 1;                       //子短信分片序号

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// 转换该段数据到目标PDU串
		}

	}
																		// 返回目标字符串长度
	return n_dstlength;
}