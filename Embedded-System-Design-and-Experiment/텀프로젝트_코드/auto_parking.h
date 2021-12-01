#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "misc.h"
//#include "lcd.h"
//#include "touch.h"

#include <stdio.h>

#define SPEED_RATIO 0.5 // [-]
#define EMERGENCY_TIME 5 // [s]
#define DISTANCE 10 // [cm]



int on_off=0; // �ڵ��� ���� ��ư (1:on, 0:off) 
int is_dangerous=0;

int motor_state=0; // �ڵ��� ���� ������ �� �ʿ��� ���� 
enum _MOTOR {	// �ڵ��� ���� ���� 
	Stop,
	// �ʿ��ϸ� ���¸� �߰��ϰų� �Լ��� �Ű������� �߰��� �� ����
	// �������� ��ȸ���� ��� �ϳ��� ������ȸ�������� �����ϱ� ���� ��� 
	Forward, Forward_Left, Forward_Right,
	Backward, Backward_Left, Backward_Right
} MOTOR; 
void Motor_Stop();
void Motor_Forward();
void Motor_Forward_Left();
void Motor_Forward_Right();
void Motor_Backward();
void Motor_Backward_Left();
void Motor_Backward_Right();

// PWM ������� �� (TIM3 CH1-4, TIM4 CH1-4 ���)
// �������� +, �������� -, �� �ű� (������ �չ�ȣ�� ������ �� ��?)
int wheel[8]={GPIO_Pin_6, GPIO_Pin_7, // Front Left +- / PA6,7 / TIM3_CH1,2
              GPIO_Pin_0, GPIO_Pin_1, // Front Right +- / PB0,1 / TIM3_CH3,4
              GPIO_Pin_6, GPIO_Pin_7, // Back Left +- / PB6,7 / TIM4_CH1,2
              GPIO_Pin_8, GPIO_Pin_9} // Back Right +- / PB8,9 / TIM4_CH3,4
// PWM ��� �ʿ� 
TIM_OCInitTypeDef TIM_OCInitStructure;

// Ultrasonic Sensor, ������ trig, �������� echo �� 
// Read_Distance �Լ� ���ο� GPIOE�� �����صױ� ������ �� ��ȣ�� �����Ѵ�. 
int Ultrasonic_pin[12]={GPIO_Pin_2, GPIO_Pin_3, // ���� ���� 
						GPIO_Pin_4, GPIO_Pin_5, // ���� �߾� 
						GPIO_Pin_6, GPIO_Pin_7, // ���� ���� 
						GPIO_Pin_8, GPIO_Pin_9, // �Ĺ� ���� 
						GPIO_Pin_10, GPIO_Pin_11, // �Ĺ� �߾� 
						GPIO_Pin_12, GPIO_Pin_13} // �Ĺ� ���� 
volatile uint32_t Ultrasonic_distance; // ������
int Ultrasonic_prev[6]={0,0,0,0,0,0}; // ����� ���� �������� �� ��� �κп��� �׷����� Ư�� �������� ���� 
int Ultrasonic_curr[6]={0,0,0,0,0,0}; // ������ �������� ����ġ�� �ش��ϴ����� ���� ���� ��� ���ϴ� ��� 
float Read_Distance(unit16_t trig, uint16_t echo);
void Close_Or_Not();

// Line-Tracing Sensor
// Line_Or_Not �Լ� ���ο� GPIOE�� �����صױ� ������ �� ��ȣ�� �����Ѵ�. 
int LT_pin[2]={GPIO_Pin_14, GPIO_Pin_15}; // ��, �� 
// �������� ����̸� 1, �������̰ų� �Ÿ��� �ָ� 0�� ���´�. 
int LT_curr[2]={1,1}; // �������� ���� ���� ��� ���ϴ� ��� 
int LT_prev[2]={1,1}; // �������� �� ��� �κп��� �׷����� Ư�� �������� ���� 
void Line_Or_Not();

void Parking(); // �������� motor_state�� ���� �ൿ 



// ====================================================================


uint32_t usTime=0; // ��� �ð� ���� us 
void Normal_Delay(void);
// ���ϴ� �ð���ŭ ������ �����ִ� �Լ� 
void Delay(uint32_t delayTime);



void RCC_Configure(void);
void GPIO_Configure(void);
void USART_Configure(void);



void TIM2_Configure(void);
void TIM2_IRQHandler(void);

void TIM3_Configure(void);
void TIM4_Configure(void);
void PWM_Change(int timerNum, int timerChannel, float dutyCycle);

// ������ �������� �ޱ� ���� ���� 
void ADC_Configure();
// ���� Ʈ���̽� ������  

// ��������� ���̽�ƽ ���� ���� - ���� �߰� ���� 
void EXTI_Configure();

void EXTI15_10_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void EXTI4_IRQHandler();
void EXTI9_5_IRQHandler();
// ���� ���� ���� 
void NVIC_Configure();

