//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include <string.h>
#include <stdarg.h>

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 empty sample (trace via ITM).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the ITM output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wtype-limits"

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include"CLCD.h"
#include"JOG_control.h"
//#include "Base_Peri_F411.h"
//#include "stm32f4xx_it.h"
//#include "stm32f4xx_nucleo.h"

// -- 주변장치 초기화용 구조체
GPIO_InitTypeDef	GPIO_Init_Struct;

TIM_HandleTypeDef  TimHandle12;
TIM_OC_InitTypeDef TIM_OCInit12;

uint32_t piezo_pulse[]	= {0, 3822, 3405, 3033, 2863, 2551, 2272, 2024, 1911}; //도레미파솔라시도
uint32_t scale_val = 0, last_scale_val = 0;

uint32_t pulse_count = 0;
uint8_t togle = 0;
uint8_t food_count = 0;
uint8_t sonar_detect = 0;
uint32_t distance[15];



// 승용이 변수
char menu[4][10]={"1. MEAT","2. FISH","3. VEGE","4. ETC"};
char sort[4][10]={"1. 2WEEKS","2. 4WEEKS","3. 2MONTH","4. NONE"};
int i=0,j=0,flag0=0;
int tmp1=0, tmp2=0;
// 승용이 변수


int tmp=0;

void Sonar_detecting(void);
void F405_UART_Config(void);
void F405_TIMER_Config(void);


ADC_HandleTypeDef AdcHandler;		// ADC의 초기화를 위한 구조체형의 변수를 선언
ADC_ChannelConfTypeDef sConfig;

int adc_value;		// ADC값 저장 변수
char output_buffer[256]={0,};

typedef struct food_info
{
	char name[20];
	int food_type;
	int expi_date;

}food_info;

food_info food_list[4]={{"pork",0,3},{"banana",1,5},{"tomato",2,10},{"pickle",3,20}};


/*
static void ms_delay_int_count(volatile unsigned int nTime)
{
	nTime = (nTime * 14000);
	for(; nTime > 0; nTime--);
}

static void us_delay_int_count(volatile unsigned int nTime)
{
	nTime = (nTime * 12);
	for(; nTime > 0; nTime--);
}
*/
void LED_Config(void)
{
	/*##-1- Enable GPIOA Clock (to be able to program the configuration registers) */
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*##-2- Configure PA2,PA3 IO in output push-pull mode to drive external LED ###*/
	GPIO_Init_Struct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_Init_Struct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init_Struct.Pull = GPIO_NOPULL;
	GPIO_Init_Struct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);
}

