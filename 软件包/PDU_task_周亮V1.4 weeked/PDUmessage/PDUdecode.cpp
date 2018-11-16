#include "pdu_decode.h"
#include "PDU.h"

/*���ܲ��Ա�ע
7bit��ʱ�������ok
7bit���û���Ϣͷ�пո�  ���
���Ĵ�ʱ�������ok
���Ĵ��û�ͷ��������     �����
*/


extern struct pdu pdu_message;
extern int head_flag  ;    //������ϱ�ʶ�ı�־λ
extern int head_flag1 ;   //������λ��ʶ�ı�־λ
extern int nUDI_flag  ;    //�������û���Ϣͷ��־λ
extern int solo_flag  ;           //�����Ӷ��ŵĽ�����־
extern unsigned char UDH_cnt;   //���������������ڴ�ӡ
extern int num;       //����


//7bit ���뺯��  ���봮->�ַ���
//Src Դ���봮��ַ
//Dst Ŀ���ַ�����ַ
//Src_length Դ���봮����
//���أ�Ŀ���ַ�������

int PDUdcode_7bit(const unsigned char *Src, char *Dst, int n_Srclength)
{
	int n_src;   // Դ�ַ����ļ���ֵ
	int n_dst;  // Ŀ����봮�ļ���ֵ 
	int n_byte; // ��ǰ���ڴ���������ֽڵ���ţ���Χ��0-6 

	unsigned char n_left; // ��һ�ֽڲ�������� 

						  // ����ֵ��ʼ��  
	n_src = 0;
	n_dst = 0;
	// �����ֽ���źͲ������ݳ�ʼ��
	n_byte = 0;
	n_left = 0;


	// ��Դ����ÿ7���ֽڷ�Ϊһ�飬��ѹ����8���ֽ�  
	// ѭ���ô�����̣�ֱ��Դ���ݱ�������  
	// ������鲻��7�ֽڣ�Ҳ����ȷ����

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


// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
// �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ������ָ��
// ����: Ŀ�����ݳ���
int PDU_StringtoBytes(const char* Src, unsigned char* Dst, int n_Srclength)
{
	//printf("gsmString2Bytes start ... nSrcLength = %d\n", n_Srclength);
	for (int i = 0; i < n_Srclength; i += 2)
	{
		// �����4λ
		if ((*Src >= '0') && (*Src <= '9'))
		{
			*Dst = (*Src - '0') << 4;   //���� '3'���� 0x03
		}
		else
		{
			*Dst = (*Src - 'A' + 10) << 4; // ���确b������ 0x0b  ����ֻ����ת�����ַ�Ϊ��д�����
		}

		//printf("Src[%d] = %c\n", i, *Src);   //��ӡԴ�ַ���
		//printf("==Dst[%d] = 0x%02x\n", i, *Dst);  //��ӡת�����16�����ַ�

		Src++;

		// �����4λ
		if ((*Src >= '0') && (*Src <= '9')) //ͬ��ת��
		{
			*Dst |= *Src - '0';
		}
		else
		{
			*Dst |= *Src - 'A' + 10;   //ͬ��ת��
		}
		//printf("Src[%d] = %c\n", i, *Src);
		//printf("==Dst[%d] = 0x%02x\n", i/2, *Dst);

		Src++;
		Dst++;
	}

	//printf("gsmString2Bytes end --- n_Srclength = %d\n", n_Srclength);
	// ����Ŀ�����ݳ���
	return (n_Srclength / 2);

}



// �����ߵ����ַ���ת��Ϊ����˳����ַ���
// �磺"683158812764F8" --> "8613851872468"
// ����: pSrc - Դ�ַ���ָ��
//       nSrcLength - Դ�ַ�������
// ���: pDst - Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int PDU_SerializeNumbers(const char* Src, char* Dst, int n_Srclength)
{
	int n_dstlength;		// Ŀ���ַ�������
	char ch;			// ���ڱ���һ���ַ�

						// ���ƴ�����
	n_dstlength = n_Srclength;

	// �����ߵ�
	for (int i = 0; i<n_Srclength; i += 2)
	{
		ch = *Src++;		// �����ȳ��ֵ��ַ�
		*Dst++ = *Src++;	// ���ƺ���ֵ��ַ�
		*Dst++ = ch;		// �����ȳ��ֵ��ַ�
	}

	// �ַ���'F'��
	if (*(Dst - 1) == 'F')
	{
		Dst--;
		n_dstlength--;		// Ŀ���ַ������ȼ�1
	}

	// ����ַ����Ӹ�������
	*Dst = '\0';

	// ����Ŀ���ַ�������
	return n_dstlength;
}



// 8bit����
// ����: pSrc - Դ���봮ָ��
//       nSrcLength -  Դ���봮����
// ���: pDst -  Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int PDUdcode_8bit(const unsigned char* Src, char* Dst, int n_Srclength)
{
	// �򵥸���
	memcpy(Dst, Src, n_Srclength);  //��Դ�ַ������ݸ��Ƶ�Ŀ���ַ���

									// ����ַ����Ӹ�������
	*Dst = '\0';

	return n_Srclength;
}


// UCS2����
// ����: Src - Դ���봮ָ��
//       n_Srclength -  Դ���봮����
// ���: Dst -  Ŀ���ַ���ָ��
// ����: Ŀ���ַ�������
int PDUdcode_Unicode(const unsigned char* Src, char* Dst, int n_Srclength)
{
	int n_dstlength;		// UNICODE���ַ���Ŀ
							//WCHAR wchar[128];	   // UNICODE�������� ��Ҫ������ַ�
	wchar_t wchar[128];


	// �ߵ��ֽڶԵ���ƴ��UNICODE
	for (int i = 0; i<n_Srclength / 2; i++)      //����24���ַ����ߵ��ֽڶԵ� һ���ֽ�8λΪһ��Ϊ����16�����ַ�������24/2
	{
		wchar[i] = *Src++ << 8;	// �ȸ�λ�ֽ�
		wchar[i] |= *Src++;		// ���λ�ֽ�
	}

	// UNICODE��-->�ַ���

	n_dstlength = WideCharToMultiByte(CP_ACP, 0, wchar, n_Srclength / 2, Dst, 160, NULL, NULL);
	//���ú�����ͬ
	//n_dstlength = wcstombs_s(&len,Dst,n_Srclength / 2,wchar,128);
	//n_dstlength = wcstombs(Dst, wchar,len);


	// ����ַ����Ӹ�������
	Dst[n_dstlength] = '\0';

	// ����Ŀ���ַ�������
	return n_dstlength;
}


// PDU����
// ����: pSrc - ԴPDU��ָ��
// ���: pDst - Ŀ��PDU����ָ��
// ����: �û���Ϣ������
int PDU_Decode(const char* Src)
{
	unsigned char UDH_number; //�ж��Ӷ��ŵĶ������

	int n_dstlength = 0;			// Ŀ��PDU������
	unsigned char tmp;		// �ڲ��õ���ʱ�ֽڱ���
	unsigned char buf[256];	// �ڲ��õĻ�����

	int UDHI;  //�ж�UDHIλ 0 1

	unsigned char UDU_ct;//�ж��Ƿ�λ�����Ų�ֱ�ʶ 00

	unsigned char UDH_ID;    //�ж��Ӷ����Ƿ�Ϊͬһ��ʶ
	unsigned char UDH_ID2;    //�ж��Ӷ����Ƿ�Ϊͬһ��ʶ

	static unsigned char sum[10];
	static unsigned char sum1[10];


	/*********** SMSC��ַ��Ϣ�� ***********/
	PDU_StringtoBytes(Src, &tmp, 2);						// ȡ����
	tmp = (tmp - 1) * 2;								// SMSC���봮����
	Src += 4;											// ָ����ƣ�������SMSC��ַ��ʽ
	PDU_SerializeNumbers(Src, pdu_message.SCA, tmp);				// ת��SMSC���뵽Ŀ��PDU��
																	//printf("SCA: %s\n", pdu_message.SCA);
	Src += tmp;										// ָ�����

															/*********** PDU -TYPE ***********/
	PDU_StringtoBytes(Src, &tmp, 2);						// ȡ�������� ��Ҫ�ж��Ƿ����û�ͷ�ı�־
															//UDHI =0  ������ͷ��Ϣ UDHI =1 ����ͷ��Ϣ 
															//UDHλ�� ��6λ ��Ҫ�ж�UDH ��0 ���� 1

	UDHI = (tmp & 0x40) >> 6;								//��UDH��ֵȡ��  !!�ɹ���ȡUDHI��ֵ
	nUDI_flag = 0;
	if (UDHI == 0)  //���û�����ͷ
	{
		nUDI_flag = 1;
		printf("UDHI=0 UD������ͷ\n");    //������ͷ���������ķ�ʽ����

		Src += 2;											// ָ�����
															/*********** ȡ�ظ����� ***********/
		PDU_StringtoBytes(Src, &tmp, 2);						// ȡ����
		if (tmp & 1) tmp += 1;								// ������ż��
		Src += 4;											// ָ����ƣ������˻ظ���ַ(TP-RA)��ʽ
		PDU_SerializeNumbers(Src, pdu_message.TPA, tmp);				// ȡTP-RA����
																		//printf("TPA: %s\n", pdu_message.TPA);
		Src += tmp;										// ָ�����

														/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_PID, 2);			// ȡЭ���ʶ(TP-PID)
		Src += 2;													// ָ�����

		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_DCS, 2);			// ȡ���뷽ʽ(TP-DCS)
		Src += 2;													// ָ�����

		PDU_SerializeNumbers(Src, pdu_message.TP_SCTS, 14);						// ����ʱ����ַ���(TP_SCTS) 

		Src += 14;													// ָ�����

		PDU_StringtoBytes(Src, &tmp, 2);								// �û���Ϣ����(TP-UDL)  
		pdu_message.TP_UDL = tmp;                                      //��UDL���ȸ�ֵ

		Src += 2;													    // ָ�����

		if (pdu_message.TP_DCS == GSM_7BIT) //7 bit����
		{
			// 7-bit����
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��


			PDUdcode_7bit(buf, pdu_message.TP_UD, n_dstlength);						// ת����TP-DU
																					//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
			n_dstlength = tmp;
			printf("�û���Ϣ����%d\n", n_dstlength);
		}

		else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
		{
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
			PDUdcode_Unicode(buf, pdu_message.TP_UD, n_dstlength);     //����������ȡ
		}
		else               //8 bit ����
		{
			n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
			n_dstlength = PDUdcode_8bit(buf, pdu_message.TP_UD, n_dstlength);   //����������ȡ
		}
	}

	else        //������ͷ�Ĵ���  ������Ҫ����û�ͷ�ı�ʶ�ж�
	{

		printf("UDHI=1 UD������ͷ\n");    //������ͷ��Ҫ��UDL���д��� UDL= UDH+UD 

		Src += 2;											// ָ�����
															/*********** ȡ�ظ����� ***********/
		PDU_StringtoBytes(Src, &tmp, 2);						// ȡ����
		if (tmp & 1) tmp += 1;								// ������ż��
		Src += 4;											// ָ����ƣ������˻ظ���ַ(TP-RA)��ʽ
		PDU_SerializeNumbers(Src, pdu_message.TPA, tmp);				// ȡTP-RA����
																		//printf("TPA: %s\n", pdu_message.TPA);
		Src += tmp;											// ָ�����

														/*********** TPDU��Э���ʶ�����뷽ʽ���û���Ϣ�� ***********/
		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_PID, 2);			// ȡЭ���ʶ(TP-PID)
		Src += 2;													// ָ�����

		PDU_StringtoBytes(Src, (unsigned char*)&pdu_message.TP_DCS, 2);			// ȡ���뷽ʽ(TP-DCS)
		Src += 2;													// ָ�����

		PDU_SerializeNumbers(Src, pdu_message.TP_SCTS, 14);						// ����ʱ����ַ���(TP_SCTS) 

		Src += 14;													// ָ�����

		PDU_StringtoBytes(Src, &tmp, 2);								// ��ʱ�û���Ϣ����TP-UDL=UDH+UD  //ȡ����
		//////////////
		pdu_message.TP_UDL = tmp;                                      //���ȷ�Ϊ�����Ų�ֺ��������
		///////////////////
		Src += 2;													    // ָ�����

		PDU_StringtoBytes(Src, pdu_message.UDH_head, 12);           //��ȡ�˴�ӡ���û�ͷ���� 050003BF0201 ��ʾ������ӡ

		PDU_StringtoBytes(Src, pdu_message.UDH_head1, 14);            //��ȡ���û���Ϣͷ����λ��ʶ   06080419420302


		PDU_StringtoBytes(Src, &pdu_message.UDH_headlength, 2);                  //��ȡ���û�ͷ����  05 06 ����Ҫ�����ж�
		Src += 2;													 // 05��6������ͷ 06��7��Э��ͷ

		PDU_StringtoBytes(Src, &UDU_ct, 2);                      //��ȡ�����Ų�ֱ�ʶ00

		if (UDU_ct == 0x00)          //00 �����Ų��   
		{
			printf("8bit�����Ų�ֱ�ʶ\n");
		}
		else if (UDU_ct == 0x08)
		{
			printf("16bit�����Ų�ֱ�ʶ\n");
		}
