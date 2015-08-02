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

static unsigned long GCN64Transaction(unsigned long ulPort, unsigned char ucPin, unsigned char *buffer, unsigned long toSend)
{
	n64Transmit(buffer, toSend);
	return n64Receive(buffer);
//	if (GCN64Send(ulPort, ucPin, buffer, toSend))
//	{
//		return GCN64Receive(ulPort, ucPin, buffer);
//	}
//	else
//	{
//		return 0;
//	}
}

void GCN64DevInitialize(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlGPIOAHBEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPadConfigSet(GPIO_PORTB_AHB_BASE, 0x0F, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);//GPIO_PIN_TYPE_OD_WPU/*GPIO_PIN_TYPE_OD_WPU*/);

	GCN64InitializeProtocol();
}

void GCN64DevReset(unsigned char controller)
{
	if (controller > COUNT_OF(controllerPorts))
	{
		return;
	}

	gcN64Buffer[0] = 0xFF;
	GCN64Send(controllerPorts[controller].ulPort, controllerPorts[controller].ucPin, gcN64Buffer, 1);
}
#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3
tGCN64Status *GCN64DevStatus(unsigned char controller)
{
	unsigned long len;

	gcN64Buffer[0] = 0x01;
	len = GCN64Transaction(controllerPorts[controller].ulPort, controllerPorts[controller].ucPin, gcN64Buffer, 1);

	if(len == 3 && gcN64Buffer[0] == 0xFA) {
		GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
	}

	if (len != sizeof(tGCN64Status))
	{
		return 0;
	}

//	n64Transmit(gcN64Buffer, 1);

	return (tGCN64Status*)gcN64Buffer;
}

tN64Buttons *N64DevButtons(unsigned char controller)
{
	unsigned long len;

	if (controller > COUNT_OF(controllerPorts))
	{
		return 0;
	}

	gcN64Buffer[0] = 0x01;
	len = GCN64Transaction(controllerPorts[controller].ulPort, controllerPorts[controller].ucPin, gcN64Buffer, 1);

	if (len != sizeof(tN64Buttons))
	{
		return 0;
	}

	return (tN64Buttons*)gcN64Buffer;
}
