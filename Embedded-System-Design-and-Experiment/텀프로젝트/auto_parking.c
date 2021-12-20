#include "auto_parking.h"

// Configuration =====================================================

void Normal_Delay(void) // 2초 주기 딜레이 함수
{
    for (int i = 0; i < 2000000; i++)
        ;
}
// 원하는 시간만큼 딜레이 시켜주는 함수
void Delay(uint32_t delay_time)
{
    delay_prev_time = us_time;
    while (1)
    {
        if (us_time - delay_prev_time > delay_time)
            break;
    }
}

void RCC_Configure(void)
{
    // GPIOA, PA2,3 : USART2 TX,RX, PA6,7 : MotorFL (+ PA9,10 : USART1 TX,RX)
    // GPIOB, PB0,1 : MotorFR, PB6,7 : MotorBL, PB8,9 : MotorBR
    // GPIOC
    // GPIOD, PD2-4,7 : LED, PD11 : S1 Button, PD12 : S2 Button
    // GPIOE, PE2-13 : 6 Ultrasonic Sensors, PE14,15 : 2 Line Tracing Sensors
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                               RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                               RCC_APB2Periph_GPIOE,
                           ENABLE);

    // Alternate Function IO clock enable
    // 외부 장치인 초음파 거리 센서나 라인 센서의 값을 통해 Interrupt를 주기 위해서는 필요 // TODO, ADC Interrupt
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    // USART2, 블루투스를 통해 휴대폰과의 통신
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    // TIM2, 초음파 센서로 측정한 거리가 너무 가까울 때 멈추고 그때부터 시간을 센다.
    // TIM3,4, 외부 전원을 이용하여 PWM으로 모터를 제어한다.
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    // ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // USART2 TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // AFIO 쓰기 때문에
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART2 RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Motor (FL, FR, BL, BR)
    GPIO_InitStructure.GPIO_Pin = wheel[0] | wheel[1];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = wheel[2] | wheel[3] | wheel[4] | wheel[5] | wheel[6] | wheel[7];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 6 Ultrasonic Sensors
    // TRIG
    GPIO_InitStructure.GPIO_Pin = Ultrasonic_pin[0] | Ultrasonic_pin[2] | Ultrasonic_pin[4] | Ultrasonic_pin[6] | Ultrasonic_pin[8] | Ultrasonic_pin[10];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // ECHO
    GPIO_InitStructure.GPIO_Pin = Ultrasonic_pin[1] | Ultrasonic_pin[3] | Ultrasonic_pin[5] | Ultrasonic_pin[7] | Ultrasonic_pin[9] | Ultrasonic_pin[11];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 2 Line Tracing Sensors
    GPIO_InitStructure.GPIO_Pin = LT_pin[0] | LT_pin[1];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // Joystick
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Button
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // LED
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void USART_Configure(void)
{
    USART_InitTypeDef USART_InitStructure;

    // USART2
    USART_Cmd(USART2, ENABLE);
    USART_InitStructure.USART_BaudRate = 9600;                                      // 데이터 전송 속도 지정
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 프레임에서 전송되거나 수신되는 데이터 비트 수를 지정
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 단어의 끝을 의미하는 Stop Bit 개수를 설정
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 패리티(오류 검출) 활성화, 비활성화 결정 -> MSB가 패리티 비트
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 // 어떤 Mode 사용할지 지정 (수신 또는 전송 모드를 활성화 또는 비활성화)
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 하드웨어 흐름 제어모드를 사용하거나 사용하지 않도록 지정
    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // RX(수신부분)에 대하여 Interrupt 처리 가능
}

// 블루투스 핸들러
void USART2_IRQHandler()
{
    uint16_t word; // 휴대폰으로 받아올 문자열을 저장하는 함수
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        word = USART_ReceiveData(USART2);

        /*word가
        0: Forward, 1: Back, 2:Forward Right, 3: Forward Left, 4: Back Right, 5: Back Left, 6: Stop, 7: Auto Parking Start
        ,8: Auto Parking Stop, 9: engine ON/OFF
        */
        if (word == '9')
        {
            if (on_off)
            {
                on_off = 0;
                motor_state = 0;
                led_state = 0;
            }
            else
            {
                motor_state = 0;
                on_off = 1;
                led_state = 1;
            }
        }
        if (on_off)
        {
            switch (word)
            {
            case '0':
                motor_state = 0;
                break;
            case '1':
                motor_state = 1;
                break;
            case '2':
                motor_state = 2;
                break;
            case '3':
                motor_state = 3;
                break;
            case '4':
                motor_state = 4;
                break;
            case '5':
                motor_state = 5;
                break;
            case '6':
                motor_state = 6;
                break;
            case '7':
                now_parking = 1;
                parking_stage = 0;
                first_move_stage = 0;
                break;
            case '8':
                now_parking = 0;
                parking_stage = 1;
                led_state = 0;
                break;
            default:
                break;
            }
        }
    }
    // clear 'Read data register not empty' flag
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
}

