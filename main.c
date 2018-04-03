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

// -- �ֺ���ġ �ʱ�ȭ�� ����ü
GPIO_InitTypeDef	GPIO_Init_Struct;

TIM_HandleTypeDef  TimHandle12;
TIM_OC_InitTypeDef TIM_OCInit12;

uint32_t piezo_pulse[]	= {0, 3822, 3405, 3033, 2863, 2551, 2272, 2024, 1911}; //�������ļֶ�õ�
uint32_t scale_val = 0, last_scale_val = 0;

uint32_t pulse_count = 0;
uint8_t togle = 0;
uint8_t food_count = 0;
uint8_t sonar_detect = 0;
uint32_t distance[15];



// �¿��� ����
char menu[4][10]={"1. MEAT","2. FISH","3. VEGE","4. ETC"};
char sort[4][10]={"1. 2WEEKS","2. 4WEEKS","3. 2MONTH","4. NONE"};
int i=0,j=0,flag0=0;
int tmp1=0, tmp2=0;
// �¿��� ����


int tmp=0;

void Sonar_detecting(void);
void F405_UART_Config(void);
void F405_TIMER_Config(void);


ADC_HandleTypeDef AdcHandler;		// ADC�� �ʱ�ȭ�� ���� ����ü���� ������ ����
ADC_ChannelConfTypeDef sConfig;

int adc_value;		// ADC�� ���� ����
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
// -- CLCD�� �ʱ⼳���� �Լ��� ����
void CLCD_config()
{
	//  CLCD�� GPIO (GPIOC)�� �ʱ⼳���� ��
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
	CLCD_write(0, 0x33);	// 4��Ʈ ���� Ư�� ���
	CLCD_write(0, 0x32);	// 4��Ʈ ���� Ư�� ���
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
// -- ADC�� �ʱ⼳���� �Լ��� ����
void ADC_Config(void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();	// ADC Ŭ�� Ȱ��ȭ
	__HAL_RCC_GPIOA_CLK_ENABLE();	// ADC ������ ����� GPIOx Ȱ��ȭ(VR:PA1)

	GPIO_Init_Struct.Pin 	= GPIO_PIN_4;			// GPIO���� ����� PIN ����
	GPIO_Init_Struct.Mode 	= GPIO_MODE_ANALOG; 	// Input Analog Mode ���
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

	AdcHandler.Instance 			= ADC1;						// ADC1 ����
	AdcHandler.Init.ClockPrescaler 	= ADC_CLOCK_SYNC_PCLK_DIV2;	// ADC clock prescaler
	AdcHandler.Init.Resolution 		= ADC_RESOLUTION_12B;		// ADC resolution
	AdcHandler.Init.DataAlign 		= ADC_DATAALIGN_RIGHT;		// ADC data alignment
	AdcHandler.Init.ScanConvMode 	= DISABLE;					// ADC scan ��� ��Ȱ��ȭ
	AdcHandler.Init.ContinuousConvMode = ENABLE;				// ADC ���� ��� Ȱ��ȭ
	AdcHandler.Init.NbrOfConversion	= 1;						// ADC ��ȯ ���� ����
	AdcHandler.Init.ExternalTrigConv = ADC_SOFTWARE_START;		// ADC �ܺ� Ʈ���� OFF
	HAL_ADC_Init(&AdcHandler);									// ADC�� ������ ������ �ʱ�ȭ

	sConfig.Channel 	= ADC_CHANNEL_4;			// ADC ä�� ����(PA�� ä�� 4��)
	sConfig.Rank 		= 1;						// ADC ä�� ���� ����
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;	// ADC ���ø� Ÿ�� ����(3Ŭ��)
	HAL_ADC_ConfigChannel(&AdcHandler, &sConfig);	// ä��  ����

	/* NVIC configuration */
	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);
}

