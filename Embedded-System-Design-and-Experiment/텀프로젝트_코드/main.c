#include "auto_parking.h"

int main(void)
{
	// Init
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
		Parking();
	}
	return 0;
}