/*
// -- CLCD의 초기설정용 함수의 선언
void CLCD_config()
{
	//  CLCD용 GPIO (GPIOC)의 초기설정을 함
	__HAL_RCC_GPIOC_CLK_ENABLE();

	// CLCD_RS(PC8), CLCD_E(PC9, DATA 4~5(PC12~15)
	GPIO_Init_Struct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_Init_Struct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init_Struct.Pull = GPIO_NOPULL;
	GPIO_Init_Struct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);	// CLCD_E = 0
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);	// CLCD_RW = 0
}

void CLCD_write(unsigned char rs, char data)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, rs);				// CLCD_RS
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);	// CLCD_E = 0
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, (data>>4) & 0x1);	// CLCD_DATA = LOW_bit
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (data>>5) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (data>>6) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (data>>7) & 0x1);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);		// CLCD_E = 1
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);	// CLCD_E = 0
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, (data>>0) & 0x1);	// CLCD_DATA = HIGH_bit
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, (data>>1) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, (data>>2) & 0x1);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, (data>>3) & 0x1);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);		// CLCD_E = 1
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);	// CLCD_E = 0
	ms_delay_int_count(2);
}

void CLCD_init()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);	// CLCD_E = 0
	CLCD_write(0, 0x33);	// 4비트 설정 특수 명령
	CLCD_write(0, 0x32);	// 4비트 설정 특수 명령
	CLCD_write(0, 0x28);	// _set_function
	CLCD_write(0, 0x0F);	// _set_display
	CLCD_write(0, 0x01);	// clcd_clear
	CLCD_write(0, 0x06);	// _set_entry_mode
	CLCD_write(0, 0x02);	// Return home
}

void clcd_put_string(uint8_t *str)
{
	while(*str)
	{
		CLCD_write(1,(uint8_t)*str++);
	}
}
*/
// -- ADC의 초기설정용 함수의 선언
void ADC_Config(void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();	// ADC 클럭 활성화
	__HAL_RCC_GPIOA_CLK_ENABLE();	// ADC 핀으로 사용할 GPIOx 활성화(VR:PA1)

	GPIO_Init_Struct.Pin 	= GPIO_PIN_4;			// GPIO에서 사용할 PIN 설정
	GPIO_Init_Struct.Mode 	= GPIO_MODE_ANALOG; 	// Input Analog Mode 모드
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

	AdcHandler.Instance 			= ADC1;						// ADC1 설정
	AdcHandler.Init.ClockPrescaler 	= ADC_CLOCK_SYNC_PCLK_DIV2;	// ADC clock prescaler
	AdcHandler.Init.Resolution 		= ADC_RESOLUTION_12B;		// ADC resolution
	AdcHandler.Init.DataAlign 		= ADC_DATAALIGN_RIGHT;		// ADC data alignment
	AdcHandler.Init.ScanConvMode 	= DISABLE;					// ADC scan 모드 비활성화
	AdcHandler.Init.ContinuousConvMode = ENABLE;				// ADC 연속 모드 활성화
	AdcHandler.Init.NbrOfConversion	= 1;						// ADC 변환 개수 설정
	AdcHandler.Init.ExternalTrigConv = ADC_SOFTWARE_START;		// ADC 외부 트리거 OFF
	HAL_ADC_Init(&AdcHandler);									// ADC를 설정된 값으로 초기화

	sConfig.Channel 	= ADC_CHANNEL_4;			// ADC 채널 설정(PA는 채널 4번)
	sConfig.Rank 		= 1;						// ADC 채널 순위 설정
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;	// ADC 샘플링 타임 설정(3클럭)
	HAL_ADC_ConfigChannel(&AdcHandler, &sConfig);	// 채널  설정

	/* NVIC configuration */
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
}

////// PIEZO PART //////
int	piezo_config(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_Init_Struct.Pin = GPIO_PIN_15;
	GPIO_Init_Struct.Mode = GPIO_MODE_AF_PP;		// Alternate Function Push Pull 모드
	GPIO_Init_Struct.Alternate = GPIO_AF9_TIM12;	// TIM12 Alternate Function mapping
	GPIO_Init_Struct.Pull = GPIO_NOPULL;
	GPIO_Init_Struct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
	return 0;
}

int timer12_config(void)
{
	__HAL_RCC_TIM12_CLK_ENABLE();

	/* Set TIMx instance */
	TimHandle12.Instance 			= TIM12;						// TIM12 사용
	TimHandle12.Init.Period 		= piezo_pulse[scale_val] - 1;
	TimHandle12.Init.Prescaler 		= 84 - 1;						// Prescaler = 83로 설정(1us)
	TimHandle12.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;		// division을 사용하지 않음
	TimHandle12.Init.CounterMode 	= TIM_COUNTERMODE_UP;			// Up Counter 모드 설정

	/*
	 - 주파수 계산
prescaler = 84 - 1   :   84Mhz/84 = 1Mhz
C4 261.64     : Period = 3822   ,  OC(CCR) = Period/2 , duty비는 항상 50%
D4 293.66     : Period = 3405   ,  OC(CCR) = Period/2
..
Period = 1M/주파수
	 */

	/* Set TIMx PWM instance */
	TIM_OCInit12.OCMode 		= TIM_OCMODE_PWM1;					// PWM mode 1 동작 모드 설정
	TIM_OCInit12.Pulse 			= (piezo_pulse[scale_val]/2) - 1;	// CCR의 설정값
	HAL_TIM_PWM_Init(&TimHandle12);									// TIM PWM을 TimHandle에 설정된 값으로 초기화함
	// TIM PWM의 Channel을  TIM_OCInit에 설정된 값으로 초기화함
	HAL_TIM_PWM_ConfigChannel(&TimHandle12, &TIM_OCInit12, TIM_CHANNEL_2);

	return 0;
}

