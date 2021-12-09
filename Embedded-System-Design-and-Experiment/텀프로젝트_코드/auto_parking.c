#include "auto_parking.h"

// Configuration =====================================================

void Normal_Delay(void)
{
	for (int i = 0; i < 2000000; i++)
		;
}
// 원하는 시간만큼 딜레이 시켜주는 함수
void Delay(uint32_t delay_time)
{
	uint32_t prev_time = us_time;
	while (1)
	{
		if (usTime - prev_time > delayTime)
			break;
	}
}

void RCC_Configure(void)
{
	// GPIOA, PA2,3 : USART2 TX,RX, PA6,7 : MotorFL (+ PA9,10 : USART1 TX,RX)
	// GPIOB, PB0,1 : MotorFR, PB6,7 : MotorBL, PB8,9 : MotorBR
	// GPIOC
	// GPIOD, PD2-4,7 : LED, PD11 : S1 Button
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
	// GPIO_IN_FLOATING도 사용할 수 있다 (버튼, 센서), Speed는 GPIO_Speed_50MHz로
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Motor (FL, FR, BL, BR)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// 6 Ultrasonic Sensors
	// TRIG
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	// ECHO
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// 2 Line Tracing Sensors
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

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
	USART_InitStructure.USART_BaudRate = 9600;										// 데이터 전송 속도 지정
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 프레임에서 전송되거나 수신되는 데이터 비트 수를 지정
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 단어의 끝을 의미하는 Stop Bit 개수를 설정
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 패리티(오류 검출) 활성화, 비활성화 결정 -> MSB가 패리티 비트
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;					// 어떤 Mode 사용할지 지정 (수신 또는 전송 모드를 활성화 또는 비활성화)
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 하드웨어 흐름 제어모드를 사용하거나 사용하지 않도록 지정
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // RX(수신부분)에 대하여 Interrupt 처리 가능
}

void TIM2_Configure(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// 단위를 1us로 지정, 72/72MHz=1us
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1;
	TIM_TimeBaseInitStructure.TIM_Period = 72;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; // CounterMode Up/Down 상관 없나?
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	TIM_ARRPreloadConfig(TIM2, ENABLE); // ?
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}
void TIM2_IRQHandler(void)
{
	// 조건이 충족되는 동안을 의미
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		usTime++;
		if (is_dangerous)
		{
			if (usTime)
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

void TIM3_Configure(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;

	// 한 주기를 20ms로 설정함
	TIM_TimeBaseInitStructure.TIM_Period = 20000;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TIM_CKD_DIV1=0
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 2000; // Duty Cycle = 2000/20000 = 10%
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

	// 한 주기를 20ms로 설정함
	TIM_TimeBaseInitStructure.TIM_Period = 20000;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // TIM_CKD_DIV1=0
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 2000; // Duty Cycle = 2000/20000 = 10%
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
// 블루투스와 조이스틱 관련 제어 - 추후 추가 예정
void EXTI_Configure()
{
	// 기본적인 자동주차 기능을 다 구현한 다음에 조이스틱, 버튼이나 블루투스 화면으로 사용자가 수동으로 자동차를 움직일 수 있게 함
	EXTI_InitTypeDef EXTI_InitStructure;

	/*
    
    // EXTI는 EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5 이렇게 안 됨?
    // 해보고 되면 ㄱㄱ 
     
   // DOWN (PD2)
   EXTI_InitStructure.EXTI_Line = EXTI_Line2;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&EXTI_InitStructure);
   // LEFT (PD3), USART2 PX (PA3)
   EXTI_InitStructure.EXTI_Line = EXTI_Line3;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&EXTI_InitStructure);
   // RIGHT (PD4)
   EXTI_InitStructure.EXTI_Line = EXTI_Line4;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&EXTI_InitStructure);
   // UP (PD5)
   EXTI_InitStructure.EXTI_Line = EXTI_Line5;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&EXTI_InitStructure);
   // S1 Button (PD11)
   EXTI_InitStructure.EXTI_Line = EXTI_Line11;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   EXTI_Init(&EXTI_InitStructure);
   // USART1 RX (PA10)
   //EXTI_InitStructure.EXTI_Line = EXTI_Line10;
   //EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   //EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   //EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
   //EXTI_Init(&EXTI_InitStructure);*/

	// Line-Tracing Sensors, 14:Left, 15:Right
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);
	EXTI_InitStructure.EXTI_Line = EXTI_Line15;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);
}
void EXTI15_10_IRQHandler()
{
	/*if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
       input_char = USART_ReceiveData(USART1);
       input_num = 10;
       EXTI_ClearITPendingBit(EXTI_Line10);
    }
    if (EXTI_GetITStatus(EXTI_Line11) != RESET) { // user button interupt
       input_num = 11;
       EXTI_ClearITPendingBit(EXTI_Line11);
    }*/
	if (EXTI_GetITStatus(EXTI_Line14) != RESET)
	{
	}
}
void EXTI2_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line2) != RESET)
	{ // joystick down interupt
		input_num = 2;
		EXTI_ClearITPendingBit(EXTI_Line2);
	}
}
void EXTI3_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line3) != RESET)
	{ // joystick left interupt
		input_num = 3;
		EXTI_ClearITPendingBit(EXTI_Line3);
	}
}
void EXTI4_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line4) != RESET)
	{ // joystick right interupt
		input_num = 4;
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}
void EXTI9_5_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line5) != RESET)
	{ // joystick up interupt
		input_num = 5;
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	* /
}
// 이후 수정 예정
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
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //Button
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn; // Joystick-Up
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn; //Joystick-Down
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; //Joystick-Right
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn; //Joystick-Left
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
	NVIC_Init(&NVIC_InitStructure);
}

