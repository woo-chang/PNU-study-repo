#include "auto_parking.h"

int led_state = 0;
int on_off = 0;      // 자동차 전원 버튼 (1:on, 0:off)
int now_parking = 0; // 0: 자동주차 중 아님, 1: 자동주차 중
int is_dangerous_sensor[NUM_OF_SENSOR_TYPE] = {
    0,
};                    // [0]:Ultrasonic, [1]:Line-Tracing
int is_dangerous = 0; // 0: 위험한 상태 X, 1: 위험한 상태

int prev_first_move_stage = 0;
int prev_parking_stage = 0; // 최근 자동 주차 상태 저장 변수
int prev_motor_state = 0; // 최근 모터 상태 저장 변수
int motor_state = 0;      // 자동차 운전 제어할 때 필요한 변수

int parking_stage = 0;     // 자동 주차 상태 저장 변수
int first_move_stage = 0;  // 첫번째 동작에서의 상태 저장 변수
int action_start_time = 0; // 첫번째 동작에서의 시간 저장 변수

// PWM 고려했을 때 (TIM3 CH1-4, TIM4 CH1-4 사용)
// 빨간선이 +, 검은선이 -, 잘 꼽기 (빨간색 앞번호에 꼽으면 될 듯?)
int wheel[8] = {GPIO_Pin_6, GPIO_Pin_7,  // Front Left +- / PA6,7 / TIM3_CH1,2
                GPIO_Pin_0, GPIO_Pin_1,  // Front Right +- / PB0,1 / TIM3_CH3,4
                GPIO_Pin_6, GPIO_Pin_7,  // Back Left +- / PB6,7 / TIM4_CH1,2
                GPIO_Pin_8, GPIO_Pin_9}; // Back Right +- / PB8,9 / TIM4_CH3,4
// PWM 제어에 필요
TIM_OCInitTypeDef TIM_OCInitStructure;

// Ultrasonic Sensor, 왼쪽이 trig, 오른쪽이 echo 핀, GPIO E
// Read_Distance 함수 내부에 GPIOE로 고정해뒀기 때문에 핀 번호만 저장한다.
int Ultrasonic_pin[12] = {
    GPIO_Pin_6, GPIO_Pin_7,   // 전방 중앙
    GPIO_Pin_8, GPIO_Pin_9,   //  후방 중앙
    GPIO_Pin_2, GPIO_Pin_3,   //  전방 좌측
    GPIO_Pin_10, GPIO_Pin_11, // 전방 우측
    GPIO_Pin_4, GPIO_Pin_5,   //후방 좌측
    GPIO_Pin_12, GPIO_Pin_13, // 후방 우측
};

int Ultrasonic_prev[6] = {0, 0, 0, 0, 0, 0}; // 가까운 것을 감지했을 때 어느 부분에서 그랬는지 특정 순간까지 저장
int Ultrasonic_curr[6] = {0, 0, 0, 0, 0, 0}; // 초음파 센서값이 기준치에 해당하는지에 따라 값이 계속 변하는 행렬

uint32_t dangerous_prev_time = 0;
uint32_t delay_prev_time = 0; // 이전 시간을 저장하는 변수(Delay 함수 전용)
uint32_t prev_time = 0; // 이전 시간을 저장하는 변수
uint32_t curr_time = 0; // 현재 시간을 저장하는 변수
uint32_t us_time = 0;   // timer 시간 변수

int line; // line이 감지 됬음을 알려주는 변수

int close; // 가까운 거리에 물체가 있음을 알려주는 변수

volatile uint32_t Ultrasonic_distance;       // 초음파 센서를 거리를 계산하는 변수
volatile uint32_t Ultrasonic_distance_front; // 전방 왼쪽, 오른쪽 거리의 차이를 계산하는 변수
volatile uint32_t Ultrasonic_distance_back;  // 후방 왼쪽, 오른쪽 거리의 차이를 계산하는 변수
volatile uint32_t Ultrasonic_distance_left;  // 후방 왼쪽, 오른쪽 거리의 차이를 계산하는 변수
volatile uint32_t Ultrasonic_distance_right; // 후방 왼쪽, 오른쪽 거리의 차이를 계산하는 변수

int Check_Repart_State[2] = {
    0,
}; // Index 0: 앞쪽 상태 Index 1: 뒤쪽 상태

// 주차 상태에 따른 state 부여
int Repark_State[5][5] = {
    {1, 2, 3, 4, 5},
    {6, 0, 0, 7, 8},
    {9, 0, 0, 0, 10},
    {11, 12, 0, 0, 13},
    {14, 15, 16, 17, 18}};

// Line-Tracing Sensor
// Line_Or_Not 함수 내부에 GPIOE로 고정해뒀기 때문에 핀 번호만 저장한다.
int LT_pin[2] = {GPIO_Pin_14, GPIO_Pin_15}; // 좌, 우
// 센서값이 흰색이면 1, 검은색이거나 거리가 멀면 0이 나온다.
int LT_curr[2] = {1, 1}; // 센서값에 따라 값이 계속 변하는 행렬
int LT_prev[2] = {1, 1}; // 감지했을 때 어느 부분에서 그랬는지 특정 순간까지 저장

int main(void)
{
    // Init
    // Configure
    SystemInit();
    
    RCC_Configure();
    GPIO_Configure();
    USART_Configure();

    ADC_Configure();
    NVIC_Configure();
    TIM2_Configure();
    TIM3_Configure();
    TIM4_Configure();

    EXTI_Configure();
    NVIC_Configure();

    // Parking
    while (1)
    {
        Set_LED_By_State();
        // 자동차 시동이 켜졌을 때
        if (on_off)
        {
            // 자동 주차 모드일 때
            if (now_parking)
            {
                Auto_Parking();
            }
            else
            {
                User_Parking();
            }
        }
    }
    return 0;
}