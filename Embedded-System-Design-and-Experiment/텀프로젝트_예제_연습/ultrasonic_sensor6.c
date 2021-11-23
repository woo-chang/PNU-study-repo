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
// 타이머 clock 인가
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

// ADC1 enable
// 센서의 아날로그 값을 받아서 디지털로 바꾸기 위해서는 필요
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  
}


void ADC_Configure(){
  
    ADC_InitTypeDef ADC_Initstruct;
    ADC_Initstruct.ADC_Mode = ADC_Mode_Independent;
    ADC_Initstruct.ADC_ScanConvMode = DISABLE; // 채널 여러개 쓰면 ENABLE
    ADC_Initstruct.ADC_ContinuousConvMode = ENABLE;
    ADC_Initstruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_Initstruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Initstruct.ADC_NbrOfChannel = 1; // 사용하는 채널 개수

    ADC_Init(ADC1, &ADC_Initstruct);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
    ADC_DMACmd(ADC1, ENABLE); // ADC_ITConfig 사용 X, ADC1의 DMA Request Enable
    //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1)); // 상태확인
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);     
}


void GPIO_Configure(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

// USART Pin Setting
// USART1 TX, USART2 TX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 외부와의 연결이므로 AF
  GPIO_Init(GPIOA, &GPIO_InitStructure);

// USART1 RX, USART2 RX
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // input pull down
  GPIO_Init(GPIOA, &GPIO_InitStructure);

// 위 코드는 실험할 때 사용했던 코드이고 아래 코드는 다른 방법도 존재
  //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 // GPIO_InitStructure.GPIO_Mode = GPIO_IN_FLOATING; // 버튼이나 센서에 사용한다고 함
//

// 초음파 센서 6개
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12; // TRIG
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 초음파 발사
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13; // Echo
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // input pull down 초음파 받기
  GPIO_Init(GPIOE, &GPIO_InitStructure);

// 라인 센서 2개
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // input pull donw 라인 값 받기
  GPIO_Init(GPIOE, &GPIO_InitStructure);



// 모터 8개
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

// LED
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  
  
}


void USART_Configure(void) {
  USART_InitTypeDef USART_InitStructure;

// USART1
  USART_Cmd(USART1, ENABLE);

  USART_InitStructure.USART_BaudRate = 9600; // 데이터 전송 속도 지정
// 프레임에서 전송되거나 수신되는 데이터 비트 수를 지정
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
// 단어의 끝을 의미하는 Stop Bit 개수를 설정
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
// 패리티(오류 검출) 활성화, 비활성화 결정 -> MSB가 패리티 비트
  USART_InitStructure.USART_Parity = USART_Parity_No;
// 어떤 Mode 사용할지 지정 (수신 또는 전송 모드를 활성화 또는 비활성화)
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
// 하드웨어 흐름 제어모드를 사용하거나 사용하지 않도록 지정
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_Init(USART1, &USART_InitStructure);

// RX(수신부분)에 대하여 Interrupt 처리 가능
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

// USART2
  USART_Cmd(USART2, ENABLE);

  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_Init(USART2, &USART_InitStructure);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

// 인터럽트의 우선순위를 결정
void NVIC_Configure(void) {
  NVIC_InitTypeDef NVIC_InitStructure;

  // Priority Group을 지원
  // 아래의 사진을 참고
  // 우선순위 그룹을 정하고 그 안에서의 sub 우선순위를 정할 수 있음
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  // USART1
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; // 0보다는 우선순위가 낮다
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00; // 우선순위가 가장 높음!
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // USART2
  NVIC_EnableIRQ(USART2_IRQn);
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  // 요런식으로 두 개의 우선순위가 모두 같으면 경쟁하지 않고 순서대로 처리
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

// IRQ Handler
void USART1_IRQHandler() {
  uint32_t word;

  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
      word = USART_ReceiveData(USART1);
      USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

void USAR2_IRQHandler() {
  uint32_t word;

  if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) {
    word = USART_ReceiveData(USART2);
    // 블루투스에 들어오는 값에 의해 if문으로 처리하는 코드를 아래에 작성

    USAR_SendData(USART2, word); // USART2가 받은 데이터를 USART2로 전달
    // 핸드폰에서 받아서 확인하기 위해
    while ((USART2->SR & USART_SR_TC) == 0); // 데이터가 송수신 상태가 아니고 전송 완료되었을 때 이 Handler를 중지
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
  }
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


void NormalDelay(void) {
   int i;
   for (i = 0; i < 2000000; i++);
}

int main(void)
{
    printf("TEST!");
    SystemInit();

    RCC_Configure();

    ADC_Configure();
    NVIC_Configure();
    TIM2_Configure();
    
    GPIO_Configure();


    NormalDelay();


    GPIO_ResetBits(GPIOC, GPIO_Pin_10); // Back Right -
    GPIO_SetBits(GPIOC, GPIO_Pin_11); // Back_Right +
    
    GPIO_ResetBits(GPIOD, GPIO_Pin_14); // Back Left -
    GPIO_SetBits(GPIOD, GPIO_Pin_15); // Back Left +
    
    GPIO_ResetBits(GPIOC, GPIO_Pin_6); // Front Right -
    GPIO_SetBits(GPIOC, GPIO_Pin_7); // Front Right +
    
    GPIO_ResetBits(GPIOC, GPIO_Pin_8); // Front Left -
    GPIO_SetBits(GPIOC, GPIO_Pin_9); // Front Left +   
      
      
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