/*
 * dvicee.c
 *
 *  Created on:
 *      Author: Tom
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <hw_types.h>
#include <hw_memmap.h>
#include <gpio.h>
#include <sysctl.h>
#include "protocol.h"
#include "device.h"

#define GPIO_PIN_TYPE_OD_WPU    0x0000000B

void GCN64DevInitialize(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlGPIOAHBEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPadConfigSet(GPIO_PORTB_AHB_BASE, 0x0F, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);
//	GPIOPadConfigSet(GPIO_PORTB_AHB_BASE, 0x0F, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	GCN64InitializeProtocol();
}

