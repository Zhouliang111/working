#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//��������
char DH_id_sep[32];  //ȫ�ֱ������飬��ദ��32����������

//����ɸѡ��־
char pos_flag=0;
char speed_flag=0;
char satsum_flag=0;
char signal_flag=0;
//���ɸѡ��
char multi_flag=0;

int GGA_num=0;
int GSA_num=0;
int GSV_num=0;
int RMC_num=0;
int VTG_num=0;

//����ɸѡ��������
double in_speed;  //������ٶ�����
int in_pos;       //���붨λ����
double speed_max; //�����ٶ����ֵ
double speed_min; //�����ٶ���Сֵ
int in_sum;      //������������
int in_signal;    //��������ǿ��

double Latitude;  //����γ�����
double Longitude; //���ھ������

 

FILE *out;
char stream[256];       //�м�ת��buffer


struct GPS_Real_buf      //����GPS���뻺����
{
	char data[256];
}GPS_buffer;   //GPS�������ݻ�����


//��Ҫ��GPS������
//��� ������ʱ���� ���� ά�� ��λ״̬ �ٶ� ���붨λ�����ǿ��� �����ź�ǿ�ȡ�
struct GPS_Information
{
	//GPGGA
		char UTC_Time[7];             //ʱ��
		char UTC_Date[7];             //����
		char Latitude[10];            //γ��
		char NS_Indicator;            //������
		char Longitude[11];           //����
		char EW_Indicator;            //����
		int GPS_status;			//gps״̬
		int Use_EPH_Sum;    //ʹ�õ���������
	//GPGSA
		char Locate_mode;   //��λģʽ  2D��3D
		char User_EPH[12];   //��ǰ�����������Ǳ��
	//GPGSV
	struct GSV//���Ǳ�ź�����ǿ��
	{
		int sat_num;
		int sat_signal;
	}GPS[12];
	//GPRMC
		double Speed;                 //�ٶ�
		double Course;                //����
	//GPVTG
		double N_speed;				//�ٶȣ��ڣ�Knots
		double K_speed;				//ˮƽ�˶��ٶȣ�0.00����ǰ��λ��������0��
}GPS_Information;//����GPS����ṹ��


void Creat_DH_Index(char *buffer); //������������
char *Real_Process_DH( char* buffer, char num ); //ƫ�ƻ�ȡ���ݺ���
int GPS_checksum(char *buffer);  //  ����֡У�麯��
void GPS_check();            //GPS��������
void GPS_select();           //ɸѡ����
void GPS_show();             //�������


int main()
{
	
		//��������
		int error=0; 
		int data_check=0;
		int check_return=0;
		char check[256];

	//*******���ļ�************
		FILE *fp;
		fp = fopen("NMEA.txt","r");
		out = fopen("out.txt","w");
		if(fp == NULL)
	{
		printf("�ļ���ʧ��\n");
		system("pause");
		exit(0);
	}
		GPS_select();            //�û�����ɸѡ


	/*****************��ѭ������GPS�ṹ��ĸ�ֵ***********************/

		while(fgets(stream,sizeof(stream),fp))
	{
			//����У��
			check[0] = '\0';
			sscanf(stream,"%*[^*]*%s",check);   //������sscanf������ȡ����֡��*����֮���У��ֵ 
			sscanf(check,"%x",&data_check);		//�ɹ���У���뱣�浽16���Ƶ�data_check����
			check_return= GPS_checksum(stream);		//���Է���16����У����
			
			if(data_check == check_return) //����У��ɹ�
		{
			strncpy(GPS_buffer.data,stream,sizeof(stream));   //�Ž�����װ��GPS��buff
			Creat_DH_Index(GPS_buffer.data);   //�������
			GPS_check();					   //GPS���ݽ���			
			GPS_show();		                   //�������
			
		}
			else
			{
				error++;
			}

	}

	printf("��������%d\n",error);

	//*****************�ر��ļ�***********************
	fclose(fp);
	fclose(out);
	system("pause");
	return 0;
}