void TIM2_Configure(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    // 단위를 1us로 지정, 72/72MHz=1us
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1;
    TIM_TimeBaseInitStructure.TIM_Period = 72;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        us_time++;
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void TIM3_Configure(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    // 한 주기를 7.5ms로 설정함
    TIM_TimeBaseInitStructure.TIM_Period = 2500;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 216;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TIM_CKD_DIV1=0
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1000; // Duty Cycle = 750/7500 = 10%
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);

    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);

    TIM_OC4Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);
}

void TIM4_Configure(void)
{

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

    // 한 주기를 7.5ms로 설정함
    TIM_TimeBaseInitStructure.TIM_Period = 2500;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 216;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TIM_CKD_DIV1=0
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 1000; // Duty Cycle = 750/7500 = 10%
    TIM_ARRPreloadConfig(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);

    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);

    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);
}

void PWM_Change(int timerNum, int timerChannel, float dutyCycle)
{
    int new_pulse = 25000 * dutyCycle;
    switch (timerNum)
    {
    case 3:
        switch (timerChannel)
        {
        case 1:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC1Init(TIM3, &TIM_OCInitStructure);
            break;
        case 2:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC2Init(TIM3, &TIM_OCInitStructure);
            break;
        case 3:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC3Init(TIM3, &TIM_OCInitStructure);
            break;
        case 4:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC4Init(TIM3, &TIM_OCInitStructure);
        default:
            break;
        }
    case 4:
        switch (timerChannel)
        {
        case 1:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC1Init(TIM4, &TIM_OCInitStructure);
            break;
        case 2:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC2Init(TIM4, &TIM_OCInitStructure);
            break;
        case 3:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC3Init(TIM4, &TIM_OCInitStructure);
            break;
        case 4:
            TIM_OCInitStructure.TIM_Pulse = new_pulse;
            TIM_OC4Init(TIM4, &TIM_OCInitStructure);
        default:
            break;
        }
    }
}

// 초음파 센서값을 받기 위해 설정
void ADC_Configure()
{
    ADC_InitTypeDef ADC_Initstruct;
    ADC_Initstruct.ADC_Mode = ADC_Mode_Independent;
    ADC_Initstruct.ADC_ScanConvMode = DISABLE;
    ADC_Initstruct.ADC_ContinuousConvMode = ENABLE;
    ADC_Initstruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_Initstruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Initstruct.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_Initstruct);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// 블루투스와 조이스틱 관련 제어
void EXTI_Configure()
{
    // 기본적인 자동주차 기능을 다 구현한 다음에 조이스틱, 버튼이나 블루투스 화면으로 사용자가 수동으로 자동차를 움직일 수 있게 함
    EXTI_InitTypeDef EXTI_InitStructure;

    /* Joystick Down */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Joystick Up */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Joystick Left */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource3);
    EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Joystick Right */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource4);
    EXTI_InitStructure.EXTI_Line = EXTI_Line4;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* user S1 Button */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* user S2 Button */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);
    EXTI_InitStructure.EXTI_Line = EXTI_Line12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* L1 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource14);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* L2 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == Bit_RESET)
        {
            if (on_off)
            {
                if (motor_state == 0)
                {
                    if (prev_motor_state == 0)
                    {
                        motor_state = 4;
                        prev_motor_state = 4;
                    }
                    else
                    {
                        motor_state = prev_motor_state;
                    }
                }
                else
                {
                    motor_state = 4;
                    prev_motor_state = motor_state;
                }
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line2);
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_RESET)
        {
            if (on_off)
            {
                if (motor_state == 0)
                {
                    if (prev_motor_state == 0)
                    {
                        motor_state = 1;
                        prev_motor_state = 1;
                    }
                    else
                    {
                        motor_state = prev_motor_state;
                    }
                }
                else
                {
                    motor_state = 1;
                    prev_motor_state = motor_state;
                }
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line5);
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == Bit_RESET)
        {
            if (on_off)
            {
                if (motor_state == 1)
                {
                    motor_state = 6;
                    prev_motor_state = motor_state;
                }
                else if (motor_state == 0)
                {
                    if (prev_motor_state == 0)
                    {
                        motor_state = 6;
                        prev_motor_state = 6;
                    }
                    else
                    {
                        motor_state = prev_motor_state;
                    }
                }
                else
                {
                    motor_state -= 1;
                    prev_motor_state = motor_state;
                }
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line3);
}

