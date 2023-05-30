#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h" 
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
/*下面的方式是通过直接操作库函数方式读取IO*/
 #define KEY0_PORT    GPIOE
 #define KEY1_PORT    GPIOE
 #define KEY2_PORT    GPIOE
 #define KEY3_PORT    GPIOE
 #define KEY0_PORT    GPIOE
 #define WK_UP_PORT    GPIOA
 #define KEYP_PORT    GPIOF
 #define KEY0_PIN      GPIO_Pin_4
 #define KEY1_PIN      GPIO_Pin_3
 #define KEY2_PIN      GPIO_Pin_2
 #define WK_UP_PIN      GPIO_Pin_0
 #define KEYP_PIN      GPIO_Pin_3
 #define KEY0_CLOCK      RCC_AHB1Periph_GPIOE
 #define KEY1_CLOCK      RCC_AHB1Periph_GPIOE
 #define KEY2_CLOCK      RCC_AHB1Periph_GPIOE
 #define KEY3_CLOCK      RCC_AHB1Periph_GPIOE
 #define WK_UP_CLOCK      RCC_AHB1Periph_GPIOA
 #define KEYP_CLOCK      RCC_AHB1Periph_GPIOF
 #define KEY0 		GPIO_ReadInputDataBit(KEY0_PORT,KEY0_PIN) //PE4
 #define KEY1 		GPIO_ReadInputDataBit(KEY1_PORT,KEY1_PIN)	//PE3 
 #define KEY2 		GPIO_ReadInputDataBit(KEY2_PORT,KEY2_PIN) //PE2
 #define WK_UP 	GPIO_ReadInputDataBit(WK_UP_PORT,WK_UP_PIN)	//PA0
 #define KEYP 		GPIO_ReadInputDataBit(KEYP_PORT,KEYP_PIN) //PF3
 #define KEY_OFF 1
 #define KEY_ON  0


/*下面方式是通过位带操作方式读取IO*/
/*
#define KEY0 		PEin(4)   	//PE4
#define KEY1 		PEin(3)		//PE3 
#define KEY2 		PEin(2)		//P32
#define WK_UP 	PAin(0)		//PA0
*/


#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES   4

void KEY_Init(void);	//IO初始化
u8 KEY_Scan(u8);  		//按键扫描函数	

#endif
