#include "auto_parking.h"

int led_state = 0;
int on_off = 0;      // �ڵ��� ���� ��ư (1:on, 0:off)
int now_parking = 0; // 0: �ڵ����� �� �ƴ�, 1: �ڵ����� ��
int is_dangerous_sensor[NUM_OF_SENSOR_TYPE] = {
    0,
};                    // [0]:Ultrasonic, [1]:Line-Tracing
int is_dangerous = 0; // 0: ������ ���� X, 1: ������ ����

int prev_first_move_stage = 0;
int prev_parking_stage = 0; // �ֱ� �ڵ� ���� ���� ���� ����
int prev_motor_state = 0; // �ֱ� ���� ���� ���� ����
int motor_state = 0;      // �ڵ��� ���� ������ �� �ʿ��� ����

int parking_stage = 0;     // �ڵ� ���� ���� ���� ����
int first_move_stage = 0;  // ù��° ���ۿ����� ���� ���� ����
int action_start_time = 0; // ù��° ���ۿ����� �ð� ���� ����

// PWM ������� �� (TIM3 CH1-4, TIM4 CH1-4 ���)
// �������� +, �������� -, �� �ű� (������ �չ�ȣ�� ������ �� ��?)
int wheel[8] = {GPIO_Pin_6, GPIO_Pin_7,  // Front Left +- / PA6,7 / TIM3_CH1,2
                GPIO_Pin_0, GPIO_Pin_1,  // Front Right +- / PB0,1 / TIM3_CH3,4
                GPIO_Pin_6, GPIO_Pin_7,  // Back Left +- / PB6,7 / TIM4_CH1,2
                GPIO_Pin_8, GPIO_Pin_9}; // Back Right +- / PB8,9 / TIM4_CH3,4
// PWM ��� �ʿ�
TIM_OCInitTypeDef TIM_OCInitStructure;

// Ultrasonic Sensor, ������ trig, �������� echo ��, GPIO E
// Read_Distance �Լ� ���ο� GPIOE�� �����صױ� ������ �� ��ȣ�� �����Ѵ�.
int Ultrasonic_pin[12] = {
    GPIO_Pin_6, GPIO_Pin_7,   // ���� �߾�
    GPIO_Pin_8, GPIO_Pin_9,   //  �Ĺ� �߾�
    GPIO_Pin_2, GPIO_Pin_3,   //  ���� ����
    GPIO_Pin_10, GPIO_Pin_11, // ���� ����
    GPIO_Pin_4, GPIO_Pin_5,   //�Ĺ� ����
    GPIO_Pin_12, GPIO_Pin_13, // �Ĺ� ����
};

int Ultrasonic_prev[6] = {0, 0, 0, 0, 0, 0}; // ����� ���� �������� �� ��� �κп��� �׷����� Ư�� �������� ����
int Ultrasonic_curr[6] = {0, 0, 0, 0, 0, 0}; // ������ �������� ����ġ�� �ش��ϴ����� ���� ���� ��� ���ϴ� ���

uint32_t dangerous_prev_time = 0;
uint32_t delay_prev_time = 0; // ���� �ð��� �����ϴ� ����(Delay �Լ� ����)
uint32_t prev_time = 0; // ���� �ð��� �����ϴ� ����
uint32_t curr_time = 0; // ���� �ð��� �����ϴ� ����
uint32_t us_time = 0;   // timer �ð� ����

int line; // line�� ���� ������ �˷��ִ� ����

int close; // ����� �Ÿ��� ��ü�� ������ �˷��ִ� ����

volatile uint32_t Ultrasonic_distance;       // ������ ������ �Ÿ��� ����ϴ� ����
volatile uint32_t Ultrasonic_distance_front; // ���� ����, ������ �Ÿ��� ���̸� ����ϴ� ����
volatile uint32_t Ultrasonic_distance_back;  // �Ĺ� ����, ������ �Ÿ��� ���̸� ����ϴ� ����
volatile uint32_t Ultrasonic_distance_left;  // �Ĺ� ����, ������ �Ÿ��� ���̸� ����ϴ� ����
volatile uint32_t Ultrasonic_distance_right; // �Ĺ� ����, ������ �Ÿ��� ���̸� ����ϴ� ����

int Check_Repart_State[2] = {
    0,
}; // Index 0: ���� ���� Index 1: ���� ����

// ���� ���¿� ���� state �ο�
int Repark_State[5][5] = {
    {1, 2, 3, 4, 5},
    {6, 0, 0, 7, 8},
    {9, 0, 0, 0, 10},
    {11, 12, 0, 0, 13},
    {14, 15, 16, 17, 18}};

// Line-Tracing Sensor
// Line_Or_Not �Լ� ���ο� GPIOE�� �����صױ� ������ �� ��ȣ�� �����Ѵ�.
int LT_pin[2] = {GPIO_Pin_14, GPIO_Pin_15}; // ��, ��
// �������� ����̸� 1, �������̰ų� �Ÿ��� �ָ� 0�� ���´�.
int LT_curr[2] = {1, 1}; // �������� ���� ���� ��� ���ϴ� ���
int LT_prev[2] = {1, 1}; // �������� �� ��� �κп��� �׷����� Ư�� �������� ����

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
        // �ڵ��� �õ��� ������ ��
        if (on_off)
        {
            // �ڵ� ���� ����� ��
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