void EXTI4_IRQHandler(void)
{

    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == Bit_RESET)
        {
            if (on_off)
            {
                if (motor_state == 6)
                {
                    motor_state = 1;
                    prev_motor_state = motor_state;
                }
                else if (motor_state == 0)
                {
                    if (prev_motor_state == 0)
                    {
                        motor_state = 2;
                        prev_motor_state = 2;
                    }
                    else
                    {
                        motor_state = prev_motor_state;
                    }
                }
                else
                {
                    motor_state += 1;
                    prev_motor_state = motor_state;
                }
            }
        }
    }
    EXTI_ClearITPendingBit(EXTI_Line4);
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) == Bit_RESET)
        {
            if (on_off)
            {
                on_off = 0;
                motor_state = 0;
            }
            else
            {
                on_off = 1;
                motor_state = 0;
            }
        }
    }

    else if (EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12) == Bit_RESET)
        {
            motor_state = 0;
        }
    }

    EXTI_ClearITPendingBit(EXTI_Line11);
    EXTI_ClearITPendingBit(EXTI_Line12);
    EXTI_ClearITPendingBit(EXTI_Line14);
    EXTI_ClearITPendingBit(EXTI_Line15);
}

void NVIC_Configure()
{
    // 두 개의 우선순위가 모두 같으면 우선순위 없이 순서대로 처리
    // 버튼 > USART2, TIM2 > 조이스틱
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;

    // USART2
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // TIMER2, 높은 우선순위
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 버튼과 조이스틱
    // Joystick Down
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Joystick Up
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Joystick Left
    NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Joystick Right
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // User S1, S2, L1, L2 Button
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// Car Operation =====================================================

// 자동차 동작
// 전진, 후진만 할 때랑 회전할 때 속도 다름
// Front Left, Front Right, Back Left, Back Right
void Motor_Stop()
{ // 00 00
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, 0);
}

void Motor_Forward()
{ // ++ ++
    PWM_Change(3, 1, SPEED_RATIO2);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, SPEED_RATIO2);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, SPEED_RATIO2);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, SPEED_RATIO2);
    PWM_Change(4, 4, 0);
}

void Motor_Forward_Left()
{ // -+ ++
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO * 2);
    PWM_Change(3, 3, SPEED_RATIO * 2);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, SPEED_RATIO * 2);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, SPEED_RATIO * 2);
    PWM_Change(4, 4, 0);
}

void Motor_Forward_Left2()
{ // 0+ -+, Go_Turn_Left
    PWM_Change(3, 1, 0.00001);
    PWM_Change(3, 2, 0.00001);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, SPEED_RATIO);
    PWM_Change(4, 4, 0);
}

void Motor_Forward_Left3()
{ // -+ -+, Go_Tank_Left
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, SPEED_RATIO);
    PWM_Change(4, 4, 0);
}

void Motor_Forward_Right()
{ // +- ++
    PWM_Change(3, 1, SPEED_RATIO);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, SPEED_RATIO);
    PWM_Change(4, 1, SPEED_RATIO);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, SPEED_RATIO);
    PWM_Change(4, 4, 0);
}

void Motor_Forward_Right2()
{ // +0 +-, Go_Turn_Right
    PWM_Change(3, 1, SPEED_RATIO);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0.00001);
    PWM_Change(3, 4, 0.00001);
    PWM_Change(4, 1, SPEED_RATIO);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO);
}

void Motor_Forward_Right3()
{ // +- +-, Go_Tank_Right
    PWM_Change(3, 1, SPEED_RATIO);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, SPEED_RATIO);
    PWM_Change(4, 1, SPEED_RATIO);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO);
}