//Ѱ�����ж��ŵ�λ��
//����GPS���ݵĻ�����
//����ȫ�ֱ��������еĶ���������ԭGPS�����еĶ��Ž��ᱻ0x00�����
void Creat_DH_Index(char *buffer)
{
	int i,len;
	int idj;
	memset(DH_id_sep,0,sizeof(DH_id_sep));
	len = strlen(buffer);
	for(i=0,idj=0;i<len;i++)
	{
		if(buffer[i] == ',')
		{
			DH_id_sep[idj] = i;
			idj++;
			buffer[i] = 0x00;
		}
	
	}
}



//����GPS���ݵĵ�N��������ƫ��
//����������Ľ���GPS���ݻ�����
//���ص�N��������֮�����Ϣ��
char *Real_Process_DH( char* buffer, char num )
{
	if ( num < 1 )
	return  &buffer[0];
	return  &buffer[ DH_id_sep[num - 1] + 1];
}




//GPS����У��
/*������䣺 $ GPZDA,082710.00,16,09,2002,00,00 *64��У��ͣ���ɫ���ֲ�����㣩��
�㷽��Ϊ��
0X47 xor 0X50 xor 0X5A xor 0X44 xor 0X41 xor 0X2C xor 0X30 xor 0X38 xor 0X32 xor
0X37 xor 0X31 xor 0X30 xor 0X2E xor 0X30 xor 0X30 xor 0X2C xor 0X31 xor 0X36 xor 0X2C
xor 0X30 xor 0X39 xor 0X2C xor 0X32 xor 0X30 xor 0X30 xor 0X32 xor 0X2C xor 0X30 xor
0X30 xor 0X2C xor 0X30 xor 0X30
�õ��Ľ������ 0X64���� ASCII ��ʾ���� 64
*/

//�����㺯��
int GPS_checksum(char *buffer)  //  ����һ��buffer
{
	int i,result;
	for(result=buffer[1],i=2;buffer[i]!='*';i++)  
	{
		result^=buffer[i];
	
	}
	return result;
	
}


