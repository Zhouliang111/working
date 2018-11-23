#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//变量定义
char DH_id_sep[32];  //全局变量数组，最多处理32个逗号索引

//单项筛选标志
char pos_flag=0;
char speed_flag=0;
char satsum_flag=0;
char signal_flag=0;
//组合筛选标
char multi_flag=0;

int GGA_num=0;
int GSA_num=0;
int GSV_num=0;
int RMC_num=0;
int VTG_num=0;

//输入筛选变量定义
double in_speed;  //输入的速度数据
int in_pos;       //输入定位数据
double speed_max; //输入速度最大值
double speed_min; //输入速度最小值
int in_sum;      //输入卫星数量
int in_signal;    //输入卫星强度

double Latitude;  //用于纬度输出
double Longitude; //用于经度输出

 

FILE *out;
char stream[256];       //中间转换buffer


struct GPS_Real_buf      //定义GPS输入缓冲区
{
	char data[256];
}GPS_buffer;   //GPS接收数据缓冲区


//需要的GPS变量：
//序号 年月日时分秒 经度 维度 定位状态 速度 参与定位的卫星颗数 卫星信号强度。
struct GPS_Information
{
	//GPGGA
		char UTC_Time[7];             //时间
		char UTC_Date[7];             //日期
		char Latitude[10];            //纬度
		char NS_Indicator;            //北半球
		char Longitude[11];           //经度
		char EW_Indicator;            //东经
		int GPS_status;			//gps状态
		int Use_EPH_Sum;    //使用的卫星数量
	//GPGSA
		char Locate_mode;   //定位模式  2D或3D
		char User_EPH[12];   //当前搜索到的卫星编号
	//GPGSV
	struct GSV//卫星编号和卫星强度
	{
		int sat_num;
		int sat_signal;
	}GPS[12];
	//GPRMC
		double Speed;                 //速度
		double Course;                //航向
	//GPVTG
		double N_speed;				//速度，节，Knots
		double K_speed;				//水平运动速度（0.00）（前导位数不足则补0）
}GPS_Information;//定义GPS输出结构体


void Creat_DH_Index(char *buffer); //创建索引函数
char *Real_Process_DH( char* buffer, char num ); //偏移获取数据函数
int GPS_checksum(char *buffer);  //  数据帧校验函数
void GPS_check();            //GPS解析函数
void GPS_select();           //筛选函数
void GPS_show();             //输出函数


int main()
{
	
		//变量声明
		int error=0; 
		int data_check=0;
		int check_return=0;
		char check[256];

	//*******打开文件************
		FILE *fp;
		fp = fopen("NMEA.txt","r");
		out = fopen("out.txt","w");
		if(fp == NULL)
	{
		printf("文件打开失败\n");
		system("pause");
		exit(0);
	}
		GPS_select();            //用户条件筛选


	/*****************主循环用于GPS结构体的赋值***********************/

		while(fgets(stream,sizeof(stream),fp))
	{
			//数据校验
			check[0] = '\0';
			sscanf(stream,"%*[^*]*%s",check);   //调用了sscanf函数提取数据帧‘*’号之后的校验值 
			sscanf(check,"%x",&data_check);		//成功将校验码保存到16进制的data_check变量
			check_return= GPS_checksum(stream);		//可以返回16进制校验结果
			
			if(data_check == check_return) //数据校验成功
		{
			strncpy(GPS_buffer.data,stream,sizeof(stream));   //才将数据装入GPS的buff
			Creat_DH_Index(GPS_buffer.data);   //添加索引
			GPS_check();					   //GPS数据解析			
			GPS_show();		                   //数据输出
			
		}
			else
			{
				error++;
			}

	}

	printf("错误数据%d\n",error);

	//*****************关闭文件***********************
	fclose(fp);
	fclose(out);
	system("pause");
	return 0;
}



//寻找所有逗号的位置
//输入GPS数据的缓冲区
//创建全局变量数组中的逗号索引，原GPS数据中的逗号将会被0x00替代。
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



//查找GPS数据的第N个参数的偏移
//创建索引后的接收GPS数据缓冲区
//返回第N个“，”之后的信息。
char *Real_Process_DH( char* buffer, char num )
{
	if ( num < 1 )
	return  &buffer[0];
	return  &buffer[ DH_id_sep[num - 1] + 1];
}




