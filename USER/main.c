#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "adc.h"
#include "key.h"
#include "beep.h" 
#include "bsp_adc.h"
#include "math.h"

//函数声明
 void Lcd_DrawNetwork(void); 					//屏幕网格
 void Set_BackGround(void);  					//屏幕背景色
 void clear_point(u16 num);   //
 void DrawOscillogram(u16 *buff);			//画波形图
 void DrawOscillogram_Clear(u16 *buff);//清除上次绘图内容
 void ADCInit_Timer(u16 arr,u16 psc);
 void Rheostat_Init(void);
 void Time_init(void);
 void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color);
 
  //全局变量
float* Get_vpp(u16 *buf);
float* Get_RMS_DutyCycle(u16 *buff);
u16   buff[800],buff1[1000];  			//波形数据存储
vu16 ADC_ConvertedValue[2400] ;  	//AD采集数据
float *Adresult,trig=0;  			//屏幕打印参数
u8  Vpp_buff[20] = {0};						//sprintf数据输出
u8  test_buff[20]={0};						//测试内容输出
u8  con_sta=1;
u8  func_sta=1;			//按键功能指示
u8 avg=1;						//波形放大系数
u8 tim=2;						//时间基准指示
u8  run_sta=1; 			//示波器运行状态指示
u16 Yinit=200;  		//画图数据参数
u32 max_data=1200; 	//此处触发电压是大于1V，符合题目要求的1-3V
u32 max_data2=0;
vu32 temp=0,freq=0; 				//频率输出
int Ypos2_bia=0;		//Y轴坐标偏置
int Xpos_bia=0;			//X轴坐标偏置
int Y_sen=1;
int Y_sen_num[6]={660,330,220,165,132,110};		//垂直灵敏度系数,对应显示的峰值电压依次为1V 2V 3V 4V 5V 6V
u16 Y_sen_buff[6]={250,500,750,1000,1250,1500};	
int X_sen=3;		//水平灵敏度系数,初始设置psc为83，TIM_X=us/div
int X_sen_num[7]={20,41,62,83,125,167,209}; 	//psc值预备
u16 X_sen_buff[7]={25,50,75,100,150,200,250};//单位us/div 
char *waveform[4]={"?","Sine","Square","Ramp"};
int wave=1;
double RMS=0.0,DutyCycle_square=0.0;
float *result;

