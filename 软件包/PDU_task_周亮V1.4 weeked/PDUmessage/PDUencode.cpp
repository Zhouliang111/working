#include "pdu_encode.h"
#include "PDU.h"



extern int Cnt ;              //���ŷ�Ƭ������
extern int RN ;        //���ڲ����ı����ʶ
extern int ok ;        //���ڱ�ż���
extern int left ;      //��������
extern struct pdu pdu_message;

//************************************���뺯��****************************************


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

							// ����ֵ��ʼ��
	n_src = 0;
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
		else
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


// PDU���룬���ڱ��ơ����Ͷ���Ϣ
//���Գɹ�
//��Ӣ������ok
//7bit�ɹ�
int PDU_Encode(char* Dst)
{

	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[256];									// �ڲ��õĻ�����

															/*********** SMSC��ַ��Ϣ�� ***********/
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
	buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

																						/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// �û���Ϣ�ַ����ĳ���
	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)

	buf[2] = 0xC4;							// ��Ч��(TP-VP)Ϊ30�죺A8-C4��VP-166��x 1��
	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		//buf[3] = n_length;					// ����ǰ����
		//n_length = PDUencode_7bit(pdu_message.In_TP_UD, &buf[4], n_length) + 4;		// ת��TP-DA��Ŀ��PDU��
		buf[3] = PDUencode_7bit(pdu_message.In_TP_UD, &buf[4], n_length);
		n_length = buf[3] + 4;
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		buf[3] = PDUencode_Unicode(pdu_message.In_TP_UD, &buf[4], n_length);			// ת��TP-DA��Ŀ��PDU��
		n_length = buf[3] + 4;											// nLength���ڸö����ݳ���
	}
	else
	{
		// 8-bit���뷽ʽ
		buf[3] = PDUencode_8bit(pdu_message.In_TP_UD, &buf[4], n_length);			// ת��TP-DA��Ŀ��PDU��
		n_length = buf[3] + 4;											// nLength���ڸö����ݳ���
	}
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
																		// ����Ŀ���ַ�������
	return n_dstlength;
}

//�������ż���ʱ���  
//���Գɹ�
//��Ӣ������ok
//7bit�ɹ�
int PDU_Encode_time(char* Dst)
{
	//���ʱ����ı���
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
															/*********** SMSC��ַ��Ϣ�� ***********/
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
	buf[0] = 0x84;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
	buf[1] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[2] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

						/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/

	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)

	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);          //ת��ʱ�����PDU��

	n_length = (int)strlen(pdu_message.In_TP_UD);			// �û���Ϣ�ַ����ĳ���
	
	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		buf[0] = PDUencode_7bit(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;

	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		buf[0] = PDUencode_Unicode(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;

	}
	else
	{
		// 8-bit���뷽ʽ
		buf[0] = PDUencode_8bit(pdu_message.In_TP_UD, &buf[1], n_length);
		n_length = buf[0] + 1;
	}
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
																			// ����Ŀ���ַ�������
	return n_dstlength;
}



