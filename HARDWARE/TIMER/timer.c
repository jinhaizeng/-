#include "timer.h"
#include "led.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//定时器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/4
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 



TIM_ICInitTypeDef  TIM5_ICInitStructure;
u32 TIM5_CH1_COUNTER=0;
u32 TIM3_COUNTER=0;
u32 TIM5_COUNTER=0;
u32 TIM3_CUR=0;


//通用定时器 3 中断初始化
//arr：自动重装值。 psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器 3!
void TIM3_Int_Init(u16 arr,u16 psc)
{
TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); //①使能 TIM3 时钟
TIM_TimeBaseInitStructure.TIM_Period = arr; //自动重装载值
TIM_TimeBaseInitStructure.TIM_Prescaler=psc; //定时器分频
TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);// ②初始化定时器 TIM3
TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //③允许定时器 3 更新中断
NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器 3 中断
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级 1
NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //响应优先级 3
NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
NVIC_Init(&NVIC_InitStructure);// ④初始化 NVIC
TIM_Cmd(TIM3,ENABLE); //⑤使能定时器 3
TIM3_CUR=TIM_TimeBaseInitStructure.TIM_Prescaler;
}
//定时器 3 中断服务函数
void TIM3_IRQHandler(void)
{
if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
{
    TIM_Cmd(TIM5,DISABLE ); 	//关闭定时器5
    TIM_Cmd(TIM3,DISABLE ); 	//关闭定时器5	
    TIM3_COUNTER++;
    TIM_Cmd(TIM5,ENABLE ); 	//关闭定时器5
    TIM_Cmd(TIM3,ENABLE ); 	//关闭定时器5	
}
TIM_ClearITPendingBit(TIM3,TIM_IT_Update); //清除中断标志位
}


//定时器5通道1输入捕获配置
//arr：自动重装值(TIM2,TIM5是32位的!!)
//psc：时钟预分频数
void TIM5_CH1_Cap_Init(u32 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  	//TIM5时钟使能    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//使能PORTA时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //GPIOA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; //下拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA0

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM5); //PA0复用位定时器5
  
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period=arr;   //自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseStructure);
	

	//初始化TIM5输入捕获参数
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  TIM5_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  TIM_ICInit(TIM5, &TIM5_ICInitStructure);
		
	TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
  

 
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器、
	
	TIM_Cmd(TIM5,ENABLE ); 	//使能定时器5
}
//捕获状态
//[7]:0,没有成功的捕获;1,成功捕获到一次.
//[6]:0,还没捕获到低电平;1,已经捕获到低电平了.
//[5:0]:捕获低电平后溢出的次数(对于32位定时器来说,1us计数器加1,溢出时间:4294秒)
u8  TIM5CH1_CAPTURE_STA=0;	//输入捕获状态		    				
u32	TIM5CH1_CAPTURE_VAL;	//输入捕获值(TIM2/TIM5是32位)
//定时器5中断服务程序	 
void TIM5_IRQHandler(void)
{ 		    


	if(TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
	{	
			
			TIM_Cmd(TIM5,DISABLE ); 	//关闭定时器5
      TIM_Cmd(TIM3,DISABLE ); 	//关闭定时器5	 
      TIM5_COUNTER++; 
      TIM_Cmd(TIM3,ENABLE ); 	//关闭定时器5	 
			TIM_Cmd(TIM5,ENABLE ); 	//使能定时器5	    
	}			     	    					   
 	
	TIM_ClearITPendingBit(TIM5, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
}

