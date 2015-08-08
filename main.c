#define PART_TM4C123GH6PM
#include <stdint.h>
#include <stdbool.h>

#include <hw_ints.h>
#include <hw_memmap.h>
#include <hw_types.h>
#include <hw_gpio.h>
#include <hw_uart.h>
#include <hw_sysctl.h>
#include <debug.h>
#include <fpu.h>
#include <gpio.h>
#include <pin_map.h>
#include <interrupt.h>
#include <sysctl.h>
#include <systick.h>
#include <timer.h>
#include <uart.h>
#include <usb.h>
#include <rom.h>
#include <hw_types.h>
#include <hw_memmap.h>
#include <ssi.h>
#include <usblib.h>
#include <usbcdc.h>
#include <usb-ids.h>
#include <usbdevice.h>
#include <usbdcdc.h>
#include <usbhid.h>

#include "uartstdio.h"
#include "n64usbhid.h"
#include "device.h"

#define SYSCTL_PERIPH_PLL       0x30000010  // PLL

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

void sysDelayUs(uint32_t ui32Us) {
	SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

void n64Transmit(uint8_t bytes[], int length);
int n64Receive(uint8_t buffer[]);

void init_usb_uart() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);
}

int main(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PLL);
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    // Configure LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

//	init_usb_uart();

	initN64USB();

	GCN64DevInitialize();

	uint8_t buffer[32];
	buffer[0] = 0x00;
	buffer[1] = 0x00;
	buffer[2] = 0x00;
	n64Transmit(buffer, 1);
	int count = n64Receive(buffer);
	if(count == 3 && buffer[0] == 0x05 && buffer[1] == 0x00) {
		GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
	}

	while(1) {
		buffer[0] = 0x01;
		n64Transmit(buffer, 1);
		int c2 = n64Receive(buffer);
		if(c2 == 4 && buffer[0] != 0) {
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
		}
//			UARTprintf("Bytes: %x %x %x %d\n", buffer[0], buffer[1], buffer[2], c2);
		sysDelayUs(1000);
	}
}
