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

//��������
 void Lcd_DrawNetwork(void); 					//��Ļ����
 void Set_BackGround(void);  					//��Ļ����ɫ
 void clear_point(u16 num);   //
 void DrawOscillogram(u16 *buff);			//������ͼ
 void DrawOscillogram_Clear(u16 *buff);//����ϴλ�ͼ����
 void ADCInit_Timer(u16 arr,u16 psc);
 void Rheostat_Init(void);
 void Time_init(void);
 void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color);
 
  //ȫ�ֱ���
float* Get_vpp(u16 *buf);
float* Get_RMS_DutyCycle(u16 *buff);
u16   buff[800],buff1[1000];  			//�������ݴ洢
vu16 ADC_ConvertedValue[2400] ;  	//AD�ɼ�����
float *Adresult,trig=0;  			//��Ļ��ӡ����
u8  Vpp_buff[20] = {0};						//sprintf�������
u8  test_buff[20]={0};						//�����������
u8  con_sta=1;
u8  func_sta=1;			//��������ָʾ
u8 avg=1;						//���ηŴ�ϵ��
u8 tim=2;						//ʱ���׼ָʾ
u8  run_sta=1; 			//ʾ��������״ָ̬ʾ
u16 Yinit=200;  		//��ͼ���ݲ���
u32 max_data=1200; 	//�˴�������ѹ�Ǵ���1V��������ĿҪ���1-3V
u32 max_data2=0;
vu32 temp=0,freq=0; 				//Ƶ�����
int Ypos2_bia=0;		//Y������ƫ��
int Xpos_bia=0;			//X������ƫ��
int Y_sen=1;
int Y_sen_num[6]={660,330,220,165,132,110};		//��ֱ������ϵ��,��Ӧ��ʾ�ķ�ֵ��ѹ����Ϊ1V 2V 3V 4V 5V 6V
u16 Y_sen_buff[6]={250,500,750,1000,1250,1500};	
int X_sen=3;		//ˮƽ������ϵ��,��ʼ����pscΪ83��TIM_X=us/div
int X_sen_num[7]={20,41,62,83,125,167,209}; 	//pscֵԤ��
u16 X_sen_buff[7]={25,50,75,100,150,200,250};//��λus/div 
char *waveform[4]={"?","Sine","Square","Ramp"};
int wave=1;
double RMS=0.0,DutyCycle_square=0.0;
float *result;

