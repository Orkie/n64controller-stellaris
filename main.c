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

#include "n64usbhid.h"
#include "device.h"

#define SYSCTL_PERIPH_PLL       0x30000010  // PLL

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

void sysDelayUs(uint32_t ui32Us) {
	SysCtlDelay(ui32Us * (SysCtlClockGet() / 3 / 1000000));
}

int main(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PLL);
	SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
//	SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN); // 50MHz


    // Configure LED
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);
	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);

	initN64USB();

	GCN64DevInitialize();

	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);

	while(1) {

		tGCN64Status* status = GCN64DevStatus(0);

		if(status->IsConnected) {
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
		}
		sysDelayUs(200);

//		N64DevButtons(0);
//		tN64Buttons* buttons = N64DevButtons(0);
//		if(buttons->Start) {
//						GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
//
//		}

//		if(isConnected()) {
//			sendString();
//			unsigned long dataToRead = isDataReadyToRead();
//			if(dataToRead != 0) {
//				uint8_t* data = readData(dataToRead);
//				if(data[0] == 'a') {
//					GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
//				}
//			}
//		}
	}
}
