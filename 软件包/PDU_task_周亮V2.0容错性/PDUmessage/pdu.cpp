/***********************
PDU解码编码任务
实现的功能：1.7bit解码，8bit解码，Unicode中文解码
			2.7bit编码，8bit编码，Unicode 中文编码
			3.超长短信的编码拆分功能
			4.超长短信解码功能

目前： 编码方式分为两种：
		第一种是正常编码，编码短信有效时间用于发送给短信服务中心
		第二种是模拟编码，编码短信模拟短信服务中心的时间戳格式		 
************************/

#include "pdu_encode.h"
#include "pdu_decode.h"
#include "PDU.h"

struct pdu pdu_message = { 0 };     //定义结构体变量
struct pdu pdu_long[100] = { 0 };

char messageok[1024] = {0};   //读取文件缓冲区
char PDU_message[1024] = {0}; //输出信息缓冲区
char head_flag = 0;    //定义符合标识的标志位
char head_flag1 = 0;   //定义两位标识的标志位
char nUDI_flag = 0;    //定义无用户信息头标志位
char solo_flag = 0;    //定义子短信的结束标志
char Decode_flag = 0;
char Encode_flag = 0;
FILE *out = NULL;

//用于长短信的编码定义
//解码
unsigned char UDH_cnt;   //长短信总条数用于打印
int num = 0;		   //长短信遍历计数
//编码
int Cnt = 0;              //短信分片的数量
int left = 0;		     //处理余数
int ok = 0;				 //用于编码编号计数
int RN;					//用于产生的编码标识
//

void pdu_show();
void pdu_send();
void user();


int main()
{
	FILE *fp = NULL;

	errno_t errfp;
	errno_t errout;
	errfp = fopen_s(&fp,"message.txt","r");   //读取PDU数据串
	errout = fopen_s(&out,"PDUout.txt", "w");
	memset(&pdu_message,0,sizeof(struct pdu));
	memset(pdu_long,0,sizeof(struct pdu)*100);    //结构体数组初始化
	if (errfp != NULL)
	{
		printf("文件打开失败\n");
		system("pause");
		exit(0);
	}
	if (errout != NULL)
	{
		printf("文件输出失败\n");
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
				while (fgets(messageok, sizeof(messageok), fp))   //可以成功读取文件并打印 打印一条
				{
					strncpy_s(pdu_message.data, messageok, sizeof(messageok));
					if (PDU_Decode(pdu_message.data) != false)    //待解码的容错性判断
					{
						pdu_show();
					}
					system("pause");
				}
			}
		}
		if (Encode_flag == 1)   //普通短信编码成功
		{
			Encode_flag = 0;
			pdu_send();
		}
		system("pause");
		break;
	}

		if (fp != NULL)
		{
			fclose(fp);
		}
			return 0;
}



