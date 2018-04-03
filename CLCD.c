/*
 * CLCD.c/*
 * This file is part of the ÂµOS++ distribution.
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

#include "stm32f4xx_hal.h"
#include <string.h>
GPIO_InitTypeDef GPIO_Init_Struct;

void ms_delay_int_count(volatile unsigned int nTime)
{
	nTime = (nTime * 14000);
	for(; nTime > 0; nTime--);
}

void us_delay_int_count(volatile unsigned int nTime)
{
	nTime = (nTime * 12);
	for(; nTime > 0; nTime--);
}

void CLCD_config()
{
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_Init_Struct.Pin=GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_Init_Struct.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_Init_Struct.Pull=GPIO_NOPULL;
	GPIO_Init_Struct.Speed=GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOC,&GPIO_Init_Struct);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
}

void CLCD_write(unsigned char rs, char data)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,rs);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,(data>>4)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,(data>>5)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,(data>>6)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,(data>>7)&0x1);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	us_delay_int_count(2);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,(data>>0)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,(data>>1)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,(data>>2)&0x1);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,(data>>3)&0x1);

	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
	us_delay_int_count(2);
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	ms_delay_int_count(2);
}

void CLCD_init()
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
	CLCD_write(0,0x33);
	CLCD_write(0,0x32);
	CLCD_write(0,0x28);
	CLCD_write(0,0x0f);
	CLCD_write(0,0x01);
	CLCD_write(0,0x06);
	CLCD_write(0,0x02);
}

void clcd_put_string(uint8_t *str,int line)
{
	CLCD_write(0,line);
	while(*str)
	{
		CLCD_write(1,(uint8_t)*str++);
	}
}

/*¸ÞÀÎ·çÆ¾*/


#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------


