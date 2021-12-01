#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "lcd.h"
#include "touch.h"
#include <stdio.h>

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void USART_Configure(void);
void NVIC_Configure(void);

void Delay(void);

void sendDataUART1(uint16_t data);
void sendDataUART2(uint16_t data);
//---------------------------------------------------------------------------------------------------

TIM_OCInitTypeDef TIM_OCInitStructure3;
TIM_OCInitTypeDef TIM_OCInitStructure4;

volatile uint32_t ADC_Value[1];

uint32_t usTime = 0;

void Delay(uint32_t delayTime){
    uint32_t prev_time = usTime;
    while(1)
    {
        if(usTime - prev_time > delayTime) break;
    }
}

void RCC_Configure(void) {
// Alternate Function IO clock enable
// 외부 장치인 초음파 거리 센서나 라인 센서의 값을 통해 Interrupt를 주기 위해서는 필요
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  

// GPIO clock enable
// GPIO pin을 사용하기 위해서는 clock 인가가 필요
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
            RCC_APB2Periph_GPIOB |
            RCC_APB2Periph_GPIOC |
            RCC_APB2Periph_GPIOD |
            RCC_APB2Periph_GPIOE, ENABLE);

// USART1 ? APB2
// 기존에는 컴퓨터와 소통하기 위해 존재했으나 필요한지 고민
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

// USART2 ? APB1
// 블루투스를 통해 휴대폰과의 통신
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

// TIM2 clock enable
// 타이머 clock  2,3,4 인가
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 ,ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


// ADC1 enable
// 센서의 아날로그 값을 받아서 디지털로 바꾸기 위해서는 필요
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
   
    /* UART1 pin setting */
    //TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   //RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* UART2 pin setting */
    //TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   //RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // LED
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //  -PWM 관련-
    // TIM3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  
    // TIM4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}



void USART_Configure(void)
{
    USART_InitTypeDef USART1_InitStructure;

    // Enable the USART1 peripheral
    USART_Cmd(USART1, ENABLE);
   
    // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    USART1_InitStructure.USART_BaudRate = 9600;
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART1_InitStructure.USART_StopBits = USART_StopBits_1;
    USART1_InitStructure.USART_Parity = USART_Parity_No;
    USART1_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART1, &USART1_InitStructure);
   
    // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);    
        
    //  USART2
    USART_InitTypeDef USART2_InitStructure;

    // Enable the USART1 peripheral
    USART_Cmd(USART2, ENABLE);
   
    // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    USART2_InitStructure.USART_BaudRate = 9600;
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART2_InitStructure.USART_StopBits = USART_StopBits_1;
    USART2_InitStructure.USART_Parity = USART_Parity_No;
    USART2_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &USART2_InitStructure);

    // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
   
    // UART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // USART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; // TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01; // TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // TIMER2
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 우선순위가 가장 높다.
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM2_Configure(void) {
  
    // 짧은 시간 많은 감지가 필요하므로 1us를 만든다고 가정
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    TIM_InitStructure.TIM_Prescaler = 72;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_Period = 1;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM3_Configure(){
    //int prescale = (uint16_t) (SystemCoreClock / 1000000);
  
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    TIM_TimeBaseStructure.TIM_Period = 2500; 
    TIM_TimeBaseStructure.TIM_Prescaler = 216;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_OCInitStructure3.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure3.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure3.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure3.TIM_Pulse = 1000; // us
  
    TIM_OC1Init(TIM3, &TIM_OCInitStructure3);
    TIM_OC2Init(TIM3, &TIM_OCInitStructure3);
    TIM_OC3Init(TIM3, &TIM_OCInitStructure3);
    TIM_OC4Init(TIM3, &TIM_OCInitStructure3);
  
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void TIM4_Configure(void){
   
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
   
    // 한 주기를 20ms로 설정함 
    TIM_TimeBaseInitStructure.TIM_Period=2500;
    TIM_TimeBaseInitStructure.TIM_Prescaler=216;
    TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Down;
    TIM_TimeBaseInitStructure.TIM_ClockDivision=0; // TIM_CKD_DIV1=0
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
   
    TIM_OCInitStructure4.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure4.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure4.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure4.TIM_Pulse = 1000; // Duty Cycle = 2000/20000 = 10%
    
    TIM_OC1Init(TIM4, &TIM_OCInitStructure4);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
   
    TIM_OC2Init(TIM4, &TIM_OCInitStructure4);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);
   
    TIM_OC3Init(TIM4, &TIM_OCInitStructure4);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);
   
    TIM_OC4Init(TIM4, &TIM_OCInitStructure4);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);
    
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void USART1_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
        // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);

        while(USART_GetFlagStatus(USART2,USART_FLAG_TXE)==RESET);
            sendDataUART2(word);
        
        // TODO implement
    }
    // clear 'Read data register not empty' flag
    USART_ClearITPendingBit(USART1,USART_IT_RXNE);
}