void play_piezo(uint32_t period)
{
	/* Set TIMx instance */
	TimHandle12.Init.Period 	= period - 1;
	/* Set TIMx PWM instance */
	TIM_OCInit12.Pulse 			= (period/2) -1;	// CCR의 설정값
	HAL_TIM_PWM_Init(&TimHandle12);									// TIM PWM을 TimHandle에 설정된 값으로 초기화함
	// TIM PWM의 Channel을  TIM_OCInit에 설정된 값으로 초기화함
	HAL_TIM_PWM_ConfigChannel(&TimHandle12, &TIM_OCInit12, TIM_CHANNEL_2);
	/* Start PWM */
	HAL_TIM_PWM_Start(&TimHandle12, TIM_CHANNEL_2);

}
////// PIEZO PART //////

/* Private functions ---------------------------------------------------------*/
void pulse(void);   //10usec High 펄스 출력

#define	us_10	86
#define	us_100	851
#define	msec	8510

static void delay_int_count(volatile unsigned int nTime)
{
  for(; nTime > 0; nTime--);
}

// ================================ UART ===============================================
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UART_TxBufferSize	(countof(UART_TxBuffer) - 1)
#define UART_RxBufferSize	0xFF
#define countof(a)	(sizeof(a) / sizeof(*(a)))
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// -- UART 초기화용 구조체
UART_HandleTypeDef	UartHandle;

// -- UART 통신용 변수 선언
uint8_t UART_TxBuffer[50] = "\n\rUART Test message...!!! \n\r";
uint8_t UART_RxBuffer[4];

/* Private functions ---------------------------------------------------------*/
// -- UART의 초기설정용 함수의 선언
void F405_UART_Config(void);

// -- printf 구현용 함수
void vprint(const char *fmt, va_list argp)
{
    char string[200];
    if(0 < vsprintf(string,fmt,argp)) // build string
    {
        HAL_UART_Transmit(&UartHandle, (uint8_t*)string, strlen(string), 0xffffff); // send message via UART
    }
}

void my_printf(const char *fmt, ...) // custom printf() function
{
    va_list argp;
    va_start(argp, fmt);
    vprint(fmt, argp);
    va_end(argp);
}

// ================================ TIMER ================================================
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef    TimHandle;

/* Private functions ---------------------------------------------------------*/
// -- TIMER의 초기설정용 함수의 선언


// =========================== Ultra Sonic(HC-SR04) =======================================
// TRIG(PC0), ECHO(PC1)

int main(int argc, char* argv[])
{
  /* Configure UART */
  //F405_UART_Config();
  //HAL_UART_Transmit(&UartHandle, (uint8_t*)UART_TxBuffer, UART_TxBufferSize, 0xFFFF);

  /* Configure TIMER */
  F405_TIMER_Config();

  /* Configure PIR */
  F405_Ultra_Sonic_Config();
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);   //TRIG Off -> PC0

  //strcpy((char *)UART_TxBuffer, (const char*)"====== PIR Test message...!!! ======\n\r");

  piezo_config();
  timer12_config();

  LED_Config();

  	/* Configure CLCD */
  CLCD_config();
  CLCD_init();

  	/* Configure ADC */
  ADC_Config();
  CLCD_write(0,0x01);


  while (1)
  {
	  CLCD_write(0,0x01);
	  if (food_count==5) break;

	  Sonar_detecting();

  }

  clcd_put_string("SELECT THE SORT",0x80);
  clcd_put_string("AND EXPIRY DATE",0xC0);
  HAL_Delay(1000);
  CLCD_write(0,0x01);
  clcd_put_string(menu[0],0x80);
  clcd_put_string(menu[1],0xC0);
  EXTILine0_2_Config();

  //HAL_ADC_Start_IT(&AdcHandler);

  while (1)
  {


  }

}