////// PIEZO PART //////
int	piezo_config(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_Init_Struct.Pin = GPIO_PIN_15;
	GPIO_Init_Struct.Mode = GPIO_MODE_AF_PP;		// Alternate Function Push Pull ���
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
	TimHandle12.Instance 			= TIM12;						// TIM12 ���
	TimHandle12.Init.Period 		= piezo_pulse[scale_val] - 1;
	TimHandle12.Init.Prescaler 		= 84 - 1;						// Prescaler = 83�� ����(1us)
	TimHandle12.Init.ClockDivision 	= TIM_CLOCKDIVISION_DIV1;		// division�� ������� ����
	TimHandle12.Init.CounterMode 	= TIM_COUNTERMODE_UP;			// Up Counter ��� ����

	/*
	 - ���ļ� ���
prescaler = 84 - 1   :   84Mhz/84 = 1Mhz
C4 261.64     : Period = 3822   ,  OC(CCR) = Period/2 , duty��� �׻� 50%
D4 293.66     : Period = 3405   ,  OC(CCR) = Period/2
..
Period = 1M/���ļ�
	 */

	/* Set TIMx PWM instance */
	TIM_OCInit12.OCMode 		= TIM_OCMODE_PWM1;					// PWM mode 1 ���� ��� ����
	TIM_OCInit12.Pulse 			= (piezo_pulse[scale_val]/2) - 1;	// CCR�� ������
	HAL_TIM_PWM_Init(&TimHandle12);									// TIM PWM�� TimHandle�� ������ ������ �ʱ�ȭ��
	// TIM PWM�� Channel��  TIM_OCInit�� ������ ������ �ʱ�ȭ��
	HAL_TIM_PWM_ConfigChannel(&TimHandle12, &TIM_OCInit12, TIM_CHANNEL_2);

	return 0;
}

void play_piezo(uint32_t period)
{
	/* Set TIMx instance */
	TimHandle12.Init.Period 	= period - 1;
	/* Set TIMx PWM instance */
	TIM_OCInit12.Pulse 			= (period/2) -1;	// CCR�� ������
	HAL_TIM_PWM_Init(&TimHandle12);									// TIM PWM�� TimHandle�� ������ ������ �ʱ�ȭ��
	// TIM PWM�� Channel��  TIM_OCInit�� ������ ������ �ʱ�ȭ��
	HAL_TIM_PWM_ConfigChannel(&TimHandle12, &TIM_OCInit12, TIM_CHANNEL_2);
	/* Start PWM */
	HAL_TIM_PWM_Start(&TimHandle12, TIM_CHANNEL_2);

}
////// PIEZO PART //////

/* Private functions ---------------------------------------------------------*/
void pulse(void);   //10usec High �޽� ���

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
// -- UART �ʱ�ȭ�� ����ü
UART_HandleTypeDef	UartHandle;

// -- UART ��ſ� ���� ����
uint8_t UART_TxBuffer[50] = "\n\rUART Test message...!!! \n\r";
uint8_t UART_RxBuffer[4];

/* Private functions ---------------------------------------------------------*/
// -- UART�� �ʱ⼳���� �Լ��� ����
void F405_UART_Config(void);

// -- printf ������ �Լ�
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
// -- TIMER�� �ʱ⼳���� �Լ��� ����


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

void pulse(void)   //10usec High �޽� ���
{
	//HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);   //input pulse -> PC0
	delay_int_count(us_10);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);   //input pulse -> PC0
	/*
	 * ���⼭ Trigger�� �����Ű�� ���̴�! Ʈ���Ű� ��⿡ output��ȣ�� ������ echo�� �۵��Ǵ°ž�!
	 */
}


// -- UART�� �ʱ⼳���� ���� �Լ�
void F405_UART_Config(void)		// USART1_TX(PA15), USART1_RX(PB7)
{
	// -- <9> UART�� Ŭ���� Ȱ��ȭ
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_USART2_CLK_ENABLE();

	// -- <10> GPIO A��Ʈ 15�� ���� UART Tx�� ����
	GPIO_Init_Struct.Pin	= GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_Init_Struct.Mode	= GPIO_MODE_AF_PP;
	GPIO_Init_Struct.Pull	= GPIO_NOPULL;
	GPIO_Init_Struct.Speed	= GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_Init_Struct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);

	// -- <12> UART�� ���� ���� ����
	UartHandle.Instance			= USART2;
	UartHandle.Init.BaudRate	= 9600;
	UartHandle.Init.WordLength	= UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits	= UART_STOPBITS_1;
	UartHandle.Init.Parity		= UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl	= UART_HWCONTROL_NONE;
	UartHandle.Init.Mode		= UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	// -- <13> UART ���������� UartHandle�� ������ ������ �ʱ�ȭ ��
	HAL_UART_Init(&UartHandle);

	USART2->BRR = 0x00001117;
}