int main(void)
{ 
  vu32 cc=200,time=1,sta_key=1;		//sta_key 功能按键切换 cc按键号码指示
	u8 color_sta=0;									//run颜色状态
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	KEY_Init();											//初始化按键函数
	delay_init(168);      					//初始化延时函数
	uart_init(115200);							//初始化串口波特率为115200
	LED_Init();					  					//初始化LED
 	LCD_Init();           					//初始化LCD
  LCD_Display_Dir(1);  						//LCD显示方向，0为横屏，1为竖屏
  Set_BackGround();								//设置背景色
  Lcd_DrawNetwork(); 							//绘制网格线
  Rheostat_Init();								//ADC初始化
  Time_init(); 										//TIM3初始化
	
	//主程序
  while(1) 
	{	
    Lcd_DrawNetwork(); //刷新屏幕网格
    BACK_COLOR=BLACK;

    if(run_sta==0)  //运行状态为停止时刷新波形
    {
			DrawOscillogram_Clear(buff);
			DrawOscillogram(buff); 
    }
		Adresult = Get_vpp(buff);//峰峰值mv		
		sprintf((char*)Vpp_buff,"Vpp:%0.3f V",Adresult[0]);
		LCD_ShowString(330,420,210,24,24,Vpp_buff);
		sprintf((char*)Vpp_buff,"V_max:%0.3f V",Adresult[1]);
		LCD_ShowString_COLOR(620,10,210,24,24,Vpp_buff,WHITE);
		sprintf((char*)Vpp_buff,"V_min:%0.3f V",Adresult[2]);
		LCD_ShowString_COLOR(620,40,210,24,24,Vpp_buff,WHITE);
		
		
		//trig = max_data*(3.3)/4096;		//采集到信号的最大值转成的电压值
		//sprintf((char*)Vpp_buff,"Trig Level:%0.2f V",trig);
		//LCD_ShowString(330,450,210,24,24,Vpp_buff);
		
		time=pow(2.0,(tim)*1.0)*50 ;
		sprintf((char*)Vpp_buff,"Tim_X:%5dus/div",X_sen_buff[X_sen]);
		LCD_ShowString(530,420,210,24,24,Vpp_buff);
		
		sprintf((char*)Vpp_buff,"Tim_Y:%5dmv/div",Y_sen_buff[Y_sen]);	
		LCD_ShowString(530,450,210,24,24,Vpp_buff);		//垂直灵敏度
		
		//freq=84*(1000000)/((temp*time));  //两个触发点之间的时间倒数即为频率
		freq=(84*1000000)/(abs(temp)*(TIM3->ARR+1)*(TIM3->PSC+1));				//根据定时器的周期和一个波形周期内ADC采集的点数来计算频率
		sprintf((char*)Vpp_buff,"FREQ:%6d Hz",freq);	
		LCD_ShowString(100,420,210,24,24,Vpp_buff);
		//LCD_ShowString_COLOR(330,450,210,24,24,"DutyCycle:   ",YELLOW);
		//sprintf((char*)Vpp_buff,"m:%4d Hz",buff[0]);	
		//LCD_ShowString(100,350,210,24,24,Vpp_buff);
		
		//根据RMS来判断波形
		result=Get_RMS_DutyCycle(buff);
		RMS=result[0];
		DutyCycle_square=result[1];
		if(RMS>0.3 && RMS<0.62)
		{
			LCD_ShowString_COLOR(100,450,210,24,24,"Wave:Ramp  ",YELLOW);
			LCD_ShowString_COLOR(330,450,210,24,24,"DutyCycle:     ",YELLOW);
		}
		if(RMS>0.68 && RMS<0.9)
		{
			LCD_ShowString_COLOR(100,450,210,24,24,"Wave:Sine  ",YELLOW);
			LCD_ShowString_COLOR(330,450,210,24,24,"DutyCycle:     ",YELLOW);
			
		}
		if(0.9<RMS)
		{
			LCD_ShowString_COLOR(100,450,210,24,24,"Wave:Square",YELLOW);
			sprintf((char*)Vpp_buff,"DutyCycle:%0.1f%%",DutyCycle_square*100.0);
			LCD_ShowString(330,450,210,24,24,Vpp_buff);
		}
		
		
		//sprintf((char*)Vpp_buff,"RMS:%0.3d",TIM3->ARR);
		//LCD_ShowString(200,450,210,24,24,Vpp_buff);
		
		if(run_sta==1)	//==1表示运行中
		{
			//RUN文字底色闪烁，黑色和蓝灰色交替闪烁
			 if(color_sta==0) 
			 {
					BACK_COLOR=GRAYBLUE;
					color_sta=1;
				 
			 }
			 else
			 {
					BACK_COLOR=BLACK;
					color_sta=0;
			 }
			 LCD_ShowString_COLOR(10,420,210,24,24,"RUN ",GREEN);
			 BACK_COLOR=BLACK;
		}
		else
		{
			 LCD_ShowString_COLOR(10,420,210,24,24,"STOP",RED);
		}
		  //LED指示灯
		
		//按键读取
		cc=KEY_Scan(0); //按键状态读取
		sprintf((char*)test_buff,"fuc:%.d",func_sta);	
		LCD_ShowString(10,450,210,24,24,test_buff);

		
		if(func_sta==1)			//func_sta=1时调节波形纵轴位置
		{
			if(cc==1)   			//波形下调
			{
				DrawOscillogram_Clear(buff);
				Ypos2_bia+=10;
			}
			else if(cc==2) 		 //波形上调
			{
				DrawOscillogram_Clear(buff);
				Ypos2_bia-=10;
			}
		}
		
		if(func_sta==2)			//func_sta=2时调节波形横轴位置
		{
			if(cc==1)   			//波形右调
			{
				DrawOscillogram_Clear(buff);
				Xpos_bia-=20;
			}
			else if(cc==2) 		//波形左调
			{
				DrawOscillogram_Clear(buff);
				Xpos_bia+=20;
			}
		}
		
		
		if(func_sta==3)			//func_sta=3为调整垂直灵敏度模式
		{
			if(cc==1) 				//key0增加灵敏度系数
			{
				DrawOscillogram_Clear(buff);
				if(Y_sen==0)
				{
					Y_sen=0;
				}
				else
				{
					Y_sen-=1;
				}
			}
			if(cc==2)					//key1减小灵敏度系数
			{
				DrawOscillogram_Clear(buff);
				if(Y_sen==5)
				{
					Y_sen=5;
				}
				else
				{
					Y_sen+=1;
				}
			}
		}
		
		if(func_sta==4)			//func_sta=3为调整水平灵敏度模式
		{
			if(cc==1)
			{
				DrawOscillogram_Clear(buff);
				X_sen+=1;
				if(X_sen>5)
				{
					X_sen=5;
				}
				TIM3->PSC=X_sen_num[X_sen];
			}
			if(cc==2)
			{
				DrawOscillogram_Clear(buff);
				X_sen-=1;
				if(X_sen<0)
				{
					X_sen=0;
				}
				TIM3->PSC=X_sen_num[X_sen];
				
			}
		}
				
		if(cc==3)
			{
				func_sta+=1;
				if(func_sta>4)
					func_sta=1;
			}
		if(cc==4)
			{
				 if(run_sta==1)
					{
						 run_sta=0;
						 TIM_Cmd(TIM3, DISABLE);
					}
					else
					{
						 run_sta=1;
						 TIM_Cmd(TIM3, ENABLE);
					}
			}
	}       
} 
float* Get_RMS_DutyCycle(u16 *buff)
{
	static u16 i=0,n=0;
	static u16 Ypos2 = 0,Ypos_max=0;
	u32 max_data1=buff[0], min_data1=buff[0], middle_data;
	int Y_ref=200,max1=0,max2=0,N=0;
	float cal_temp=0.0,sum=0.0,rms=0.0,DutyCycle=0.0,num_temp=0.0;
	static float rms_DutyCycle[2];
	
	for(i = 1;i < 800;i++)
	{
		if(buff[i]>max_data1)
			max_data1=buff[i];
		if(buff[i]<min_data1)
			min_data1=buff[i];
	}
	middle_data=min_data1+(max_data1-min_data1)/2;
	for(n=0;n<800;n++)
	{
		if(ADC_ConvertedValue[n] > max_data&&ADC_ConvertedValue[n+3] < max_data)
		 {
				max1=n;
				break;
		 }
	}
	for(n = max1+3;n<800;n++)
	{
		 if(ADC_ConvertedValue[n] > max_data&&ADC_ConvertedValue[n+3] < max_data)
		 {
				max2=n;
				break;
		 }			
	}
	
	Ypos_max=abs(Yinit + (middle_data * Y_sen_num[Y_sen] / 4096) - Y_ref);
	for(i=max1;i<max2;i++)
	{
		Ypos2=Yinit + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
		Y_ref=200;
		cal_temp=pow((Ypos2-Y_ref),2);
		if((Ypos2-Y_ref)<=0)
		{
			num_temp+=1;
		}
		sum+=cal_temp;
	}
	N=abs(max2-max1);
	rms=sqrt(sum/N)/Ypos_max;
	DutyCycle=num_temp/N;
	rms_DutyCycle[0]=rms;
	rms_DutyCycle[1]=DutyCycle;
	
	return rms_DutyCycle;
}



