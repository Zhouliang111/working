#include "pdu_decode.h"
#include "PDU.h"

/*功能测试备注
7bit带时间戳解码ok
7bit带用户信息头有空格  解决
中文带时间戳解码ok
中文带用户头解码乱码     待解决
*/


extern struct pdu pdu_message;
extern int head_flag  ;    //定义符合标识的标志位
extern int head_flag1 ;   //定义两位标识的标志位
extern int nUDI_flag  ;    //定义无用户信息头标志位
extern int solo_flag  ;           //定义子短信的结束标志
extern unsigned char UDH_cnt;   //长短信总条数用于打印
extern int num;       //计数


//7bit 解码函数  编码串->字符串
//Src 源编码串地址
//Dst 目标字符串地址
//Src_length 源编码串长度
//返回：目标字符串长度

int PDUdcode_7bit(const unsigned char *Src, char *Dst, int n_Srclength)
{
	int n_src;   // 源字符串的计数值
	int n_dst;  // 目标解码串的计数值 
	int n_byte; // 当前正在处理的组内字节的序号，范围是0-6 

	unsigned char n_left; // 上一字节残余的数据 

						  // 计数值初始化  
	n_src = 0;
	n_dst = 0;
	// 组内字节序号和残余数据初始化
	n_byte = 0;
	n_left = 0;


	// 将源数据每7个字节分为一组，解压缩成8个字节  
	// 循环该处理过程，直至源数据被处理完  
	// 如果分组不到7字节，也能正确处理

	while (n_src <n_Srclength)
	{
		*Dst = ((*Src << n_byte) | n_left) & 0x7f;
		n_left = *Src >> (7 - n_byte);

		Dst++;
		n_dst++;

		n_byte++;

		if (n_byte == 7)
		{
			*Dst = n_left;

			Dst++;
			n_dst++;

			n_byte = 0;
			n_left = 0;
		}

		Src++;
		n_src++;
	}
	*Dst = '\0';
	return n_dst;

}


// 可打印字符串转换为字节数据
// 如："C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// 输入: pSrc - 源字符串指针
//       nSrcLength - 源字符串长度
// 输出: pDst - 目标数据指针
// 返回: 目标数据长度
int PDU_StringtoBytes(const char* Src, unsigned char* Dst, int n_Srclength)
{
	//printf("gsmString2Bytes start ... nSrcLength = %d\n", n_Srclength);
	for (int i = 0; i < n_Srclength; i += 2)
	{
		// 输出高4位
		if ((*Src >= '0') && (*Src <= '9'))
		{
			*Dst = (*Src - '0') << 4;   //例如 '3'换成 0x03
		}
		else
		{
			*Dst = (*Src - 'A' + 10) << 4; // 例如‘b’换成 0x0b  这里只符合转换的字符为大写的情况
		}

		//printf("Src[%d] = %c\n", i, *Src);   //打印源字符串
		//printf("==Dst[%d] = 0x%02x\n", i, *Dst);  //打印转换后的16进制字符

		Src++;

		// 输出低4位
		if ((*Src >= '0') && (*Src <= '9')) //同理转换
		{
			*Dst |= *Src - '0';
		}
		else
		{
			*Dst |= *Src - 'A' + 10;   //同理转换
		}
		//printf("Src[%d] = %c\n", i, *Src);
		//printf("==Dst[%d] = 0x%02x\n", i/2, *Dst);

		Src++;
		Dst++;
	}

	//printf("gsmString2Bytes end --- n_Srclength = %d\n", n_Srclength);
	// 返回目标数据长度
	return (n_Srclength / 2);

}



// 两两颠倒的字符串转换为正常顺序的字符串
// 如："683158812764F8" --> "8613851872468"
// 输入: pSrc - 源字符串指针
//       nSrcLength - 源字符串长度
// 输出: pDst - 目标字符串指针
// 返回: 目标字符串长度
int PDU_SerializeNumbers(const char* Src, char* Dst, int n_Srclength)
{
	int n_dstlength;		// 目标字符串长度
	char ch;			// 用于保存一个字符

						// 复制串长度
	n_dstlength = n_Srclength;

	// 两两颠倒
	for (int i = 0; i<n_Srclength; i += 2)
	{
		ch = *Src++;		// 保存先出现的字符
		*Dst++ = *Src++;	// 复制后出现的字符
		*Dst++ = ch;		// 复制先出现的字符
	}

	// 字符是'F'吗？
	if (*(Dst - 1) == 'F')
	{
		Dst--;
		n_dstlength--;		// 目标字符串长度减1
	}

	// 输出字符串加个结束符
	*Dst = '\0';

	// 返回目标字符串长度
	return n_dstlength;
}