void user()
{
	int select = 0;
	printf("PDU短信编码解码  \
		   \n解码输入1   解码的PDU数据已存入    \
		   \n编码输入2   手动输入编码数据\n");
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
	
	if (nUDI_flag == 1)   //没有用户头的情况
	{
		printf("********************PDU短信解码TASK*********************\n");
		printf("等待解析的PDU数据为%s\n", messageok);
		printf("解析结果如下:\n");
		printf("服务中心号码为: +%s\n", pdu_message.SCA);
		printf("发送方手机号码: %s\n", pdu_message.TPA);
		printf("接收到的短信内容为:\n");

		for (int i = 0; i < pdu_message.TP_UDL; i++)
		{
			printf("%c", pdu_message.TP_UD[i]);
		}

		printf("\n");
		printf("短信时间：\n");
		printf("20%c%c-%c%c-%c%c  %c%c-%c%c-%c%c +%c%c时区:[>24时 -24]\n", pdu_message.TP_SCTS[0], pdu_message.TP_SCTS[1], pdu_message.TP_SCTS[2], pdu_message.TP_SCTS[3], pdu_message.TP_SCTS[4], pdu_message.TP_SCTS[5], pdu_message.TP_SCTS[6], pdu_message.TP_SCTS[7], pdu_message.TP_SCTS[8], pdu_message.TP_SCTS[9], pdu_message.TP_SCTS[10], pdu_message.TP_SCTS[11], pdu_message.TP_SCTS[12], pdu_message.TP_SCTS[13]);


	}

	if (solo_flag == 1)                      //有用户头单条的情况
	{

		printf("********************PDU短信解码TASK*********************\n");
		printf("等待解析的PDU数据为%s\n", messageok);
		printf("解析结果如下:\n");
		printf("服务中心号码为: +%s\n", pdu_message.SCA);
		printf("发送方手机号码: %s\n", pdu_message.TPA);
		printf("接收到的短信内容为:\n");

		if (pdu_message.TP_DCS == GSM_7BIT)   //OK
		{
			for (int i = (int)pdu_message.UDH_headlength + 2; i < pdu_message.TP_UDL; i++)  //输出7bit情况比较特殊
			{
				printf("%c", pdu_message.TP_UD[i]);
			}

		}
		else if(pdu_message.TP_DCS == GSM_UCS2) //中文和8bit
		{
			for (int i = 0 ; i < pdu_message.TP_UDL; i++)    //8bit和中文跳过头打印
			{
				printf("%c", pdu_message.TP_UD[i]);
			}
		}
		printf("\n");
		printf("短信时间：\n");
		printf("20%c%c-%c%c-%c%c  %c%c-%c%c-%c%c +%c%c时区:[>24时 -24]\n", pdu_message.TP_SCTS[0], pdu_message.TP_SCTS[1], pdu_message.TP_SCTS[2], pdu_message.TP_SCTS[3], pdu_message.TP_SCTS[4], pdu_message.TP_SCTS[5], pdu_message.TP_SCTS[6], pdu_message.TP_SCTS[7], pdu_message.TP_SCTS[8], pdu_message.TP_SCTS[9], pdu_message.TP_SCTS[10], pdu_message.TP_SCTS[11], pdu_message.TP_SCTS[12], pdu_message.TP_SCTS[13]);

		printf("用户数据头为: 0x 0x xx xx xx 格式\n");


		if (pdu_message.UDH_headlength == 0x05)                        //打印6个用户数据长度
		{

			for (int i = 0; i<(int)pdu_message.UDH_headlength + 1; i++)
			{
				printf("%x ", pdu_message.UDH_head[i]);
			}
			printf("\n");

			printf("用户信息长度length=%d\n", pdu_message.TP_UDL);

		}
		else if (pdu_message.UDH_headlength == 0x06)                    //打印7个用户数据长度
		{

			for (int i = 0; i<(int)pdu_message.UDH_headlength + 1; i++)
			{
				printf("%x ", pdu_message.UDH_head1[i]);
			}
			printf("\n");

			printf("用户信息长度length=%d\n", pdu_message.TP_UDL);

		}
	}

	if (head_flag == 1)        //标识匹配继续输出之后的短消息
	{
		if (num == UDH_cnt)           // 这里需要隔离开长短信之前的短信分片问题
		{
			printf("此条长短信的内容为：\n");
			for (int i = 1; i <= UDH_cnt; i++)
			{
				printf("%s", pdu_long[i].TP_UD);
			}

			for (int i = 1; i <= UDH_cnt; i++)   //清空数组 
			{
				memset(&pdu_long[i].TP_UD, 0, sizeof(pdu_long));
			}
			printf("\n");
		}

	}

	if (head_flag1 == 1)
	{
		if (num == UDH_cnt)
		{
			printf("此条长短信的内容为：\n");
			for (int i = 1; i <= UDH_cnt; i++)
			{
				printf("%s", pdu_long[i].TP_UD);
			}
			for (int i = 1; i <= UDH_cnt; i++)  //清空数组
			{
				memset(&pdu_long[i].TP_UD, 0, sizeof(pdu_long));
			}
			printf("\n");
		}

	}

}