//GPS数据校验
/*例如语句： $ GPZDA,082710.00,16,09,2002,00,00 *64，校验和（红色部分参与计算）计
算方法为：
0X47 xor 0X50 xor 0X5A xor 0X44 xor 0X41 xor 0X2C xor 0X30 xor 0X38 xor 0X32 xor
0X37 xor 0X31 xor 0X30 xor 0X2E xor 0X30 xor 0X30 xor 0X2C xor 0X31 xor 0X36 xor 0X2C
xor 0X30 xor 0X39 xor 0X2C xor 0X32 xor 0X30 xor 0X30 xor 0X32 xor 0X2C xor 0X30 xor
0X30 xor 0X2C xor 0X30 xor 0X30
得到的结果就是 0X64，用 ASCII 表示就是 64
*/

//异或计算函数
int GPS_checksum(char *buffer)  //  传入一个buffer
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

//************************剖析函数**************************************************
		if(strstr(GPS_buffer.data,"GGA"))   //GPGGA
{
			
				temp=Real_Process_DH(GPS_buffer.data,1);
				memcpy(GPS_Information.UTC_Time,temp,6);	//时间
				temp=Real_Process_DH(GPS_buffer.data,2);
				memcpy(GPS_Information.Latitude,temp,10);	//纬度

				sscanf(GPS_Information.Latitude,"%lf",&Latitude);

				GPS_Information.NS_Indicator=*(Real_Process_DH(GPS_buffer.data,3));	//南北纬
				temp=Real_Process_DH(GPS_buffer.data,4);
				memcpy(GPS_Information.Longitude,temp,11);  //经度
			
				sscanf(GPS_Information.Longitude, "%lf", &Longitude);

				GPS_Information.EW_Indicator=*(Real_Process_DH(GPS_buffer.data,5));	//东西经
				GPS_Information.Use_EPH_Sum=atoi(Real_Process_DH(GPS_buffer.data,7));	//参与定位卫星数

//************************************************************
		if(pos_flag == 1)    //判断筛选条件为定位标志    
		{
			GGA_num++;
			printf("*****************************************************\n序号：%d\n",GGA_num);  //序号
			printf("%s\n",stream);         
			//文件打印
			fprintf(out,"*****************************************************\n序号：%d\n",GGA_num);  //序号
			fprintf(out,"%s\n",stream);       

			if(GPS_Information.GPS_status != '0' )
			{
			printf("定位状态=%c  卫星已定位\n",GPS_Information.GPS_status);

			fprintf(out,"定位状态=%c  卫星已定位\n",GPS_Information.GPS_status);
			}
			else if(GPS_Information.GPS_status == '0')
			{
			printf("定位状态=%c  卫星未定位\n",GPS_Information.GPS_status);
			fprintf(out,"定位状态=%c  卫星未定位\n",GPS_Information.GPS_status);
			}
			
			printf("\n");
			printf("*****************************************************\n");

			fprintf(out,"\n");
			fprintf(out,"*****************************************************\n");
		}

		if(satsum_flag == 1)  //正在使用的卫星颗数标志位
		{
				GGA_num++;
				printf("*****************************************************\n序号：%d\n",GGA_num);  //序号
				printf("%s\n",stream);         

				fprintf(out,"*****************************************************\n序号：%d\n",GGA_num);  //序号
				fprintf(out,"%s\n",stream);     


				printf("正在使用的卫星数量为%d颗",GPS_Information.Use_EPH_Sum );
			
				printf("\n");
				printf("*****************************************************\n");


				fprintf(out,"正在使用的卫星数量为%d颗",GPS_Information.Use_EPH_Sum );
			
				fprintf(out,"\n");
				fprintf(out,"*****************************************************\n");	
				
		}

}	
		
