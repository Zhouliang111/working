/***********************
PDU�����������
ʵ�ֵĹ��ܣ�1.7bit���룬8bit���룬Unicode���Ľ���
			2.7bit���룬8bit���룬Unicode ���ı���
			3.�������ŵı����ֹ���
			4.�������Ž��빦��

Ŀǰ�� ���뷽ʽ��Ϊ���֣�
		��һ�����������룬���������Чʱ�����ڷ��͸����ŷ�������
		�ڶ�����ģ����룬�������ģ����ŷ������ĵ�ʱ�����ʽ		 
************************/

#include "pdu_encode.h"
#include "pdu_decode.h"
#include "PDU.h"

FILE *out = NULL;
struct pdu pdu_message = { 0 };     //����ṹ�����
struct pdu pdu_long[100] = { 0 };   //�����Žṹ��������

char messageok[1024] = {0};		//��ȡ�ļ�������
char PDU_message[1024] = {0};	 //�����Ϣ������
//���ܱ�־λ
char Decode_flag = 0;			//�û����빦�ܱ�־λ
char Encode_flag = 0;			 //�û����빦�ܱ�־λ
//�����־λ
char head_flag = 0;				 //������ϱ�ʶ�ı�־λ
char head_flag1 = 0;			//������λ��ʶ�ı�־λ
char nUDI_flag = 0;				 //�������û���Ϣͷ��־λ
char solo_flag = 0;				 //�����Ӷ��ŵĽ�����־

//���ڳ����ŵı��붨��
//����
unsigned char UDH_cnt = 0;   //���������������ڴ�ӡ
int decode_num = 0;		   //�����ű�������
//����
int Cnt = 0;              //���ŷ�Ƭ������
int left = 0;		     //��������
int encode_num = 0;				 //���ڱ����ż���
int RN;					//���ڲ����ı����ʶ
int encode_mode = 0;  //����ѡ������PDU��ʽ
//


void pdu_show();
void pdu_send();
void user();


int main()
{

	FILE *fp = NULL;
	unsigned int mode = 1;
	errno_t errfp;
	errno_t errout;
	errfp = fopen_s(&fp,"message.txt","r");   //��ȡPDU���ݴ�
	errout = fopen_s(&out,"PDUout.txt", "w"); //д��Ŀ��TXT
	memset(&pdu_message,0,sizeof(struct pdu));
	memset(pdu_long,0,sizeof(struct pdu)*100);    //�ṹ�������ʼ��
	if (errfp != NULL)
	{
		printf("�ļ���ʧ��\n");
		system("pause");
		exit(0);
	}
	if (errout != NULL)
	{
		printf("�ļ����ʧ��\n");
		system("pause");
		exit(0);
	}
	user();
	while (1)
	{
		if (Decode_flag == 1)
		{
			Decode_flag = 0;
			if (fp != NULL)
			{
				while (fgets(messageok, sizeof(messageok), fp))   //�ı��������
				{
					strncpy_s(pdu_message.data, messageok, sizeof(messageok));
					if (PDU_Decode(pdu_message.data) != false)    //��������ݴ����ж�
					{
						pdu_show();
					}
					system("pause");
				}
				break;
			}
		}
		if (Encode_flag == 1)   //���ű��벢����
		{
			Encode_flag = 0;
			pdu_send();
			printf("��ѡ�������˳�: 1 ���� 2 �˳�\n");
			scanf_s("%d",&mode);
			if (mode == 1)
			{
				errout = fopen_s(&out, "PDUout.txt", "r"); 
				if (out != NULL)
				{
					while (fgets(messageok, sizeof(messageok), out))   //�ı��������
					{
						strncpy_s(pdu_message.data, messageok, sizeof(messageok));
						if (PDU_Decode(pdu_message.data) != false)
						{
							pdu_show();
						}
						system("pause");
					}
					system("pause");
					break;
				}
			}
			else
			{
				printf("PDU�������\n");
				system("pause");
				break;
			}
		}
	}
		if (fp != NULL || out != NULL)
		{
			fclose(fp);
			fclose(out);
		}
			return 0;
}


void user()
{
	int select = 0;
	printf("PDU���ű������  \
		   \n��������1   �����PDU�����Ѵ���    \
		   \n��������2   �ֶ������������\n");
	scanf_s("%d", &select);
	if (select == 1)
	{
		Decode_flag = 1;
	}
	else if (select == 2)
	{
		Encode_flag = 1;
	}



}

