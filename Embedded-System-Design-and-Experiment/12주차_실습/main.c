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

// ADC 값을 저장할 공간
// volatile : 저장할 공간을 항상 참조하도록
volatile uint32_t ADC_Value[1];

void RCC_Configure(void)
{
    // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
                           | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE
                             | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    
    
    /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // DMA1
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        
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

void DMA_Configure(void) {
    DMA_InitTypeDef DMA_InitStructure;
    DMA_DeInit(DMA1_Channel1); // 1-7중에서 1번 선택
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR; // 어디에 있는 걸 가져올 지(ADC->DR = conversion 된 데이터 결과 쓰여짐)
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value[0]; // 가져온 걸 어디에 쓸 지
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE); // 채널 Enable 할지 Disable 할 지 결정
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
   // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
   
    /* UART1 pin setting */
    //TX
    //ADC
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // Analog input
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_Init(GPIOC, &GPIO_InitStructure);
  
}

void Delay(void) {
   int i;
   for (i = 0; i < 2000000; i++);
}

int color[12] = {WHITE,CYAN,BLUE,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int main(void)
{
    
    SystemInit();

    RCC_Configure();

    ADC_Configure();

    DMA_Configure();

    GPIO_Configure();
    
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);
    Delay();

    LCD_Fill(0, 0, 240, 320,BLACK);
    
    while (1) {
      
      if (ADC_Value[0] < 3000) {
          LCD_Fill(0, 0, 240, 320, GRAY);
          LCD_ShowNum(100, 120, ADC_Value[0], 5, WHITE, GRAY);
          
      } else {
          LCD_Fill(0, 0, 240, 320,WHITE);
          LCD_ShowNum(100, 120, ADC_Value[0], 5, GRAY, WHITE);
      }
      Delay();
    }
    return 0;
}