void Motor_Backward()
{ // -- --
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO2);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, SPEED_RATIO2);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO2);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO2);
}

void Motor_Backward_Left()
{ // +- --
    PWM_Change(3, 1, SPEED_RATIO);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, SPEED_RATIO);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO);
}

void Motor_Backward_Left2()
{ // -+ -0, Back_Turn_Left
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, 0.00001);
    PWM_Change(4, 4, 0.00001);
}

void Motor_Backward_Left3()
{ // +- +-, Back_Tank_Left
    PWM_Change(3, 1, SPEED_RATIO);
    PWM_Change(3, 2, 0);
    PWM_Change(3, 3, 0);
    PWM_Change(3, 4, SPEED_RATIO);
    PWM_Change(4, 1, SPEED_RATIO);
    PWM_Change(4, 2, 0);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO);
}

void Motor_Backward_Right()
{ // -+ --
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, 0);
    PWM_Change(4, 4, SPEED_RATIO);
}

void Motor_Backward_Right2()
{ // -+ -0, Back_Turn_Right
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, 0.00001);
    PWM_Change(4, 4, 0.00001);
}

void Motor_Backward_Right3()
{ // -+ -+, Back_Tank_Right
    PWM_Change(3, 1, 0);
    PWM_Change(3, 2, SPEED_RATIO);
    PWM_Change(3, 3, SPEED_RATIO);
    PWM_Change(3, 4, 0);
    PWM_Change(4, 1, 0);
    PWM_Change(4, 2, SPEED_RATIO);
    PWM_Change(4, 3, SPEED_RATIO);
    PWM_Change(4, 4, 0);
}

void Moving()
{
    switch (motor_state)
    {
    case Stop:
        Motor_Stop();
        break;
    case Forward:
        Motor_Forward();
        break;
    case Right_Forward:
        Motor_Forward_Right();
        break;
    case Right_Back:
        Motor_Backward_Right();
        break;
    case Back:
        Motor_Backward();
        break;
    case Left_Back:
        Motor_Backward_Left();
        break;
    case Left_Forward:
        Motor_Forward_Left();
        break;
    case Right_Forward2:
        Motor_Forward_Right2();
        break;
    case Right_Forward3:
        Motor_Forward_Right3();
        break;
    case Right_Back2:
        Motor_Backward_Right2();
        break;
    case Right_Back3:
        Motor_Backward_Right3();
        break;
    case Left_Back2:
        Motor_Backward_Left2();
        break;
    case Left_Back3:
        Motor_Backward_Left3();
        break;
    case Left_Forward2:
        Motor_Forward_Left2();
        break;
    case Left_Forward3:
        Motor_Forward_Left3();
        break;
    }
}