//��������û�ͷ
//��Ӣ��������
//7bit���
int PDU_Encode_longmessage_NO1(char* Dst)      //�������ַ�����û���Ϣͷ�ĳ����ű��� ֻ��һ��
{

	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[1024];									// �ڲ��õĻ�����

	char Temp[1024];

																/*********** SMSC��ַ��Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//���ŷ������ĺ��볤��

	if (n_length > 0)
	{
		//�ֶ����ö������ĺ���
		// SMSC��ַ��Ϣ����, lenghth = Type + Address. AddressΪ����ʱ, Ҫ����'F'.
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
	buf[0] = 0x51;					// ֧���û���Ϣͷ�ı�ʶ
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

																						/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// �û���Ϣ�ַ����ĳ���


	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)
													//buf[2] = 0;							// ��Ч��(TP-VP)Ϊ5����:00-8F��VP+1��x 5����  ��5���Ӽ����12Сʱ
	buf[2] = 0xC4;							// ��Ч��(TP-VP)Ϊ30�죺A8-C4��VP-166��x 1��



	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		buf[3] = n_length + 7;					// ����ǰ����

		/*buf[3] =   PDUencode_7bit(pdu_message.In_TP_UD, &buf[11], n_length) + 7 ;*/
		buf[4] = 0x05;                     //��ӳ������Ų���
		buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[6] = 0x03;                     //������3������
		buf[7] = RN;                       //�жϱ�ʶ
		buf[8] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[9] = 0x01;                       //�Ӷ��ŷ�Ƭ���
																					 //�����û�ͷ
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// ת��4���ֽڵ�Ŀ��PDU��

		Temp[0] = 0x05;
		Temp[1] = 0x00;
		Temp[2] = 0x03;
		Temp[3] = RN;
		Temp[4] = 0x01;
		Temp[5] = 0x01;
		Temp[6] = 0;
		memcpy(&Temp[7], pdu_message.In_TP_UD, n_length);                     //SUCCESS

		//��ӡ����
		/*for (int i=0;i<n_length + 7;i++)
		{
			printf("%c", Temp[i]);
		}*/
		n_length = PDUencode_7bit(Temp, buf, n_length + 8) ;		// ����7bitת���󳤶���һ��
		n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)   //�����Ӣ������
	{
		// UCS2���뷽ʽ

		buf[3] =  PDUencode_Unicode(pdu_message.In_TP_UD, &buf[10], n_length) + 6;
		buf[4] = 0x05;                     //��ӳ������Ų���
		buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[6] = 0x03;                     //������3������
		buf[7] = RN;                       //�жϱ�ʶ
		buf[8] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[9] = 0x01;                       //�Ӷ��ŷ�Ƭ���

		n_length = buf[3] + 10 - 6;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
	}
	else
	{
		// 8-bit���뷽ʽ

		buf[3] = n_length + 6;					// ����ǰ����
		//buf[3] =   PDUencode_8bit(pdu_message.In_TP_UD, &buf[11], n_length) + 6;
		buf[4] = 0x05;                     //��ӳ������Ų���
		buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[6] = 0x03;                     //������3������
		buf[7] = RN;                       //�жϱ�ʶ
		buf[8] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[9] = 0x01;                       //�Ӷ��ŷ�Ƭ���

		n_length = PDUencode_8bit(pdu_message.In_TP_UD, &buf[10], n_length) + 10;
		//n_length = buf[3] +10 -6

		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
	}
	

																			// ����Ŀ���ַ�������
	return n_dstlength;
}

