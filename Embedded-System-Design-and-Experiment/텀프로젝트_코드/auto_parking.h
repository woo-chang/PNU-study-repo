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

#define SPEED_RATIO 0.5	 // [-]
#define EMERGENCY_TIME 5 // [s]
#define DISTANCE 10		 // [cm]

int on_off = 0; // 자동차 전원 버튼 (1:on, 0:off)
int is_dangerous = 0;

int motor_state = 0; // 자동차 운전 제어할 때 필요한 변수
enum _MOTOR
{ // 자동차 운전 상태
	Stop,
	// 필요하면 상태를 추가하거나 함수에 매개변수를 추가할 수 있음
	// 전방으로 좌회전할 경우 하나의 전방좌회전만으로 제어하기 힘들 경우
	Forward,
	Forward_Left,
	Forward_Right,
	Backward,
	Backward_Left,
	Backward_Right
} MOTOR;
void Motor_Stop();
void Motor_Forward();
void Motor_Forward_Left();
void Motor_Forward_Right();
void Motor_Backward();
void Motor_Backward_Left();
void Motor_Backward_Right();

// PWM 고려했을 때 (TIM3 CH1-4, TIM4 CH1-4 사용)
// 빨간선이 +, 검은선이 -, 잘 꼽기 (빨간색 앞번호에 꼽으면 될 듯?)
int wheel[8] = {GPIO_Pin_6, GPIO_Pin_7, // Front Left +- / PA6,7 / TIM3_CH1,2
				GPIO_Pin_0, GPIO_Pin_1, // Front Right +- / PB0,1 / TIM3_CH3,4
				GPIO_Pin_6, GPIO_Pin_7, // Back Left +- / PB6,7 / TIM4_CH1,2
				GPIO_Pin_8, GPIO_Pin_9} // Back Right +- / PB8,9 / TIM4_CH3,4
// PWM 제어에 필요
TIM_OCInitTypeDef TIM_OCInitStructure;

// Ultrasonic Sensor, 왼쪽이 trig, 오른쪽이 echo 핀
// Read_Distance 함수 내부에 GPIOE로 고정해뒀기 때문에 핀 번호만 저장한다.
int Ultrasonic_pin[12] = {GPIO_Pin_2, GPIO_Pin_3,	// 전방 좌측
						  GPIO_Pin_4, GPIO_Pin_5,	// 전방 중앙
						  GPIO_Pin_6, GPIO_Pin_7,	// 전방 우측
						  GPIO_Pin_8, GPIO_Pin_9,	// 후방 좌측
						  GPIO_Pin_10, GPIO_Pin_11, // 후방 중앙
						  GPIO_Pin_12, GPIO_Pin_13} // 후방 우측
volatile uint32_t Ultrasonic_distance;				// 센서값
int Ultrasonic_prev[6] = {0, 0, 0, 0, 0, 0};		// 가까운 것을 감지했을 때 어느 부분에서 그랬는지 특정 순간까지 저장
int Ultrasonic_curr[6] = {0, 0, 0, 0, 0, 0};		// 초음파 센서값이 기준치에 해당하는지에 따라 값이 계속 변하는 행렬
float Read_Distance(unit16_t trig, uint16_t echo);
void Close_Or_Not();

// Line-Tracing Sensor
// Line_Or_Not 함수 내부에 GPIOE로 고정해뒀기 때문에 핀 번호만 저장한다.
int LT_pin[2] = {GPIO_Pin_14, GPIO_Pin_15}; // 좌, 우
// 센서값이 흰색이면 1, 검은색이거나 거리가 멀면 0이 나온다.
int LT_curr[2] = {1, 1}; // 센서값에 따라 값이 계속 변하는 행렬
int LT_prev[2] = {1, 1}; // 감지했을 때 어느 부분에서 그랬는지 특정 순간까지 저장
void Line_Or_Not();

void Parking(); // 전역변수 motor_state에 따라서 행동

// ====================================================================

uint32_t usTime = 0; // 모든 시간 단위 us
void Normal_Delay(void);
// 원하는 시간만큼 딜레이 시켜주는 함수
void Delay(uint32_t delayTime);

void RCC_Configure(void);
void GPIO_Configure(void);
void USART_Configure(void);

void TIM2_Configure(void);
void TIM2_IRQHandler(void);

void TIM3_Configure(void);
void TIM4_Configure(void);
void PWM_Change(int timerNum, int timerChannel, float dutyCycle);

// 초음파 센서값을 받기 위해 설정
void ADC_Configure();
// 라인 트레이싱 센서값

// 블루투스와 조이스틱 관련 제어 - 추후 추가 예정
void EXTI_Configure();

void EXTI15_10_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void EXTI4_IRQHandler();
void EXTI9_5_IRQHandler();
// 이후 수정 예정
void NVIC_Configure();