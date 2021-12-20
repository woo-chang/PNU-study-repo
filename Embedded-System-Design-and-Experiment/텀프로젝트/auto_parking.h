#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "misc.h"

#include <stdio.h>

// 운전 관련
#define SPEED_RATIO 0.2  // [-]
#define SPEED_RATIO2 0.075  // [-]
#define EMERGENCY_TIME 5 // [s]
#define DISTANCE_SIDE 2  // [cm]
#define DISTANCE 6       // [cm]

// 센서 관련
#define NUM_OF_SENSOR_TYPE 2
#define NUM_OF_ULTRASONIC 6
#define NUM_OF_LINE_TRACING 2

enum MOTOR
{ // 자동차 운전 상태
    Stop,
    // 필요하면 상태를 추가하거나 함수에 매개변수를 추가할 수 있음
    // 전방으로 좌회전할 경우 하나의 전방좌회전만으로 제어하기 힘들 경우
    // 시계방향으로 설정함
    Forward,
    Right_Forward,
    Right_Back,
    Back,
    Left_Back,
    Left_Forward,
    Right_Forward2,
    Right_Forward3,
    Right_Back2,
    Right_Back3,
    Left_Back2,
    Left_Back3,
    Left_Forward2,
    Left_Forward3,
};
void Motor_Stop();            // 자동차 정지 동작
void Motor_Forward();         // 자동차 전진 동작
void Motor_Forward_Left();    // 자동차 전방 좌회전 동작 1
void Motor_Forward_Left2();   // 자동차 전방 좌회전 동작 2
void Motor_Forward_Left3();   // 자동차 전방 좌회전 동작 3
void Motor_Forward_Right();   // 자동차 전방 우회전 동작 1
void Motor_Forward_Right2();  // 자동차 전방 우회전 동작 2
void Motor_Forward_Right3();  // 자동차 전방 우회전 동작 3
void Motor_Backward();        // 자동차 후진 동작
void Motor_Backward_Left();   // 자동차 후방 좌회전 동작 1
void Motor_Backward_Left2();  // 자동차 후방 좌회전 동작 2
void Motor_Backward_Left3();  // 자동차 후방 좌회전 동작 3
void Motor_Backward_Right();  // 자동차 후방 우회전 동작 1
void Motor_Backward_Right2(); // 자동차 후방 우회전 동작 2
void Motor_Backward_Right3(); // 자동차 후방 우회전 동작 3

void Moving(); // 자동차 현재 상태에 따라 동작을 결정하는 함수

int Read_Distance(uint16_t trig, uint16_t echo); // 초음파 센서 센서값을 측정하는 함수
void Close_Or_Not();                             // 초음파 센서 값을 통해 거리가 가까운지 측정을 통해 위험 여부를 감지하는 함수
void Line_Or_Not();                              // 라인 센서를 통해 라인을 확인한 후 위험 여부를 감지하는 함수
void Is_Dangerous();                             // 위험 여부가 감지되었으면 위험 상태로 변경해주는 함수

void Auto_Parking();                                     // 전역변수 motor_state에 따라서 행동
void First_Move();                                       // 자동 주차 상태에 따라 처음으로 동작하게 되는 행동


void User_Parking(); // 자동 주차가 아닌 유저가 원하는 동작대로 차량이 움직이게 하는 함수

void Parking_End(); // 주차가 완료됨을 알려주는 함수
void Set_LED_By_State(); // 보드의 LED로 현재 상태를 알려주기 위해 LED를 세팅하는 함수

// ====================================================================

// Configure 함수
// Interrupt Handler 함수

void Normal_Delay(void);
void Delay(uint32_t delay_time); // 원하는 시간만큼 딜레이 시켜주는 함수

void RCC_Configure(void);
void GPIO_Configure(void);
void USART_Configure(void);

void TIM2_Configure(void);
void TIM2_IRQHandler(void);

void TIM3_Configure(void);
void TIM4_Configure(void);
void PWM_Change(int timerNum, int timerChannel, float dutyCycle);
void ADC_Configure();

void EXTI_Configure();
void EXTI15_10_IRQHandler();
void EXTI2_IRQHandler();
void EXTI3_IRQHandler();
void EXTI4_IRQHandler();
void EXTI9_5_IRQHandler();
void NVIC_Configure();

// PWM 제어에 필요
extern TIM_OCInitTypeDef TIM_OCInitStructure;
extern int on_off;
extern int now_parking;
extern int is_dangerous_sensor[NUM_OF_SENSOR_TYPE];
extern int is_dangerous;
extern int prev_motor_state;
extern int prev_parking_stage;
extern int prev_first_move_stage;
extern int motor_state;
extern int wheel[8];
extern int Ultrasonic_pin[12];
extern int Ultrasonic_prev[6];
extern int Ultrasonic_curr[6];
extern int LT_pin[2];
extern int LT_curr[2];
extern int LT_prev[2];
extern uint32_t dangerous_prev_time;
extern uint32_t delay_prev_time;
extern uint32_t prev_time;
extern uint32_t curr_time;
extern uint32_t us_time;
extern int line;
extern int close;
extern volatile uint32_t Ultrasonic_distance; // 센서값
extern volatile uint32_t Ultrasonic_distance_front;
extern volatile uint32_t Ultrasonic_distance_back;
extern volatile uint32_t Ultrasonic_distance_left;
extern volatile uint32_t Ultrasonic_distance_right;
extern int Check_Repart_State[2];
extern int Repark_State[5][5];
extern int parking_stage;
extern int first_move_stage;
extern int action_start_time;
extern int led_state;