void Lcd_DrawNetwork(void)	//这一部分画出了示波器界面的网格线
{
	u16 index_y = 0;
	u16 index_x = 0;	
	
    //画列点连成竖直线，x轴宽度800，y轴高度400，x轴上每隔50单位设置一点
	for(index_x = 50;index_x < 800;index_x += 50)
	{
		for(index_y = 0;index_y < 400;index_y += 1)
		{
			LCD_Fast_DrawPoint(index_x,index_y,0X534c);		//534C偏蓝色相
		}
	}
	//画行点连成水平线
	for(index_y = 50;index_y < 400;index_y += 50)
	{
		for(index_x = 0;index_x < 800;index_x += 1)
		{
			LCD_Fast_DrawPoint(index_x,index_y,0X534c);	
		}
	}
   LCD_DrawLine_color (0 ,200 , 800 ,200,BROWN);
   LCD_DrawLine_color (400 ,000 , 400 ,400,BROWN);
}
void Set_BackGround(void)		//设置背景颜色为黑色
{
	POINT_COLOR = 0x5510;
    LCD_Clear(BLACK);
	LCD_DrawRectangle(0,0,798,401);//矩形	

}
void clear_point(u16 num)//更新显示屏当前列
{
	u16 index_clear_lie = 0; 
	POINT_COLOR = BLACK ;
	for(index_clear_lie = 1;index_clear_lie < 400;index_clear_lie++)
	{		
		LCD_DrawPoint(num,index_clear_lie );
	}
	if(!(num%50))//判断hang是否为50的倍数 画列点
	{
		for(index_clear_lie = 10;index_clear_lie < 400;index_clear_lie += 10)
		{		
			LCD_Fast_DrawPoint(num ,index_clear_lie,WHITE );
		}
	}
	if(!(num%10))//判断hang是否为10的倍数 画行点
	{
		for(index_clear_lie = 50;index_clear_lie <400;index_clear_lie += 50)
		{		
			LCD_Fast_DrawPoint(num ,index_clear_lie,WHITE );
		}
	}	
	POINT_COLOR = YELLOW;	
}


