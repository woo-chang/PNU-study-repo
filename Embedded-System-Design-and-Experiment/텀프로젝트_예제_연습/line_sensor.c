#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "misc.h"
#include "lcd.h"
#include "touch.h"


void RCC_Configure(void) {
   // GPIOA, PA2,3 : USART2 TX,RX (+ PA9,10 : USART1 TX,RX)
   // GPIOB
   // GPIOC, PC6-11 : 3 Motors
   // GPIOD, PD2-4,7 : LED, PD11 : S1 Button, PD14,15 : 1 Motor
   // GPIOE, PE2-13 : 6 Ultrasonic Sensors, PE14,15 : 2 Line Tracing Sensors 
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                     RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                        RCC_APB2Periph_GPIOE, ENABLE);
   
   // Alternate Function IO clock enable
   // 외부 장치인 초음파 거리 센서나 라인 센서의 값을 통해 Interrupt를 주기 위해서는 필요 // TODO, ADC Interrupt
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
   
}

void GPIO_Configure(void) {
   GPIO_InitTypeDef GPIO_InitStructure;
   
   // 2 Line Tracing Sensors
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // input pull donw 라인 값 받기
   GPIO_Init(GPIOE, &GPIO_InitStructure);
   
   // LED
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
   GPIO_Init(GPIOD, &GPIO_InitStructure);
}



void Delay(void) {
int i;
for (i = 0; i < 2000000; i++);
}

int main(void)
{
  SystemInit();
  RCC_Configure();
  GPIO_Configure();


 while (1) { // TODO: implement 
   if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14) == Bit_SET) {
     GPIO_SetBits(GPIOD,GPIO_Pin_2);
     Delay(); 
     GPIO_ResetBits(GPIOD,GPIO_Pin_2);
   }
   
    } return 0;
}