// 초음파 센서를 이용해서 거리 측정
// GPIOE로 고정해놨기 때문에 매개변수로 trig, echo 핀 번호만 넣어준다.
int Read_Distance(uint16_t trig, uint16_t echo)
{
    uint32_t prev = 0;
    GPIO_SetBits(GPIOE, trig);
    GPIO_ResetBits(GPIOE, echo);
    Delay(10);
    GPIO_ResetBits(GPIOE, trig);

    /* 버스트 발생 직후 에코는 HIGH 레벨을 가진다.
    따라서 버스트가 발생했는지 알기 위해 while문을 통해
    에코가 LOW 레벨(RESET)을 가질 때(버스트 발생 X)는 반복문에 머물게 하고 
    에코가 HIGH 레벨(SET)을 가질 때(버스트 발생)는 반복문을 탈출한다.*/
    while (GPIO_ReadInputDataBit(GPIOE, echo) == RESET)
        ;

    // 반복문을 탈출한 이후엔 시간 측정을 위해 prev 변수에 현재 시각을 저장한다.
    prev = us_time;

    /* 에코에 버스트가 다시 들어오면 에코는 LOW 레벨을 가진다.
    따라서 에코가 HIGH 레벨(SET)일 동안은 아직 버스트가 돌아 오지 않은 거니까
    반복문에 머물게 하고 에코가 LOW 레벨을 가졌을 땐 버스트가 들어왔다는
    의미니까 반복문을 탈출해 거리를 계산한다.*/
    while (GPIO_ReadInputDataBit(GPIOE, echo) != RESET)
        ;

    // 거리는 (버스트 왕복거리) / 0.034cm/us 로 구해진다.
    int distance = (us_time - prev) * 34 / 1000;

    return distance;
}
void Close_Or_Not()
{

    // 기준 거리보다 가까운지 여부에 따라 curr 행렬의 해당 부분에 1과 0을 넣어준다.
    for (int i = 0; i < NUM_OF_ULTRASONIC; i++)
    {
        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[2 * i], Ultrasonic_pin[2 * i + 1]);
        int d;
        if (i == 0 || i == 1)
            d = DISTANCE;
        else
            d = DISTANCE_SIDE;
        if (Ultrasonic_distance > d)
        {
            Ultrasonic_curr[i] = 0;
        }
        else
        {
            Ultrasonic_curr[i] = 1;
        }
    }
    // 기준 거리보다 가까운 부분이 하나라도 있으면 is_dangerous 변수를 1로 만들고,
    // prev 행렬에 curr 행렬의 값을 저장한다. 이후 상태가 해결되면 0으로 초기화한다.
    close = 0;
    for (int i = 0; i < NUM_OF_ULTRASONIC; i++)
    {
        if (Ultrasonic_curr[i] == 1)
        {
            close = 1;
            is_dangerous_sensor[0] = 1;
            for (int j = 0; j < 6; j++)
            {
                Ultrasonic_prev[i] = Ultrasonic_curr[i];
            }
            break;
        }
    }
    if (!close)
    {
        is_dangerous_sensor[0] = 0;
        is_dangerous = 0;
    }

    prev_time = us_time;
    while (1)
    {
        curr_time = us_time;
        if (curr_time - prev_time >= 60000)
            break;

        Moving();
    }
}

// 라인 트레이싱 센서값 받고, 선 위에 있는지 여부까지 확인
void Line_Or_Not()
{
    // 선 위에 있는지 여부에 따라 curr 행렬의 해당 부분에 0과 1을 넣어준다.
    for (int i = 0; i < NUM_OF_LINE_TRACING; i++)
    {
        if (GPIO_ReadInputDataBit(GPIOE, LT_pin[i]) == Bit_SET)
        {
            // Line Tracing 센서는 흰색일 때 1이다.
            LT_curr[i] = 1;
        }
        else
        {
            LT_curr[i] = 0;
        }
    }
    // 선 위에 있는 것이 하나라도 있으면 is_dangerous 변수를 1로 만들고,
    // prev 행렬에 curr 행렬의 값을 저장한다. 이후 상태가 해결되면 1로 초기화된다.
    line = 0;
    for (int i = 0; i < NUM_OF_LINE_TRACING; i++)
    {
        if (LT_curr[i] == 0)
        {
            line = 1;
            is_dangerous_sensor[1] = 1;
            for (int j = 0; j < 2; j++)
            {
                LT_prev[i] = LT_curr[i];
            }
            break;
        }
    }
    if (!line)
    {
        is_dangerous_sensor[1] = 0;
        is_dangerous = 0;
    }
}

void Is_Dangerous()
{
    for (int i = 0; i < NUM_OF_SENSOR_TYPE; i++)
    {
        is_dangerous = is_dangerous || is_dangerous_sensor[i];
    }
}

// Parking Function ==================================================

void Auto_Parking()
{

    switch (parking_stage)
    {
    case 0:
        First_Move();
        break;
    case 1:
        Parking_End();
        break;
    }
    Moving();
}