// ----------------------------------------------------------------------------
// -- TIMER�� �ʱ⼳���� �Լ��� ����
void F405_TIMER_Config(void)
{
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* TIMx Peripheral clock enable */
	__HAL_RCC_TIM2_CLK_ENABLE();

	/* Set TIMx instance */
	TimHandle.Instance = TIM2;
	TimHandle.Init.Period = 58 - 1;		//	58 us
	TimHandle.Init.Prescaler = 84 - 1;	// CK_CNT = ���������Ϸ��� ���޵Ǵ� Ŭ��(CK_PSC) / (���������Ϸ� ������ +1) : 1us
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
	 * 1cm = 29us�̴�. �̰� �պ��ϸ� 58us�� �Ǵ� ���̾ 58us�� ������!
	 * �� Ÿ�̸Ӱ� �� 1cm�� �Ǵ� �������� �۵��� �Ǵ� ����! �׷��� 1cm������ �Ÿ��� �����ϰ� �����İ� �ǵ��ƿ� ���� ������ cm�Ÿ��� ��� ����.
	 * */
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(togle == 0)   //ī���� �����ϰ� �ܺ����ͷ�Ʈ(INT0)�� �������� falling Edge���� �ɸ����� ��.
	{
		pulse_count = 0;   //������ ���� �Ÿ��� �ʱ�ȭ
		/* Start TIMER */
		HAL_TIM_Base_Start_IT(&TimHandle);
		/*
		 * �� Start_IT �Լ��� ������ Ÿ�̸Ӹ� �۵���Ű�� �Լ���! �� pulse_count++��Ű�� �Լ�!
		 * ���� �ִ� F405_TIMER_Config(void)�� ������ �صΰ� START �Լ��� �ּ�ó���ؼ� ������ѳ����� �ʾҾ���!
		 * JOG ����ġ���� ���� �Է½�ȣ�� rising, falling�� �� �� �־����� ���⼭�� Start�� �۵��ؼ� ��ȣ�� �޴°��̾�!
		 */

		//�ܺ����ͷ�Ʈ INT0 falling edge���� ���ͷ�Ʈ �߻�
		GPIO_Init_Struct.Pin = GPIO_PIN_1;
		GPIO_Init_Struct.Mode = GPIO_MODE_IT_FALLING;
		/*
		 * Falling edge�� ã�����ν� ���� �Ÿ� ���� ��Ÿ�� �� �ְ���! �Ƹ� Stop_IT���� �۵��� �� �ְ�!
		 */
		GPIO_Init_Struct.Pull = GPIO_NOPULL;
		GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);

		togle = 1;
	}
	else if(togle == 1)   //ī���͸� ������Ű�� �ܺ� ���ͷ�Ʈ(IN0)�� �������� Rising Edge���� �ɸ����� ��.
	{
		/* Stop TIMER */
		HAL_TIM_Base_Stop_IT(&TimHandle);
		/*
		 * ������ ������ Falling edge�� ���⼭ �۵� �Ǵ°��̾�! Falling edge�� �޾����� ���� �Ÿ����� ����!
		 */
		//�ܺ����ͷ�Ʈ INT0 falling edge���� ���ͷ�Ʈ �߻�
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
		adc_value = adc_value*0.00586;// ADC ��ȯ ��� ���� ����

		// ��ȯ ������� �̿��Ͽ� LED�� �ֱ⸦ ����
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,1);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,1);
		us_delay_int_count(adc_value);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,0);
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,0);
		us_delay_int_count(3000);

		// ��ȯ ������� CLCD�� ǥ��
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