void GPS_check()
{
	char *temp;

//************************��������**************************************************
		if(strstr(GPS_buffer.data,"GGA"))   //GPGGA
{
			
				temp=Real_Process_DH(GPS_buffer.data,1);
				memcpy(GPS_Information.UTC_Time,temp,6);	//ʱ��
				temp=Real_Process_DH(GPS_buffer.data,2);
				memcpy(GPS_Information.Latitude,temp,10);	//γ��

				sscanf(GPS_Information.Latitude,"%lf",&Latitude);

				GPS_Information.NS_Indicator=*(Real_Process_DH(GPS_buffer.data,3));	//�ϱ�γ
				temp=Real_Process_DH(GPS_buffer.data,4);
				memcpy(GPS_Information.Longitude,temp,11);  //����
			
				sscanf(GPS_Information.Longitude, "%lf", &Longitude);

				GPS_Information.EW_Indicator=*(Real_Process_DH(GPS_buffer.data,5));	//������
				GPS_Information.Use_EPH_Sum=atoi(Real_Process_DH(GPS_buffer.data,7));	//���붨λ������

//************************************************************
		if(pos_flag == 1)    //�ж�ɸѡ����Ϊ��λ��־    
		{
			GGA_num++;
			printf("*****************************************************\n��ţ�%d\n",GGA_num);  //���
			printf("%s\n",stream);         
			//�ļ���ӡ
			fprintf(out,"*****************************************************\n��ţ�%d\n",GGA_num);  //���
			fprintf(out,"%s\n",stream);       

			if(GPS_Information.GPS_status != '0' )
			{
			printf("��λ״̬=%c  �����Ѷ�λ\n",GPS_Information.GPS_status);

			fprintf(out,"��λ״̬=%c  �����Ѷ�λ\n",GPS_Information.GPS_status);
			}
			else if(GPS_Information.GPS_status == '0')
			{
			printf("��λ״̬=%c  ����δ��λ\n",GPS_Information.GPS_status);
			fprintf(out,"��λ״̬=%c  ����δ��λ\n",GPS_Information.GPS_status);
			}
			
			printf("\n");
			printf("*****************************************************\n");

			fprintf(out,"\n");
			fprintf(out,"*****************************************************\n");
		}

		if(satsum_flag == 1)  //����ʹ�õ����ǿ�����־λ
		{
				GGA_num++;
				printf("*****************************************************\n��ţ�%d\n",GGA_num);  //���
				printf("%s\n",stream);         

				fprintf(out,"*****************************************************\n��ţ�%d\n",GGA_num);  //���
				fprintf(out,"%s\n",stream);     


				printf("����ʹ�õ���������Ϊ%d��",GPS_Information.Use_EPH_Sum );
			
				printf("\n");
				printf("*****************************************************\n");


				fprintf(out,"����ʹ�õ���������Ϊ%d��",GPS_Information.Use_EPH_Sum );
			
				fprintf(out,"\n");
				fprintf(out,"*****************************************************\n");	
				
		}

}	
		
//***********************************************************************************

			if(strstr(GPS_buffer.data,"GSA")) //GPGSA
{

			temp = Real_Process_DH( GPS_buffer.data, 2 ); //��2������Ϊ��λģʽ
			if((*temp == '2')||(*temp == '3'))
			{	
				GPS_Information.Locate_mode =*temp;
			}
			else 
					GPS_Information.Locate_mode ='1';  //δ��λ
			
			for(int i=0;i<12;i++)
			{
				 GPS_Information.User_EPH[i] = atoi( Real_Process_DH( GPS_buffer.data, i + 3 ) ); //�ӵ�3��������ʼΪ��ʹ�õ����Ǳ��			
				
			}

//****************************************************************   FINSIH

			if(pos_flag == 1)    //�ж�ɸѡ����Ϊ��λ��־               
		{
				GSA_num++;
				printf("*****************************************************\n��ţ�%d\n",GSA_num);  
				printf("%s\n",stream);         //��ӡ����ǰ���

				fprintf(out,"*****************************************************\n��ţ�%d\n",GSA_num); 
				fprintf(out,"%s\n",stream);         //��ӡ����ǰ���

			if(GPS_Information.Locate_mode == '2')
			{
			printf("��λģʽ: %cD��λ",GPS_Information.Locate_mode);

			fprintf(out,"��λģʽ: %cD��λ",GPS_Information.Locate_mode);

			}
			else if(GPS_Information.Locate_mode == '3')
			{
			printf("��λģʽ: %cD��λ",GPS_Information.Locate_mode);

			fprintf(out,"��λģʽ: %cD��λ",GPS_Information.Locate_mode);
			}
			else 
			{
			printf("��λģʽ=%c  ����δ��λ\n",GPS_Information.Locate_mode);

			fprintf(out,"��λģʽ=%c  ����δ��λ\n",GPS_Information.Locate_mode);
			}
			
			printf("\n");
			printf("*****************************************************\n");

			fprintf(out,"\n");
			fprintf(out,"*****************************************************\n");

		}

}	

//******************************************************************************8
			if(strstr(GPS_buffer.data,"RMC"))  //GPRMC       
{
				temp=Real_Process_DH(GPS_buffer.data,1);  //��ȡRMC-UTCʱ��
				if(*temp != 0)
				
				memcpy(GPS_Information.UTC_Time,temp,6);

			if ( *( Real_Process_DH( GPS_buffer.data, 2 ) ) == 'A' ) //��ȡ��λ״̬ 
			{
				GPS_Information.GPS_status =1;   //1 ��λ 
			
			}
			else
			{
				GPS_Information.GPS_status =0;   //0 δ��λ  
			
			}

				temp = Real_Process_DH( GPS_buffer.data, 3 ); //��3������Ϊγ��

			if ( ( *temp >= 0x31 ) && ( *temp <= 0x39 ) )
			{
				memcpy( GPS_Information.Latitude, temp, 9 );
				GPS_Information.Latitude[9] = 0;
			
			}
			else
			{
				GPS_Information.Latitude[0] = '0';
				GPS_Information.Latitude[1] = 0;
			
			}
			GPS_Information.NS_Indicator = *( Real_Process_DH( GPS_buffer.data, 4 ) ); //��4������Ϊ�ϱ�

			temp = Real_Process_DH( GPS_buffer.data, 5); //��5������Ϊ����

			if ( ( *temp >= 0x31 ) && ( *temp <= 0x39 ) )
			{
				memcpy( GPS_Information.Longitude, temp, 10 );
				GPS_Information.Longitude[10] = 0;
			
			}
			else
			{
				GPS_Information.Longitude[0] = '0';
				GPS_Information.Longitude[1] = 0;
			
			}
				GPS_Information.EW_Indicator = *( Real_Process_DH( GPS_buffer.data, 6 ) ); //��6������Ϊ����
				GPS_Information.Speed = atof( Real_Process_DH( GPS_buffer.data, 7 ) ); //��7������Ϊ�ٶ�
				GPS_Information.Course = atof( Real_Process_DH( GPS_buffer.data, 8 ) ); //��8������Ϊ����
				temp = Real_Process_DH( GPS_buffer.data, 9 ); //��9������Ϊ����

			if ( *temp != 0 )
	    {
			memcpy( GPS_Information.UTC_Date, temp, 6 );
	    }

}

//*********GSV**********************************
		if ( strstr( GPS_buffer.data, "GSV" ) )//$GPGSV,3,1,11,28,73,321,32,17,39,289,43,11,38,053,17,09,37,250,41*78
{

				for(int m2 = 0; m2<12 ; m2++)  //һ������12����
				{
					if( GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 16 ) ) )   //���ĸ����Ǳ��
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (19) ) );   //���ĸ�����ǿ��
					}

 					if( GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 12 ) ) )   //���������Ǳ��
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (15) ) );   //����������ǿ��
					}

					if(  GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 8 ) )	)  //�ڶ������Ǳ��
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (11) ) );    //�ڶ�������ǿ��
					}

					if(  GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 4 ) ) )   //��һ�����Ǳ��
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (7) ) );   //��һ������ǿ��
					}

				}

}

		if(strstr(GPS_buffer.data,"GLL")) //��λ������Ϣ
		{
			
				temp=Real_Process_DH(GPS_buffer.data,1);
				memcpy(GPS_Information.Latitude,temp,10);  //γ��

				sscanf(GPS_Information.Latitude, "%lf", &Latitude);

				GPS_Information.NS_Indicator=*(Real_Process_DH(GPS_buffer.data,2));//�ϱ�γ
				temp=Real_Process_DH(GPS_buffer.data,3);
				memcpy(GPS_Information.Longitude,temp,11);  //����

				sscanf(GPS_Information.Latitude, "%lf", &Latitude);

				GPS_Information.EW_Indicator=*(Real_Process_DH(GPS_buffer.data,4));//������
				temp=Real_Process_DH(GPS_buffer.data,5);
				memcpy(GPS_Information.UTC_Time,temp,6);//��ȡUTC��ʱ��

				
		}