//***********************************************************************************

			if(strstr(GPS_buffer.data,"GSA")) //GPGSA
{

			temp = Real_Process_DH( GPS_buffer.data, 2 ); //第2个参数为定位模式
			if((*temp == '2')||(*temp == '3'))
			{	
				GPS_Information.Locate_mode =*temp;
			}
			else 
					GPS_Information.Locate_mode ='1';  //未定位
			
			for(int i=0;i<12;i++)
			{
				 GPS_Information.User_EPH[i] = atoi( Real_Process_DH( GPS_buffer.data, i + 3 ) ); //从第3个参数开始为所使用的卫星编号			
				
			}

//****************************************************************   FINSIH

			if(pos_flag == 1)    //判断筛选条件为定位标志               
		{
				GSA_num++;
				printf("*****************************************************\n序号：%d\n",GSA_num);  
				printf("%s\n",stream);         //打印出当前语句

				fprintf(out,"*****************************************************\n序号：%d\n",GSA_num); 
				fprintf(out,"%s\n",stream);         //打印出当前语句

			if(GPS_Information.Locate_mode == '2')
			{
			printf("定位模式: %cD定位",GPS_Information.Locate_mode);

			fprintf(out,"定位模式: %cD定位",GPS_Information.Locate_mode);

			}
			else if(GPS_Information.Locate_mode == '3')
			{
			printf("定位模式: %cD定位",GPS_Information.Locate_mode);

			fprintf(out,"定位模式: %cD定位",GPS_Information.Locate_mode);
			}
			else 
			{
			printf("定位模式=%c  卫星未定位\n",GPS_Information.Locate_mode);

			fprintf(out,"定位模式=%c  卫星未定位\n",GPS_Information.Locate_mode);
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
				temp=Real_Process_DH(GPS_buffer.data,1);  //获取RMC-UTC时间
				if(*temp != 0)
				
				memcpy(GPS_Information.UTC_Time,temp,6);

			if ( *( Real_Process_DH( GPS_buffer.data, 2 ) ) == 'A' ) //获取定位状态 
			{
				GPS_Information.GPS_status =1;   //1 定位 
			
			}
			else
			{
				GPS_Information.GPS_status =0;   //0 未定位  
			
			}

				temp = Real_Process_DH( GPS_buffer.data, 3 ); //第3个参数为纬度

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
			GPS_Information.NS_Indicator = *( Real_Process_DH( GPS_buffer.data, 4 ) ); //第4个参数为南北

			temp = Real_Process_DH( GPS_buffer.data, 5); //第5个参数为经度

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
				GPS_Information.EW_Indicator = *( Real_Process_DH( GPS_buffer.data, 6 ) ); //第6个参数为东西
				GPS_Information.Speed = atof( Real_Process_DH( GPS_buffer.data, 7 ) ); //第7个参数为速度
				GPS_Information.Course = atof( Real_Process_DH( GPS_buffer.data, 8 ) ); //第8个参数为航向
				temp = Real_Process_DH( GPS_buffer.data, 9 ); //第9个参数为日期

			if ( *temp != 0 )
	    {
			memcpy( GPS_Information.UTC_Date, temp, 6 );
	    }

}

//*********GSV**********************************
		if ( strstr( GPS_buffer.data, "GSV" ) )//$GPGSV,3,1,11,28,73,321,32,17,39,289,43,11,38,053,17,09,37,250,41*78
{

				for(int m2 = 0; m2<12 ; m2++)  //一共会有12卫星
				{
					if( GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 16 ) ) )   //第四个卫星编号
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (19) ) );   //第四个卫星强度
					}

 					if( GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 12 ) ) )   //第三个卫星编号
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (15) ) );   //第三个卫星强度
					}

					if(  GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 8 ) )	)  //第二个卫星编号
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (11) ) );    //第二个卫星强度
					}

					if(  GPS_Information.User_EPH[m2] == atoi(Real_Process_DH( GPS_buffer.data, 4 ) ) )   //第一个卫星编号
					{
						GPS_Information.GPS[m2].sat_signal = atoi( Real_Process_DH( GPS_buffer.data, (7) ) );   //第一个卫星强度
					}

				}

}

		if(strstr(GPS_buffer.data,"GLL")) //定位地理信息
		{
			
				temp=Real_Process_DH(GPS_buffer.data,1);
				memcpy(GPS_Information.Latitude,temp,10);  //纬度

				sscanf(GPS_Information.Latitude, "%lf", &Latitude);

				GPS_Information.NS_Indicator=*(Real_Process_DH(GPS_buffer.data,2));//南北纬
				temp=Real_Process_DH(GPS_buffer.data,3);
				memcpy(GPS_Information.Longitude,temp,11);  //经度

				sscanf(GPS_Information.Latitude, "%lf", &Latitude);

				GPS_Information.EW_Indicator=*(Real_Process_DH(GPS_buffer.data,4));//东西经
				temp=Real_Process_DH(GPS_buffer.data,5);
				memcpy(GPS_Information.UTC_Time,temp,6);//获取UTC的时间

				
		}


