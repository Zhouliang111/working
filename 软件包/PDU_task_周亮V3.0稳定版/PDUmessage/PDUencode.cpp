#include "pdu_encode.h"
#include "PDU.h"


extern int Cnt ;              //���ŷ�Ƭ������
extern int RN ;        //���ڲ����ı����ʶ
extern int encode_num;        //���ڱ�ż���
extern int left ;      //��������
extern struct pdu pdu_message;
extern int encode_mode;   //����ѡ������PDU��ʽ

// 7bit����
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ����봮ָ��
// ����: Ŀ����봮����
int PDUencode_7bit(const char* Src, unsigned char* Dst, int n_Srclength)
{
	int n_src;		// Դ�ַ����ļ���ֵ
	int n_dst;		// Ŀ����봮�ļ���ֵ
	int n_char;		// ��ǰ���ڴ���������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char n_left;	// ��һ�ֽڲ��������
			
	n_src = 0;				// ����ֵ��ʼ��
	n_dst = 0;


	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô�����̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	//+1 ��Ϊ�������һ���ֽڵĸ�4λ���Բ���0000
	while (n_src < (n_Srclength + 1))
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		n_char = n_src & 7;

		// ����Դ����ÿ���ֽ�
		if (n_char == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			n_left = *Src;
		}
		else       //�����ַ��Ĵ���
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*Dst = (*Src << (8 - n_char)) | n_left;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			n_left = *Src >> n_char;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			Dst++;
			n_dst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		Src++;
		n_src++;
	}

	// ����Ŀ�괮����
	return n_dst;
}


// 8bit����
int PDUencode_8bit(const char* Src, unsigned char* Dst, int n_srclength)
{
	// �򵥸���
	memcpy(Dst, Src, n_srclength);

	return n_srclength;
}


// UCS2����
int PDUencode_Unicode(const char* Src, unsigned char* Dst, int n_srclength)
{
	int n_dstlength;		// UNICODE���ַ���Ŀ
	wchar_t wchar[128];	// UNICODE��������

						// GB�ַ���-->UNICODE��
						//nDstLength = strGB2Unicode(pSrc, wchar, nSrcLength);
	n_dstlength = MultiByteToWideChar(CP_ACP, 0, Src, n_srclength, wchar, 128);

	// �ߵ��ֽڶԵ������
	for (int i = 0; i<n_dstlength; i++)
	{
		*Dst++ = wchar[i] >> 8;		// �������λ�ֽ�
		*Dst++ = wchar[i] & 0xff;		// �������λ�ֽ�
	}

	// ����Ŀ����봮����
	return n_dstlength * 2;
}


// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613851872468" --> "683158812764F8"
int PDU_InvertNumbers(const char* Src, char* Dst, int n_srclength)
{
	int n_dstlength;		// Ŀ���ַ�������
	char ch;			// ���ڱ���һ���ַ�

						// ���ƴ�����
	n_dstlength = n_srclength;

	// �����ߵ�
	for (int i = 0; i<n_srclength; i += 2)
	{
		ch = *Src++;		// �����ȳ��ֵ��ַ�
		*Dst++ = *Src++;	// ���ƺ���ֵ��ַ�
		*Dst++ = ch;		// �����ȳ��ֵ��ַ�
	}

	// Դ��������������   һ��Ϊ13λ�绰����
	if (n_srclength & 1)
	{
		*(Dst - 2) = 'F';	// ��'F'
		n_dstlength++;		// Ŀ�괮���ȼ�1  �ӵ�14λ
	}

	// ����ַ����Ӹ�������
	*Dst = '\0';

	// ����Ŀ���ַ�������
	return n_dstlength;
}


// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
// �磺{0xC8} --> "C8" 
int PDU_BytestoString(const unsigned char* Src, char* Dst, int n_srclength)
{
	const char tab[] = "0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�

	for (int i = 0; i < n_srclength; i++)
	{
		*Dst++ = tab[*Src >> 4];			// �����4λ ����λ�Ƴ�ȥ ����߲�0000 
		*Dst++ = tab[*Src & 0x0f];		// �����4λ  0x0f Ϊ 00001111 ȡ������λ
		Src++;
	}

	// ����ַ����Ӹ�������
	*Dst = '\0';

	// ����Ŀ���ַ�������
	return (n_srclength * 2);
}


// ��ͨ���ű���
int PDU_Encode(char* Dst)
{
	char now_time[24];
	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[256];									// �ڲ��õĻ�����

	memset(now_time, 0, sizeof(now_time));
	PDU_time = localtime(&t);
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //��ʱ���תΪ�ַ��� 18082708525108

	n_length = (int)strlen(pdu_message.In_SCA);									//���ŷ������ĺ��볤��

	if (n_length > 0)
	{
		//���ö������ĺ���
		// SMSC��ַ��Ϣ����, lenghthΪ����ʱ, Ҫ����'F'.
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											// �̶�: �ù��ʸ�ʽ����
		n_dstlength = PDU_BytestoString(buf, Dst, 2);				// ת��2���ֽڵ�Ŀ��PDU�� 

																	// ת�����ŷ������ĺ��뵽Ŀ��PDU��
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		//�ն˽��Զ���SIM���ж�ȡ�������ĺ��벢���SCA
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				// ת��1���ֽڵ�Ŀ��PDU��
	}

	/*********** TPDU�λ���������Ŀ���ַ�� ***********/
	n_length = (int)strlen(pdu_message.In_TPA);	// TP-DA��ַ�ַ����ĳ���

	//ѡ���Ƿ��ʱ���
	if (encode_mode == 1) //����ʱ���
	{
		buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
		buf[1] = 0;						// TP-MR=0
		buf[2] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
		buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// ת��4���ֽڵ�Ŀ��PDU��
	}

	else          //��ʱ���
	{
		buf[0] = 0x84;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
		buf[1] = (char)n_length;
		buf[2] = 0x91;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

	if (encode_mode == 1)
	{
		buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)
		buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)
		buf[2] = 0xC4;							// ��Ч��(TP-VP)Ϊ30�죺A8-C4��VP-166��x 1��
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}

	else
	{
		buf[0] = pdu_message.In_TP_PID;
		buf[1] = pdu_message.In_TP_DCS;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);
		n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);
	}

	n_length = (int)strlen(pdu_message.In_TP_UD);

	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		buf[0] = n_length;					// ����ǰ����
		n_length = PDUencode_7bit(pdu_message.In_TP_UD, &buf[1], n_length) + 1;		// ת��TP-DA��Ŀ��PDU��
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		buf[0] = PDUencode_Unicode(pdu_message.In_TP_UD, &buf[1], n_length);			// ת��TP-DA��Ŀ��PDU��
		n_length = buf[0] + 1;											// nLength���ڸö����ݳ���
	}
	else
	{
		// 8-bit���뷽ʽ
		buf[0] = PDUencode_8bit(pdu_message.In_TP_UD, &buf[1], n_length);			// ת��TP-DA��Ŀ��PDU��
		n_length = buf[0] + 1;											// nLength���ڸö����ݳ���
	}
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
																			// ����Ŀ���ַ�������
	return n_dstlength;
}