//��������û�ͷ��ʱ���
//��Ӣ��������ȷ
//7bit�ɹ�
int PDU_Encode_longmessage_NO1time(char* Dst)      //�������ַ�����û���Ϣͷ�ĳ����ű��� ֻ��һ��
{
	char now_time[24];
	char Temp[1024];

	time_t t;
	t = time(NULL);
	struct tm *PDU_time = NULL;

	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[1024];									// �ڲ��õĻ�����

	memset(now_time, 0, sizeof(now_time));

	//���ݴ���
	PDU_time = localtime(&t);

	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //��ʱ���תΪ�ַ��� 18082708525108	

																/*********** SMSC��ַ��Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//���ŷ������ĺ��볤��

	if (n_length > 0)
	{
		//�ֶ����ö������ĺ���
		// SMSC��ַ��Ϣ����, lenghth = Type + Address. AddressΪ����ʱ, Ҫ����'F'.
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
	buf[0] = 0x64;					// ֧���û���Ϣͷ�ı�ʶ
	buf[1] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[2] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength],3);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// ת��2���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);
																						
														/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_TP_UD);			// �û���Ϣ�ַ����ĳ���


	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		buf[0] = n_length + 7;					// ����ǰ����
		buf[1] = 0x05;                     //��ӳ������Ų���
		buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[3] = 0x03;                     //������3������
		buf[4] = RN;                       //�жϱ�ʶ
		buf[5] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[6] = 0x01;                       //�Ӷ��ŷ�Ƭ���

		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

		Temp[0] = 0x05;
		Temp[1] = 0x00;
		Temp[2] = 0x03;
		Temp[3] = RN;
		Temp[4] = 0x01;
		Temp[5] = 0x01;
		Temp[6] = 0;
		memcpy(&Temp[7], pdu_message.In_TP_UD, n_length);                     //SUCCESS

		n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// ����7bitת���󳤶���һ��
		n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		buf[0] =   PDUencode_Unicode(pdu_message.In_TP_UD, &buf[7], n_length) + 6;
		buf[1] = 0x05;                     //��ӳ������Ų���
		buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[3] = 0x03;                     //������3������
		buf[4] = RN;                       //�жϱ�ʶ
		buf[5] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[6] = 0x01;                       //�Ӷ��ŷ�Ƭ���
		n_length = buf[0] + 7 - 6;
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
	}
	else
	{
		// 8-bit���뷽ʽ

		buf[0] = n_length + 6;					// ����ǰ����

		//buf[0] =   PDUencode_8bit(pdu_message.In_TP_UD, &buf[11], n_length) + 6;
		buf[1] = 0x05;                     //��ӳ������Ų���
		buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
		buf[3] = 0x03;                     //������3������
		buf[4] = RN;                       //�жϱ�ʶ
		buf[5] = 0x01;                      //�Ӷ��ŷ�Ƭ����
		buf[6] = 0x01;                       //�Ӷ��ŷ�Ƭ���

		n_length = PDUencode_8bit(pdu_message.In_TP_UD, &buf[7], n_length) + 7;
		//n_length = buf[0] +7 -6
		n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
	}
	
				
	return n_dstlength;														// ����Ŀ���ַ�������
}



//�����Ų�ֱ���
//���Ĳ������
//7bit���ok
int PDU_Encode_longmessage_NO2(char* Dst)              //�����ַ��ĳ����Ų�ֱ��� ����ʱ���
{

	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[1024];									// �ڲ��õĻ�����

	char Temp[1024];
																/*********** SMSC��ַ��Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//���ŷ������ĺ��볤��

	if (n_length > 0)
	{
		//�ֶ����ö������ĺ���
		// SMSC��ַ��Ϣ����, lenghth = Type + Address. AddressΪ����ʱ, Ҫ����'F'.
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
	buf[0] = 0x51;					// ֧���û���Ϣͷ�ı�ʶ
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[3] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 4);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

													/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
													//n_length = strlen(pdu_message.In_TP_UD);			// �û���Ϣ�ַ����ĳ���    ���ʱ�򳤶���Ҫ�仯��

	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)


	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)
													//buf[2] = 0;							// ��Ч��(TP-VP)Ϊ5����:00-8F��VP+1��x 5����  ��5���Ӽ����12Сʱ
	buf[2] = 0xC4;							// ��Ч��(TP-VP)Ϊ30�죺A8-C4��VP-166��x 1��

	if (pdu_message.In_TP_DCS == GSM_7BIT)
	{
		// 7-bit���뷽ʽ
		if (ok == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 160 - 7;                                    //7bit���
			buf[3] = left + 7 ;					// ����ǰ����
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// ת��10���ֽڵ�Ŀ��PDU��

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], left);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// ����7bitת���󳤶���һ��
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��

		}

		else
		{
			n_length = 160 - 7;                                    //7bit���
			buf[3] = 160;					// ����ǰ����
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 10);				// ת��10���ֽڵ�Ŀ��PDU��

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], n_length);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// ����7bitת���󳤶���һ��
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;*/

		}
	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		if (ok == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 140 - 6;                  
			//buf[3] = left + 6;					// ����ǰ���� 6�����ݳ���
			buf[3] = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 6;          //����
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = buf[3] + 10 - 6;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��

			/*n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 10;*/
		}

		else
		{
			n_length = 140 - 6;                                    //7bit���
			buf[3] = 140;					// ����ǰ����
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}
	}
	else
	{
		// 8-bit���뷽ʽ
		if (ok == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 140 - 6;                                    //7bit���
			buf[3] = left + 6;					// ����ǰ����  6λ���ݳ���
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], left) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}
		else
		{
			n_length = 140 - 6;                                    //7bit���

			buf[3] = 140;					// ����ǰ����
			buf[4] = 0x05;                     //��ӳ������Ų���
			buf[5] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[6] = 0x03;                     //������3������
			buf[7] = RN;                       //�жϱ�ʶ
			buf[8] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[9] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[10], n_length) + 10;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}
	}
																			// ����Ŀ���ַ�������
	return n_dstlength;
}

