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

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void USART_Configure(void);
void NVIC_Configure(void);

void Delay(void);

void sendDataUART1(uint16_t data);
void sendDataUART2(uint16_t data);
//---------------------------------------------------------------------------------------------------

int sensorValue = 100;
void RCC_Configure(void)
{
    // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
                           | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE
                             | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    

    /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    // Timer RCC Configure
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'
   
    /* UART1 pin setting */
    //TX
    //ADC
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // Analog input
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // LED 1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // LED 2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

// Analog To Digital -> 센서 값  받을 때만 사용
// void ADC_Configure(){
  
//     ADC_InitTypeDef ADC_Initstruct;
//     ADC_Initstruct.ADC_Mode = ADC_Mode_Independent;
//     ADC_Initstruct.ADC_ScanConvMode = DISABLE;
//     ADC_Initstruct.ADC_ContinuousConvMode = ENABLE;
//     ADC_Initstruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//     ADC_Initstruct.ADC_DataAlign = ADC_DataAlign_Right;
//     ADC_Initstruct.ADC_NbrOfChannel = 1;

//     ADC_Init(ADC1, &ADC_Initstruct);
        
//     ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
//     ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
//     ADC_Cmd(ADC1, ENABLE);

//     ADC_ResetCalibration(ADC1);
//     while (ADC_GetResetCalibrationStatus(ADC1));
//     ADC_StartCalibration(ADC1);
//     while (ADC_GetCalibrationStatus(ADC1));

//     ADC_SoftwareStartConvCmd(ADC1, ENABLE);
            
// }

void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
   
    // ADC1
    // 'NVIC_EnableIRQ' is only required for USART setting
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; // TIM2 global Interrupt
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
}

void TIMER2_Configure(void) {

    // 1초 생성
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // timer_base
    TIM_TimeBaseStructure.TIM_Prescaler = 10000;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseStructure.TIM_Period = 7200;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

}

int flag1 = 0;
int flag2 = 0;
int count = 0;
int btnFlag = 0;
void TIM2_IRQHandler(void) {

    // 조건이 충족되는 동안을 의미
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        count++;
        if(flag1 == 0) {
            flag1 = 1;
        } else {
            flag1 = 0;
        }

        if(count % 5 == 0) {
            if(flag2 == 0) {
                flag2 = 1;
            } else {
                flag2 = 0;
            }
            count = 0;
        }
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

}



// 조도센서에 사용
// void ADC1_2_IRQHandler() {
//   if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET){
//     sensorValue = ADC_GetConversionValue(ADC1);
//   }
//   ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
// }

void Delay(void) {
   int i;
   for (i = 0; i < 2000000; i++);
}


int color[12] = {WHITE,CYAN,BLUE,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int main(void)
{
    
    SystemInit();

    RCC_Configure();

    GPIO_Configure();

    // ADC_Configure();

    NVIC_Configure();

    TIMER2_Configure();
    
    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);
    Delay();

    LCD_ShowString(80, 80, "Mon_Team 06", BLACK, WHITE);
    LCD_DrawRectangle(80,120,140,160);
    
    u16 x_in, y_in, x_out, y_out;
    while (1) {
        if (!T_INT) {
            Touch_GetXY(&x_in, &y_in, 0);
            Convert_Pos(x_in, y_in, &x_out, &y_out);
            if(x_out > 80 && x_out < 140 && y_out > 120 && y_out < 160) {
                if(btnFlag == 0) {
                    LCD_ShowString(90, 100, "ON ", BLACK, WHITE); 
                    btnFlag = 1;
                    count = 0;
                    flag1 = 1;
                    flag2 = 1;
                }
                else {
                    LCD_ShowString(90, 100, "OFF", BLACK, WHITE);
                    btnFlag = 0;
                }
            }            
        }
        (btnFlag*flag1) ? GPIO_SetBits(GPIOD, GPIO_Pin_2) : GPIO_ResetBits(GPIOD, GPIO_Pin_2);
        (btnFlag*flag2) ? GPIO_SetBits(GPIOD, GPIO_Pin_3) : GPIO_ResetBits(GPIOD, GPIO_Pin_3);
    }
    return 0;

}