int main(void)
{ 
  vu32 cc=200,time=1,sta_key=1;		//sta_key ���ܰ����л� cc��������ָʾ
	u8 color_sta=0;									//run��ɫ״̬
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	KEY_Init();											//��ʼ����������
	delay_init(168);      					//��ʼ����ʱ����
	uart_init(115200);							//��ʼ�����ڲ�����Ϊ115200
	LED_Init();					  					//��ʼ��LED
 	LCD_Init();           					//��ʼ��LCD
  LCD_Display_Dir(1);  						//LCD��ʾ����0Ϊ������1Ϊ����
  Set_BackGround();								//���ñ���ɫ
  Lcd_DrawNetwork(); 							//����������
  Rheostat_Init();								//ADC��ʼ��
  Time_init(); 										//TIM3��ʼ��
	
	//������
  while(1) 
	{	
    Lcd_DrawNetwork(); //ˢ����Ļ����
    BACK_COLOR=BLACK;

    if(run_sta==0)  //����״̬Ϊֹͣʱˢ�²���
    {
			DrawOscillogram_Clear(buff);
			DrawOscillogram(buff); 
    }
		Adresult = Get_vpp(buff);//���ֵmv		
		sprintf((char*)Vpp_buff,"Vpp:%0.3f V",Adresult[0]);
		LCD_ShowString(330,420,210,24,24,Vpp_buff);
		sprintf((char*)Vpp_buff,"V_max:%0.3f V",Adresult[1]);
		LCD_ShowString_COLOR(620,10,210,24,24,Vpp_buff,WHITE);
		sprintf((char*)Vpp_buff,"V_min:%0.3f V",Adresult[2]);
		LCD_ShowString_COLOR(620,40,210,24,24,Vpp_buff,WHITE);
		
		
		//trig = max_data*(3.3)/4096;		//�ɼ����źŵ����ֵת�ɵĵ�ѹֵ
		//sprintf((char*)Vpp_buff,"Trig Level:%0.2f V",trig);
		//LCD_ShowString(330,450,210,24,24,Vpp_buff);
		
		time=pow(2.0,(tim)*1.0)*50 ;
		sprintf((char*)Vpp_buff,"Tim_X:%5dus/div",X_sen_buff[X_sen]);
		LCD_ShowString(530,420,210,24,24,Vpp_buff);
		
		sprintf((char*)Vpp_buff,"Tim_Y:%5dmv/div",Y_sen_buff[Y_sen]);	
		LCD_ShowString(530,450,210,24,24,Vpp_buff);		//��ֱ������
		
		//freq=84*(1000000)/((temp*time));  //����������֮���ʱ�䵹����ΪƵ��
		freq=(84*1000000)/(abs(temp)*(TIM3->ARR+1)*(TIM3->PSC+1));				//���ݶ�ʱ�������ں�һ������������ADC�ɼ��ĵ���������Ƶ��
		sprintf((char*)Vpp_buff,"FREQ:%6d Hz",freq);	
		LCD_ShowString(100,420,210,24,24,Vpp_buff);
		//LCD_ShowString_COLOR(330,450,210,24,24,"DutyCycle:   ",YELLOW);
		//sprintf((char*)Vpp_buff,"m:%4d Hz",buff[0]);	
		//LCD_ShowString(100,350,210,24,24,Vpp_buff);
		
		//����RMS���жϲ���
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
		
		if(run_sta==1)	//==1��ʾ������
		{
			//RUN���ֵ�ɫ��˸����ɫ������ɫ������˸
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
		  //LEDָʾ��
		
		//������ȡ
		cc=KEY_Scan(0); //����״̬��ȡ
		sprintf((char*)test_buff,"fuc:%.d",func_sta);	
		LCD_ShowString(10,450,210,24,24,test_buff);

		
		if(func_sta==1)			//func_sta=1ʱ���ڲ�������λ��
		{
			if(cc==1)   			//�����µ�
			{
				DrawOscillogram_Clear(buff);
				Ypos2_bia+=10;
			}
			else if(cc==2) 		 //�����ϵ�
			{
				DrawOscillogram_Clear(buff);
				Ypos2_bia-=10;
			}
		}
		
		if(func_sta==2)			//func_sta=2ʱ���ڲ��κ���λ��
		{
			if(cc==1)   			//�����ҵ�
			{
				DrawOscillogram_Clear(buff);
				Xpos_bia-=20;
			}
			else if(cc==2) 		//�������
			{
				DrawOscillogram_Clear(buff);
				Xpos_bia+=20;
			}
		}
		
		
		if(func_sta==3)			//func_sta=3Ϊ������ֱ������ģʽ
		{
			if(cc==1) 				//key0����������ϵ��
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
			if(cc==2)					//key1��С������ϵ��
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
		
		if(func_sta==4)			//func_sta=3Ϊ����ˮƽ������ģʽ
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



void Lcd_DrawNetwork(void)	//��һ���ֻ�����ʾ���������������
{
	u16 index_y = 0;
	u16 index_x = 0;	
	
    //���е�������ֱ�ߣ�x����800��y��߶�400��x����ÿ��50��λ����һ��
	for(index_x = 50;index_x < 800;index_x += 50)
	{
		for(index_y = 0;index_y < 400;index_y += 1)
		{
			LCD_Fast_DrawPoint(index_x,index_y,0X534c);		//534Cƫ��ɫ��
		}
	}
	//���е�����ˮƽ��
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
void Set_BackGround(void)		//���ñ�����ɫΪ��ɫ
{
	POINT_COLOR = 0x5510;
    LCD_Clear(BLACK);
	LCD_DrawRectangle(0,0,798,401);//����	

}
void clear_point(u16 num)//������ʾ����ǰ��
{
	u16 index_clear_lie = 0; 
	POINT_COLOR = BLACK ;
	for(index_clear_lie = 1;index_clear_lie < 400;index_clear_lie++)
	{		
		LCD_DrawPoint(num,index_clear_lie );
	}
	if(!(num%50))//�ж�hang�Ƿ�Ϊ50�ı��� ���е�
	{
		for(index_clear_lie = 10;index_clear_lie < 400;index_clear_lie += 10)
		{		
			LCD_Fast_DrawPoint(num ,index_clear_lie,WHITE );
		}
	}
	if(!(num%10))//�ж�hang�Ƿ�Ϊ10�ı��� ���е�
	{
		for(index_clear_lie = 50;index_clear_lie <400;index_clear_lie += 50)
		{		
			LCD_Fast_DrawPoint(num ,index_clear_lie,WHITE );
		}
	}	
	POINT_COLOR = YELLOW;	
}


void DrawOscillogram(u16 *buff)//������ͼ
{
	static u16 Ypos1 = 0,Ypos2 = 0;
	static vu32 min_data=0;//buf[1];
	static vu32 n=0,con_t=0,con_t1=0;
  static	u16 i = 0;
	vu32 cc=0;
	int		temp1=0,temp2=0;
	u32 max_data1=buff[0], min_data1=buff[0], middle_data;
	POINT_COLOR = YELLOW;					//ʾ�����¶���ʾ���ֵ���ɫΪ��ɫ
	
	//��ȡADC����

	for(n = 0;n<800;n++)
	{
		 buff[n]=ADC_ConvertedValue[con_t+n-400] ;//Դ�����õ�
	}	
	
	for(n = 0;n<1000;n++)
	{
		 buff1[n]=ADC_ConvertedValue[con_t+n-500] ;//Դ�����õ�
	}	

	
	//temp֮ǰ�Ĵ���Ϊ���Ƶ�ʵģ���������������ʱ���temp�������������д������
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
	
	//�����ֵ
	for(i = 1;i < 800;i++)
	{
		if(buff[i]>max_data1)
			max_data1=buff[i];
		if(buff[i]<min_data1)
			min_data1=buff[i];
	}
	max_data2=max_data1;
	middle_data=min_data1+(max_data1-min_data1)/2;
	//��λ�����س�ʼλ��
	temp1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096);
	temp2=Yinit + Ypos2_bia - (middle_data * Y_sen_num[Y_sen] / 4096);
	if(temp1>400)			//������ʾ������ʾ������ײ���ʱ��ص�ԭλ��
	{
		Ypos2_bia=0;
	}
	if(temp2<0)			//������ʾ������ʾ���������ʱ��ص�ԭλ��
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
					Ypos2 =400; //������Χ����ʾ		
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
					Ypos2 =400; //������Χ����ʾ
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
					Ypos2 =400; //������Χ����ʾ
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
				Ypos2 =400; //������Χ����ʾ
			LCD_DrawLine (i ,Ypos1 , i+1 ,Ypos2);
			Ypos1 = Ypos2 ;
		}
	}
	Ypos1 = 0;
		
}



void DrawOscillogram_Clear(u16 *buff)//������ͼ
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
	//func_sta=1ʱ��������ʱ����=2ʱ��������ʱ��
	temp1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096);
	temp2=Yinit + Ypos2_bia - (middle_data * Y_sen_num[Y_sen] / 4096);
	if(temp1>400)			//������ʾ������ʾ������ײ���ʱ��ص�ԭλ��
	{
		Ypos2_bia=0;
	}
	if(temp2<0)			//������ʾ������ʾ���������ʱ��ص�ԭλ��
	{
		Ypos2_bia=0;
	}
	
	Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[1] * Y_sen_num[Y_sen] / 4096);	
	if(func_sta==2&&run_sta==0)		//����к���λ��ʱ����ʾ
	{
		if(Xpos_bia<0)
		{
			Ypos1=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[1+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);	
			for(i = 1;i < 800;i++)
			{
				Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff1[i+100-Xpos_bia] * Y_sen_num[Y_sen] / 4096);
				if(Ypos2 >400)
					Ypos2 =400; //������Χ����ʾ
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
					Ypos2 =400; //������Χ����ʾ
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
					Ypos2 =400; //������Χ����ʾ
				LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
				Ypos1 = Ypos2 ;
			}
		}
	}
	else				//������λ�Ƶ���ʾ
	{
		for(i = 1;i < 800;i++)
		{
			
			Ypos2=Yinit + Ypos2_bia + (middle_data * Y_sen_num[Y_sen] / 4096) - (buff[i] * Y_sen_num[Y_sen] / 4096);
			if(Ypos2 >400)
				Ypos2 =400; //������Χ����ʾ
			LCD_DrawLine_color (i ,Ypos1 , i+1 ,Ypos2,BLACK);
			Ypos1 = Ypos2 ;
		}
	}
	Ypos1 = 0;
}

