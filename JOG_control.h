/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"

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

#include "stm32f4xx_it.h"
#include "CLCD.h"


GPIO_InitTypeDef JOG,JOG_RL;
void EXTILine0_2_Config(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	JOG.Pin=GPIO_PIN_2|GPIO_PIN_0|GPIO_PIN_1;
	JOG.Mode=GPIO_MODE_IT_RISING;
	JOG.Pull=GPIO_NOPULL;
	JOG.Speed=GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB,&JOG);

	NVIC_SetPriority(EXTI0_IRQn, 2);
	NVIC_SetPriority(EXTI1_IRQn, 3);
	NVIC_SetPriority(EXTI2_IRQn, 4);
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
}

/*void EXTILine12_13_Config(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();

	JOG_RL.Pin=GPIO_PIN_12|GPIO_PIN_13;
	JOG_RL.Mode=GPIO_MODE_IT_RISING;
	JOG_RL.Pull=GPIO_NOPULL;
	JOG_RL.Speed=GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB,&JOG_RL);

	HAL_NVIC_SetPriority(EXTI12_IRQn, 5);
	HAL_NVIC_SetPriority(EXTI13_IRQn, 6);
	HAL_NVIC_EnableIRQ(EXTI12_IRQn);
	HAL_NVIC_EnableIRQ(EXTI13_IRQn);
}*/

/*���η�ƾ*/


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