void DrawOscillogram(u16 *buff)//画波形图
{
	static u16 Ypos1 = 0,Ypos2 = 0;
	static vu32 min_data=0;//buf[1];
	static vu32 n=0,con_t=0,con_t1=0;
  static	u16 i = 0;
	vu32 cc=0;
	int		temp1=0,temp2=0;
	u32 max_data1=buff[0], min_data1=buff[0], middle_data;
	POINT_COLOR = YELLOW;					//示波器下端显示文字的颜色为黄色
	
	//获取ADC参数

	for(n = 0;n<800;n++)
	{
		 buff[n]=ADC_ConvertedValue[con_t+n-400] ;//源代码用的
	}	
	
	for(n = 0;n<1000;n++)
	{
		 buff1[n]=ADC_ConvertedValue[con_t+n-500] ;//源代码用的
	}	

	
	//temp之前的代码为检测频率的，找两个波峰点求出时间差temp，在主函数里有处理代码
	for(n = 500;n<2000;n++)
	{
		 if(ADC_ConvertedValue[n] > max_data&&ADC_ConvertedValue[n+3] < max_data)
		 {
				con_t=n;
				break;
		 }			
	} 
	for(n = con_t+3;n<2000;n++)
	{
		 if(ADC_ConvertedValue[n] > max_data&&ADC_ConvertedValue[n+3] < max_data)
		 {
				con_t1=n;
				break;
		 }			
	}
	temp= con_t1- con_t;
	
	//求最大值
	for(i = 1;i < 800;i++)
	{
		if(buff[i]>max_data1)
			max_data1=buff[i];
		if(buff[i]<min_data1)
			min_data1=buff[i];
	}
	max_data2=max_data1;
	middle_data=min_data1+(max_data1-min_data1)/2;
	//错位调整回初始位置
	temp1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096);
	temp2=Yinit + Ypos2_bia - (middle_data * Y_sen_num[Y_sen] / 4096);
	if(temp1>400)			//当调到示波器显示区域最底部的时候回到原位置
	{
		Ypos2_bia=0;
	}
	if(temp2<0)			//当调到示波器显示区域最顶部的时候回到原位置
	{
		Ypos2_bia=0;
	}
	
	Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[1] * Y_sen_num[Y_sen] / 4096);	
	
	
	if(func_sta==2&&run_sta==0)
	{
		if(Xpos_bia<0)
		{
			Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[1+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);	
			for(i = 1;i < 800;i++)
			{
				
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[i+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);			
		
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示		
					LCD_DrawLine (i ,Ypos1 , i+1 ,Ypos2);
				Ypos1 = Ypos2 ;
			}
		}
		
		if(Xpos_bia>0)
		{
			Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[1+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);	
			for(i = 1;i < 800;i++)
			{
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[i+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示
				LCD_DrawLine (i ,Ypos1 , i+1 ,Ypos2);
				Ypos1 = Ypos2 ;
			}
		}
		else
		{
			for(i = 1;i < 800;i++)
			{
				
				Ypos2 = Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示
				LCD_DrawLine (i ,Ypos1 , i+1 ,Ypos2);
				Ypos1 = Ypos2 ;
			}
		}
	}
	else
	{
		for(i = 1;i < 800;i++)
		{
			
			Ypos2 = Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
			if(Ypos2 >400)
				Ypos2 =400; //超出范围不显示
			LCD_DrawLine (i ,Ypos1 , i+1 ,Ypos2);
			Ypos1 = Ypos2 ;
		}
	}
	Ypos1 = 0;
		
}