float* Get_vpp(u16 *buf)	   //��ȡ���ֵ
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
	Vpp = Vpp*(3.3/4096);					//ADC�ɼ��ź�ת��ѹֵ��ʽ
	max=((float)(max_data))*(3.3/4096);
	min=((float)(min_data))*(3.3/4096);
	Data[0]=Vpp;
	Data[1]=max;
	Data[2]=min;
	return Data;	
}
///////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
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
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
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

static void Rheostat_ADC_GPIO_Config(void)	//����PA��ʱ�Ӻ�ADC1ʱ�ӣ�����PA5Ϊģ������
{
		GPIO_InitTypeDef GPIO_InitStructure;
	
	// ʹ�� GPIO ʱ��
	RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_GPIO_CLK, ENABLE);//ʹ��GPIOAʱ��
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ʱ��
		
	// ���� GPIO5�ṹ�����
	GPIO_InitStructure.GPIO_Pin = RHEOSTAT_ADC_GPIO_PIN;	//����ADC1�õ���PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;	    //����Ϊģ������
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ; //����������
	GPIO_Init(RHEOSTAT_ADC_GPIO_PORT, &GPIO_InitStructure);		
}


static void Rheostat_ADC_Mode_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
  // ------------------DMA Init �ṹ����� ��ʼ��--------------------------
  // ADC1ʹ��DMA2��������0��ͨ��0��������ֲ�̶�����
  // ����DMAʱ��
  RCC_AHB1PeriphClockCmd(RHEOSTAT_ADC_DMA_CLK, ENABLE); 
	// �����ַΪ��ADC ���ݼĴ�����ַ
	DMA_InitStructure.DMA_PeripheralBaseAddr = RHEOSTAT_ADC_DR_ADDR;	
  // �洢����ַ��ʵ���Ͼ���һ���ڲ�SRAM�ı���	
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&ADC_ConvertedValue;  
  // ���ݴ��䷽��Ϊ���赽�洢��	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;	
	// ��������СΪ��ָһ�δ����������
	DMA_InitStructure.DMA_BufferSize = 2400;	
	// ����Ĵ���ֻ��һ������ַ���õ���
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  // �洢����ַ�̶�
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; 
  // // �������ݴ�СΪ���֣��������ֽ� 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; 
  //	�洢�����ݴ�СҲΪ���֣����������ݴ�С��ͬ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	
	// ѭ������ģʽ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  // DMA ����ͨ�����ȼ�Ϊ�ߣ���ʹ��һ��DMAͨ��ʱ�����ȼ����ò�Ӱ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  // ��ֹDMA FIFO	��ʹ��ֱ��ģʽ
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;  
  // FIFO ��С��FIFOģʽ��ֹʱ�������������	
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
	// ѡ�� DMA ͨ����ͨ������������
  DMA_InitStructure.DMA_Channel = RHEOSTAT_ADC_DMA_CHANNEL; 
  //��ʼ��DMA�������൱��һ����Ĺܵ����ܵ������кܶ�ͨ��
	DMA_Init(RHEOSTAT_ADC_DMA_STREAM, &DMA_InitStructure);
   //����DMA�ж�
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TC); //����жϱ�־   
    DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE); //��������ж�                                       
    //DMA_Cmd(DMA2_Stream0, ENABLE); //ʹ��DMA
	// ʹ��DMA��
  DMA_Cmd(RHEOSTAT_ADC_DMA_STREAM, ENABLE);
	
	// ����ADCʱ��
	RCC_APB2PeriphClockCmd(RHEOSTAT_ADC_CLK , ENABLE);
	
	
	
  // -------------------ADC Common �ṹ�� ���� ��ʼ��------------------------
	// ����ADCģʽ
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  // ʱ��Ϊfpclk x��Ƶ	
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;	//Ԥ��Ƶ4��Ƶ��ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz 
  // ��ֹDMAֱ�ӷ���ģʽ		
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;		//DMAʧ��
  // ����ʱ����	
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  	//���������׶�֮����ӳ�5��ʱ��,����adc���õ���һ��adc�ò���
  ADC_CommonInit(&ADC_CommonInitStructure);	//��ʼ��
	
	
	
  // -------------------ADC Init �ṹ�� ���� ��ʼ��--------------------------
  // ADC �ֱ�������Ϊ12λ
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;	//12λģʽ
  // ��ֹɨ��ģʽ����ͨ���ɼ�����Ҫ	
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; 	//��ɨ��ģʽ	
	
  // ����ת��	
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;		//�ر�����ת��
 
  //��ֹ�ⲿ���ش���
  //ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;	//��ֹ������⣬ʹ���������
	
	//Դ���뽫��һ��ע�ͣ�ʹ��������
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising; //ʹ���ⲿ������ʱ�����ѡ�񴥷����ԣ������������ش���
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;  //��ʱ��3 TGRO�ⲿ��������ADC
	
  //ʹ������������ⲿ�����������ã�ע�͵�����
  //ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	
  //�����Ҷ���	
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //ת��ͨ�� 1����������������һ��ת��
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                    
  ADC_Init(RHEOSTAT_ADC, &ADC_InitStructure);
  //---------------------------------------------------------------------------
	
	
	
  // ���� ADC ͨ��ת��˳��Ϊ1����һ��ת��������ʱ��Ϊ3��ʱ������
  ADC_RegularChannelConfig(RHEOSTAT_ADC, RHEOSTAT_ADC_CHANNEL, 1, ADC_SampleTime_3Cycles);			//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��	

  // ʹ��DMA���� after last transfer (Single-ADC mode)
  ADC_DMARequestAfterLastTransferCmd(RHEOSTAT_ADC, ENABLE);
  // ʹ��ADC DMA
  ADC_DMACmd(RHEOSTAT_ADC, ENABLE);
	
	// ʹ��ADC
  ADC_Cmd(RHEOSTAT_ADC, ENABLE);  //����ADת����	
  //��ʼadcת�����������
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
    //ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);          
	

    //ʧ�ܶ�ʱ��
    TIM_Cmd(TIM3, DISABLE);
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); //��ʼ����ʱ��

    TIM_TimeBaseStructure.TIM_Prescaler = psc; //Ԥ��Ƶϵ��PSCΪ167
    TIM_TimeBaseStructure.TIM_Period = arr; //�Զ�װ��ֵARR
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷ�Ƶ����
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ; //���ϼ���ģʽ
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //��ʼ����ʱ��3
    
    //ʹ�ܶ�ʱ���ж�    
    TIM_ARRPreloadConfig(TIM3, ENABLE); //����TIM3��ʱ����
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);  //ѡ��TIM3��UPDATA�¼�����Ϊ����Դ
//    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //����TIM3�ж�����
     NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;  //DMA2_Stream0�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x00;  //��ռ���ȼ�0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority =1;        //�����ȼ�0,Դ����ʹ�õ��������ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;            //IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);    //����ָ���Ĳ�����ʼ��NVIC�Ĵ���
    //ʹ��TIM3
    TIM_Cmd(TIM3, ENABLE); 
}

void Time_init(void)
{
	ADCInit_Timer(1,83);		
}

void DMA2_Stream0_IRQHandler(void)  
{
    if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))  //�ж�DMA��������ж�  
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


//��ʱ��5�жϷ������	 
void TIM5_IRQHandler(void)
{ 		    
}
void EXTI0_IRQHandler(void)
{

}
 void TIM3_IRQHandler(void)
{
     if(TIM_GetITStatus(TIM3, TIM_IT_Update))  //�жϷ���update�¼��ж�  
    {        
 LED0=!LED0;	       
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //���update�¼��жϱ�־    
    }

}