// Car Operation =====================================================

// 자동차 동작
void Motor_Stop()
{
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
{
	PWM_Change(3, 1, SPEED_RATIO);
	PWM_Change(3, 2, 0);
	PWM_Change(3, 3, SPEED_RATIO);
	PWM_Change(3, 4, 0);
	PWM_Change(4, 1, SPEED_RATIO);
	PWM_Change(4, 2, 0);
	PWM_Change(4, 3, SPEED_RATIO);
	PWM_Change(4, 4, 0);
}
void Motor_Forward_Left()
{
	PWM_Change(3, 1, 0);
	PWM_Change(3, 2, SPEED_RATIO);
	PWM_Change(3, 3, SPEED_RATIO);
	PWM_Change(3, 4, 0);
	PWM_Change(4, 1, SPEED_RATIO);
	PWM_Change(4, 2, 0);
	PWM_Change(4, 3, SPEED_RATIO);
	PWM_Change(4, 4, 0);
}
void Motor_Forward_Right()
{
	PWM_Change(3, 1, SPEED_RATIO);
	PWM_Change(3, 2, 0);
	PWM_Change(3, 3, 0);
	PWM_Change(3, 4, SPEED_RATIO);
	PWM_Change(4, 1, SPEED_RATIO);
	PWM_Change(4, 2, 0);
	PWM_Change(4, 3, SPEED_RATIO);
	PWM_Change(4, 4, 0);
}
void Motor_Backward()
{
	PWM_Change(3, 1, 0);
	PWM_Change(3, 2, SPEED_RATIO);
	PWM_Change(3, 3, 0);
	PWM_Change(3, 4, SPEED_RATIO);
	PWM_Change(4, 1, 0);
	PWM_Change(4, 2, SPEED_RATIO);
	PWM_Change(4, 3, 0);
	PWM_Change(4, 4, SPEED_RATIO);
}
void Motor_Backward_Left()
{
	PWM_Change(3, 1, 0);
	PWM_Change(3, 2, SPEED_RATIO);
	PWM_Change(3, 3, SPEED_RATIO);
	PWM_Change(3, 4, 0);
	PWM_Change(4, 1, 0);
	PWM_Change(4, 2, SPEED_RATIO);
	PWM_Change(4, 3, 0);
	PWM_Change(4, 4, SPEED_RATIO);
}
void Motor_Backward_Right()
{
	PWM_Change(3, 1, SPEED_RATIO);
	PWM_Change(3, 2, 0);
	PWM_Change(3, 3, 0);
	PWM_Change(3, 4, SPEED_RATIO);
	PWM_Change(4, 1, 0);
	PWM_Change(4, 2, SPEED_RATIO);
	PWM_Change(4, 3, 0);
	PWM_Change(4, 4, SPEED_RATIO);
}

// 초음파 센서를 이용해서 거리 측정
// GPIOE로 고정해놨기 때문에 매개변수로 trig, echo 핀 번호만 넣어준다.
float Read_Distance(uint16_t trig, uint16_t echo)
{
	uint32_t prev = 0;
	GPIO_SetBits(GPIOE, trig;
    GPIO_ResetBits(GPIOE, echo);
    Delay(10);
    GPIO_ResetBits(GPIOE, echo);
    //uint8_t val = GPIO_ReadInputDataBit(GPIOE, echo);

    /* 버스트 발생 직후 에코는 HIGH 레벨을 가진다.
    따라서 버스트가 발생했는지 알기 위해 while문을 통해
    에코가 LOW 레벨(RESET)을 가질 때(버스트 발생 X)는 반복문에 머물게 하고 
    에코가 HIGH 레벨(SET)을 가질 때(버스트 발생)는 반복문을 탈출한다.*/  
    while(GPIO_ReadInputDataBit(GPIOE, echo) == RESET);
    
    // 반복문을 탈출한 이후엔 시간 측정을 위해 prev 변수에 현재 시각을 저장한다.
    prev = usTime; 
    
    /* 에코에 버스트가 다시 들어오면 에코는 LOW 레벨을 가진다.
    따라서 에코가 HIGH 레벨(SET)일 동안은 아직 버스트가 돌아 오지 않은 거니까
    반복문에 머물게 하고 에코가 LOW 레벨을 가졌을 땐 버스트가 들어왔다는
    의미니까 반복문을 탈출해 거리를 계산한다.*/ 
    while(GPIO_ReadInputDataBit(GPIOE, echo) != RESET);

    // 거리는 (버스트 왕복거리) / 2 / 0.034cm/us 로 구해진다.
    float distance = (usTime - prev)*34/1000;

    return distance;
}
void Close_Or_Not()
{
	// 기준 거리보다 가까운지 여부에 따라 curr 행렬의 해당 부분에 1과 0을 넣어준다.
	for (int i = 0; i < 6; i++)
	{
		Ultrasonic_distance = Read_Distance(Ultrasonic_pin[2 * i], Ultrasonic_pin[2 * i + 1]);
		if (Ultrasonic_distance < DISTANCE)
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
	for (int i = 0; i < 6; i++)
	{
		if (Ultrasonic_curr[i] == 1)
		{
			is_dangerous = 1;
			for (int j = 0; j < 6; j++)
			{
				Ultrasonic_prev[i] = Ultrasonic_curr[i];
			}
			break;
		}
	}
	else { is_dangerous = 0; }
}

// 라인 트레이싱 센서값 받고, 선 위에 있는지 여부까지 확인
// 조교님이 초음파센서 ADC 핸들러를 쓰지 않으셔서 초음파센서 ADC 핸들러를 쓰지 않음
// -> 라인 트레이싱 센서도 EXTI 핸들러를 쓰지 않음
// -> 문제가 되나.. 되면 이후에 EXTI로 바꾸기
void Line_Or_Not()
{
	// 선 위에 있는지 여부에 따라 curr 행렬의 해당 부분에 0과 1을 넣어준다.
	for (int i = 0; i < 2; i++)
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
	for (int i = 0; i < 2; i++)
	{
		if (LT_curr[i] == 0)
		{
			is_dangerous = 1;
			for (int j = 0; j < 2; j++)
			{
				LT_prev[i] = LT_curr[i];
			}
			break;
		}
	}
	else { is_dangerous = 0; }
}

// Parking Function ==================================================

void Parking()
{ // TODO
	Close_Or_Not();
	Line_Or_Not();
	if (is_dangerous == 1)
	{
		motor_state = _MOTOR::Stop;
		// 이제 안전한지 보고, 안전하면 is_dangerous=0으로 하고, 다시 동작
		// 상황이 해소되지 않으면 자동주차 종료. 아 그러면 자동주차도 켜고 끌 수 있도록 해야겠는데?
	}

	// 전역변수 motor_state에 따라서 행동
	// 초음파 센서가 60ms 기준으로 측정해야하기 때문에
	// 60ms 이상의 시간 동안은 motor_state에 따라 행동하도록 한다.
}