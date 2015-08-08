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

typedef struct
{
	unsigned long ulPort;
	unsigned char ucPin;
} tControllerPortMap;

static const tControllerPortMap controllerPorts[] =
{
	{ GPIO_PORTB_AHB_BASE, GPIO_PIN_0 },
	{ GPIO_PORTB_AHB_BASE, GPIO_PIN_1 },
	{ GPIO_PORTB_AHB_BASE, GPIO_PIN_2 },
	{ GPIO_PORTB_AHB_BASE, GPIO_PIN_3 }
};

// http://stackoverflow.com/a/1598827/1474139
#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static unsigned char gcN64Buffer[36];

void GCN64DevInitialize(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlGPIOAHBEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPadConfigSet(GPIO_PORTB_AHB_BASE, 0x0F, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);//GPIO_PIN_TYPE_OD_WPU/*GPIO_PIN_TYPE_OD_WPU*/);

	GCN64InitializeProtocol();
}