//�����Ų�ֱ����ʱ���
//7bit�޸�ok
//��Ӣ�Ĳ������
int PDU_Encode_longmessage_NO2time(char* Dst)              //�����ַ��ĳ����Ų�ֱ���
{
	//���ʱ����ı���
	char now_time[24];
	time_t t;
	t = time(NULL);

	char Temp[1024];

	struct tm *PDU_time = NULL;
	//
	int n_length;											// �ڲ��õĴ�����
	int n_dstlength;											// Ŀ��PDU������
	unsigned char buf[1024];									// �ڲ��õĻ�����
	
	memset(now_time, 0, sizeof(now_time));
	
																//���ݴ���
	PDU_time = localtime(&t);
	
	strftime(now_time, 20, "18%m%d%H%M%S32", PDU_time);        //��ʱ���תΪ�ַ��� 18082708525108													   
																   /*********** SMSC��ַ��Ϣ�� ***********/
	n_length = (int)strlen(pdu_message.In_SCA);									//���ŷ������ĺ��볤��

	if (n_length > 0)
	{
		//�ֶ����ö������ĺ���
		// SMSC��ַ��Ϣ����, lenghth = Type + Address. AddressΪ����ʱ, Ҫ����'F'.
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
	buf[0] = 0x64;					// ֧���û���Ϣͷ�ı�ʶ
	buf[1] = (char)n_length;			// Ŀ���ַ���ָ���(Ŀ���ֻ������ַ�����ʵ����)
	buf[2] = 0x91;					// �̶�: �ù��ʸ�ʽ����
	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 3);				// ת��4���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(pdu_message.In_TPA, &Dst[n_dstlength], n_length);	// ת��TP-DA��Ŀ��PDU��

												/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
												//n_length = strlen(pdu_message.In_TP_UD);			


	buf[0] = pdu_message.In_TP_PID;					// Э���ʶ(TP-PID)
	buf[1] = pdu_message.In_TP_DCS;					// �û���Ϣ���뷽ʽ(TP-DCS)

	n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 2);				// ת��2���ֽڵ�Ŀ��PDU��
	n_dstlength += PDU_InvertNumbers(now_time, &Dst[n_dstlength], 14);

	//printf("ʱ�������˳��Ϊ%s\n", now_time);

	if (pdu_message.In_TP_DCS == GSM_7BIT)    //���������
	{
		// 7-bit���뷽ʽ
		if (ok == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 160 - 7;                                    //7bit�����160
			buf[0] = left + 7;					// ����ǰ����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length * ok], left);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, left + 8);		// ����7bitת���󳤶���һ��
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 7;*/
		}
		else
		{
			n_length = 160 - 7;                                    //7bit�����160
			buf[0] = 160;					// ����ǰ����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], 7);

			Temp[0] = 0x05;
			Temp[1] = 0x00;
			Temp[2] = 0x03;
			Temp[3] = RN;
			Temp[4] = Cnt;
			Temp[5] = ok + 1;
			Temp[6] = 0;
			memcpy(&Temp[7], &pdu_message.In_TP_UD[n_length*ok], n_length);                     //SUCCESS

			n_length = PDUencode_7bit(Temp, buf, n_length + 8);		// ����7bitת���󳤶���һ��
			n_dstlength += PDU_BytestoString(&buf[6], &Dst[n_dstlength], n_length - 7);		// ת���ö����ݵ�Ŀ��PDU��

			/*n_length = PDUencode_7bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;*/
		}

	}
	else if (pdu_message.In_TP_DCS == GSM_UCS2)
	{
		if (ok == Cnt-1 )                       //δ�������һ���Ӷ���
		{
			// UCS2���뷽ʽ
			n_length = 140 - 6;                                    //USC2�����140
			buf[0] = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 6;          //����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���
			n_length = buf[0] + 7 - 6;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}

		else 
		{
			n_length = 140 - 6;                                    //USC2�����140
			buf[0] = 140;					// ����ǰ����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = PDUencode_Unicode(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��

		}
	}
	else
	{
		// 8-bit���뷽ʽ
		if (ok == Cnt - 1)                       //δ�������һ���Ӷ���
		{
			n_length = 140 - 6;                                    //USC2�����140
			buf[0] = left + 6;					// ����ǰ����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], left) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}
		else
		{
			// 8-bit���뷽ʽ
			n_length = 140 - 6;                                    //USC2�����140
			buf[0] = 140;					// ����ǰ����
			buf[1] = 0x05;                     //��ӳ������Ų���
			buf[2] = 0x00;                     //�����Ų�ֱ�ʶ
			buf[3] = 0x03;                     //������3������
			buf[4] = RN;                       //�жϱ�ʶ
			buf[5] = Cnt;                      //�Ӷ��ŷ�Ƭ����
			buf[6] = ok + 1;                       //�Ӷ��ŷ�Ƭ���

			n_length = PDUencode_8bit(&pdu_message.In_TP_UD[n_length * ok], &buf[7], n_length) + 7;
			n_dstlength += PDU_BytestoString(buf, &Dst[n_dstlength], n_length);		// ת���ö����ݵ�Ŀ��PDU��
		}

	}
																		// ����Ŀ���ַ�������
	return n_dstlength;
}