// 8bit解码
// 输入: pSrc - 源编码串指针
//       nSrcLength -  源编码串长度
// 输出: pDst -  目标字符串指针
// 返回: 目标字符串长度
int PDUdcode_8bit(const unsigned char* Src, char* Dst, int n_Srclength)
{
	// 简单复制
	memcpy(Dst, Src, n_Srclength);  //将源字符串数据复制到目标字符串

									// 输出字符串加个结束符
	*Dst = '\0';

	return n_Srclength;
}


// UCS2解码
// 输入: Src - 源编码串指针
//       n_Srclength -  源编码串长度
// 输出: Dst -  目标字符串指针
// 返回: 目标字符串长度
int PDUdcode_Unicode(const unsigned char* Src, char* Dst, int n_Srclength)
{
	int n_dstlength;		// UNICODE宽字符数目
							//WCHAR wchar[128];	   // UNICODE串缓冲区 需要定义宽字符
	wchar_t wchar[128];


	// 高低字节对调，拼成UNICODE
	for (int i = 0; i<n_Srclength / 2; i++)      //加入24个字符，高低字节对调 一个字节8位为一组为两个16进制字符，所以24/2
	{
		wchar[i] = *Src++ << 8;	// 先高位字节
		wchar[i] |= *Src++;		// 后低位字节
	}

	// UNICODE串-->字符串

	n_dstlength = WideCharToMultiByte(CP_ACP, 0, wchar, n_Srclength / 2, Dst, 160, NULL, NULL);
	//调用函数不同
	//n_dstlength = wcstombs_s(&len,Dst,n_Srclength / 2,wchar,128);
	//n_dstlength = wcstombs(Dst, wchar,len);


	// 输出字符串加个结束符
	Dst[n_dstlength] = '\0';

	// 返回目标字符串长度
	return n_dstlength;
}


