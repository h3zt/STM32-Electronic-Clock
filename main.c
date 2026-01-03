#include "KEY.h"
#include "TIMER.h"

#define KEY1  PAin(11)
#define KEY2  PAin(12)

uint8_t SEG_TABLE[16]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
uint8_t SEG_BUF[8];


uint8_t SEG_DISP_MODE = 0;//界面切换
int8_t Hour = 0;
int8_t Min = 0;
int8_t Sec = 0;

uint8_t Key_Old,Key_Down;

void KEY_Init()
{
  GPIO_InitTypeDef GPIO_InitStruct; 
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
  
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;//下拉输入模式
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_12;
  GPIO_Init(GPIOA,&GPIO_InitStruct);  
}


void SEG_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启GPIOA时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//开启GPIOB时钟
  
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOA,&GPIO_InitStruct); 

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_Init(GPIOB,&GPIO_InitStruct);  
}
void TIME_Init(void)//定时器初始化
{
  NVIC_InitTypeDef NVIC_InitStruct;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//开启TIME2时钟
  
  TIM_InternalClockConfig(TIM2);//选择内部时钟源
  
  TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//选择向上计数模式
  TIM_TimeBaseInitStruct.TIM_Period = 1000 - 1;//1M / 1000 = 1ms
  TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;//8M / 8 = 1M
  TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0; //重复计数器（高级定时器才包含）
  TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);//配置完定时器后会立即产生一个更新标志位
  TIM_ClearFlag(TIM2,TIM_IT_Update);//清除更新标志，防止上电立即进入一次中断
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组
  
  NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;//选择定时器2中断通道
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;//设置响应优先级
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;//设置抢占优先级
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;//使能
  NVIC_Init(&NVIC_InitStruct);
  
  TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//使能TIM2更新中断
  TIM_Cmd(TIM2,ENABLE);//使能TIM2
}

void KEY_SCAN()
{
  uint8_t  key_temp = 0;
  
  if(KEY1 == 0)key_temp=1;
  if(KEY2 == 0)key_temp=2;
  
  Key_Down = key_temp & (key_temp ^ Key_Old);//按下只触发一次动作
  Key_Old = key_temp;
  
  if(Key_Down == 1)
  {
    if(++SEG_DISP_MODE > 2)
    {
      SEG_DISP_MODE = 0;
    }
  }    
  else if(Key_Down == 2)
  {
    if(SEG_DISP_MODE == 1)
    {
      if(++Min > 59)
      {
        Min = 0;
      }
    }
    else if(SEG_DISP_MODE == 2)
    {
      if(++Hour > 23)
      {
        Min = 0;
      }
    }
  }
}

void DIP_SCAN()
{
  switch(SEG_DISP_MODE)
  {
    case 0://时钟界面
        SEG_BUF[0] = SEG_TABLE[Hour/10];
        SEG_BUF[1] = SEG_TABLE[Hour%10];
        SEG_BUF[2] = 0x40;
        SEG_BUF[3] = SEG_TABLE[Min/10];
        SEG_BUF[4] = SEG_TABLE[Min%10];
        SEG_BUF[5] = 0x40;
        SEG_BUF[6] = SEG_TABLE[Sec/10];
        SEG_BUF[7] = SEG_TABLE[Sec%10]; 
      break;
    case 1://
        SEG_BUF[0] = 0x00;
        SEG_BUF[1] = 0X00;
        SEG_BUF[2] = 0x00;
        SEG_BUF[3] = SEG_TABLE[Min/10];
        SEG_BUF[4] = SEG_TABLE[Min%10];
        SEG_BUF[5] = 0x00;
        SEG_BUF[6] = 0X00;
        SEG_BUF[7] = 0X00;           
      break;
    case 2://
        SEG_BUF[0] = SEG_TABLE[Hour/10];
        SEG_BUF[1] = SEG_TABLE[Hour%10];
        SEG_BUF[2] = 0X00;
        SEG_BUF[3] = 0X00;
        SEG_BUF[4] = 0X00;
        SEG_BUF[5] = 0x00;
        SEG_BUF[6] = 0X00;
        SEG_BUF[7] = 0X00;       
      break;    
  }
}


int main(void)
{	
  SEG_Init();
  KEY_Init();
  TIME_Init();
	while(1)
	{	
    DIP_SCAN();
    KEY_SCAN();
	}
}

void SEG_Scan(void)
{
  static uint16_t Seg_Temp = 0x0001;
  static uint8_t Seg_Count = 0;
  uint16_t temp = 0;
  
  GPIO_Write(GPIOA,0xFF00);
  
  temp = GPIO_ReadOutputData(GPIOA);
  temp &= 0xFF00;
  
  GPIO_Write(GPIOA,(temp|SEG_BUF[Seg_Count]));
  GPIO_Write(GPIOB,~Seg_Temp);
  Seg_Temp <<= 1;
  if(++Seg_Count >= 8)
  {
    Seg_Count = 0;
    Seg_Temp = 0x0001;
  } 
}

void TIM2_IRQHandler(void)
{
  static uint16_t Time_Count = 0;
  
  if(TIM_GetITStatus(TIM2,TIM_IT_Update) == SET)//等待产生更新标志
  {
    SEG_Scan();
    if(++Time_Count >= 1000)
    {
      Time_Count = 0;
      if(SEG_DISP_MODE == 0)
      {
        if(++Sec >= 60)
        {
          Sec = 0;
          if(++Min >= 60)
          {
            Min = 0;
            if(++Hour >= 24)
            {
              Hour = 0;
            }
          }
        }
      }
    }
   TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除更新标志  
  }
}