void pdu_show()
{
	
	if (nUDI_flag == 1)   //û���û�ͷ�����
	{
		nUDI_flag = 0;
		printf("********************PDU���Ž���TASK*********************\n");
		printf("�����������:\n");
		printf("�������ĺ���Ϊ: +%s\n", pdu_message.SCA);
		printf("���ͷ��ֻ�����: %s\n", pdu_message.TPA);
		printf("���յ��Ķ�������Ϊ:\n");
		for (int i = 0; i < pdu_message.TP_UDL; i++)
		{
			printf("%c", pdu_message.TP_UD[i]);
		}
		printf("\n");
		printf("����ʱ�䣺\n");
		printf("20%c%c-%c%c-%c%c  %c%c-%c%c-%c%c +%c%cʱ��:[>24ʱ -24]\n", pdu_message.TP_SCTS[0], pdu_message.TP_SCTS[1], pdu_message.TP_SCTS[2], pdu_message.TP_SCTS[3], pdu_message.TP_SCTS[4], pdu_message.TP_SCTS[5], pdu_message.TP_SCTS[6], pdu_message.TP_SCTS[7], pdu_message.TP_SCTS[8], pdu_message.TP_SCTS[9], pdu_message.TP_SCTS[10], pdu_message.TP_SCTS[11], pdu_message.TP_SCTS[12], pdu_message.TP_SCTS[13]);
	}

	if (solo_flag == 1)         //���û�ͷ���������
	{
		solo_flag = 0;
		printf("********************PDU���Ž���TASK*********************\n");
		printf("�����������:\n");
		printf("�������ĺ���Ϊ: +%s\n", pdu_message.SCA);
		printf("���ͷ��ֻ�����: %s\n", pdu_message.TPA);
		printf("���յ��Ķ�������Ϊ:\n");

		if (pdu_message.TP_DCS == GSM_7BIT)   //OK
		{
			for (int i = (int)pdu_message.UDH_headlength + 2; i < pdu_message.TP_UDL; i++)  //���7bit����Ƚ�����
			{
				printf("%c", pdu_message.TP_UD[i]);
			}
		}
		else if(pdu_message.TP_DCS == GSM_UCS2) //���ĺ�8bit
		{
			for (int i = 0 ; i < pdu_message.TP_UDL; i++)    //8bit����������ͷ��ӡ
			{
				printf("%c", pdu_message.TP_UD[i]);
			}
		}
		printf("\n");
		printf("����ʱ�䣺\n");
		printf("20%c%c-%c%c-%c%c  %c%c-%c%c-%c%c +%c%cʱ��:[>24ʱ -24]\n", pdu_message.TP_SCTS[0], pdu_message.TP_SCTS[1], pdu_message.TP_SCTS[2], pdu_message.TP_SCTS[3], pdu_message.TP_SCTS[4], pdu_message.TP_SCTS[5], pdu_message.TP_SCTS[6], pdu_message.TP_SCTS[7], pdu_message.TP_SCTS[8], pdu_message.TP_SCTS[9], pdu_message.TP_SCTS[10], pdu_message.TP_SCTS[11], pdu_message.TP_SCTS[12], pdu_message.TP_SCTS[13]);

		printf("�û�����ͷΪ: 0x 0x xx xx xx ��ʽ\n");


		if (pdu_message.UDH_headlength == 0x05  || pdu_message.UDH_headlength == 0x06)              //��ӡ6���û����ݳ���
		{
			for (int i = 0; i<(int)pdu_message.UDH_headlength + 1; i++)
			{
				printf("%x ", pdu_message.UDH_head[i]);
			}
			printf("\n");
			printf("�û���Ϣ����length=%d\n", pdu_message.TP_UDL);
		}
	}

	if (head_flag == 1 || head_flag1 == 1)        //��ʶƥ��������֮��Ķ���Ϣ
	{
		head_flag = 0;   //��ʶλ����
		head_flag1 = 0;
		if (decode_num == UDH_cnt)           // ������Ҫ���뿪������֮ǰ�Ķ��ŷ�Ƭ����
		{
			decode_num = 0;
			printf("********************PDU���Ž���TASK*********************\n");
			printf("�����������:\n");
			printf("�������ĺ���Ϊ: +%s\n", pdu_message.SCA);
			printf("���ͷ��ֻ�����: %s\n", pdu_message.TPA);
			printf("���������ŵ�����Ϊ��\n");

			if (pdu_message.TP_DCS == GSM_7BIT)  //7bit���
			{
				for (int k = 1;k <= UDH_cnt; k++ )
				{
				
					for (int j = (int)pdu_message.UDH_headlength +2;j < 160;j++)
					{
						printf("%c", pdu_long[k].TP_UD[j]);
					}
				}

			}
			else       //8bit��USC2���
			{
				for (int i = 1; i <= UDH_cnt; i++)
				{
					printf("%s", pdu_long[i].TP_UD);
				}
			}
			

			printf("\n");
			printf("����ʱ�䣺\n");
			printf("20%c%c-%c%c-%c%c  %c%c-%c%c-%c%c +%c%cʱ��:[>24ʱ -24]\n", pdu_message.TP_SCTS[0], pdu_message.TP_SCTS[1], pdu_message.TP_SCTS[2], pdu_message.TP_SCTS[3], pdu_message.TP_SCTS[4], pdu_message.TP_SCTS[5], pdu_message.TP_SCTS[6], pdu_message.TP_SCTS[7], pdu_message.TP_SCTS[8], pdu_message.TP_SCTS[9], pdu_message.TP_SCTS[10], pdu_message.TP_SCTS[11], pdu_message.TP_SCTS[12], pdu_message.TP_SCTS[13]);

			for (int i = 1; i <= UDH_cnt; i++)   //������� 
			{
				memset(&pdu_long[i].TP_UD, 0, sizeof(pdu_long));
			}
			printf("\n");
		}
	}
}


