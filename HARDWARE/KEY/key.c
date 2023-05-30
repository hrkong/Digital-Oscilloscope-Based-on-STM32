#include "key.h"
#include "delay.h" 
#include "led.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//����������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
 
//������ʼ������
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(KEY0_CLOCK|KEY1_CLOCK|KEY2_CLOCK|KEY3_CLOCK|WK_UP_CLOCK|KEYP_CLOCK, ENABLE);//ʹ��GPIOA,GPIOEʱ��
 
  GPIO_InitStructure.GPIO_Pin = KEY0_PIN|KEY1_PIN|KEY2_PIN; //KEY0 KEY1 KEY2��Ӧ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(KEY0_PORT, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
	
	 
  GPIO_InitStructure.GPIO_Pin = WK_UP_PIN;//WK_UP��Ӧ����PA0
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//����
  GPIO_Init(WK_UP_PORT, &GPIO_InitStructure);//��ʼ��GPIOA0
 
} 
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��WKUP���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	u8 num=0;
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		delay_ms(100);//ȥ���� 
      //while(KEY0==KEY_ON);   
		key_up=0;
		if(KEY0==0)
      {
				num=1;
				LED1=!LED1;
				delay_ms(100);
				LED1=!LED1;
      }
		else if(KEY1==0)
      {
				num=2;
				LED1=!LED1;
				delay_ms(100);
				LED1=!LED1;
      }
		else if(KEY2==0)
      {
				num=3;
				LED1=!LED1;
				delay_ms(100);
				LED1=!LED1;
				
      }
		else if(WK_UP==1)
      {
				num=4;
				LED1=!LED1;
				delay_ms(100);
				LED1=!LED1;
      }
	}
   else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)
   {
      key_up=1; 	    
	
   }
	 
	 return num;
}




