//******************************************************************************************88
		if(strstr(GPS_buffer.data,"VTG"))
{
			
			GPS_Information.N_speed = atof( Real_Process_DH( GPS_buffer.data, 5 ) ); //地面速率 单位：节
			GPS_Information.K_speed = atof( Real_Process_DH( GPS_buffer.data, 7 ) ); //地面速率 单位，KM/H


			if(speed_flag == 1)    //判断筛选条件为速度标志         
			{ 

			if(GPS_Information.K_speed >= in_speed)
				{
					VTG_num++;
					printf("*****************************************************\n序号：%d\n",VTG_num);  //序号
					printf("%s\n",stream);        
					fprintf(out,"*****************************************************\n序号：%d\n",VTG_num);  //序号
					fprintf(out,"%s\n",stream);        

					printf("速度为%3.3f 节\n",GPS_Information.N_speed);
					printf("速度为%3.3f km/h\n",GPS_Information.K_speed);
					fprintf(out,"速度为%3.3f 节\n",GPS_Information.N_speed);
					fprintf(out,"速度为%3.3f km/h\n",GPS_Information.K_speed);
				}
			}

						
}
		
}

void GPS_select()
{
	int flag1;

	int access;

//******************************筛选函数***********************************
	printf("/******************************************************/\n");
	printf(" NMEA-0183解析\n");
	printf("\n");
	printf("1. 请输入筛选条件\n");
	printf("2. 请选择单项筛选或组合筛选 1 单项筛选 2 组合筛选\n");
	printf("\n");
	printf("3. 组合筛选格式: 序号 年月日时分秒\n  ");
	printf(" 经度 纬度 定位状态 速度\n");
	printf("   参与定位的卫星颗数 卫星信号强度 \n");
	printf("\n");
	printf("请确认单项筛选或组合筛选\n");
	printf("单项 1 组合 2 \n");
	//*************************************************************************
	scanf("%d", &access);

		if(access == 1)
	{
		printf("选择单项筛选：请输入对应编号：\n");
		printf("1 是否定位 2 速度值限定值  3 参与定位的卫星颗数 4 卫星信号强度 \n");
		
		scanf("%d", &flag1);
		
		if(flag1 == 1)         //定位信息筛选 
		{
			printf("定位信息如下：\n");

			fprintf(out,"定位信息如下：\n");

			pos_flag=1;        
		}
	

		else if(flag1 == 2 )  //速度信息筛选
		{
			printf("输入限定的速度值 0000.0 -1851.8( km/h )\n");
			scanf("%lf",&in_speed);
			speed_flag=1;   //okok
		
		}


		else if(flag1 == 3)   //参与定位卫星颗数筛选
		{
			printf("参与定位的卫星颗数信息如下: \n");

			fprintf(out,"参与定位的卫星颗数信息如下: \n");
			satsum_flag=1;     

		}

		else if(flag1 == 4)  //卫星信号强度筛选
		{
			printf("卫星信号强度信息如下：\n");

			fprintf(out,"卫星信号强度信息如下：\n");
			signal_flag=1;   
		}

	
	}


	/*****************组合筛选**************************/
	if (access == 2)          //筛选条件输入 
	{
	
		printf("筛选是否定位：输入 1 (A 定位)/ 0 ( V未定位)\n");  
		scanf("%d",&in_pos);
		printf("%d\n",in_pos);

	
		printf("筛选速度：输入速度上下限值 (0000.0 - 1851.8) km/h \n");  
		printf("请输入速度上限值 (km/h)\n");
		scanf("%lf",&speed_max);
		printf("速度上限值为 %lf (km/h)\n",speed_max);

		printf("请输入速度下限值 (km/h)\n");
		scanf("%lf",&speed_min);
		printf("速度下限值为 %lf  (km/h) \n",speed_min);

		printf("筛选卫星定位颗数：输入卫星定位颗数最小值:\n");   
		scanf("%d",&in_sum);
		printf("定位颗数最小值为 %d \n",in_sum);

	
		printf("筛选卫星强度：输入卫星强度大小最小值:\n");  
		scanf("%d",&in_signal);
		printf("卫星强度最小值为 %d \n",in_signal);

		multi_flag=1;   // 后续需要添加所有数据的判断语句
	}
}