//���뺯��ok
void pdu_send()          //�����ж����ֱ��뷽ʽ�Ĺ���
{
	int CSMS_Length;  //�ж������û���Ϣ�ĳ���
	int back_num;
	int code_number = 0;
	int select = 0;
	int back = 0;  //����getchar()�ķ���ֵ
	srand((unsigned)time(NULL));      //��������Ӳ�����

	printf("\n\n\n\n");
	printf("��������Ҫ���͵ķ������ĺ���SCA  \
		    \n����8613800250500			 \
			\n");
	scanf_s("%s", pdu_message.In_SCA, 24);
	printf("������Է�����TPA                 \
		    \n����8613693092030               \
			\n");
	scanf_s("%s", pdu_message.In_TPA, 24);
	printf("���ͻ�������Ĭ�� PDU-Type= 11h;MR =0 ;���'91'\n");


	printf("ѡ��PDU�����ʽ 1 ���͸����ŷ������� 2 ģ����ŷ������Ĵ�ʱ���\n");
	scanf_s("%d", &encode_mode);
	if (encode_mode == 1)
	{
		printf("ѡ����뷽ʽ1\n");
	}
	else
	{
		printf("ѡ����뷽ʽ2\n");
	}

	printf("�������û���Ϣ���뷽ʽDCS: 7bit ����1 8bit ����2 Unicode ����3 \n");
	scanf_s("%d", &code_number);

	if (code_number == 1)       //7bit�ַ�153
	{
		printf("��������Ҫ���͵Ķ�������UD\n");                           //��ӳ������ű��빦��
		memset(pdu_message.In_TP_UD, 0, 1024);
		//scanf_s("%s", pdu_message.In_TP_UD,1024);             //scanf����������ո�
		back = getchar();
		gets_s(pdu_message.In_TP_UD, 1024);                    //gets_s��������ո�
		CSMS_Length = (int)strlen(pdu_message.In_TP_UD);             //����������û���Ϣ����

		back_num = special_find(pdu_message.In_TP_UD, CSMS_Length);          //���������ַ�����ok
		CSMS_Length += back_num;
		printf("������ַ�����Ϊ%d\n", CSMS_Length);                            //��ӡ������ַ�����

		extend_change(pdu_message.In_TP_UD, CSMS_Length,back_num);

		printf("%s", pdu_message.In_TP_UD);

		if (CSMS_Length > 160)  //�����������ֽ� �ַ�һ���ֽ�  ����70 Ӣ��140 
		{
			printf("�������Ϣ�������ƣ���ַ���\n");

			if (code_number == 1)              //������Ҫ�����ֱ���ֱ����
			{
				pdu_message.In_TP_DCS = 0x00;  //7bit����
				Cnt = (CSMS_Length % 153 == 0) ? (CSMS_Length / 153) : (CSMS_Length / 153 + 1);           //ȡ��Ƭ
				left = CSMS_Length % 153;   //����ֶ���֮�������
			}
			printf("��ʼ����PDU���ű���\n load ......\n");
			printf("���ɵ�PDU���ű���Ϊ��\n");

			if (encode_mode == 1 || encode_mode == 2)       //��Ч�ڵĽ���
			{
				RN = rand();                        //���ú����������һ���û���Ϣ�Ӷ��ű�ʶ /*	���� 0-255���û���ʶ*/
				for (encode_num = 0; encode_num < Cnt; encode_num++)
				{
					PDU_Encode_longmessage_NO2(PDU_message);   //��Ч�ڽ���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
		}

		else             //Ϊ������ͨ����
		{
			printf("����ǳ����ţ���ѡ������Ƿ���û�ͷ�� \
				\n 1 ��������  2���û�ͷ����	       \
				\n");
			scanf_s("%d", &select);

			if (code_number == 1)
			{
				pdu_message.In_TP_DCS = 0x00;
			}
			printf("��ʼ����PDU���ű���\n load ......\n");
			if (select == 1)   //�����û�ͷ
			{
				if (encode_mode == 1 || encode_mode == 2)  //����Чʱ��ı���  //ע�� �� ǰ��Ϊ����߽���ִ��
				{
					PDU_Encode(PDU_message);       //û���û���Ϣͷ�ı���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
			else if (select == 2)       //���û�ͷ
			{
				if (encode_mode == 1 || encode_mode == 2)  //����Чʱ��ı���
				{
					RN = rand();
					PDU_Encode_longmessage_NO1(PDU_message);       //���û�ͷ��û��ʱ���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
		}
	}
	else		 //Ϊ8bit��USC2�Ĳ���ж�
	{
		printf("��������Ҫ���͵Ķ�������UD\n");                           //��ӳ������ű��빦��
		memset(pdu_message.In_TP_UD, 0, 1024);
		//scanf_s("%s", pdu_message.In_TP_UD,1024);             //scanf����������ո�
		back = getchar();
		gets_s(pdu_message.In_TP_UD, 1024);                    //gets_s��������ո�
		CSMS_Length = (int)strlen(pdu_message.In_TP_UD);             //����������û���Ϣ����
		printf("������ַ�����Ϊ%d\n", CSMS_Length);                            //��ӡ������ַ�����

		if (CSMS_Length > 140)   //�����������ֽ� �ַ�һ���ֽ�  ����70 Ӣ��140 
		{
			printf("�������Ϣ�������ƣ���ַ���\n");

			if (code_number == 2)
			{
				pdu_message.In_TP_DCS = 0x04;  //8bit����

				Cnt = (CSMS_Length % 140 == 0) ? (CSMS_Length / 140) : (CSMS_Length / 140 + 1);           //ȡ��Ƭ
				left = CSMS_Length % 134;   //����ֶ���֮�������
			}
			else if (code_number == 3)
			{
				pdu_message.In_TP_DCS = 0x08;  //USC2����
				Cnt = (CSMS_Length % 140 == 0) ? (CSMS_Length / 140) : (CSMS_Length / 140 + 1);           //ȡ��Ƭ
				left = CSMS_Length % 134;   //����ֶ���֮�������
			}
			printf("��ʼ����PDU���ű���\n load ......\n");
			printf("���ɵ�PDU���ű���Ϊ��\n");
			if (encode_mode == 1 || encode_mode == 2)       //��Ч�ڵĽ���
			{
				RN = rand();                        //���ú����������һ���û���Ϣ�Ӷ��ű�ʶ /*	���� 0-255���û���ʶ*/
				for (encode_num = 0; encode_num < Cnt; encode_num++)
				{
					PDU_Encode_longmessage_NO2(PDU_message);   //��Ч�ڽ���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
		}
		else             //Ϊ������ͨ����
		{
			printf("����ǳ����ţ���ѡ������Ƿ���û�ͷ�� \
				\n 1 ��������  2���û�ͷ����	       \
				\n");
			scanf_s("%d", &select);

			if (code_number == 2)
			{
				pdu_message.In_TP_DCS = 0x04;
			}
			else if (code_number == 3)
			{
				pdu_message.In_TP_DCS = 0x08;
			}
			printf("��ʼ����PDU���ű���\n load ......\n");
			if (select == 1)   //�����û�ͷ
			{
				if (encode_mode == 1 || encode_mode == 2)  //����Чʱ��ı���  //ע�� �� ǰ��Ϊ����߽���ִ��
				{
					PDU_Encode(PDU_message);       //û���û���Ϣͷ�ı���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
			else if (select == 2)       //���û�ͷ
			{
				if (encode_mode == 1 || encode_mode == 2)  //����Чʱ��ı���
				{
					RN = rand();
					PDU_Encode_longmessage_NO1(PDU_message);       //���û�ͷ��û��ʱ���
					printf("%s\n", PDU_message);
					fprintf(out, "%s\n", PDU_message);
				}
			}
		}
	}
					printf("���������鿴txt !!\n");
					if (out != NULL)
					{
					fclose(out);
					}
}