void USART2_IRQHandler() {
    uint16_t word;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET){
       // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART2);
        
        // TODO implement
        while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
            //sendDataUART1(word);
        if(word == '1'){
          GPIO_SetBits(GPIOD,GPIO_Pin_2);
        }
        if(word == '3'){
          GPIO_ResetBits(GPIOD,GPIO_Pin_2);
        }
           
       
    }
    // clear 'Read data register not empty' flag
    USART_ClearITPendingBit(USART2,USART_IT_RXNE);
}

void TIM2_IRQHandler() {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    //printf("%d",usTime);
    usTime++; // 1us마다 Interrupt가 걸리도록 설정해두었으니 usTime을 측정하는 변수
    
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

//TIM을 1us로 맞추는 걸로 수정
// E2, E3
int readDistance(uint16_t GPIO_PIN_TRIG, uint16_t GPIO_PIN_ECHO){
    uint32_t prev = 0;
    GPIO_SetBits(GPIOE, GPIO_PIN_TRIG);
    GPIO_ResetBits(GPIOE, GPIO_PIN_ECHO);
    Delay(10);
    GPIO_ResetBits(GPIOE, GPIO_PIN_TRIG);
    //uint8_t val = GPIO_ReadInputDataBit(GPIOE, GPIO_PIN_ECHO);

    /* 버스트 발생 직후 에코는 HIGH 레벨을 가진다.
    따라서 버스트가 발생했는지 알기 위해 while문을 통해
    에코가 LOW 레벨(RESET)을 가질 때(버스트 발생 X)는 반복문에 머물게 하고 
    에코가 HIGH 레벨(SET)을 가질 때(버스트 발생)는 반복문을 탈출한다.*/  
    while(GPIO_ReadInputDataBit(GPIOE, GPIO_PIN_ECHO) == RESET);
    
    // 반복문을 탈출한 이후엔 시간 측정을 위해 prev 변수에 현재 시각을 저장한다.
    prev = usTime; 
    
    /* 에코에 버스트가 다시 들어오면 에코는 LOW 레벨을 가진다.
    따라서 에코가 HIGH 레벨(SET)일 동안은 아직 버스트가 돌아 오지 않은 거니까
    반복문에 머물게 하고 에코가 LOW 레벨을 가졌을 땐 버스트가 들어왔다는
    의미니까 반복문을 탈출해 거리를 계산한다.*/ 
    while(GPIO_ReadInputDataBit(GPIOE, GPIO_PIN_ECHO) != RESET);

    // 거리는 (버스트 왕복거리) / 2 / 0.034cm/us 로 구해진다.
    int distance = (usTime - prev)*34/1000;

    return distance;

}

// PWM 테스트용 
//채널 3는 PB0, 채널 4는 PB1에 매핑.
void testPWM(int pulse1, int pulse2){
    TIM_OCInitStructure3.TIM_Pulse = pulse1;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure3);
  
    TIM_OCInitStructure3.TIM_Pulse = pulse2;
    TIM_OC4Init(TIM3, &TIM_OCInitStructure3);
}

void testPWM2(int pulse1, int pulse2){
    TIM_OCInitStructure3.TIM_Pulse = pulse1;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure3);
  
    TIM_OCInitStructure3.TIM_Pulse = pulse2;
    TIM_OC2Init(TIM3, &TIM_OCInitStructure3);
}