/////////////////////////////////////////////////////////////////////////////////////

		//����6���ֽڵ�����ͷ
		if (pdu_message.UDH_headlength == 0x05)    //ֻ���ж�һ����ʶ ȡ��ǰ6���ֽ�
		{
			Src += 4;            //6����Ϣͷ�ƶ�4�� 
			PDU_StringtoBytes(Src, &UDH_ID, 2);                    //��ȡ�����Ų���Ӷ��ű�ʶ BF
			Src += 2;

			PDU_StringtoBytes(Src, &UDH_cnt, 2);                //��ȡһ���ж���������
			printf("����������һ��%d��\n", UDH_cnt);

			if (num < UDH_cnt)                                       //�����û�ͷ��ʶ����
			{
				sum[num] = UDH_ID;                              //�������֧�ֳ����źϲ�������

				if (num > 0)
				{
					head_flag = 0;   //��ʶλ����
					if (sum[num] == sum[num - 1])   //�жϱ�ʶ�Ƿ����
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
			PDU_StringtoBytes(Src, &UDH_number, 2);          //��ȡ���Ӷ��ŵ������ 01

			printf("�˶��ŵ����Ϊ%d\n", UDH_number);

			if (UDH_number == 0x01)
			{
				if (pdu_message.TP_DCS == GSM_7BIT && solo_flag == 1)
				{
					tmp = pdu_message.TP_UDL;						
					Src -= 10;                                      //�ص��û�������Ϣ��λ��								
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// ת����TP-DU
				}
				else if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					tmp = pdu_message.TP_UDL;                     //ʮ���� 160
					Src -= 10;                                      //�ص��û�������Ϣ��λ��
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// ת����TP-DU
				}
				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					tmp = pdu_message.TP_UDL - 6;     //ʮ���� 140
					Src += 2;
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[0], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					tmp = pdu_message.TP_UDL - 6;     //ʮ���� 140
					Src += 2;
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[0], n_dstlength);   //����������ȡ
				}
			}
			else if (UDH_number == 0x02)
			{
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit����
					tmp = pdu_message.TP_UDL;                     //ʮ���� 160
					Src -= 10;                                      //�ص��û�������Ϣ��λ��
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[160], n_dstlength);						// ת����TP-DU
																									//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("�û���Ϣ����%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[160], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[160], n_dstlength);   //����������ȡ

				}
			}
			else if (UDH_number == 0x03)
			{
				Src += 12;

				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit����
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[320], n_dstlength);						// ת����TP-DU
																									//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("�û���Ϣ����%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[320], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[320], n_dstlength);   //����������ȡ
				}

			}

		}