//编码函数ok
void pdu_send()          //增加判断两种编码方式的功能
{
	int CSMS_Length;  //判断输入用户信息的长度
	int code_number = 0;
	int select = 0;
	int mode = 0;  //用于选择编码的PDU格式
	int back = 0;  //用于getchar()的返回值
	srand((unsigned)time(NULL));      //随机数种子产生器

	printf("\n\n\n\n");
	printf("请输入需要发送的服务中心号码SCA  \
		    \n例：8613800250500			 \
			\n");
	scanf_s("%s", pdu_message.In_SCA,24);
	printf("请输入对方号码TPA                 \
		    \n例：8613693092030               \
			\n");
	scanf_s("%s", pdu_message.In_TPA,24 );
	printf("发送基本参数默认 PDU-Type= 11h;MR =0 ;添加'91'\n");   


	printf("选择PDU编码格式 1 发送给短信服务中心 2 模拟短信服务中心带时间戳\n");
	scanf_s("%d", &mode);
	if (mode == 1)
	{
		printf("选择编码方式1\n");
	}
	else
	{
		printf("选择编码方式2\n");
	}

	printf("请输入需要发送的短信内容UD\n");                           //添加超长短信编码功能
	
	memset(pdu_message.In_TP_UD,0,1024);
	//scanf_s("%s", pdu_message.In_TP_UD,1024);             //scanf不可以输入空格
	back = getchar();
	gets_s(pdu_message.In_TP_UD,1024);                    //gets_s可以输入空格

	CSMS_Length = (int)strlen(pdu_message.In_TP_UD);             //返回输入的用户信息长度
	printf("输入的字符个数为%d\n", CSMS_Length);                            //打印输入的字符个数

	if (CSMS_Length > 140)  //汉字是两个字节 字符一个字节  中文70 英文140 
	{
		printf("输入的信息超过限制，拆分发送\n");

		printf("请输入用户信息编码方式DCS: 7bit 输入1 8bit 输入2 Unicode 输入3 \n");
		scanf_s("%d", &code_number);

		if (code_number == 1)              //这里需要对三种编码分别归类
		{
			pdu_message.In_TP_DCS = 0x00;  //7bit编码

			Cnt = (CSMS_Length % 153 == 0) ? (CSMS_Length / 153) : (CSMS_Length / 153 + 1);           //取分片

			left = CSMS_Length % 153;   //处理分短信之后的余数

		}
		else if (code_number == 2)
		{

			pdu_message.In_TP_DCS = 0x04;  //8bit编码

			Cnt = (CSMS_Length % 140 == 0) ? (CSMS_Length / 140) : (CSMS_Length / 140 + 1);           //取分片
			left = CSMS_Length % 134;   //处理分短信之后的余数
		}
		else if (code_number == 3)
		{
			pdu_message.In_TP_DCS = 0x08;  //USC2编码
			Cnt = (CSMS_Length % 140 == 0) ? (CSMS_Length / 140) : (CSMS_Length / 140 + 1);           //取分片
			
			left = CSMS_Length % 134;   //处理分短信之后的余数
		}
		printf("开始生成PDU短信编码\n load ......\n");
		printf("生成的PDU短信编码为：\n");

		if (mode == 1)       //有效期的解码
		{
			RN = rand();                        //调用函数随机生成一个用户信息子短信标识 /*	产生 0-255的用户标识*/
			for (ok = 0; ok < Cnt; ok++)
			{

				PDU_Encode_longmessage_NO2(PDU_message);   //有效期解码
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);

			}
		}

		else      //带时间戳的编码  mode=2
		{
			RN = rand();                        //调用函数随机生成一个用户信息子短信标识 /*	产生 0-255的用户标识*/
			for (ok = 0; ok < Cnt; ok++)
			{

				PDU_Encode_longmessage_NO2time(PDU_message);  //时间戳解码
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);

			}
		
		
		}
		printf("输出编码请查看txt !!\n");

		if (out != NULL)
		{
			fclose(out);
		}
	}

	else             //为正常普通短信
	{
		printf("编码非长短信，请选择编码是否带用户头： \
				\n 1 正常短信  2带用户头短信	       \
				\n");
		scanf_s("%d", &select);

		printf("请输入用户信息编码方式DCS: 7bit 输入1 8bit 输入2 Unicode 输入3 \n");
		scanf_s("%d", &code_number);

		if (code_number == 1)
		{
			pdu_message.In_TP_DCS = 0x00;
		}
		else if (code_number == 2)
		{
			pdu_message.In_TP_DCS = 0x04;
		}
		else if (code_number == 3)
		{
			pdu_message.In_TP_DCS = 0x08;
		}
		printf("发送时间默认为30天\n");
		printf("开始生成PDU短信编码\n load ......\n");


		if (select == 1)   //不带用户头
		{
			if (mode == 1)  //带有效时间的编码
			{
				PDU_Encode(PDU_message);       //没有用户信息头的编码
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);


			}

			else  //带时间戳的编码
			{

				PDU_Encode_time(PDU_message);                      //不带用户头带时间戳的编码
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);
			}

		}
		else if( select == 2)       //带用户头
		{
			if (mode == 1)  //带有效时间的编码
			{
				
				RN = rand();
				PDU_Encode_longmessage_NO1(PDU_message);       //带用户头，没有时间戳
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);

			}

			else  //带时间戳的编码
			{
				RN = rand();
				PDU_Encode_longmessage_NO1time(PDU_message);//添加带时间戳的独立编码函数
				printf("%s\n", PDU_message);
				fprintf(out, "%s\n", PDU_message);
			}

		}
		printf("输出编码请查看txt !!\n");
		if (out != NULL)
		{
			fclose(out);
		}
	}

}