// PDU解码
// 输入: pSrc - 源PDU串指针
// 输出: pDst - 目标PDU参数指针
// 返回: 用户信息串长度
int PDU_Decode(const char* Src)
{
	unsigned char UDH_number; //判断子短信的短信序号

	int n_dstlength = 0;			// 目标PDU串长度
	unsigned char tmp;		// 内部用的临时字节变量
	unsigned char buf[256];	// 内部用的缓冲区

	int UDHI;  //判断UDHI位 0 1

	unsigned char UDU_ct;//判断是否位长短信拆分标识 00

	unsigned char UDH_ID;    //判断子短信是否为同一标识
	unsigned char UDH_ID2;    //判断子短信是否为同一标识

	static unsigned char sum[10];
	static unsigned char sum1[10];


	/*********** SMSC地址信息段 ***********/
	PDU_StringtoBytes(Src, &tmp, 2);						// 取长度
	tmp = (tmp - 1) * 2;								// SMSC号码串长度
	Src += 4;											// 指针后移，忽略了SMSC地址格式
	PDU_SerializeNumbers(Src, pdu_message.SCA, tmp);				// 转换SMSC号码到目标PDU串
																	//printf("SCA: %s\n", pdu_message.SCA);
	Src += tmp;										// 指针后移

															/*********** PDU -TYPE ***********/
	PDU_StringtoBytes(Src, &tmp, 2);						// 取基本参数 需要判断是否有用户头的标志
															//UDHI =0  不包含头信息 UDHI =1 包含头信息 
															//UDH位于 第6位 需要判断UDH 是0 还是 1

	UDHI = (tmp & 0x40) >> 6;								//将UDH的值取出  !!成功读取UDHI的值
	nUDI_flag = 0;
	if (UDHI == 0)  //无用户数据头
	{
		nUDI_flag = 1;
		printf("UDHI=0 UD无数据头\n");    //无数据头按照正常的方式解码

		Src += 2;											// 指针后移
															/*********** 取回复号码 ***********/
		PDU_StringtoBytes(Src, &tmp, 2);						// 取长度
		if (tmp & 1) tmp += 1;								// 调整奇偶性
		Src += 4;											// 指针后移，忽略了回复地址(TP-RA)格式
		PDU_SerializeNumbers(Src, pdu_message.TPA, tmp);				// 取TP-RA号码
																		//printf("TPA: %s\n", pdu_message.TPA);
		Src += tmp;										// 指针后移

														/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_PID, 2);			// 取协议标识(TP-PID)
		Src += 2;													// 指针后移

		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_DCS, 2);			// 取编码方式(TP-DCS)
		Src += 2;													// 指针后移

		PDU_SerializeNumbers(Src, pdu_message.TP_SCTS, 14);						// 服务时间戳字符串(TP_SCTS) 

		Src += 14;													// 指针后移

		PDU_StringtoBytes(Src, &tmp, 2);								// 用户信息长度(TP-UDL)  
		pdu_message.TP_UDL = tmp;                                      //将UDL长度赋值

		Src += 2;													    // 指针后移

		if (pdu_message.TP_DCS == GSM_7BIT) //7 bit解码
		{
			// 7-bit解码
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换


			PDUdcode_7bit(buf, pdu_message.TP_UD, n_dstlength);						// 转换到TP-DU
																					//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
			n_dstlength = tmp;
			printf("用户信息长度%d\n", n_dstlength);
		}

		else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
		{
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
			PDUdcode_Unicode(buf, pdu_message.TP_UD, n_dstlength);     //短信内容提取
		}
		else               //8 bit 解码
		{
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
			n_dstlength = PDUdcode_8bit(buf, pdu_message.TP_UD, n_dstlength);   //短信内容提取
		}
	}

	else        //有数据头的处理  这里需要添加用户头的标识判断
	{

		printf("UDHI=1 UD有数据头\n");    //有数据头需要对UDL进行处理 UDL= UDH+UD 

		Src += 2;											// 指针后移
															/*********** 取回复号码 ***********/
		PDU_StringtoBytes(Src, &tmp, 2);						// 取长度
		if (tmp & 1) tmp += 1;								// 调整奇偶性
		Src += 4;											// 指针后移，忽略了回复地址(TP-RA)格式
		PDU_SerializeNumbers(Src, pdu_message.TPA, tmp);				// 取TP-RA号码
																		//printf("TPA: %s\n", pdu_message.TPA);
		Src += tmp;											// 指针后移

														/*********** TPDU段协议标识、编码方式、用户信息等 ***********/
		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_PID, 2);			// 取协议标识(TP-PID)
		Src += 2;													// 指针后移

		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_DCS, 2);			// 取编码方式(TP-DCS)
		Src += 2;													// 指针后移

		PDU_SerializeNumbers(Src, pdu_message.TP_SCTS, 14);						// 服务时间戳字符串(TP_SCTS) 

		Src += 14;													// 指针后移

		PDU_StringtoBytes(Src, &tmp, 2);								// 此时用户信息长度TP-UDL=UDH+UD  //取长度
		//////////////
		pdu_message.TP_UDL = tmp;                                      //长度分为长短信拆分和正常情况
		///////////////////
		Src += 2;													    // 指针后移

		PDU_StringtoBytes(Src, pdu_message.UDH_head, 12);           //获取了打印的用户头数据 050003BF0201 显示函数打印

		PDU_StringtoBytes(Src, pdu_message.UDH_head1, 14);            //获取了用户信息头，两位标识   06080419420302


		PDU_StringtoBytes(Src, &pdu_message.UDH_headlength, 2);                  //获取了用户头长度  05 06 这里要加入判断
		Src += 2;													 // 05是6个数据头 06是7个协议头

		PDU_StringtoBytes(Src, &UDU_ct, 2);                      //获取长短信拆分标识00

		if (UDU_ct == 0x00)          //00 长短信拆分   
		{
			printf("8bit长短信拆分标识\n");
		}
		else if (UDU_ct == 0x08)
		{
			printf("16bit长短信拆分标识\n");
		}