//�ڶ������
		//����7���ֽڵ�����ͷ
		else                        //06   ��Ҫ�ж������û���Ϣ��ʶȡ��ǰ7���ֽ�
		{
			Src += 4;            //  7����Ϣͷ��������ʶ

			PDU_StringtoBytes(Src, &UDH_ID, 2);                    //��ȡ�����ŵ�1�����ű�ʶ
			Src += 2;
			PDU_StringtoBytes(Src, &UDH_ID2, 2);                     //��ȡ�����ŵ�2�����ű�ʶ
			Src += 2;

			PDU_StringtoBytes(Src, &UDH_cnt, 2);               //��������һ������
			printf("����������һ��%d��\n", UDH_cnt);

			if (num < UDH_cnt)                                       //�����û�ͷ��ʶ����
			{
				sum[num] = UDH_ID;                              //�������֧�ֳ����źϲ�������
				sum1[num] = UDH_ID2;
				if (num > 0)
				{
					head_flag1 = 0;   //��ʶλ����
					if (sum[num] == sum[num - 1] && sum1[num] == sum1[num - 1])   //�жϱ�ʶ�Ƿ����
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

			Src += 2;                                         //Խ��������ʶ

			PDU_StringtoBytes(Src, &UDH_number, 2);          //��ȡ���Ӷ��ŵ������ 01

			printf("�˶��ŵ����Ϊ%d\n", UDH_number);

			Src -= 14;                                      //�ص��û�������Ϣ��λ��

			//tmp = 0x8c;            //140
			tmp = 0xA0;              //160

			if (UDH_number == 0x01)
			{
				if (pdu_message.TP_DCS == GSM_7BIT && solo_flag == 1) //00
				{
					// 7-bit����
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// ת����TP-DU
																					
				}
				 else if (pdu_message.TP_DCS == GSM_7BIT) //00
				{

					// 7-bit����
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[0], n_dstlength);						// ת����TP-DU
																								//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("�û���Ϣ����%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[0], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[0], n_dstlength);   //����������ȡ

				}

			}

			else if (UDH_number == 0x02)
			{
				Src += 16;
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit����
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[160], n_dstlength);						// ת����TP-DU
																									//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("�û���Ϣ����%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[160], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[160], n_dstlength);   //����������ȡ

				}

			}
			else if (UDH_number == 0x03)
			{
				Src += 16;
				if (pdu_message.TP_DCS == GSM_7BIT) //00
				{
					// 7-bit����
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp & 7 ? (int)tmp * 7 / 4 + 2 : (int)tmp * 7 / 4);	// ��ʽת��

					PDUdcode_7bit(buf, &pdu_message.TP_UD[320], n_dstlength);						// ת����TP-DU
																									//printf("���յ��Ķ�������Ϊ:  %s\n", pdu_message.TP_UD);
					n_dstlength = tmp;
					//printf("�û���Ϣ����%d\n", n_dstlength);
				}

				else if (pdu_message.TP_DCS == GSM_UCS2)  //Unicode ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);            //��ʽת��
					PDUdcode_Unicode(buf, &pdu_message.TP_UD[320], n_dstlength);     //����������ȡ
				}
				else               //8 bit ����
				{
					n_dstlength = PDU_StringtoBytes(Src, buf, tmp * 2);                     //��ʽת��
					n_dstlength = PDUdcode_8bit(buf, &pdu_message.TP_UD[320], n_dstlength);   //����������ȡ
				}
			}
		}
	}

	// ����Ŀ���ַ�������
	return n_dstlength;
}