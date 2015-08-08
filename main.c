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
#include "protocol.h"
#include "device.h"

#define SYSCTL_PERIPH_PLL       0x30000010  // PLL
#define GPIO_PIN_TYPE_OD_WPU    0x0000000B

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

void initN64Controller() {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlGPIOAHBEnable(SYSCTL_PERIPH_GPIOB);
	GPIOPadConfigSet(GPIO_PORTB_AHB_BASE, 0x0F, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);

	GCN64InitializeProtocol();
}

int main(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PLL);
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Configure LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

	initN64USB();
	initN64Controller();

	uint8_t n64Buffer[35];

	int i;

	while(1) {
		for(i = 0 ; i < 35 ; i++) {
			n64Buffer[i] = 0x00;
		}
		while(!isDataReadyToRead());
		uint8_t* receivedUsbData = receiveUsbData(35);

		switch(receivedUsbData[0]) {
		case 0x02:
			// TODO - implement this!
			break;
		case 0x03:
			// TODO - implement this!
			break;
		case 0x01:
		case 0xFF:
		case 0x00:
		default:
			n64Transmit(receivedUsbData, 1);
			n64Receive(n64Buffer);
			if(n64Buffer[0] == 0x80) {
				GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
			}
			for(i = 0 ; i < 35 ; i+=5) {
				sendUsbData(n64Buffer+i, 5);
			}
			sysDelayUs(1000);
//			sendUsbData(n64Buffer, 35);
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
		}

//		sysDelayUs(1000);
	}
}