void DrawOscillogram_Clear(u16 *buff)//画波形图
{
	static u16 Ypos1 = 0,Ypos2 = 0;
	static vu32 n=0,con_t=0;
	u32 max_data1=buff[0], min_data1=buff[0], middle_data;
	u16 i = 0;
	vu32 cc=0;
	int		temp1=0,temp2=0;
	POINT_COLOR = YELLOW;
	
	for(i = 1;i < 800;i++)
	{
		if(buff[i]>max_data1)
			max_data1=buff[i];
		if(buff[i]<min_data1)
			min_data1=buff[i];
	}
	middle_data=min_data1+(max_data1-min_data1)/2;
	//func_sta=1时调整纵轴时基，=2时调整横轴时基
	temp1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096);
	temp2=Yinit + Ypos2_bia - (middle_data * Y_sen_num[Y_sen] / 4096);
	if(temp1>400)			//当调到示波器显示区域最底部的时候回到原位置
	{
		Ypos2_bia=0;
	}
	if(temp2<0)			//当调到示波器显示区域最顶部的时候回到原位置
	{
		Ypos2_bia=0;
	}
	
	Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[1] * Y_sen_num[Y_sen] / 4096);	
	if(func_sta==2&&run_sta==0)		//如果有横轴位移时的显示
	{
		if(Xpos_bia<0)
		{
			Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[1+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);	
			for(i = 1;i < 800;i++)
			{
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[i+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示
				LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
				Ypos1 = Ypos2 ;
			}
		}
		
		if(Xpos_bia>0)
		{
			Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[1+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);	
			for(i = 1;i < 800;i++)
			{
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[i+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示
				LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
				Ypos1 = Ypos2 ;
			}
		}
		else
		{
			for(i = 1;i < 800;i++)
			{
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //超出范围不显示
				LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
				Ypos1 = Ypos2 ;
			}
		}
	}
	else				//横轴无位移的显示
	{
		for(i = 1;i < 800;i++)
		{
			
			Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
			if(Ypos2 >400)
				Ypos2 =400; //超出范围不显示
			LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
			Ypos1 = Ypos2 ;
		}
	}
	Ypos1 = 0;
}

float* Get_vpp(u16 *buf)	   //获取峰峰值
{
	u32 max_data=buf[0];
	u32 min_data=buf[0];//buf[1];
	u32 n=0;
	static float Data[3];
	float Vpp=0,max=0,min=0;
	for(n = 1;n<800;n++)
	{
		if(buf[n] > max_data)
		{
			max_data = buf[n];
		}
		if(buf[n] < min_data)
	{
			min_data = buf[n];
		}			
	} 	
	Vpp = (float)(max_data - min_data);
	Vpp = Vpp*(3.3/4096);					//ADC采集信号转电压值公式
	max=((float)(max_data))*(3.3/4096);
	min=((float)(min_data))*(3.3/4096);
	Data[0]=Vpp;
	Data[1]=max;
	Data[2]=min;
	return Data;	
}
///////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   

static void Rheostat_ADC_GPIO_Config(void)	//开启PA口时钟和ADC1时钟，设置PA5为模拟输入
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
	// 使能 GPIO 时钟
	RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);//使能GPIOA时钟
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟
		
	// 配置 GPIO5结构体参数
	GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;	//这里ADC1用的是PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    //设置为模拟输入
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; //不带上下拉
	GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);		
}