void testPWM3(int pulse1, int pulse2){
    TIM_OCInitStructure4.TIM_Pulse = pulse1;
    TIM_OC1Init(TIM4, &TIM_OCInitStructure4);
  
    TIM_OCInitStructure4.TIM_Pulse = pulse2;
    TIM_OC2Init(TIM4, &TIM_OCInitStructure4);
}

void testPWM4(int pulse1, int pulse2){
    TIM_OCInitStructure4.TIM_Pulse = pulse1;
    TIM_OC3Init(TIM4, &TIM_OCInitStructure4);
  
    TIM_OCInitStructure4.TIM_Pulse = pulse2;
    TIM_OC4Init(TIM4, &TIM_OCInitStructure4);
}

void NormalDelay(void) {
   int i;
   for (i = 0; i < 2000000; i++);
}

void sendDataUART1(uint16_t data) {
   /* Wait till TC is set */
   //while ((USART1->SR & USART_SR_TC) == 0);
   USART_SendData(USART1, data);
}

void sendDataUART2(uint16_t data) {
   /* Wait till TC is set */
   //while ((USART1->SR & USART_SR_TC) == 0);
   USART_SendData(USART2, data);
}

int main(void)
{
    
    printf("TEST!");
    SystemInit();

    RCC_Configure();

    ADC_Configure();
    NVIC_Configure();
    TIM2_Configure();
    TIM3_Configure();
    TIM4_Configure();
    GPIO_Configure();


    NormalDelay();

    
      
    
    
    testPWM(0,100);
    testPWM2(0,100);
    
    
    
    testPWM3(0,12500);
    testPWM4(0,12500);


    // NormalDelay();
    // NormalDelay();
    // NormalDelay();
    
    
    // NormalDelay();
    // NormalDelay();
    // NormalDelay();
    
    // testPWM(0,200);
  
    /*GPIO_ResetBits(GPIOC, GPIO_Pin_10); // Back Right -
    GPIO_SetBits(GPIOC, GPIO_Pin_11); // Back_Right +

    
    GPIO_ResetBits(GPIOC, GPIO_Pin_6); // Front Right -
    GPIO_SetBits(GPIOC, GPIO_Pin_7); // Front Right +
    
    GPIO_ResetBits(GPIOC, GPIO_Pin_8); // Front Left -
    GPIO_SetBits(GPIOC, GPIO_Pin_9); // Front Left +   
      */
      
    while (1) {
      GPIO_ResetBits(GPIOD, GPIO_Pin_2);
      GPIO_ResetBits(GPIOD, GPIO_Pin_3);
      GPIO_ResetBits(GPIOD, GPIO_Pin_4);
      GPIO_ResetBits(GPIOD, GPIO_Pin_7);
      printf("%d\n",usTime);
      uint32_t v1 = readDistance(GPIO_Pin_2,GPIO_Pin_3); // 전방
      uint32_t v2 = readDistance(GPIO_Pin_4,GPIO_Pin_5); //  우측 전방
      uint32_t v3 = readDistance(GPIO_Pin_8,GPIO_Pin_9); //  우측 후방
      uint32_t v4 = readDistance(GPIO_Pin_10,GPIO_Pin_11); // 후방
      uint32_t v5 = readDistance(GPIO_Pin_12,GPIO_Pin_13); // 좌측 후방
      uint32_t v6 = readDistance(GPIO_Pin_6,GPIO_Pin_7); // 좌측 전방
      
      /*if(v1 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_2);
        printf("전방 감지\n");
      }
      
      if(v4 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_7);
        printf("후방 감지\n");
      }
     
      if(v6 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_3);
        printf("좌측 전방 감지\n");
      }
      
      if(v5 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_4);
        printf("좌측 후방 감지\n");
      }*/
      
      if(v2 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_2);
        printf("우측 전방 감지\n");
      }
      
      if(v3 < 10){
        GPIO_SetBits(GPIOD, GPIO_Pin_3);
        printf("우측 후방 감지\n");
      }
      
     
      
      printf("Sensor1: %d\n",v1);
      printf("Sensor2: %d\n",v2);
      printf("Sensor3: %d\n",v3);
      printf("Sensor4: %d\n",v4);
      printf("Sensor5: %d\n",v5);
      printf("Sensor6: %d\n",v6);
      
      NormalDelay();
    }
    return 0;
}