void First_Move()
{ // 우측 자동직각주차, Auto_Parking() 함수에서 motor_state를 바꿔주는 역할
    switch (first_move_stage)
    {
    // 처음 운전을 시작하여 직진하는 단계, 주차 가능한 길이를 측정하면
    // 주차 공간 탐색을 위해 다음 단계로 넘어감
    case 0:
        led_state = 1;
        motor_state = Forward; // 기본적으로 전진

        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[6], Ultrasonic_pin[7]); // 전방 우측 초음파센서 값을 읽는다.
        Delay(60000);
        if (Ultrasonic_distance > 30)
        { // TODO, 거리 수정
            //printf("d: %d, 30넘는 공간 발견",Ultrasonic_distance);
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 1;
            action_start_time = us_time;
        }
        break;

    // 처음 공간이 있다고 생각했을 때로부터 시간이 얼마나 경과했는지를 이용하여
    // 거리를 대략적으로 생각하고, 그 거리가 주차하기에 충분하다고 생각되면
    // first_moving_stage를 증가시켜 주차를 위해 후진하고,
    // 충분하지 않다고 판단되면 first_moving_stage를 감소시켜
    // 다시 주차할 수 있는 공간의 시작부터 탐색한다.
    case 1:

        motor_state = Forward; // 기본적으로 전진

        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[6], Ultrasonic_pin[7]); // 전방 우측 초음파센서 값을 읽는다.
        Delay(60000);
        if (Ultrasonic_distance < 40)
        { // TODO, 거리 수정
            if (us_time - action_start_time > 200000)
            { // TODO, 시간 수정
                motor_state = Stop;
                Moving();
                USART_SendData(USART2, 'P');
                led_state = 2;      // led State 2 로 변경
                Set_LED_By_State(); // sate에 따라 led 세팅

                // 주차공간을 탐지했을 때 사용자한테 자동 주차 시작 여부를 응답받을 때까지 기다린다.
                while (1)
                {
                    if (motor_state == Forward)// 주차 시작
                    {
                        first_move_stage = 2;
                        action_start_time = us_time;
                        break;
                    }
                    else if (motor_state == Back) // 주차 시작하지 않고 다시 공간 탐지
                    {
                        first_move_stage = 0;
                        break;
                    }
                    else
                    {
                        Delay(100000);
                    }
                }
            }
            else
            {
                first_move_stage = 0;
            }
        }
        break;

    // 주차공간을 탐색한 후 원하는 궤적을 위해
    // 주차공간 탐색 기준 시간의 일정배수의 시간 동안 후진한다.
    case 2:
        led_state = 3;
        if (motor_state == Right_Back || motor_state == Left_Forward)
        {
            USART_SendData(USART2, 'B');
        }
        motor_state = Back; // 기본적으로 후진

        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[10], Ultrasonic_pin[11]); // 전방 우측 초음파센서 값을 읽는다.
        Delay(60000);
        if (Ultrasonic_distance < 40)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            motor_state = Left_Forward;
            first_move_stage = 3;
            action_start_time = us_time;
        }
        break;

    // 원하는 궤적을 위해 일정 시간 동안 곡률반경을 늘리며 전방좌회전한다.
    case 3:

        if (us_time - action_start_time < 950000)
        {
            motor_state = Left_Forward;
        }

        else if (us_time - action_start_time >= 950000)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 4;
            action_start_time = us_time;
        }
        break;

    // 원하는 궤적을 위해 일정 시간 동안 후방우회전한다.
    case 4:
        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[2], Ultrasonic_pin[3]); // 후방 초음파센서 값을 읽는다.
        
        if (Ultrasonic_distance < 15) // 차량 후방의 여유 공간이 15보다 작으면
        {
            // 후방 좌우측 초음파 센서로 거리 계산
            Ultrasonic_distance_left = Read_Distance(Ultrasonic_pin[8], Ultrasonic_pin[9]);
            Ultrasonic_distance_right = Read_Distance(Ultrasonic_pin[10], Ultrasonic_pin[11]);
            int sub = Ultrasonic_distance_left - Ultrasonic_distance_right;
            if (sub > 8) // 후방 좌우측 거리차가 8보다 크면 오른쪽 여유공간이 없다는 뜻이니까 LB로 거리차를 줄이고 case 6로 이동.
            {
                motor_state = Left_Back3;
                action_start_time = us_time;
                first_move_stage = 6;
            }
            else if (sub < -8) // 후방 좌우측 거리차가 -8보다 작으면 왼쪽 여유공간이 없다는 뜻이니까 RB로 거리차를 줄이고 case 7로 이동.
            {
                motor_state = Right_Back3;
                action_start_time = us_time;
                first_move_stage = 7;
            }
        }
        if (us_time - action_start_time < 250000)
        {
            USART_SendData(USART2, 'u');
            motor_state = Right_Back3;
        }

        else if (us_time - action_start_time >= 250000 && us_time - action_start_time < 1500000)
        {
            motor_state = Back;
        }
        else if (us_time - action_start_time >= 1500000)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 5;
            action_start_time = us_time;
        }
        break;

    // 주차가 완료되면 멈춘다.
    case 5:
        motor_state = Stop;
        Moving();
        Delay(250000);
        parking_stage = 1;
        break;

    
    case 6:
        if (us_time - action_start_time < 200000)
        {
            motor_state = Left_Back3;
        }
        else if (us_time - action_start_time >= 200000)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 8;
            action_start_time = us_time;
        }
        break;
    case 7:
        if (us_time - action_start_time < 200000)
        {
            motor_state = Right_Back3;
        }
        else if (us_time - action_start_time >= 200000)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 8;
            action_start_time = us_time;
        }
        break;
    case 8:
        Ultrasonic_distance = Read_Distance(Ultrasonic_pin[2], Ultrasonic_pin[3]); // 후방 초음파센서 값을 읽는다.
        //Delay(60000);
        if (Ultrasonic_distance < 15)
        {

            Ultrasonic_distance_left = Read_Distance(Ultrasonic_pin[8], Ultrasonic_pin[9]);
            Ultrasonic_distance_right = Read_Distance(Ultrasonic_pin[10], Ultrasonic_pin[11]);
            int sub = Ultrasonic_distance_left - Ultrasonic_distance_right;
            if (sub > 8)
            {
                motor_state = Left_Forward3;
                action_start_time = us_time;
                first_move_stage = 6;
            }
            else if (sub < -8)
            {
                motor_state = Right_Forward3;
                action_start_time = us_time;
                first_move_stage = 7;
            }
        }
        if (us_time - action_start_time < 400000)
        {
            motor_state = Back;
        }
        else if (us_time - action_start_time >= 400000)
        {
            motor_state = Stop;
            Moving();
            Delay(250000);
            first_move_stage = 5;
            action_start_time = us_time;
        }
        break;
    }
    prev_first_move_stage = first_move_stage;
}