void pulse(void)   //10usec High 펄스 출력
{
	//HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);   //input pulse -> PC0
	delay_int_count(us_10);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);   //input pulse -> PC0
	/*
	 * 여기서 Trigger를 실행시키는 것이다! 트리거가 모듈에 output신호를 넣으면 echo가 작동되는거야!
	 */
}


// -- UART의 초기설정을 위한 함수
void F405_UART_Config(void)		// USART1_TX(PA15), USART1_RX(PB7)
{
	// -- <9> UART의 클럭을 활성화
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();

	// -- <10> GPIO A포트 15번 핀을 UART Tx로 설정
	GPIO_Init_Struct.Pin	= GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_Init_Struct.Mode	= GPIO_MODE_AF_PP;
	GPIO_Init_Struct.Pull	= GPIO_NOPULL;
	GPIO_Init_Struct.Speed	= GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_Init_Struct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

	// -- <12> UART의 동작 조건 설정
	UartHandle.Instance			= USART2;
	UartHandle.Init.BaudRate	= 9600;
	UartHandle.Init.WordLength	= UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits	= UART_STOPBITS_1;
	UartHandle.Init.Parity		= UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	UartHandle.Init.Mode		= UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	// -- <13> UART 구성정보를 UartHandle에 설정된 값으로 초기화 함
	HAL_UART_Init(&UartHandle);

	USART2->BRR = 0x00001117;
}