//******************************************************************************************88
		if(strstr(GPS_buffer.data,"VTG"))
{
			
			GPS_Information.N_speed = atof( Real_Process_DH( GPS_buffer.data, 5 ) ); //�������� ��λ����
			GPS_Information.K_speed = atof( Real_Process_DH( GPS_buffer.data, 7 ) ); //�������� ��λ��KM/H


			if(speed_flag == 1)    //�ж�ɸѡ����Ϊ�ٶȱ�־         
			{ 

			if(GPS_Information.K_speed >= in_speed)
				{
					VTG_num++;
					printf("*****************************************************\n��ţ�%d\n",VTG_num);  //���
					printf("%s\n",stream);        
					fprintf(out,"*****************************************************\n��ţ�%d\n",VTG_num);  //���
					fprintf(out,"%s\n",stream);        

					printf("�ٶ�Ϊ%3.3f ��\n",GPS_Information.N_speed);
					printf("�ٶ�Ϊ%3.3f km/h\n",GPS_Information.K_speed);
					fprintf(out,"�ٶ�Ϊ%3.3f ��\n",GPS_Information.N_speed);
					fprintf(out,"�ٶ�Ϊ%3.3f km/h\n",GPS_Information.K_speed);
				}
			}

						
}
		
}

void GPS_select()
{
	int flag1;

	int access;

//******************************ɸѡ����***********************************
	printf("/******************************************************/\n");
	printf(" NMEA-0183����\n");
	printf("\n");
	printf("1. ������ɸѡ����\n");
	printf("2. ��ѡ����ɸѡ�����ɸѡ 1 ����ɸѡ 2 ���ɸѡ\n");
	printf("\n");
	printf("3. ���ɸѡ��ʽ: ��� ������ʱ����\n  ");
	printf(" ���� γ�� ��λ״̬ �ٶ�\n");
	printf("   ���붨λ�����ǿ��� �����ź�ǿ�� \n");
	printf("\n");
	printf("��ȷ�ϵ���ɸѡ�����ɸѡ\n");
	printf("���� 1 ��� 2 \n");
	//*************************************************************************
	scanf("%d", &access);

		if(access == 1)
	{
		printf("ѡ����ɸѡ���������Ӧ��ţ�\n");
		printf("1 �Ƿ�λ 2 �ٶ�ֵ�޶�ֵ  3 ���붨λ�����ǿ��� 4 �����ź�ǿ�� \n");
		
		scanf("%d", &flag1);
		
		if(flag1 == 1)         //��λ��Ϣɸѡ 
		{
			printf("��λ��Ϣ���£�\n");

			fprintf(out,"��λ��Ϣ���£�\n");

			pos_flag=1;        
		}
	

		else if(flag1 == 2 )  //�ٶ���Ϣɸѡ
		{
			printf("�����޶����ٶ�ֵ 0000.0 -1851.8( km/h )\n");
			scanf("%lf",&in_speed);
			speed_flag=1;   //okok
		
		}


		else if(flag1 == 3)   //���붨λ���ǿ���ɸѡ
		{
			printf("���붨λ�����ǿ�����Ϣ����: \n");

			fprintf(out,"���붨λ�����ǿ�����Ϣ����: \n");
			satsum_flag=1;     

		}

		else if(flag1 == 4)  //�����ź�ǿ��ɸѡ
		{
			printf("�����ź�ǿ����Ϣ���£�\n");

			fprintf(out,"�����ź�ǿ����Ϣ���£�\n");
			signal_flag=1;   
		}

	
	}


	/*****************���ɸѡ**************************/
	if (access == 2)          //ɸѡ�������� 
	{
	
		printf("ɸѡ�Ƿ�λ������ 1 (A ��λ)/ 0 ( Vδ��λ)\n");  
		scanf("%d",&in_pos);
		printf("%d\n",in_pos);

	
		printf("ɸѡ�ٶȣ������ٶ�������ֵ (0000.0 - 1851.8) km/h \n");  
		printf("�������ٶ�����ֵ (km/h)\n");
		scanf("%lf",&speed_max);
		printf("�ٶ�����ֵΪ %lf (km/h)\n",speed_max);

		printf("�������ٶ�����ֵ (km/h)\n");
		scanf("%lf",&speed_min);
		printf("�ٶ�����ֵΪ %lf  (km/h) \n",speed_min);

		printf("ɸѡ���Ƕ�λ�������������Ƕ�λ������Сֵ:\n");   
		scanf("%d",&in_sum);
		printf("��λ������СֵΪ %d \n",in_sum);

	
		printf("ɸѡ����ǿ�ȣ���������ǿ�ȴ�С��Сֵ:\n");  
		scanf("%d",&in_signal);
		printf("����ǿ����СֵΪ %d \n",in_signal);

		multi_flag=1;   // ������Ҫ����������ݵ��ж����
	}
}