void User_Parking()
{
    Close_Or_Not();
    Line_Or_Not();
    Is_Dangerous();
    prev_motor_state = motor_state;
    if (is_dangerous == 1)
    {
        motor_state = Stop;
        Moving();
        // 이제 안전한지 보고, 안전하면 is_dangerous=0으로 하고, 다시 동작
        // 상황이 해소되지 않으면 자동주차 종료. 아 그러면 자동주차도 켜고 끌 수 있도록 해야겠는데?

        dangerous_prev_time = us_time;
        while (1)
        {
            curr_time = us_time;
            if (curr_time - dangerous_prev_time >= 5000000)
            { // 5초 이상 지나면
                now_parking = 0;
                USART_SendData(USART2, 'N');
                motor_state = 0;
                break;
            }

            Delay(500000);
            Close_Or_Not();
             Line_Or_Not();
            Is_Dangerous();
            if (!is_dangerous)
            {

                motor_state = prev_motor_state;
                USART_SendData(USART2, 'Y');
                break;
            } // 5초 이전에 위험 상태가 해제되면 while문만 벗어난다.
        }
    }
    Moving();

    // 전역변수 motor_state에 따라서 행동
    // 초음파 센서가 60ms 기준으로 측정해야하기 때문에
    // 60ms 이상의 시간 동안은 motor_state에 따라 행동하도록 한다.
}

void Parking_End()
{
    USART_SendData(USART2, 'i'); // 주차완료 메세지
    led_state = 4;
    now_parking = 0; // 자동 주차 종료
    motor_state = Stop;
}

void Set_LED_By_State()
{ // 보드의 LED로 현재 상태를 알려주기 위해 LED를 세팅하는 함수
    GPIO_ResetBits(GPIOD, GPIO_Pin_2);
    GPIO_ResetBits(GPIOD, GPIO_Pin_3);
    GPIO_ResetBits(GPIOD, GPIO_Pin_4);
    GPIO_ResetBits(GPIOD, GPIO_Pin_7);
    switch (led_state)
    {
    case 0:
        break;

    case 1:
        GPIO_SetBits(GPIOD, GPIO_Pin_2);
        break;

    case 2:
        GPIO_SetBits(GPIOD, GPIO_Pin_3);
        break;

    case 3:
        GPIO_SetBits(GPIOD, GPIO_Pin_4);
        break;

    case 4:
        GPIO_SetBits(GPIOD, GPIO_Pin_7);
        break;

    default:
        break;
    }
}