void GPS_show()//输出函数
{	
	int GSVok_flag=0;
	int GSVcheck_flag=0;
	char endess;

		
			if((multi_flag == 1))                //组合筛选标志
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
			&&(GSVok_flag == 1)&&GSVcheck_flag==1) //判断条件符合上才进行输出
		{


				RMC_num++;
				printf("*****************************************************\n序号：%d\n",RMC_num);  //序号
				fprintf(out,"*****************************************************\n序号：%d\n",RMC_num);  //序号
				printf("日期：20%c%c年%c%c月%c%c日",GPS_Information.UTC_Date[4],GPS_Information.UTC_Date[5],GPS_Information.UTC_Date[2],
				GPS_Information.UTC_Date[3],GPS_Information.UTC_Date[0],GPS_Information.UTC_Date[1]);
				printf("%c%c时%c%c分%c%c秒 \n",GPS_Information.UTC_Time[0],GPS_Information.UTC_Time[1],GPS_Information.UTC_Time[2],
				GPS_Information.UTC_Time[3],GPS_Information.UTC_Time[4],GPS_Information.UTC_Time[5]);	//年月日时分秒

				fprintf(out,"日期：20%c%c年%c%c月%c%c日",GPS_Information.UTC_Date[4],GPS_Information.UTC_Date[5],GPS_Information.UTC_Date[2],
				GPS_Information.UTC_Date[3],GPS_Information.UTC_Date[0],GPS_Information.UTC_Date[1]);
				fprintf(out,"%c%c时%c%c分%c%c秒 \n",GPS_Information.UTC_Time[0],GPS_Information.UTC_Time[1],GPS_Information.UTC_Time[2],
				GPS_Information.UTC_Time[3],GPS_Information.UTC_Time[4],GPS_Information.UTC_Time[5]);	//年月日时分秒

				if(GPS_Information.EW_Indicator=='E')
			{
					printf("东经：");
					fprintf(out,"东经：");
			}
				else
			{
					printf("西经：");
					fprintf(out,"西经：");
			}
			printf("%5.5f\n",Longitude);//经度
			fprintf(out,"%5.5f\n",Longitude);//经度
				if(GPS_Information.NS_Indicator=='N')
			{
				printf("北纬：");
				fprintf(out,"北纬：");
			}
				else
			{
				printf("南纬：");

				fprintf(out,"南纬：");
			}	
				
				printf("%5.5f\n",Latitude);//纬度
				fprintf(out,"%5.5f\n",Latitude);//纬度

			if(GPS_Information.GPS_status== 0 )
			{
				printf("GPS未定位 \n");//定位状态

				fprintf(out,"GPS未定位 \n");//定位状态
			}
				else if(GPS_Information.GPS_status== 1 )
			{
				printf("GPS已定位 \n");

				fprintf(out,"GPS已定位 \n");
			}

				printf("使用的卫星数%d\n",GPS_Information.Use_EPH_Sum);//参与卫星颗数
				fprintf(out,"使用的卫星数%d\n",GPS_Information.Use_EPH_Sum);//参与卫星颗数

	
				printf("海里速度：%3.3f节 \n",GPS_Information.N_speed);//速度
				printf("公里速度：%3.3fkm/h \n",GPS_Information.K_speed);//速度
				fprintf(out,"海里速度：%3.3f节 \n",GPS_Information.N_speed);//速度
				fprintf(out,"公里速度：%3.3fkm/h \n",GPS_Information.K_speed);//速度
							
				for(int m1=0;m1 <12; m1++)
				{	
					 if( 0 == GPS_Information.User_EPH[m1])
					{	
							continue;
					}
					if( (GPS_Information.GPS[m1].sat_signal >= in_signal ))
					{	

							printf("第%d颗在用卫星数据为: PRN=%d SNR=%d \n",m1+1,GPS_Information.User_EPH[m1],GPS_Information.GPS[m1].sat_signal);
							fprintf(out,"第%d颗在用卫星数据为: PRN=%d SNR=%d \n",m1+1,GPS_Information.User_EPH[m1],GPS_Information.GPS[m1].sat_signal);	
					}			

				}

			}
		}

			
		
}