void GPS_show()//�������
{	
	int GSVok_flag=0;
	int GSVcheck_flag=0;
	char endess;

		
			if((multi_flag == 1))                //���ɸѡ��־
		{
					GSVok_flag = 0;
				if(endess=strstr(GPS_buffer.data,"GNGLL")!=NULL)
				{
					GSVok_flag = 1;	
				}
					GSVcheck_flag=0;
				for(int m=0;m<12;m++)
				{
				if( (GPS_Information.GPS[m].sat_signal >= in_signal))
					{		
								GSVcheck_flag=1;
					}
				}	
					
	if	(  (GPS_Information.K_speed < speed_max) && (GPS_Information.K_speed >speed_min) && (GPS_Information.GPS_status == in_pos) && (GPS_Information.Use_EPH_Sum > in_sum)
			&&(GSVok_flag == 1)&&GSVcheck_flag==1) //�ж����������ϲŽ������
		{


				RMC_num++;
				printf("*****************************************************\n��ţ�%d\n",RMC_num);  //���
				fprintf(out,"*****************************************************\n��ţ�%d\n",RMC_num);  //���
				printf("���ڣ�20%c%c��%c%c��%c%c��",GPS_Information.UTC_Date[4],GPS_Information.UTC_Date[5],GPS_Information.UTC_Date[2],
				GPS_Information.UTC_Date[3],GPS_Information.UTC_Date[0],GPS_Information.UTC_Date[1]);
				printf("%c%cʱ%c%c��%c%c�� \n",GPS_Information.UTC_Time[0],GPS_Information.UTC_Time[1],GPS_Information.UTC_Time[2],
				GPS_Information.UTC_Time[3],GPS_Information.UTC_Time[4],GPS_Information.UTC_Time[5]);	//������ʱ����

				fprintf(out,"���ڣ�20%c%c��%c%c��%c%c��",GPS_Information.UTC_Date[4],GPS_Information.UTC_Date[5],GPS_Information.UTC_Date[2],
				GPS_Information.UTC_Date[3],GPS_Information.UTC_Date[0],GPS_Information.UTC_Date[1]);
				fprintf(out,"%c%cʱ%c%c��%c%c�� \n",GPS_Information.UTC_Time[0],GPS_Information.UTC_Time[1],GPS_Information.UTC_Time[2],
				GPS_Information.UTC_Time[3],GPS_Information.UTC_Time[4],GPS_Information.UTC_Time[5]);	//������ʱ����

				if(GPS_Information.EW_Indicator=='E')
			{
					printf("������");
					fprintf(out,"������");
			}
				else
			{
					printf("������");
					fprintf(out,"������");
			}
			printf("%5.5f\n",Longitude);//����
			fprintf(out,"%5.5f\n",Longitude);//����
				if(GPS_Information.NS_Indicator=='N')
			{
				printf("��γ��");
				fprintf(out,"��γ��");
			}
				else
			{
				printf("��γ��");

				fprintf(out,"��γ��");
			}	
				
				printf("%5.5f\n",Latitude);//γ��
				fprintf(out,"%5.5f\n",Latitude);//γ��

			if(GPS_Information.GPS_status== 0 )
			{
				printf("GPSδ��λ \n");//��λ״̬

				fprintf(out,"GPSδ��λ \n");//��λ״̬
			}
				else if(GPS_Information.GPS_status== 1 )
			{
				printf("GPS�Ѷ�λ \n");

				fprintf(out,"GPS�Ѷ�λ \n");
			}

				printf("ʹ�õ�������%d\n",GPS_Information.Use_EPH_Sum);//�������ǿ���
				fprintf(out,"ʹ�õ�������%d\n",GPS_Information.Use_EPH_Sum);//�������ǿ���

	
				printf("�����ٶȣ�%3.3f�� \n",GPS_Information.N_speed);//�ٶ�
				printf("�����ٶȣ�%3.3fkm/h \n",GPS_Information.K_speed);//�ٶ�
				fprintf(out,"�����ٶȣ�%3.3f�� \n",GPS_Information.N_speed);//�ٶ�
				fprintf(out,"�����ٶȣ�%3.3fkm/h \n",GPS_Information.K_speed);//�ٶ�
							
				for(int m1=0;m1 <12; m1++)
				{	
					 if( 0 == GPS_Information.User_EPH[m1])
					{	
							continue;
					}
					if( (GPS_Information.GPS[m1].sat_signal >= in_signal ))
					{	

							printf("��%d��������������Ϊ: PRN=%d SNR=%d \n",m1+1,GPS_Information.User_EPH[m1],GPS_Information.GPS[m1].sat_signal);
							fprintf(out,"��%d��������������Ϊ: PRN=%d SNR=%d \n",m1+1,GPS_Information.User_EPH[m1],GPS_Information.GPS[m1].sat_signal);	
					}			

				}

			}
		}

			
		
}






