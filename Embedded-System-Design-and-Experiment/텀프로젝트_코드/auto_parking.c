#include "auto_parking.h"

// Configuration =====================================================

void Normal_Delay(void) {
	for(int i=0;i<2000000;i++);
}
// ���ϴ� �ð���ŭ ������ �����ִ� �Լ�
void Delay(uint32_t delay_time){
	uint32_t prev_time=us_time;
	while(1){ if(usTime-prev_time>delayTime) break;}
} 

void RCC_Configure(void) {
	// GPIOA, PA2,3 : USART2 TX,RX, PA6,7 : MotorFL (+ PA9,10 : USART1 TX,RX)
	// GPIOB, PB0,1 : MotorFR, PB6,7 : MotorBL, PB8,9 : MotorBR
	// GPIOC
	// GPIOD, PD2-4,7 : LED, PD11 : S1 Button
	// GPIOE, PE2-13 : 6 Ultrasonic Sensors, PE14,15 : 2 Line Tracing Sensors 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
				   		   RCC_APB2Periph_GPIOE, ENABLE);
	
	// Alternate Function IO clock enable
	// �ܺ� ��ġ�� ������ �Ÿ� ������ ���� ������ ���� ���� Interrupt�� �ֱ� ���ؼ��� �ʿ� // TODO, ADC Interrupt
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// USART2, ��������� ���� �޴������� ���
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	// TIM2, ������ ������ ������ �Ÿ��� �ʹ� ����� �� ���߰� �׶����� �ð��� ����.
	// TIM3,4, �ܺ� ������ �̿��Ͽ� PWM���� ���͸� �����Ѵ�. 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	// ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
}
void GPIO_Configure(void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// USART2 TX 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // AFIO ���� ������ 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// USART2 RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	// GPIO_IN_FLOATING�� ����� �� �ִ� (��ư, ����), Speed�� GPIO_Speed_50MHz�� 
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
void USART_Configure(void) {
	USART_InitTypeDef USART_InitStructure;
	
	// USART2
	USART_Cmd(USART2, ENABLE);
	USART_InitStructure.USART_BaudRate = 9600; // ������ ���� �ӵ� ����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; // �����ӿ��� ���۵ǰų� ���ŵǴ� ������ ��Ʈ ���� ����
	USART_InitStructure.USART_StopBits = USART_StopBits_1; // �ܾ��� ���� �ǹ��ϴ� Stop Bit ������ ����
	USART_InitStructure.USART_Parity = USART_Parity_No; // �и�Ƽ(���� ����) Ȱ��ȭ, ��Ȱ��ȭ ���� -> MSB�� �и�Ƽ ��Ʈ
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // � Mode ������� ���� (���� �Ǵ� ���� ��带 Ȱ��ȭ �Ǵ� ��Ȱ��ȭ)
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // �ϵ���� �帧 �����带 ����ϰų� ������� �ʵ��� ����
	USART_Init(USART2, &USART_InitStructure);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // RX(���źκ�)�� ���Ͽ� Interrupt ó�� ����
}

void TIM2_Configure(void) {
 TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
 
 // ������ 1us�� ����, 72/72MHz=1us 
 TIM_TimeBaseInitStructure.TIM_Prescaler = 1;
 TIM_TimeBaseInitStructure.TIM_Period = 72;
 TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down; // CounterMode Up/Down ��� ����? 
 TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
 TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
 
 TIM_ARRPreloadConfig(TIM2, ENABLE); // ?
 TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
 TIM_Cmd(TIM2, ENABLE);
}
void TIM2_IRQHandler(void) {
 // ������ �����Ǵ� ������ �ǹ�
 if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) { 
 	usTime++; 
 	if(is_dangerous) {
		if(usTime)
	}
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
 }
}