static void Rheostat_ADC_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
  // ------------------DMA Init 结构体参数 初始化--------------------------
  // ADC1使用DMA2，数据流0，通道0，这个是手册固定死的
  // 开启DMA时钟
  RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_DMA_CLK, ENABLE); 
	// 外设基址为：ADC 数据寄存器地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_DR_ADDR;	
  // 存储器地址，实际上就是一个内部SRAM的变量	
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ADC_ConvertedValue;  
  // 数据传输方向为外设到存储器	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	
	// 缓冲区大小为，指一次传输的数据量
	DMA_InitStructure.DMA_BufferSize = 2400;	
	// 外设寄存器只有一个，地址不用递增
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // 存储器地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  // // 外设数据大小为半字，即两个字节 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  //	存储器数据大小也为半字，跟外设数据大小相同
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// 循环传输模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  // DMA 传输通道优先级为高，当使用一个DMA通道时，优先级设置不影响
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  // 禁止DMA FIFO	，使用直连模式
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  // FIFO 大小，FIFO模式禁止时，这个不用配置	
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	// 选择 DMA 通道，通道存在于流中
  DMA_InitStructure.DMA_Channel = RHEOSTAT_ADC_DMA_CHANNEL; 
  //初始化DMA流，流相当于一个大的管道，管道里面有很多通道
	DMA_Init(RHEOSTAT_ADC_DMA_STREAM, &DMA_InitStructure);
   //设置DMA中断
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); //清除中断标志   
    DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE); //传输完成中断                                       
    //DMA_Cmd(DMA2_Stream0, ENABLE); //使能DMA
	// 使能DMA流
  DMA_Cmd(RHEOSTAT_ADC_DMA_STREAM, ENABLE);
	
	// 开启ADC时钟
	RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);
	
	
	
  // -------------------ADC Common 结构体 参数 初始化------------------------
	// 独立ADC模式
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  // 时钟为fpclk x分频	
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;	//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
  // 禁止DMA直接访问模式		
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;		//DMA失能
  // 采样时间间隔	
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  	//两个采样阶段之间的延迟5个时钟,多重adc才用到，一个adc用不到
  ADC_CommonInit(&ADC_CommonInitStructure);	//初始化
	
	
	
  // -------------------ADC Init 结构体 参数 初始化--------------------------
  // ADC 分辨率设置为12位
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;	//12位模式
  // 禁止扫描模式，多通道采集才需要	
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; 	//非扫描模式	
	
  // 连续转换	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//关闭连续转换
 
  //禁止外部边沿触发
  //ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;	//禁止触发检测，使用软件触发
	
	//源代码将上一行注释，使用下两行
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //使用外部触发的时候可以选择触发记性，这里是上升沿触发
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;  //定时器3 TGRO外部触发启动ADC
	
  //使用软件触发，外部触发不用配置，注释掉即可
  //ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	
  //数据右对齐	
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //转换通道 1个，规则序列中有一个转换
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                    
  ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
  //---------------------------------------------------------------------------
	
	
	
  // 配置 ADC 通道转换顺序为1，第一个转换，采样时间为3个时钟周期
  ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL, 1, ADC_SampleTime_3Cycles);			//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度	

  // 使能DMA请求 after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(RHEOSTAT_ADC, ENABLE);
  // 使能ADC DMA
  ADC_DMACmd(RHEOSTAT_ADC, ENABLE);
	
	// 使能ADC
  ADC_Cmd(RHEOSTAT_ADC, ENABLE);  //开启AD转换器	
  //开始adc转换，软件触发
  ADC_SoftwareStartConv(RHEOSTAT_ADC);
}

void Rheostat_Init(void)
{
	Rheostat_ADC_GPIO_Config();
	Rheostat_ADC_Mode_Config();
}

void ADCInit_Timer(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
     NVIC_InitTypeDef NVIC_InitStructure;
    //时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);          
	

    //失能定时器
    TIM_Cmd(TIM3, DISABLE);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //初始化定时器

    TIM_TimeBaseStructure.TIM_Prescaler = psc; //预分频系数PSC为167
    TIM_TimeBaseStructure.TIM_Period = arr; //自动装载值ARR
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分频因子
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; //向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //初始化定时器3
    
    //使能定时器中断    
    TIM_ARRPreloadConfig(TIM3, ENABLE); //允许TIM3定时重载
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);  //选择TIM3的UPDATA事件更新为触发源
//    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //配置TIM3中断类型
     NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;  //DMA2_Stream0中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;  //抢占优先级0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //子优先级0,源代码使用的是子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);    //根据指定的参数初始化NVIC寄存器
    //使能TIM3
    TIM_Cmd(TIM3, ENABLE); 
}

void Time_init(void)
{
	ADCInit_Timer(1,83);		
}

void DMA2_Stream0_IRQHandler(void)  
{
    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))  //判断DMA传输完成中断  
    {
         
      if(run_sta==1)
       {
          con_sta=1;
       TIM_Cmd(TIM3, DISABLE);
			 DrawOscillogram(buff); 
			 //delay_ms(50);
       DrawOscillogram_Clear(buff);
       TIM_Cmd(TIM3, ENABLE);
       }
       LED0=!LED0;
       DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);  
    }
}


//定时器5中断服务程序	 
void TIM5_IRQHandler(void)
{ 		    
}
void EXTI0_IRQHandler(void)
{

}
 void TIM3_IRQHandler(void)
{
     if(TIM_GetITStatus(TIM3, TIM_IT_Update))  //判断发生update事件中断  
    {        
 LED0=!LED0;	       
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //清除update事件中断标志    
    }

}

