#include "key.h"
#include "delay.h" 
#include "led.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//按键输入驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 
 
//按键初始化函数
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(KEY0_CLOCK|KEY1_CLOCK|KEY2_CLOCK|KEY3_CLOCK|WK_UP_CLOCK|KEYP_CLOCK, ENABLE);//使能GPIOA,GPIOE时钟
 
  GPIO_InitStructure.GPIO_Pin = KEY0_PIN|KEY1_PIN|KEY2_PIN; //KEY0 KEY1 KEY2对应引脚
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(KEY0_PORT, &GPIO_InitStructure);//初始化GPIOE2,3,4
	
	 
  GPIO_InitStructure.GPIO_Pin = WK_UP_PIN;//WK_UP对应引脚PA0
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN ;//下拉
  GPIO_Init(WK_UP_PORT, &GPIO_InitStructure);//初始化GPIOA0
 
} 
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，WKUP按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	u8 num=0;
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		delay_ms(100);//去抖动 
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




