void TIM3_Configure(void){
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	// �� �ֱ⸦ 20ms�� ������ 
	TIM_TimeBaseInitStructure.TIM_Period=20000;
	TIM_TimeBaseInitStructure.TIM_Prescaler=72;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Down;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; // TIM_CKD_DIV1=0
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
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
void TIM4_Configure(void){
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	// �� �ֱ⸦ 20ms�� ������ 
	TIM_TimeBaseInitStructure.TIM_Period=20000;
	TIM_TimeBaseInitStructure.TIM_Prescaler=72;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Down;
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; // TIM_CKD_DIV1=0
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);
	
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
void PWM_Change(int timerNum, int timerChannel, float dutyCycle){
	int new_pulse=25000*dutyCycle;
	switch(timerNum){
		case 3:
			switch(timerChannel){
				case 1:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC1Init(TIM3,&TIM_OCInitStructure);
					break;
				case 2:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC2Init(TIM3,&TIM_OCInitStructure);
					break;
				case 3:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC3Init(TIM3,&TIM_OCInitStructure);
					break;
				case 4:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC4Init(TIM3,&TIM_OCInitStructure);
				default:
					break;
			}
		case 4:
			switch(timerChannel){
				case 1:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC1Init(TIM4,&TIM_OCInitStructure);
					break;
				case 2:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC2Init(TIM4,&TIM_OCInitStructure);
					break;
				case 3:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC3Init(TIM4,&TIM_OCInitStructure);
					break;
				case 4:
					TIM_OCInitStructure.TIM_Pulse=new_pulse;
					TIM_OC4Init(TIM4,&TIM_OCInitStructure);
				default:
					break;
			}
	}
}

// ������ �������� �ޱ� ���� ���� 
void ADC_Configure(){ 
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
 while (ADC_GetResetCalibrationStatus(ADC1));
 ADC_StartCalibration(ADC1);
 while (ADC_GetCalibrationStatus(ADC1));
 ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}
// ��������� ���̽�ƽ ���� ���� - ���� �߰� ���� 
void EXTI_Configure() {
// �⺻���� �ڵ����� ����� �� ������ ������ ���̽�ƽ, ��ư�̳� ������� ȭ������ ����ڰ� �������� �ڵ����� ������ �� �ְ� ��
	EXTI_InitTypeDef EXTI_InitStructure;
	 
	 /*
	 
	 // EXTI�� EXTI_Line2 | EXTI_Line3 | EXTI_Line4 | EXTI_Line5 �̷��� �� ��?
	 // �غ��� �Ǹ� ���� 
	  
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
void EXTI15_10_IRQHandler() {
	 /*if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
		 input_char = USART_ReceiveData(USART1);
		 input_num = 10;
		 EXTI_ClearITPendingBit(EXTI_Line10);
	 }
	 if (EXTI_GetITStatus(EXTI_Line11) != RESET) { // user button interupt
		 input_num = 11;
		 EXTI_ClearITPendingBit(EXTI_Line11);
	 }*/
	 if(EXTI_GetITStatus(EXTI_Line14)!=RESET){
	 	
	 }
}
void EXTI2_IRQHandler() {
	 if (EXTI_GetITStatus(EXTI_Line2) != RESET) { // joystick down interupt
		 input_num = 2;
		 EXTI_ClearITPendingBit(EXTI_Line2);
	 }
}
void EXTI3_IRQHandler() {
	 if (EXTI_GetITStatus(EXTI_Line3) != RESET) { // joystick left interupt
		 input_num = 3;
		 EXTI_ClearITPendingBit(EXTI_Line3);
	 }
}
void EXTI4_IRQHandler() {
	 if (EXTI_GetITStatus(EXTI_Line4) != RESET) { // joystick right interupt
		 input_num = 4;
		 EXTI_ClearITPendingBit(EXTI_Line4);
	 }
	}
void EXTI9_5_IRQHandler() {
	 if (EXTI_GetITStatus(EXTI_Line5) != RESET) { // joystick up interupt
		 input_num = 5;
		 EXTI_ClearITPendingBit(EXTI_Line5);
	 }*/
}
// ���� ���� ���� 
void NVIC_Configure() {
	 // �� ���� �켱������ ��� ������ �켱���� ���� ������� ó�� 
	 // ��ư > USART2, TIM2 > ���̽�ƽ 
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	 NVIC_InitTypeDef NVIC_InitStructure;
	 
     // USART2
     NVIC_EnableIRQ(USART2_IRQn);
     NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

     // TIMER2, ���� �켱���� 
     NVIC_EnableIRQ(TIM2_IRQn);
     NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
     NVIC_Init(&NVIC_InitStructure);

	 // ��ư�� ���̽�ƽ
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

// �ڵ��� ���� 
void Motor_Stop(){
	PWM_Change(3,1,0);
	PWM_Change(3,2,0);
	PWM_Change(3,3,0);
	PWM_Change(3,4,0);
	PWM_Change(4,1,0);
	PWM_Change(4,2,0);
	PWM_Change(4,3,0);
	PWM_Change(4,4,0);
}
void Motor_Forward(){
	PWM_Change(3,1,SPEED_RATIO);
	PWM_Change(3,2,0);
	PWM_Change(3,3,SPEED_RATIO);
	PWM_Change(3,4,0);
	PWM_Change(4,1,SPEED_RATIO);
	PWM_Change(4,2,0);
	PWM_Change(4,3,SPEED_RATIO);
	PWM_Change(4,4,0);
}
void Motor_Forward_Left(){
	PWM_Change(3,1,0);
	PWM_Change(3,2,SPEED_RATIO);
	PWM_Change(3,3,SPEED_RATIO);
	PWM_Change(3,4,0);
	PWM_Change(4,1,SPEED_RATIO);
	PWM_Change(4,2,0);
	PWM_Change(4,3,SPEED_RATIO);
	PWM_Change(4,4,0);
}
void Motor_Forward_Right(){
	PWM_Change(3,1,SPEED_RATIO);
	PWM_Change(3,2,0);
	PWM_Change(3,3,0);
	PWM_Change(3,4,SPEED_RATIO);
	PWM_Change(4,1,SPEED_RATIO);
	PWM_Change(4,2,0);
	PWM_Change(4,3,SPEED_RATIO);
	PWM_Change(4,4,0);
}
void Motor_Backward(){
	PWM_Change(3,1,0);
	PWM_Change(3,2,SPEED_RATIO);
	PWM_Change(3,3,0);
	PWM_Change(3,4,SPEED_RATIO);
	PWM_Change(4,1,0);
	PWM_Change(4,2,SPEED_RATIO);
	PWM_Change(4,3,0);
	PWM_Change(4,4,SPEED_RATIO);
} 
void Motor_Backward_Left(){
	PWM_Change(3,1,0);
	PWM_Change(3,2,SPEED_RATIO);
	PWM_Change(3,3,SPEED_RATIO);
	PWM_Change(3,4,0);
	PWM_Change(4,1,0);
	PWM_Change(4,2,SPEED_RATIO);
	PWM_Change(4,3,0);
	PWM_Change(4,4,SPEED_RATIO);
}
void Motor_Backward_Right(){
	PWM_Change(3,1,SPEED_RATIO);
	PWM_Change(3,2,0);
	PWM_Change(3,3,0);
	PWM_Change(3,4,SPEED_RATIO);
	PWM_Change(4,1,0);
	PWM_Change(4,2,SPEED_RATIO);
	PWM_Change(4,3,0);
	PWM_Change(4,4,SPEED_RATIO);
}

// ������ ������ �̿��ؼ� �Ÿ� ���� 
// GPIOE�� �����س��� ������ �Ű������� trig, echo �� ��ȣ�� �־��ش�. 
float Read_Distance(uint16_t trig, uint16_t echo){
    uint32_t prev = 0;
    GPIO_SetBits(GPIOE, trig;
    GPIO_ResetBits(GPIOE, echo);
    Delay(10);
    GPIO_ResetBits(GPIOE, echo);
    //uint8_t val = GPIO_ReadInputDataBit(GPIOE, echo);

    /* ����Ʈ �߻� ���� ���ڴ� HIGH ������ ������.
    ���� ����Ʈ�� �߻��ߴ��� �˱� ���� while���� ����
    ���ڰ� LOW ����(RESET)�� ���� ��(����Ʈ �߻� X)�� �ݺ����� �ӹ��� �ϰ� 
    ���ڰ� HIGH ����(SET)�� ���� ��(����Ʈ �߻�)�� �ݺ����� Ż���Ѵ�.*/  
    while(GPIO_ReadInputDataBit(GPIOE, echo) == RESET);
    
    // �ݺ����� Ż���� ���Ŀ� �ð� ������ ���� prev ������ ���� �ð��� �����Ѵ�.
    prev = usTime; 
    
    /* ���ڿ� ����Ʈ�� �ٽ� ������ ���ڴ� LOW ������ ������.
    ���� ���ڰ� HIGH ����(SET)�� ������ ���� ����Ʈ�� ���� ���� ���� �Ŵϱ�
    �ݺ����� �ӹ��� �ϰ� ���ڰ� LOW ������ ������ �� ����Ʈ�� ���Դٴ�
    �ǹ̴ϱ� �ݺ����� Ż���� �Ÿ��� ����Ѵ�.*/ 
    while(GPIO_ReadInputDataBit(GPIOE, echo) != RESET);

    // �Ÿ��� (����Ʈ �պ��Ÿ�) / 2 / 0.034cm/us �� ��������.
    float distance = (usTime - prev)*34/1000;

    return distance;
}
void Close_Or_Not(){
	// ���� �Ÿ����� ������� ���ο� ���� curr ����� �ش� �κп� 1�� 0�� �־��ش�. 
	for(int i=0;i<6;i++){
		Ultrasonic_distance=Read_Distance(Ultrasonic_pin[2*i],Ultrasonic_pin[2*i+1]);
		if (Ultrasonic_distance<DISTANCE){Ultrasonic_curr[i]=0;} 
		else {Ultrasonic_curr[i]=1;}
	}
	// ���� �Ÿ����� ����� �κ��� �ϳ��� ������ is_dangerous ������ 1�� �����,
	// prev ��Ŀ� curr ����� ���� �����Ѵ�. ���� ���°� �ذ�Ǹ� 0���� �ʱ�ȭ�Ѵ�. 
	for(int i=0;i<6;i++){
		if(Ultrasonic_curr[i]==1){
			is_dangerous=1;
			for(int j=0;j<6;j++){Ultrasonic_prev[i]=Ultrasonic_curr[i];}
			break;
		}
	}
	else{is_dangerous=0;}
}

// ���� Ʈ���̽� ������ �ް�, �� ���� �ִ��� ���α��� Ȯ�� 
// �������� �����ļ��� ADC �ڵ鷯�� ���� �����ż� �����ļ��� ADC �ڵ鷯�� ���� ����
// -> ���� Ʈ���̽� ������ EXTI �ڵ鷯�� ���� ����
// -> ������ �ǳ�.. �Ǹ� ���Ŀ� EXTI�� �ٲٱ� 
void Line_Or_Not(){
	// �� ���� �ִ��� ���ο� ���� curr ����� �ش� �κп� 0�� 1�� �־��ش�. 
	for(int i=0;i<2;i++){
		if (GPIO_ReadInputDataBit(GPIOE, LT_pin[i])==Bit_SET){
			// Line Tracing ������ ����� �� 1�̴�.
			LT_curr[i]=1;
		}
		else{LT_curr[i]=0;}
	}
	// �� ���� �ִ� ���� �ϳ��� ������ is_dangerous ������ 1�� �����,
	// prev ��Ŀ� curr ����� ���� �����Ѵ�. ���� ���°� �ذ�Ǹ� 1�� �ʱ�ȭ�ȴ�.
	for(int i=0;i<2;i++){
		if(LT_curr[i]==0){
			is_dangerous=1;
			for(int j=0;j<2;j++){LT_prev[i]=LT_curr[i];}
			break;
		}
	}
	else{is_dangerous=0;}
}


// Parking Function ==================================================

void Parking(){ // TODO
	Close_Or_Not();
	Line_Or_Not();
	if(is_dangerous==1){
		motor_state=_MOTOR::Stop;
		// ���� �������� ����, �����ϸ� is_dangerous=0���� �ϰ�, �ٽ� ����
		// ��Ȳ�� �ؼҵ��� ������ �ڵ����� ����. �� �׷��� �ڵ������� �Ѱ� �� �� �ֵ��� �ؾ߰ڴµ�? 
	} 
	


	// �������� motor_state�� ���� �ൿ 
	// ������ ������ 60ms �������� �����ؾ��ϱ� ������
	// 60ms �̻��� �ð� ������ motor_state�� ���� �ൿ�ϵ��� �Ѵ�. 
} 