/////////////////////////////////////////////////////////////////////////////////////

		//处理6个字节的数据头
		if (pdu_message.UDH_headlength == 0x05)    //只需判断一个标识 取出前6个字节
		{
			Src += 4;            //6个信息头移动4个 
			PDU_StringtoBytes(Src, &UDH_ID, 2);                    //获取长短信拆分子短信标识 BF
			Src += 2;

			PDU_StringtoBytes(Src, &UDH_cnt, 2);                //获取一共有多少条短信
			printf("这条长短信一共%d条\n", UDH_cnt);

			if (num < UDH_cnt)                                       //处理用户头标识问题
			{
				sum[num] = UDH_ID;                              //这里后续支持长短信合并的问题

				if (num > 0)
				{
					head_flag = 0;   //标识位清零
					if (sum[num] == sum[num - 1])   //判断标识是否相等
					{
						head_flag = 1;
					}
				}
			}
			else
			{
				num = 0;
			}
			solo_flag = 0;
			if (UDH_cnt == 0x01)
			{
				solo_flag = 1;
			}
			else
			{
				num++;
			}

			Src += 2;
			PDU_StringtoBytes(Src, &UDH_number, 2);          //获取了子短信单独序号 01

			printf("此短信的序号为%d\n", UDH_number);

			if (UDH_number == 0x01)
			{
				if (pdu_message.TP_DCS == GSM_7BIT && solo_flag == 1)
				{
					tmp = pdu_message.TP_UDL;						
					Src -= 10;                                      //回到用户数据信息的位置								
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// 转换到TP-DU
				}
				else if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					tmp = pdu_message.TP_UDL;                     //十进制 160
					Src -= 10;                                      //回到用户数据信息的位置
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// 转换到TP-DU
				}
				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					tmp = pdu_message.TP_UDL - 6;     //十进制 140
					Src += 2;
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[0], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					tmp = pdu_message.TP_UDL - 6;     //十进制 140
					Src += 2;
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[0], n_dstlength);   //短信内容提取
				}
			}
			else if (UDH_number == 0x02)
			{
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit解码
					tmp = pdu_message.TP_UDL;                     //十进制 160
					Src -= 10;                                      //回到用户数据信息的位置
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[160], n_dstlength);						// 转换到TP-DU
																									//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("用户信息长度%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[160], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[160], n_dstlength);   //短信内容提取

				}
			}
			else if (UDH_number == 0x03)
			{
				Src += 12;

				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit解码
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[320], n_dstlength);						// 转换到TP-DU
																									//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("用户信息长度%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[320], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[320], n_dstlength);   //短信内容提取
				}

			}

		}

//第二种情况
		//处理7个字节的数据头
		else                        //06   需要判断两个用户信息标识取出前7个字节
		{
			Src += 4;            //  7个信息头有两个标识

			PDU_StringtoBytes(Src, &UDH_ID, 2);                    //获取长短信第1个短信标识
			Src += 2;
			PDU_StringtoBytes(Src, &UDH_ID2, 2);                     //获取长短信第2个短信标识
			Src += 2;

			PDU_StringtoBytes(Src, &UDH_cnt, 2);               //短信总数一共多条
			printf("这条长短信一共%d条\n", UDH_cnt);

			if (num < UDH_cnt)                                       //处理用户头标识问题
			{
				sum[num] = UDH_ID;                              //这里后续支持长短信合并的问题
				sum1[num] = UDH_ID2;
				if (num > 0)
				{
					head_flag1 = 0;   //标识位清零
					if (sum[num] == sum[num - 1] && sum1[num] == sum1[num - 1])   //判断标识是否相等
					{
						head_flag1 = 1;
					}

				}
			}
			else
			{
				num = 0;
			}

			solo_flag = 0;
			if (UDH_cnt == 0x01)
			{
				solo_flag = 1;
			}
			else
			{
				num++;
			}

			Src += 2;                                         //越过两个标识

			PDU_StringtoBytes(Src, &UDH_number, 2);          //获取了子短信单独序号 01

			printf("此短信的序号为%d\n", UDH_number);

			Src -= 14;                                      //回到用户数据信息的位置

			//tmp = 0x8c;            //140
			tmp = 0xA0;              //160

			if (UDH_number == 0x01)
			{
				if (pdu_message.TP_DCS == GSM_7BIT && solo_flag == 1) //00
				{
					// 7-bit解码
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// 转换到TP-DU
																					
				}
				 else if (pdu_message.TP_DCS == GSM_7BIT) //00
				{

					// 7-bit解码
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// 转换到TP-DU
																								//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("用户信息长度%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[0], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[0], n_dstlength);   //短信内容提取

				}

			}

			else if (UDH_number == 0x02)
			{
				Src += 16;
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit解码
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[160], n_dstlength);						// 转换到TP-DU
																									//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("用户信息长度%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[160], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[160], n_dstlength);   //短信内容提取

				}

			}
			else if (UDH_number == 0x03)
			{
				Src += 16;
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit解码
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// 格式转换

					PDUdcode_7bit(buf, &pdu_message.TP_UD[320], n_dstlength);						// 转换到TP-DU
																									//printf("接收到的短信内容为:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("用户信息长度%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //格式转换
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[320], n_dstlength);     //短信内容提取
				}
				else               //8 bit 解码
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //格式转换
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[320], n_dstlength);   //短信内容提取
				}
			}
		}
	}

	// 返回目标字符串长度
	return n_dstlength;
}