//��������û�ͷ����
int PDU_Encode_longmessage_NO1(char* Dst)  
{
	char now_time[24];
	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	int n_length;											
	int n_dstlength;											
	unsigned char buf[1024];								
	char Temp[1024];

	memset(now_time, 0, sizeof(now_time));
	PDU_time = localtime(&t);
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);
											
	n_length = (int)strlen(pdu_message.In_SCA);									
	if (n_length > 0)
	{
		
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;										
		n_dstlength = PDU_BytestoString(buf, Dst, 2);																	
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				
	}
	n_length = (int)strlen(pdu_message.In_TPA);

	if (encode_mode == 1)
	{
		buf[0] = 0x51;
		buf[1] = 0;
		buf[2] = (char)n_length;
		buf[3] = 0x91;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);
	}
	else
	{
		buf[0] = 0x64;
		buf[1] = (char)n_length;
		buf[2] = 0x91;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}

	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);

	if (encode_mode == 1)
	{
		buf[0] = pdu_message.In_TP_PID;
		buf[1] = pdu_message.In_TP_DCS;
		buf[2] = 0xC4;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}
	else
	{
		buf[0] = pdu_message.In_TP_PID;
		buf[1] = pdu_message.In_TP_DCS;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);
		n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);
	}
																				
	n_length = (int)strlen(pdu_message.In_TP_UD);			

	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		buf[0] = n_length + 7;					

		buf[1] = 0x05;                   
		buf[2] = 0x00;                    
		buf[3] = 0x03;                    
		buf[4] = RN;                      
		buf[5] = 0x01;                     
		buf[6] = 0x01;                      
																					
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);	

		Temp[0] = 0x05;
		Temp[1] = 0x00;
		Temp[2] = 0x03;
		Temp[3] = RN;
		Temp[4] = 0x01;
		Temp[5] = 0x01;
		Temp[6] = 0;
		memcpy(&Temp[7], pdu_message.In_TP_UD, n_length);                   

		n_length = PDUencode_7bit(Temp, buf, n_length + 8) ;		
		n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);	
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2) 
	{
		// UCS2���뷽ʽ

		buf[0] =  PDUencode_Unicode(pdu_message.In_TP_UD, &buf[7], n_length) + 6;
		buf[1] = 0x05;                    
		buf[2] = 0x00;                    
		buf[3] = 0x03;                    
		buf[4] = RN;                       
		buf[5] = 0x01;                      
		buf[6] = 0x01;                     
		n_length = buf[0] + 7 - 6;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
	}
	else
	{
		// 8-bit���뷽ʽ
		buf[0] = n_length + 6;					
		buf[1] = 0x05;                    
		buf[2] = 0x00;                    
		buf[3] = 0x03;                    
		buf[4] = RN;                       
		buf[5] = 0x01;                     
		buf[6] = 0x01;                    
		n_length = PDUencode_8bit(pdu_message.In_TP_UD, &buf[7], n_length) + 7;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		
	}
	return n_dstlength;
}