// ----------------------------------------------------------------------------
// -- TIMER의 초기설정용 함수의 선언
void F405_TIMER_Config(void)
{
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* Set TIMx instance */
	TimHandle.Instance = TIM2;
	TimHandle.Init.Period = 58 - 1;		//	58 us
	TimHandle.Init.Prescaler = 84 - 1;	// CK_CNT = 프리스케일러에 공급되는 클럭(CK_PSC) / (프리스케일러 설정값 +1) : 1us
	TimHandle.Init.ClockDivision = 0;
	TimHandle.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	HAL_TIM_Base_Init(&TimHandle);

	/*##-2- Start the TIM Base generation in interrupt mode ####################*/
	/* Start Channel1 */
	//HAL_TIM_Base_Start_IT(&TimHandle);

	/*##-3- Configure the NVIC for TIMx ########################################*/
	/* Set Interrupt Group Priority */
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);

	/* Enable the TIMx global Interrupt */
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void F405_Ultra_Sonic_Config(void)	// TRIG(PC0), ECHO(PC1)
{
  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure PC0 IO in output */
  GPIO_Init_Struct.Pin = GPIO_PIN_0;
  GPIO_Init_Struct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Init_Struct.Pull = GPIO_NOPULL;
  GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

  /* Configure PC1 pin as input floating */
  GPIO_Init_Struct.Pin = GPIO_PIN_1;
  GPIO_Init_Struct.Mode = GPIO_MODE_IT_RISING;
  GPIO_Init_Struct.Pull = GPIO_NOPULL;
  GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	pulse_count++;
	/*
	 * 1cm = 29us이다. 이걸 왕복하면 58us가 되는 것이어서 58us로 지정함!
	 * 즉 타이머가 각 1cm가 되는 순간마다 작동이 되는 것임! 그래서 1cm마다의 거리를 측정하고 초음파가 되돌아온 순간 끝내서 cm거리를 재는 것임.
	 * */
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(togle == 0)   //카운터 시작하고 외부인터럽트(INT0)를 다음에는 falling Edge에서 걸리도록 함.
	{
		pulse_count = 0;   //측정된 이전 거리값 초기화
		/* Start TIMER */
		HAL_TIM_Base_Start_IT(&TimHandle);
		/*
		 * 이 Start_IT 함수가 실제로 타이머를 작동시키는 함수임! 즉 pulse_count++시키는 함수!
		 * 위에 있는 F405_TIMER_Config(void)는 설정만 해두고 START 함수를 주석처리해서 실행시켜놓지는 않았었어!
		 * JOG 스위치같은 것은 입력신호로 rising, falling을 줄 수 있었지만 여기서는 Start로 작동해서 신호를 받는것이야!
		 */

		//외부인터럽트 INT0 falling edge에서 인터럽트 발생
		GPIO_Init_Struct.Pin = GPIO_PIN_1;
		GPIO_Init_Struct.Mode = GPIO_MODE_IT_FALLING;
		/*
		 * Falling edge를 찾음으로써 최종 거리 값을 나타낼 수 있겠지! 아리 Stop_IT에서 작동될 수 있게!
		 */
		GPIO_Init_Struct.Pull = GPIO_NOPULL;
		GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

		togle = 1;
	}
	else if(togle == 1)   //카운터를 정지시키고 외부 인터럽트(IN0)를 다음에는 Rising Edge에서 걸리도록 함.
	{
		/* Stop TIMER */
		HAL_TIM_Base_Stop_IT(&TimHandle);
		/*
		 * 위에서 설정한 Falling edge가 여기서 작동 되는것이야! Falling edge를 받았으니 최종 거리값이 산출!
		 */
		//외부인터럽트 INT0 falling edge에서 인터럽트 발생
		GPIO_Init_Struct.Pin = GPIO_PIN_1;
		GPIO_Init_Struct.Mode = GPIO_MODE_IT_RISING;
		GPIO_Init_Struct.Pull = GPIO_NOPULL;
		GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

		togle = 0;


		my_printf("time : %d us, distance(cm) : %d\r\n", pulse_count*58, pulse_count);

		if(GPIO_Pin==GPIO_PIN_0)
		 {

		  if(flag0==0)
		  {
			  tmp1++;
		   i--;
		   if(i<0)
		   {
		    i=3;
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[3],0x80);
		    clcd_put_string(menu[0],0xC0);

		   }
		   else if(i>0)
		   {
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[i],0x80);
		    clcd_put_string(menu[i+1],0xC0);
		   }
		   else if(i==0)
		   {
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[i],0x80);
		    clcd_put_string(menu[i+1],0xC0);
		   }
		  }
		  else if(flag0==1)
		  {
			  tmp2++;
			  j--;
			if(j<0)
			{
			    j=3;
			    CLCD_write(0,0x01);
			    clcd_put_string(sort[3],0x80);
			    clcd_put_string(sort[0],0xC0);

			}
			else if(j>0)
			   {
			    CLCD_write(0,0x01);
			    clcd_put_string(sort[j],0x80);
			    clcd_put_string(sort[j+1],0xC0);
			   }
		    else if(j==0)
			   {
			    CLCD_write(0,0x01);
			    clcd_put_string(sort[j],0x80);
			    clcd_put_string(sort[j+1],0xC0);
			   }
			  }
		 }
/*
		 if(GPIO_Pin==GPIO_PIN_1)
		 {

		  if(flag0==0)
		  {
			  tmp1--;
		   i++;
		   if(i>3)
		   {
		    i=0;
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[i],0x80);
		    clcd_put_string(menu[i+1],0xC0);
		   }

		   else if(i==3)
		   {
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[3],0x80);
		    clcd_put_string(menu[0],0xC0);
		   }
		   else if(i<3)
		   {
		    CLCD_write(0,0x01);
		    clcd_put_string(menu[i],0x80);
		    clcd_put_string(menu[i+1],0xC0);
		   }
		  }

		  else if(flag0==1)
		  {
			  tmp2--;
			  j++;
		   //if(J==0)
		   if(j>3)
		    {
		     j=0;
		     CLCD_write(0,0x01);
		     clcd_put_string(sort[j],0x80);
		     clcd_put_string(sort[j+1],0xC0);
		    }
		    else if(j<3)
		    {
		     CLCD_write(0,0x01);
		     clcd_put_string(sort[j],0x80);
		     clcd_put_string(sort[j+1],0xC0);
		    }
		    else if(j==3)
		    {
		     CLCD_write(0,0x01);
		     clcd_put_string(sort[3],0x80);
		     clcd_put_string(sort[0],0xC0);
		    }
		   }
		 }
*/
		 if(GPIO_Pin==GPIO_PIN_2)
		  {
		  if(flag0==0)
		  {
		   flag0=1;j=0;
		   CLCD_write(0,0x01);

		   clcd_put_string(sort[0],0x80);
		   clcd_put_string(sort[1],0xC0);

		  }
		  else if(flag0==1)
		   {
			  flag0=2;
			   CLCD_write(0,0x01);
			   clcd_put_string("COMPLETE",0x80);
			   HAL_Delay(500);
			   CLCD_write(0,0x01);
			   HAL_Delay(100);
			   HAL_ADC_Start_IT(&AdcHandler);
		   }
		 }

	}
}

void Sonar_detecting(void)
{

	clcd_put_string((uint8_t*)output_buffer);

			  pulse();   //10usec High Pulse Output
			  HAL_Delay(100);

			  if(pulse_count < 10 && sonar_detect ==0)
			  {
				 tmp++;
				 if (tmp>=4){
			     sonar_detect = 1;
				 }
			  }

			  if(pulse_count >= 10 && sonar_detect ==0)
			  		  {
			  			 tmp=0;
			  		  }

			  if(tmp >= 4 && sonar_detect == 1)
			  {
				 food_count++;
				 my_printf("Write information about new food!");
				 sprintf((char*)output_buffer, "%s", food_list[food_count-1].name);

				 tmp = 0;
				 scale_val = 1;
				 play_piezo(piezo_pulse[scale_val]);
				 HAL_Delay(100);
				 scale_val = 2;
				 play_piezo(piezo_pulse[scale_val]);
				 HAL_Delay(100);
				 scale_val = 3;
				 play_piezo(piezo_pulse[scale_val]);
				 HAL_Delay(100);
				 scale_val = 0;
				 play_piezo(piezo_pulse[scale_val]);

				 tmp = 0;

				 //operate tmddyd3223's function
			  }

			  if (sonar_detect ==1 && pulse_count>50)
			  {
				  sonar_detect = 0;

			  }


}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	adc_value = HAL_ADC_GetValue(hadc);
		adc_value = adc_value*0.00586;// ADC 변환 결과 값을 저장

		// 변환 결과값을 이용하여 LED의 주기를 변경
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,1);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,1);
		us_delay_int_count(adc_value);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,0);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,0);
		us_delay_int_count(3000);

		// 변환 결과값을 CLCD에 표시
		sprintf((char*)output_buffer, "%4d", adc_value);
		CLCD_write(0, 0xC0);	// Line2
		clcd_put_string((uint8_t*)output_buffer);

		for (int i=0; i<4; i++)
		{

		if (adc_value == food_list[i].expi_date )
		  {
			CLCD_write(0,0x01);
			sprintf((char*)output_buffer, "%s", "check expi date!");
			clcd_put_string((uint8_t*)output_buffer);

			sprintf((char*)output_buffer, "%s", food_list[i].name);
			CLCD_write(0, 0xC0);	// Line2
			clcd_put_string((uint8_t*)output_buffer);

			 play_piezo(piezo_pulse[scale_val]);

			 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,1);
			 HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,1);

			 ms_delay_int_count(100);

			 scale_val = 7;
			 play_piezo(piezo_pulse[scale_val]);
			 ms_delay_int_count(300);
			 scale_val = 7;
			 play_piezo(piezo_pulse[scale_val]);
			 ms_delay_int_count(300);
			 scale_val = 0;
			 play_piezo(piezo_pulse[scale_val]);
			 ms_delay_int_count(800);


			 CLCD_write(0,0x01);



		  }

		}



}


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