//�����Ų�ֱ��� 
//7bit bug
int PDU_Encode_longmessage_NO2(char* Dst)              //�����ַ��ĳ����Ų�ֱ��� ����ʱ���
{
	char now_time[24];
	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	int n_length;											
	int n_dstlength;											
	unsigned char buf[1024];
	char Temp[1024];

	memset(now_time, 0, sizeof(now_time));
	PDU_time = localtime(&t);
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);
															
	n_length = (int)strlen(pdu_message.In_SCA);									

	if (n_length > 0)
	{
		buf[0] = (char)((n_length & 1) == 0 ? n_length : n_length + 1) / 2 + 1;
		buf[1] = 0x91;											
		n_dstlength = PDU_BytestoString(buf, Dst, 2);																			
		n_dstlength += PDU_InvertNumbers(pdu_message.In_SCA, &Dst[n_dstlength], n_length);
	}
	else
	{
		buf[0] = 0x00;
		n_dstlength = PDU_BytestoString(buf, Dst, 1);				
	}
	n_length = (int)strlen(pdu_message.In_TPA);

	if (encode_mode == 1)
	{
		buf[0] = 0x51;
		buf[1] = 0;
		buf[2] = (char)n_length;
		buf[3] = 0x91;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);

	}
	else
	{
		buf[0] = 0x64;
		buf[1] = (char)n_length;
		buf[2] = 0x91;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);

	if (encode_mode == 1)
	{
		buf[0] = pdu_message.In_TP_PID;
		buf[1] = pdu_message.In_TP_DCS;
		buf[2] = 0xC4;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);
	}
	else
	{
		buf[0] = pdu_message.In_TP_PID;
		buf[1] = pdu_message.In_TP_DCS;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);
		n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);
	}
					
	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		if (encode_num == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 160 - 7;                                   
			buf[0] = left + 7 ;					
			buf[1] = 0x05;                    
			buf[2] = 0x00;                    
			buf[3] = 0x03;                    
			buf[4] = RN;                       
			buf[5] = Cnt;                     
			buf[6] = encode_num + 1;                      
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);	

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = encode_num + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*encode_num], left);                    

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);	

		}

		else
		{
			n_length = 160 - 7;                                   
			buf[0] = 160;					
			buf[1] = 0x05;                    
			buf[2] = 0x00;                    
			buf[3] = 0x03;                    
			buf[4] = RN;                      
			buf[5] = Cnt;                      
			buf[6] = encode_num + 1;                       
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);			

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = encode_num + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*encode_num], n_length);                    

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);	
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);	

		}
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		if (encode_num == Cnt - 1)                     
		{
			n_length = 140 - 6;                  
			buf[0] = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * encode_num], &buf[7], left) + 6;          //����
			buf[1] = 0x05;                    
			buf[2] = 0x00;                     
			buf[3] = 0x03;                    
			buf[4] = RN;                       
			buf[5] = Cnt;                     
			buf[6] = encode_num + 1;                    

			n_length = buf[0] + 7 - 6;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}

		else
		{
			n_length = 140 - 6;                                   
			buf[0] = 140;					
			buf[1] = 0x05;                     
			buf[2] = 0x00;                     
			buf[3] = 0x03;                     
			buf[4] = RN;                      
			buf[5] = Cnt;                      
			buf[6] = encode_num + 1;                     
			n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * encode_num], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);
		}
	}
	else
	{
		// 8-bit���뷽ʽ
		if (encode_num == Cnt - 1)                     
		{
			n_length = 140 - 6;                                   
			buf[0] = left + 6;					
			buf[1] = 0x05;                    
			buf[2] = 0x00;                    
			buf[3] = 0x03;                     
			buf[4] = RN;                      
			buf[5] = Cnt;                      
			buf[6] = encode_num + 1;                       

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * encode_num], &buf[7], left) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);	
		}
		else
		{
			n_length = 140 - 6;                                  
			buf[0] = 140;					
			buf[1] = 0x05;                   
			buf[2] = 0x00;                    
			buf[3] = 0x03;                    
			buf[4] = RN;                       
			buf[5] = Cnt;                      
			buf[6] = encode_num + 1;                     
			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * encode_num], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);	
		}
	}																		
			return n_dstlength;
}


//�����ַ����Һ���
int special_find(char *src,int num)
{
	const char extend[9] = {'^','{','}','\\','[','~',']','|','�'};
	int back_value = 0;
	for (int i = 0;i < num;i++)
	{
		for (int j = 0;j < 9;j++)
		{
			if (*src == extend[j])
			{
				back_value++;
				break;
			}
		}
		src++;
	}
	printf("�û���Ϣ����%d�������ַ�\n", back_value);
	return back_value;

}

//int special_find(char *src,int num)
//{
//	int back_value = 0;
//	for (int i=0;i<num;i++)
//	{
//		if (*src == '^')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '{')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '}')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '\\')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '[')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '~')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == ']')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '|')
//		{
//			src++;
//			back_value++;
//		}
//		else if (*src == '�')
//		{
//			
//			src++;
//			back_value++;
//		}
//		else
//		{
//			
//			src++;
//		}
//	}
//	printf("�û���Ϣ����%d�������ַ�\n", back_value);
//	return back_value;
//	
//}


void extend_change(char *src,char *dst,int num)
{
	const char extend[9] = { '^','{','}','\\','[','~',']','|','�' };
	const char extend_value[9] = {0x14,0x28,0x29,0x2F,0x3C,0x3D,0x3E,0x40,0x65};

	for (int i=0;i<num;i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (*src == extend[j])
			{
				*src = 0x1B;
				*(src + 1) = extend_value[j];
			}
		}
		*dst++ = *src++;
	}

}